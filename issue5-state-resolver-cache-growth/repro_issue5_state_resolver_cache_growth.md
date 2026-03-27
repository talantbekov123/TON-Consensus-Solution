# 1. Report title
Unbounded cache accumulation in simplex state resolver can cause long-session memory exhaustion

# 2. Report impact
The simplex state resolver keeps multiple per-id maps that grow with chain progress and are not pruned in normal successful operation.

Simple explanation:
- Each new parent/candidate id can create cache entries.
- Error-path cleanup exists, but success-path lifecycle pruning is missing.
- Over long sessions, memory grows with historical ids.

Security impact:
- Adversarial traffic and high churn can amplify cache growth.
- Memory pressure can degrade validator performance or trigger OOM conditions.
- Consensus liveness is affected when validators become unstable/unavailable.

# 3. Short description
In `validator/consensus/simplex/state-resolver.cpp`, `state_cache_` and `finalized_blocks_` are populated through `state_cache_[id]` and `finalized_blocks_[id]`. Existing erases are tied to error-path cleanup, while normal successful operation keeps accumulating historical entries. This exposes an availability risk for long-lived validator sessions.

# 4. Reproduction details (script location and run)
Script location:
- Outside the `ton` repo, in your external folder:
  - `issue5-state-resolver-cache-growth/repro_issue5_state_resolver_cache_growth.cpp`

Run from folder:
- `issue5-state-resolver-cache-growth/` folder
- Pass TON repo root as argument.

Compile:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  repro_issue5_state_resolver_cache_growth.cpp \
  -o repro_issue5_state_resolver_cache_growth
```

Execute:
```bash
./repro_issue5_state_resolver_cache_growth ../ton
```

Expected result:
- Script prints `PASS: State resolver cache growth vulnerability reproduced.`
