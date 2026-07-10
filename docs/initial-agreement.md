# Initial agreement observations

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

Collection returns an empty effect list. C016 owns the fast Commit path and C017
owns the grace timer and selection of the slow handoff.
