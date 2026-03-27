# 1. Report title
Unbounded candidate state map growth in simplex candidate resolver can exhaust validator memory

# 2. Report impact
The simplex candidate resolver tracks per-candidate state in an in-memory map and does not prune entries in normal operation.

Simple explanation:
- New candidate ids continuously create new entries.
- Entries are not evicted after they are no longer needed.
- Long-running sessions can accumulate large memory footprints.

Security impact:
- Remote adversaries can increase memory pressure by driving high candidate churn.
- Validator process can degrade or terminate under memory exhaustion.
- Reduced validator availability harms consensus liveness and participation.

# 3. Short description
In `validator/consensus/simplex/candidate-resolver.cpp`, the `state_` map (`std::map<CandidateId, CandidateState>`) is populated for new ids (for example through `state_[request->id]`) but has no normal-path pruning (`state_.erase(...)` / `state_.clear(...)`). This creates an unbounded growth vector over long sessions.

# 4. Reproduction details (script location and run)
Script location:
- Outside the `ton` repo, in your external folder:
  - `issue4-candidate-resolver-state-growth/repro_issue4_candidate_resolver_state_growth.cpp`

Run from folder:
- `issue4-candidate-resolver-state-growth/` folder
- Pass TON repo root as argument.

Compile:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  repro_issue4_candidate_resolver_state_growth.cpp \
  -o repro_issue4_candidate_resolver_state_growth
```

Execute:
```bash
./repro_issue4_candidate_resolver_state_growth ../ton
```

Expected result:
- Script prints `PASS: Candidate resolver state growth vulnerability reproduced.`
