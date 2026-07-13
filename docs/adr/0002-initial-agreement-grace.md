# Initial agreement grace and slow handoff

Authority: EPaxos* arXiv 2511.02743v1, Section 3, Figure 3 left 12–18.
The approved M10 plan selects this deterministic local policy; it does not add
an agreement guard or change the pinned baseline.

The first valid observation from every distinct responder remains in Q. Fewer
than three observations cannot choose a branch. At three or more, any value
that differs from the initial dependency proposal selects slow immediately.
Three matching observations start one positive logical tick of grace. A fourth
matching observation before expiry selects fast. No mismatch is discarded to
manufacture a matching subset: a fourth mismatching response freezes all four.
The first serialized deciding input seals Q and its observations. Late replies,
duplicates, old firings and repeated decisions cannot reopen the branch.

The grace guard has its own purpose-bound slot, separate from generic timers.
Both use the same checked, monotonically allocated token namespace. A generic
arm or firing cannot replace or expire grace. Selection cancels pending grace;
Commit receipt also cancels it. Expiry requires exact instance, token and ballot,
an active unselected local collection, preaccepted phase, ballot zero and at
least three observations. Equal scheduler timestamps follow insertion sequence:
a fourth reply first chooses fast; expiry first freezes the slow handoff.

Suspend emits cancellations for every owned timer and clears both slots while
retaining volatile protocol state. Resume returns explicit ScheduleTimer effects
with fresh tokens and one tick for every still-eligible matching collection.
The runner executes these effects before accepting further external input.
Old core guards and old network-generation callbacks remain inert. Selected,
committed or subquorum records do not rearm. This is retained-memory resume, not
durable restart or a recovery mechanism.

SlowPathReady owns the frozen instance, zero ballot and sender/dependency map.
It is a local effect recorded by the runner, never a peer message, vote, commit,
application operation or client success. The collector also remains sealed.
C019 must consume this effect automatically to compute the union and broadcast
Accept without another peer or client input; actual slow agreement is absent
in C017. Owning the effect payload makes later input unable to mutate it. The
network owns the endpoint and timers, and cancels callbacks on lifetime changes.

One tick supplies bounded handoff with three responders. Under the paper's exact
synchronous-round model the matching replies arrive at one logical instant, so
the fourth precedes a positive expiry. Unequal bounded delays can select slow
before the fourth reply; no arbitrary bounded-delay fast-path or end-to-end
2Delta execution guarantee is claimed. No Section 4 optimization is included.
