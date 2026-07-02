#include "egalis/sim/trace.hpp"

#include <charconv>
#include <locale>
#include <sstream>
#include <utility>

namespace egalis::sim {
namespace {
void versions(const TraceData& data) {
  if (data.version != 1 || data.random_version != 1) {
    throw std::invalid_argument("unsupported trace or PRNG version");
  }
}
std::uint64_t integer(const std::string& token) {
  std::uint64_t value = 0;
  const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), value);
  if (error != std::errc{} || end != token.data() + token.size()) {
    throw std::invalid_argument("invalid trace integer");
  }
  return value;
}
std::string take(std::istream& in) {
  std::string token;
  if (!(in >> token)) throw std::invalid_argument("truncated trace record");
  return token;
}
void end_of_record(std::istream& in) {
  std::string extra;
  if (in >> extra) throw std::invalid_argument("trailing trace fields");
}
std::string hex(const std::string& bytes) {
  constexpr char digits[] = "0123456789abcdef";
  std::string result;
  result.reserve(bytes.size() * 2);
  for (const unsigned char byte : bytes) {
    result.push_back(digits[byte >> 4]);
    result.push_back(digits[byte & 15]);
  }
  return result.empty() ? "-" : result;
}
std::string unhex(const std::string& token) {
  if (token == "-") return {};
  if (token.size() % 2 != 0) throw std::invalid_argument("odd trace hex length");
  const auto digit = [](char c) -> unsigned {
    if (c >= '0' && c <= '9') return static_cast<unsigned>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
    throw std::invalid_argument("invalid trace hex");
  };
  std::string bytes;
  bytes.reserve(token.size() / 2);
  for (std::size_t i = 0; i < token.size(); i += 2) {
    bytes.push_back(static_cast<char>((digit(token[i]) << 4) | digit(token[i + 1])));
  }
  return bytes;
}
void valid_decision(const Decision& decision) {
  if (static_cast<unsigned>(decision.kind) > static_cast<unsigned>(DecisionKind::timer) ||
      decision.fields.size() > 16 || decision.detail.size() > 1024 * 1024) {
    throw std::invalid_argument("invalid or oversized trace decision");
  }
}
}  // namespace

std::string serialize(const TraceData& data) {
  versions(data);
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << "EGALIS_TRACE " << data.version << ' ' << data.random_version << ' ' << data.seed << '\n';
  for (const auto& decision : data.decisions) {
    valid_decision(decision);
    out << static_cast<unsigned>(decision.kind) << ' ' << decision.fields.size();
    for (const auto field : decision.fields) out << ' ' << field;
    out << ' ' << hex(decision.detail) << '\n';
  }
  return out.str();
}

TraceData parse_trace(std::string_view text) {
  std::istringstream input{std::string{text}};
  input.imbue(std::locale::classic());
  std::string line;
  if (!std::getline(input, line)) throw std::invalid_argument("missing trace header");
  std::istringstream header{line};
  if (take(header) != "EGALIS_TRACE") throw std::invalid_argument("invalid trace magic");
  const auto version = integer(take(header));
  const auto random_version = integer(take(header));
  TraceData data;
  if (version != 1 || random_version != 1) throw std::invalid_argument("unsupported trace version");
  data.seed = integer(take(header));
  end_of_record(header);
  while (std::getline(input, line)) {
    if (line.size() > 2 * 1024 * 1024 + 1024) throw std::invalid_argument("oversized trace line");
    std::istringstream row{line};
    const auto kind = integer(take(row));
    const auto count = integer(take(row));
    if (kind > static_cast<unsigned>(DecisionKind::timer) || count > 16) {
      throw std::invalid_argument("invalid trace decision header");
    }
    Decision decision{static_cast<DecisionKind>(kind), {}, {}};
    decision.fields.reserve(static_cast<std::size_t>(count));
    for (std::uint64_t i = 0; i < count; ++i) decision.fields.push_back(integer(take(row)));
    decision.detail = unhex(take(row));
    end_of_record(row);
    valid_decision(decision);
    data.decisions.push_back(std::move(decision));
  }
  return data;
}

Trace::Trace(std::uint64_t seed) { data_.seed = seed; }
Trace::Trace(TraceData replay) : data_(std::move(replay)), replay_(true) {
  versions(data_);
  for (const auto& decision : data_.decisions) valid_decision(decision);
}

void Trace::check_healthy() const {
  if (failed_) throw ReplayDivergence("trace failed; start a fresh simulation");
}

void Trace::diverge(const std::string& reason) const {
  failed_ = true;
  throw ReplayDivergence("trace decision " + std::to_string(cursor_) + ": " + reason);
}

void Trace::record(Decision decision) {
  check_healthy();
  valid_decision(decision);
  if (!replay_) {
    data_.decisions.push_back(std::move(decision));
    return;
  }
  if (cursor_ >= data_.decisions.size()) diverge("execution exceeded recorded trace");
  if (data_.decisions[cursor_] != decision) diverge("event, input, or outcome differs");
  ++cursor_;
}

std::uint64_t Trace::choose(Random& random, std::uint64_t lower, std::uint64_t upper,
                            std::string label) {
  check_healthy();
  if (upper < lower) throw std::invalid_argument("inverted decision range");
  std::uint64_t value;
  if (replay_) {
    if (cursor_ >= data_.decisions.size()) diverge("missing random decision");
    const auto& decision = data_.decisions[cursor_];
    if (decision.kind != DecisionKind::random || decision.fields.size() != 3 ||
        decision.fields[0] != lower || decision.fields[1] != upper ||
        decision.fields[2] < lower || decision.fields[2] > upper || decision.detail != label) {
      diverge("random request differs or recorded value is out of range");
    }
    value = decision.fields[2];
  } else {
    value = random.uniform(lower, upper);
  }
  record({DecisionKind::random, {lower, upper, value}, std::move(label)});
  return value;
}

void Trace::verify_complete() const {
  check_healthy();
  if (replay_ && cursor_ != data_.decisions.size()) diverge("execution left unconsumed decisions");
}

}  // namespace egalis::sim
