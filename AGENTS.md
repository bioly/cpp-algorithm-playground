# AGENTS.md

Instructions for any AI assistant (Claude, Copilot, Codex, ...) working in this repository.

## What this is, and who it is for

A C++ refresher and interview-prep playground. The owner is a **former C++ developer** returning to the language and preparing for coding interviews (Google style: one problem, whiteboard thinking, 30 to 45 minutes). The goal is the algorithm AND the STL AND modern C++ (C++11 to C++17).

**Teaching clarity beats cleverness.** Prefer the simplest thing that works: plain loops, arrays, `std::vector`, raw pointers where a problem gives them. Use heavier STL or templates only when they make the lesson clearer. Each exercise should take 10 to 20 minutes to study.

## Hard constraints

- **No dependencies.** Only the standard library, a **C++17** compiler and GNU Make **3.81**. Do not use C++20 (concepts, ranges, `<format>`, `span`, `contains`). Do not add CMake, test libraries or third-party code.
- Makefile: only Make 3.81 features. Never `.ONESHELL`, `$(file ...)`, `!=`, grouped targets or `.RECIPEPREFIX`. Shell scripts must work with macOS bash 3.2 and BSD sed.
- **Never `git commit`, `git push`, force-push or rewrite history unless the user explicitly asks.** The user does the Git pushes themselves; you prepare the files and say what changed.

## Commands

```bash
make run                       # every exercise in the table
make run ONLY="name" V=1       # one exercise, with its drawn trace (V=1 = verbose)
make list                      # names in the table
make debug WERROR=1            # sanitizers + warnings as errors: what CI runs
make new NAME=foo TITLE="Foo"  # scaffold exercises/foo/
make sandbox [SAN=1]           # run the private scratch pad (see "Sandbox mode")
```

## How we work: two modes

### Mode 1: Learning an exercise (default for a new problem)

When the user gives a problem (a LeetCode link, a pasted task, a system-design topic), **do not create files yet**. Walk through it in chat first:

1. **Story and clarifying questions** (with the answers you assume).
2. **Brute force** and why it is too slow.
3. **Key insight**, explained simply, with a small terminal drawing.
4. **Step-by-step pseudocode** with CLEAR variable names (not `dp`, `i`, `j`, `tmp`), and explained loops.
5. **A trace of a real example, produced by really running the idea** (a quick script or the real code). Never invent trace output or numbers.
6. Complexity with the reason, the traps, and which C++ constructs the code will use.

Adapt the depth to what the user asks: "simplest", "like a junior", "like a child", "shorter", "with drawings". When asked for the simplest version: one story, one drawing, one small pseudocode, three things to remember, no rambling.

**Wait until the user says they understand it and asks for the code** ("create the code", "generate the zip", "add it"). Only then create the exercise in the repo. Do not add files earlier.

### Mode 2: Sandbox mode (free experiments)

`exercises/sandbox/` is a private scratch pad. When the user says "experiment", "try", "play", "sandbox", "what happens if...":

- Work ONLY in `exercises/sandbox/` (usually `scratch.cpp`; create more files freely). Run with `make sandbox` (add `SAN=1` for sanitizers; `SANDBOX_FILE=...` for another file; `SANDBOX_LIBS=exercises/x/x.cpp` to reuse an exercise).
- Everything there is git-ignored. Do not register it in `src/main.cpp`, do not touch the real exercises, do not "clean it up" or add polish.
- Keep it quick and informal, but still correct. Explain surprising results in a sentence or two.
- Never run `make sandbox-reset` without asking: it overwrites the scratch file.
- If an experiment grows into a real exercise, propose `make new NAME=... TITLE="..."` and switch to Mode 1.

## One exercise = exactly one `.h` + one `.cpp`

Folder `exercises/<name>/` (snake_case, no number), files `<name>.h` and `<name>.cpp`, namespace `refresher::<name>` (nested: `namespace refresher { namespace <name> {`), entry point `bool Run();`. Include the header as `"<name>/<name>.h"`. Register it in `src/main.cpp` (include in alphabetical order, table row at the END of the table = study order) and add a row to the README table. Size: about **150 to 250 lines for the pair**, comments included. Keep it SIMPLE: the algorithm must be the most visible thing in the file.

