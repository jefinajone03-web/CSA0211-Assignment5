# CSA 0211 – Assignment 5: District Blood Bank Network (C)

**Student:** S Madhu Shri  **Register No:** 1921260293  **Slot:** D

| File | Task | What it does |
|------|------|--------------|
| `partA_demos.c` | A1.3, A2, A3 | Working versions of the design code (multiple return values, digital root, fast power, pointer operations, lowest stock) |
| `b1_recursion.c` | B1.1 – B1.3 | Test cases, call counting, linear vs halving recursion, "called twice" experiment |
| `b1_crash_limit.c` | B1.4 | Finds the stack limit of linear recursion (takes the exponent as an argument) |
| `b2_adversarial.c` | B2 | Long name, missing/read-only file, wrong arguments, `getline()` |
| `b3_text_vs_binary.c` | B3.1 – B3.3 | Text vs binary files: correctness, size/time table, `update_units()` |
| `b4_growable_list.c` | B4.1 – B4.2 | Growable donor list with `realloc()` doubling and `free()` |
| `bloodbank.c` | Whole system | Command-line blood bank (`register`, `find`, `issue`, `receive`, `report`) |

## Compile and run

```bash
gcc partA_demos.c      -o partA_demos      -Wall -Wextra && ./partA_demos
gcc b1_recursion.c     -o b1_recursion     -Wall -Wextra && ./b1_recursion
gcc -O0 b1_crash_limit.c -o b1_crash_limit && ./b1_crash_limit 100000
gcc b2_adversarial.c   -o b2_adversarial   -Wall -Wextra -fsanitize=address -g
gcc b3_text_vs_binary.c -o b3_text_vs_binary -Wall -Wextra -O2 && ./b3_text_vs_binary
gcc b4_growable_list.c -o b4_growable_list -Wall -Wextra -fsanitize=address -g && ./b4_growable_list
gcc bloodbank.c        -o bloodbank        -Wall -Wextra -fsanitize=address -g
```

On Windows (MinGW) `-fsanitize=address` may not be available; leave it out there
and use Linux/WSL for the AddressSanitizer screenshot.

## bloodbank usage

```bash
./bloodbank register 1001
./bloodbank find O+
./bloodbank receive O+ 10
./bloodbank issue O+ 3
./bloodbank report
```
Donors are stored in `donors.txt` (text), stock in `stock.dat` (binary).
