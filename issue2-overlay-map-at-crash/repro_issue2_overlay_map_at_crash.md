# 1. Report title
Unchecked peer lookup in private overlay callbacks can crash consensus node (`std::map::at`)

# 2. Report impact
An attacker (or malformed/unexpected network input) can trigger a crash path in consensus networking callbacks by causing a lookup for a non-existing peer id.

Simple explanation:
- The code uses `std::map::at(...)` for incoming peer identifiers.
- `map::at` throws an exception if key is missing.
- In these callback paths there is no safe fallback (`find` + early return), so the process/actor can terminate.

Security impact:
- Denial of service against validator consensus path.
- Repeated triggering can cause instability/restarts and reduce validator availability.
- Availability loss at validator layer can affect participation in consensus rounds.

# 3. Short description
In `validator/consensus/private-overlay.cpp`, callback handlers (`on_overlay_message`, `on_overlay_broadcast`, `on_query`) use `adnl_id_to_peer_.at(...)` / `short_id_to_peer_.at(...)` directly. If an unknown key is encountered, `std::map::at` throws `std::out_of_range`; without defensive handling this can terminate processing and destabilize the consensus actor/process.

# 4. Reproduction details (script location and run)
Script location:
- Outside the `ton` repo, in your external folder:
  - `issue2-overlay-map-at-crash/repro_issue2_overlay_map_at_crash.cpp`

Run from folder:
- `issue2-overlay-map-at-crash/` folder
- Pass TON repo root as argument.

Compile:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  repro_issue2_overlay_map_at_crash.cpp \
  -o repro_issue2_overlay_map_at_crash
```

Execute:
```bash
./repro_issue2_overlay_map_at_crash ../ton
```

Expected result:
- [Screenshot 1](https://drive.google.com/file/d/1Hnc3hpnYMBe1LCZAa4f0ds4HxALWhZUa/view?usp=sharing)
- Script prints `PASS: Crash condition reproduced (missing key causes map::at exception).`
