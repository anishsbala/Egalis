# Foundation and simulator commit plan

Dates are reconstructed calendar metadata, not actual engineering dates.
Identity: `anishsbala <anishsbala@gmail.com>`; branch: `main`.
No remote or push is authorized. The roadmap governs milestone dependencies.
The simulator and future servers must share the same protocol core.

| Milestone | Message | Author and committer timestamp | Boundary status |
| --- | --- | --- | --- |
| 1 | build: establish C++20 and GoogleTest harness | 2026-07-01T09:35:00-04:00 | Implemented and verified |
| 2 | protocol: define baseline instance and ballot types | 2026-07-01T11:10:00-04:00 | Implemented and verified |
| 3 | kv: define deterministic commands and conflicts | 2026-07-01T14:05:00-04:00 | Planned |
| 4 | protocol: extract baseline transitions and messages | 2026-07-01T16:40:00-04:00 | Planned |
| 5 | sim: add deterministic network scheduler | 2026-07-02T10:20:00-04:00 | Planned |
| 6 | sim: add fault schedules and trace replay | 2026-07-02T11:35:00-04:00 | Planned |

Future work requires real functionality, focused tests, build and review.
Do not manufacture changes to meet a quota. Agreement, recovery, persistence
and TCP networking are not implemented at this boundary.
