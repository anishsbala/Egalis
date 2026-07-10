# Initial PreAccept (baseline EPaxos* v1)

Authority: [Section 3.2, Figure 3](https://arxiv.org/html/2511.02743v1#S3.SS2).
SubmitCommand allocates a checked local ID. Submit accepts an explicitly
preallocated local ID, rejects existing/foreign IDs and advances the same volatile
high-water mark. Explicit IDs are a trusted internal input, not client-chosen
network request IDs. Both inputs freeze the current known-conflict proposal and emit
the same PreAccept broadcast, self first and other members in canonical order.
The last representable ID is usable; exhaustion does not wrap or reset on resume.
Durable reservations and replacement of a lost Replica are not implemented.

The initial command and proposal are frozen in the pending local record before
effects run. They are not a vote: phase stays initial until self-PreAccept. This
is a serialized staging refinement of the paper's immediate self-delivery; no
external event may intervene before local consequences complete. Current dep is
not confused with the initial proposal. Registration is not a KV success reply.
C012 provides submission only; PreAccept receipt and duplicate replay are separate
subsequent slices. There is no PreAcceptOK collection or commit decision yet.

## C013 receive transition

The exact Figure 3 guard is bal=0 and phase=initial, never a phase ordinal test.
Structural sender/destination and self-dependency checks precede dispatch. A new
record or pending self-request gets cmd/initCmd=c, initDep=D, dep=D union current
known conflicts, and phase=preaccepted; abal stays zero. The response is one
PreAcceptOK to the initial coordinator. All allocations precede state replacement.
Pending local metadata must match the frozen self-request; inconsistent inputs
are rejected without overwriting it. Higher ballots or noninitial phases are
stale, with no second transition. C014 will refine identical retransmissions.
PreAcceptOK collection remains unsupported: receiving five votes cannot commit.
