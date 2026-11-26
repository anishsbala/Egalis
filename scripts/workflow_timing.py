"""Record observed workflow spans, never commands or credentials.

One exclusively created activity file prevents duplicate IDs. A per-activity lock
serializes finishes; summaries reject running records. Elapsed values describe
wall spans, not CPU time. A clock-domain/UTC consistency check rejects resets.
"""
import argparse
from datetime import datetime, timezone
import json
import math
import os
from pathlib import Path
import platform
import re
import time

PHASES = ('spec', 'implementation', 'handoff', 'wait', 'build', 'tests', 'review',
          'debugging', 'repair', 'git', 'ci')


def _path(root, activity):
    if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_-]{0,127}', activity):
        raise ValueError('activity ID must be 1-128 letters, digits, underscores or hyphens')
    return Path(root) / (activity + '.json')


def _now(utc, monotonic, clock):
    utc = utc if utc is not None else datetime.now(timezone.utc).isoformat()
    parsed = datetime.fromisoformat(utc)
    if parsed.utcoffset() is None or parsed.utcoffset().total_seconds() != 0:
        raise ValueError('timestamp must be UTC with an explicit offset')
    mono = time.monotonic() if monotonic is None else monotonic
    if not isinstance(mono, (int, float)) or not math.isfinite(mono) or mono < 0:
        raise ValueError('invalid monotonic timestamp')
    return utc, mono, clock if clock is not None else platform.node() + ':' + time.get_clock_info('monotonic').implementation


def start(root, activity, phase, *, parent=None, revision=None, tree=None,
          utc=None, monotonic=None, clock=None):
    path = _path(root, activity)
    if phase not in PHASES:
        raise ValueError('unknown phase')
    if parent is not None:
        _path(root, parent)
        if parent == activity:
            raise ValueError('activity cannot parent itself')
    utc, mono, clock = _now(utc, monotonic, clock)
    record = dict(activityId=activity, parentId=parent, phase=phase, revision=revision,
                  tree=tree, startedUtc=utc, startedMonotonic=mono, clock=clock,
                  completedUtc=None, completedMonotonic=None, elapsedSeconds=None,
                  status='running', exitCode=None)
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        with path.open('x', encoding='utf-8') as stream:
            json.dump(record, stream, indent=2)
    except FileExistsError as error:
        raise ValueError('duplicate activity ID') from error
    return record


def _validate(record):
    _path('.', record['activityId'])
    if not isinstance(record['clock'], str) or not record['clock']:
        raise ValueError('missing clock domain')
    for field in ('startedUtc', 'startedMonotonic', 'completedUtc', 'completedMonotonic'):
        if record[field] is None:
            raise ValueError('missing recorded clock: ' + field)
    if record['phase'] not in PHASES:
        raise ValueError('unknown phase in record')
    _now(record['startedUtc'], record['startedMonotonic'], record['clock'])
    if record['status'] == 'running':
        raise ValueError('unfinished activity: ' + record['activityId'])
    _now(record['completedUtc'], record['completedMonotonic'], record['clock'])
    elapsed = record['completedMonotonic'] - record['startedMonotonic']
    wall = (datetime.fromisoformat(record['completedUtc']) - datetime.fromisoformat(record['startedUtc'])).total_seconds()
    if elapsed < 0 or wall < 0 or abs(elapsed - wall) > 1:
        raise ValueError('negative interval or incompatible UTC/monotonic clocks')
    if record['elapsedSeconds'] != elapsed:
        raise ValueError('inconsistent elapsed duration')
    code = record['exitCode']
    if type(code) is not int or record['status'] != ('succeeded' if code == 0 else 'failed'):
        raise ValueError('inconsistent completion status')
    return elapsed


def finish(root, activity, exit_code, *, utc=None, monotonic=None, clock=None):
    path = _path(root, activity)
    lock = path.with_suffix('.lock')
    try:
        handle = lock.open('x')
    except FileExistsError as error:
        raise ValueError('activity is being finished') from error
    try:
        handle.close()
        try:
            record = json.loads(path.read_text(encoding='utf-8'))
        except FileNotFoundError as error:
            raise ValueError('activity does not exist') from error
        if record['status'] != 'running':
            raise ValueError('activity is already finished')
        utc, mono, clock = _now(utc, monotonic, clock)
        if clock != record['clock']:
            raise ValueError('incompatible monotonic clock domain')
        record.update(completedUtc=utc, completedMonotonic=mono,
                      elapsedSeconds=mono-record['startedMonotonic'],
                      status='succeeded' if exit_code == 0 else 'failed', exitCode=exit_code)
        _validate(record)
        temporary = path.with_suffix('.tmp')
        temporary.write_text(json.dumps(record, indent=2), encoding='utf-8')
        os.replace(temporary, path)
        return record
    finally:
        lock.unlink()


def _union(intervals):
    total = 0
    end = None
    for left, right in sorted(intervals):
        total += max(0, right - max(left, end if end is not None else left))
        end = max(end if end is not None else right, right)
    return total


def summary(root):
    records = [json.loads(p.read_text(encoding='utf-8')) for p in sorted(Path(root).glob('*.json'))]
    intervals = []
    phases = {}
    ids = set()
    for record in records:
        elapsed = _validate(record)
        if record['activityId'] in ids:
            raise ValueError('duplicate activity ID in ledger')
        ids.add(record['activityId'])
        left = datetime.fromisoformat(record['startedUtc']).timestamp()
        right = datetime.fromisoformat(record['completedUtc']).timestamp()
        intervals.append((left, right))
        phase = phases.setdefault(record['phase'], dict(count=0, inclusiveSeconds=0))
        phase['count'] += 1
        phase['inclusiveSeconds'] += elapsed
    return dict(completed=len(records), failed=sum(r['status']=='failed' for r in records),
                elapsedUnionSeconds=_union(intervals), phases=phases,
                interpretation='Observed wall spans; phase totals are inclusive and may overlap; not CPU time.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory', type=Path, required=True)
    commands = parser.add_subparsers(dest='operation', required=True)
    begin = commands.add_parser('start')
    begin.add_argument('activity')
    begin.add_argument('phase', choices=PHASES)
    for field in ('parent', 'revision', 'tree'):
        begin.add_argument('--' + field)
    end = commands.add_parser('finish')
    end.add_argument('activity')
    end.add_argument('--exit-code', type=int, required=True)
    commands.add_parser('summary')
    args = vars(parser.parse_args())
    operation = args.pop('operation')
    root = args.pop('directory')
    try:
        result = globals()[operation](root, **args)
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.exit(1, str(error) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