### Header contains, in this order
1. Title, pattern, level (and source, e.g. `LeetCode 104`).
2. **STORY**: a short scenario with a small drawing and its answer. Use fresh themes (bookstore, farm, library, ...), not always the same one.
3. **HOW THE INTERVIEW GOES**: four short steps: Clarify (questions plus assumed answers), Brute force (and why it fails), Insight (the key idea, with a small ASCII drawing), Complexity (time and space, with the reason).
4. **TRAPS**: the mistakes the tests cover.
5. Declarations with a short contract comment, then `bool Run();`.

### Source contains, in this order
1. **The solution first**, short, with a `C++ NOTE:` at every interesting construct (what it is, why here, the old way it replaces) and a `CONTAINER:` note wherever a container is chosen: why this one, why not the alternatives, one gotcha, a cppreference link (`https://en.cppreference.com/w/cpp/...`), and a pointer to `docs/containers.md`. Comments explain WHY, not what.
2. **Tests in `Run()` directly below the solution**, using `CHECK(...)` from `src/check.h`. Build test data **by hand**, the way you would on a whiteboard (for trees: `TreeNode leaf{7}; TreeNode parent{20, &leaf, nullptr};`). Do not write builders, pools, lambdas for setup, or random generators unless the exercise needs them. Cover: empty/minimal input, the examples from the task, each named TRAP, a large input if performance matters. Mirror cases (a rule on the left side also gets a test on the right side).
3. **A narrated trace at the very bottom**, in a section marked "Bonus", printed only when `Verbose()`. It must use the real algorithm and be drawn in the terminal (a table, arrows, rows), and it must match the pictures used in the explanation.
4. `return AllChecksPassed();`

Work out expected values independently; a wrong expected value is a test bug.

## Style

- C++17, no extensions. Warning-clean under `-Wall -Wextra -Wpedantic -Wshadow -Werror`. `clang-format` style in `.clang-format`.
- Clear names: `can_split_first`, `letters_seen`, `best_so_far`, not `dp`, `i2`, `tmp`. Short loop indices (`i`) are fine for plain indexing.
- Use `std::cout` streams, not `printf`. Prefer common algorithms (`sort`, `max`, `lower_bound`, `max_element`, ...) when they shorten the code, plain loops when they are clearer.
- `std::vector<char>` instead of `std::vector<bool>`.
- **Never end a `//` comment line with a backslash** (ASCII-art trees): the next line becomes part of the comment and `-Werror=comment` fails. Put a few words after it.
- `size_t` is unsigned: check sizes BEFORE subtracting.
- When printing a result of a function that itself prints (a trace), call it first, store the result, then print.

## Definition of done (run this, do not assume)

- [ ] `make clean && make debug WERROR=1` passes for ALL exercises (CI runs exactly this on Ubuntu and macOS).
- [ ] If `clang++` is available: `make run CXX=clang++ CXXSTD=c++20 WERROR=1` also passes.
- [ ] **Mutation check**: break the algorithm on purpose, one classic bug at a time (wrong initial value, missing base case, `+` instead of `max`, off-by-one, forgotten mirror case, missing size check...), and confirm the tests FAIL each time. A bug that is not caught means a missing test: add it. Make sure each mutation was really applied (the text to replace must match exactly once).
- [ ] `make run ONLY=<name> V=1` output was read and the trace matches the explanation.
- [ ] The exercise is in the `src/main.cpp` table and in the README table.
- [ ] Header and source follow the structure above, and the pair is about 150 to 250 lines.
- [ ] No build output, `.zip`, sandbox files or editor files are staged.

## Where the project is

Exercises done (study order): `maximal_square`, `count_fields`, `longest_unique_run`, `merge_schedules`, `min_harvest_speed`, `busiest_fields`, `lru_cache`, `rate_limiter` (strategy pattern, 5 algorithms), `consistent_hashing`, `house_robber_circle`, `word_break`, `max_depth_tree`, `max_path_sum`.

Planned ideas (mix themes and levels, not only farm): `byte_shuffler`, `log_sorter`, `car_rental` (minimal OO: `enum class CarType {Sedan, Suv, Van}`, `CarRental(int sedans, int suvs, int vans)`, `bool Reserve(CarType, int start_hour, int days)`, overlap counting, tests per requirement), `rotated_phone_book`, `weather_station`, `ring_buffer`, `anagram_desk`, `running_median`, `pair_finder`, `merge_in_place`, `autocomplete`, `job_order`, `rain_totals`, `crop_rotations`. Natural follow-ups: tree diameter, balanced tree, house robber I and III, word break II, system-design topics (rate limiter and consistent hashing came from the "System Design: Zero to Interview Hero" notes).

Reference for container choice: `docs/containers.md`.
