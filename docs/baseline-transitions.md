# Baseline transition contract (milestone 4)

Authority: [EPaxos* v1, Section 3, Figures 1, 3, 5; Appendix C, Figure 8](https://arxiv.org/html/2511.02743v1).
This is a reviewed implementation mapping, not implemented agreement or recovery.
Use the paper's exact predicates when implementing handlers. `messages.hpp`
mirrors baseline message arguments; `validate` checks structure and sender roles,
not receiver-state guards. Typed replica IDs enforce fixed membership.

| Future handler / message | Source | Guard / obligations to test at implementation |
| --- | --- | --- |
| Submit / PreAccept | Fig. 3 left 1–3 | Unique ID, complete known-conflict scan |
| PreAccept receive | Fig. 3 left 4–11 | bal=0 and initial phase; preserve initCmd/initDep |
| PreAcceptOK collection | Fig. 3 left 12–18 | Local preaccepted state and ballot zero; freeze distinct Q; test initial-dependency equality and quorum thresholds |
| Accept receive | Fig. 3 right 1–8 | bal<=b; when equal, not committed; record bal and abal separately |
| AcceptOK collection | Fig. 3 right 9–11 | Matching ballot, accepted phase, distinct quorum |
| Commit receive | Fig. 3 right 12–17 | Exact bal=b, including on delayed broadcasts |
| Recover / RecoverOK | Fig. 5 1–15 | Higher owned ballot; receiver bal<b; snapshot Q and maximum abal; committed branch precedes accepted branch |
| Candidate selection | Fig. 5 18,20–21,41 | Evaluate the baseline possible-fast predicate; otherwise propose Nop |
| Validate receive | Fig. 5 42–48 | bal=b; set cmd/initCmd/initDep without preaccepting or changing abal; preserve exact invalidator predicate |
| ValidateOK collection | Fig. 5 22–29 | Every original Q member; union observations; decide candidate, Nop, or wait |
| Waiting resolution | Fig. 5 30–36 | Observe committed invalidators, resolved candidates, or baseline Waiting; recheck current ballot before delayed continuation |
| TryRecover / owner resubmission | Fig. 8 | Eventual detector convergence and increasing retry delays; Nop retries retain logical operation identity later |
| Execute | Fig. 1 | Closed committed graph, dependency-first SCCs, ID tie break; no early reply |

## Variant exclusions

In Figure 5 remove highlighted lines 16–17, 19, and 37–40; remove the highlighted
second disjunct at line 26, the R_max argument at line 29, and the k' threshold
at line 35. Thus baseline Waiting carries only id. Do not use Section 5/Appendix B
thrifty changes, original TentativePreAccept, or original sequence-number ordering.

## Field mapping

| Paper | C++ |
| --- | --- |
| id, initCoord(id) | InstanceId, coordinator() |
| bal / abal | BallotState::current / accepted |
| cmd / dep | InstanceState::command / dependencies |
| initCmd / initDep | initial_command / initial_dependencies |
| bottom / Nop | Payload monostate / kv::Nop |
| phase | Phase, never compared by ordinal to infer transition legality |
| I | set of Invalidator{id,phase}; different replica observations may contain different phases for one ID |

RecoverOK contains no initCmd field because Figure 5 does not send it. A validated
but still initial record may contain cmd/initDep; that does not count as preaccept.
RecoverOK's accepted ballot predates its newly joined positive ballot. ValidateOK
is broadcast to all, so its envelope destination need not own the recovery ballot.
No added NACK or extra recovery messages are implied by transport retries.

## Outstanding policy decisions before handlers

Keep the existing design gates: fast-path grace policy, duplicate reply caching,
snapshot/candidate freeze, detector convergence and durable self-delivery cuts.
The message layer cannot establish quorum decisions, legal phase transitions,
validation safety, or restart correctness. Milestones 7 onward implement and test
those behaviors. Canonical message bytes are a versioned trace fingerprint, not
a substitute network wire format; Protobuf remains milestone 28.

Milestone 9 implements Figure 3's initial submission and guarded PreAccept receive
only. Exact duplicate response replay is documented as a transport refinement in
preaccept.md; it does not re-run the guarded transition. Duplicate caching is now
resolved for ballot zero / PreAccepted. Vote collection, grace policy, candidate
freeze for agreement, recovery and durable self-delivery remain later work.
