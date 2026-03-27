# 1. Report title
Unbounded broadcast dedup set growth in consensus block accepter can degrade validator availability

# 2. Report impact
The block accepter tracks sent candidate broadcasts in an in-memory set that is appended to but never pruned.

Simple explanation:
- Each unique block id inserts into `sent_candidate_broadcasts_`.
- The set is consulted for deduplication but entries are not removed.
- Long-running operation accumulates stale ids in memory.

Security impact:
- Sustained block production/churn increases memory usage over time.
- Availability degradation can reduce validator stability and uptime.
- Consensus liveness can be impacted when validator nodes are resource-exhausted.

# 3. Short description
In `validator/consensus/block-accepter.cpp`, `sent_candidate_broadcasts_` is used as a dedup structure (`contains` + `insert`) but there is no erase/clear lifecycle on normal operation. This creates monotonic memory growth in long-lived validator processes.

# 4. Reproduction details (script location and run)
Script location:
- Outside the `ton` repo, in your external folder:
  - `issue6-block-accepter-broadcast-set-growth/repro_issue6_block_accepter_broadcast_set_growth.cpp`

Run from folder:
- `issue6-block-accepter-broadcast-set-growth/` folder
- Pass TON repo root as argument.

Compile:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  repro_issue6_block_accepter_broadcast_set_growth.cpp \
  -o repro_issue6_block_accepter_broadcast_set_growth
```

Execute:
```bash
./repro_issue6_block_accepter_broadcast_set_growth ../ton
```

Expected result:
- Script prints `PASS: Block accepter broadcast set growth vulnerability reproduced.`
