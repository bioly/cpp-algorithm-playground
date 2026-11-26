---
description: Run the full definition of done, including mutation checks
---
Run the "Definition of done" from AGENTS.md and report real results:

1. `make clean && make debug WERROR=1` (all exercises must pass).
2. If `clang++` exists: `make run CXX=clang++ CXXSTD=c++20 WERROR=1`.
3. Mutation check on the exercise that changed (work on a COPY of the repo, never the real files): apply one classic bug at a time, confirm each makes the tests fail, and confirm the mutation really applied. For any bug that is NOT caught, add a test to the exercise and repeat.
4. `make run ONLY=<name> V=1` and compare the trace with the explanation.
5. Confirm with `git status` that no build output, zip or sandbox file is staged.

Report: what passed, which mutations were caught, anything you had to fix.
