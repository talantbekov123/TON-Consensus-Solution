# 1. Report title
Missing misbehavior reporting in simplex consensus for invalid/conflicting/rejected candidate paths

# 2. Report impact
Malformed or conflicting candidate behavior is currently dropped/logged in some simplex paths without emitting structured `MisbehaviorReport` evidence.

Simple explanation:
- Invalid behavior is noticed, but not formally reported in all paths.
- Without report evidence, malicious validators are harder to prove/penalize.
- Attackers can repeatedly send bad inputs with lower risk of consequences.

Security impact:
- Weaker accountability and slashing/evidence pipeline.
- Higher griefing budget (spam invalid/conflicting candidates repeatedly).
- Long-term degradation of network security economics and operator visibility.

# 3. Short description
In `validator/consensus/simplex/consensus.cpp`, several candidate-handling paths contain `FIXME: report misbehavior` / `FIXME: Report misbehavior` comments and currently return/log without publishing `MisbehaviorReport`. Notably affected paths include invalid parent relation, conflicting pending candidate, and validation-rejected candidate branches.

# 4. Reproduction details (script location and run)
Script location:
- Outside the `ton` repo, in your external folder:
  - `issue3-misbehavior-reporting-gap/repro_issue3_misbehavior_reporting_gap.cpp`

Run from folder:
- `issue3-misbehavior-reporting-gap/` folder
- Pass TON repo root as argument.

Compile:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  repro_issue3_misbehavior_reporting_gap.cpp \
  -o repro_issue3_misbehavior_reporting_gap
```

Execute:
```bash
./repro_issue3_misbehavior_reporting_gap ../ton
```

Expected result:
- [Screenshot 2](https://drive.google.com/file/d/1H5YLQ8QqPjFIlLCytkjGET0XatjPozGu/view?usp=sharing)
- Script prints `PASS: Misbehavior reporting gap reproduced.`
