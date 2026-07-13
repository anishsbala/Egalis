# Initial agreement

C015 collects ballot-zero replies for local submissions under Figure 3 of the
baseline EPaxos* protocol. [Section 3.2, Figure 3](https://arxiv.org/html/2511.02743v1#S3.F3)

Submit freezes the command and initial dependency proposal and opens an empty
collector on the local instance record. The runner delivers the self-PreAccept
and its real PreAcceptOK before remote effects. The self reply follows the same
collection path as a peer reply.

PreAcceptOK carries an instance ID and dependencies. Envelope validation checks
that it targets the initial owner, and dispatch checks that it reached that
replica. Collection requires an existing local submission in the preaccepted
phase with current ballot zero. Unknown, unsubmitted, pending and superseded
instances yield a stale result.

The collector retains the first dependency set for each sender. An identical
duplicate yields a replayed result. A conflicting duplicate is rejected and
preserves the original observation. The collector is separate from the local
dependency field used to replay the replica's own PreAcceptOK. Matching compares
each observed dependency set with the frozen initial proposal, so even four
identical expansions fail that comparison.

## Fast decision and Commit receipt

The fourth distinct reply triggers a fast decision when all four replies match
the initial proposal. The coordinator freezes the selected quorum before
returning one ballot-zero Commit to each member, self first and the remaining
members in canonical order. The Commit carries the current command and the
initial dependencies. Later replies leave the selected quorum intact.

Every recipient, including the coordinator, uses the same Commit handler.
Figure 3 requires the current ballot to equal the message ballot. An unknown
instance has ballot zero and can become committed before its PreAccept arrives.
Receipt sets the accepted ballot, command, chosen dependencies and committed
phase. The current ballot and initial metadata retain their prior values.
Chosen dependencies can replace a larger local expansion. An equivalent
duplicate is inert. A contradictory value is rejected and the decision remains
intact. A locally owned instance learned through Commit advances the shared ID
allocator.

C017 implements the grace timer and frozen local slow handoff; see
adr/0002-initial-agreement-grace.md for token ownership, lifecycle and timing
limits. C019 implements the frozen Accept proposal from that handoff, as described
in slow-agreement.md. Commit receipt has
no client or execution effects at this stage.

## Deterministic failure coverage

Simulation scenarios exercise nonempty initial proposals, duplicate requests,
three-survivor bounded slow handoff, two-survivor nondecision, disagreement before
quorum and on the fourth reply, and both insertion orders when a fourth reply
shares the grace deadline. Coordinator crash scenarios distinguish loss before
decision from loss after immediate self-commit with remote Commit traffic queued.
Each new scenario checks observed committed payload/dependencies after scheduler
events and reconstructs a serialized trace for replay. These are bounded safety
regressions, not recovery liveness, durable restart, execution or a safety proof.
