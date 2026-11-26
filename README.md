# C++ Algorithm Playground

A playground for classic interview-style algorithm problems and a C++ refresher for a developer coming back to the language. Each exercise is a small, self-contained **header + source pair** that teaches three things at once:

1. **The algorithm:** the story, the brute force, the key idea and the complexity with the reason.
2. **The STL:** which container or algorithm to pick, why, and how to use it.
3. **Modern C++ (C++11 to C++17):** `auto`, lambdas, `std::optional`, `std::string_view`, structured bindings, functors, `explicit`, `emplace`, `splice`, and so on. Every interesting construct has a `C++ NOTE:` or inline hint where it is used.

No dependencies: only a C++17 compiler, GNU Make (3.81 is enough, which is what macOS ships) and the standard library.

## Quick start

```bash
make run                      # build and run every exercise in the table
make run ONLY="lru_cache"     # run selected exercises (space-separated names)
make run V=1                  # also print each exercise's step-by-step trace
make list                     # list the exercises in the table
make debug                    # run with AddressSanitizer + UBSan (catches memory bugs)
make sandbox                  # your private scratch pad (see "Sandbox" below)
make clean
```

Example output:

```
PASS  maximal_square  (9 checks)
...
PASS  max_path_sum  (9 checks)
13 passed, 0 failed
```

The exit code is non-zero if anything fails, so `make test` also works in scripts and CI.

Override the toolchain when needed: `make CXX=g++`, `make CXXSTD=c++20`, `make CXXFLAGS=-O3`.

## Layout

```
cpp-algorithm-playground/
├── Makefile              # builds src/ and every exercises/*/*.cpp (except the sandbox)
├── src/
│   ├── main.cpp          # the TABLE of exercises + the runner loop
│   └── check.h           # one CHECK(...) macro (no test library)
├── exercises/
│   ├── lru_cache/        # one folder per exercise: <name>.h + <name>.cpp
│   │   ├── lru_cache.h   # story, interview conversation, traps, declarations
│   │   └── lru_cache.cpp # solution, tests in Run(), narrated trace at the bottom
│   ├── ...
│   └── sandbox/          # private scratch pad (git-ignored except its README)
├── docs/containers.md    # which STL container to pick, and why
├── templates/            # starting files used by `make new` and `make sandbox`
├── scripts/new_exercise.sh
├── AGENTS.md             # how we work (for any AI assistant)
├── CLAUDE.md, .claude/   # Claude Code: commands and permissions
└── .github/              # CI: make debug WERROR=1 on Ubuntu and macOS
```

## The table in `src/main.cpp`

The runner holds a table of function pointers and calls each one in a range-for loop. To skip an exercise, comment out its row:

```cpp
constexpr Exercise kExercises[] = {
    {"lru_cache", &refresher::lru_cache::Run},
    // {"two_sum", &refresher::two_sum::Run},   // skipped
};
```

## How to study an exercise (10 to 20 minutes)

1. Read the **header**: the story and the "how the interview goes" conversation. Stop before the code and try it yourself.
2. Run it with `make run ONLY=<name> V=1` and read the trace.
3. Read the **source** and the `C++ NOTE:` comments.
4. Read the tests in `Run()`. Each one targets a specific trap.
5. Break it on purpose (remove a line the comments call essential) and see which check fails.
6. Rewrite it from a blank file.

## Adding an exercise

```bash
make new NAME=two_sum TITLE="Two Sum"
```

This creates `exercises/two_sum/two_sum.h` and `.cpp` from the template. Then add the include and the table row in `src/main.cpp` (the command prints the exact lines). The Makefile picks up the new folder automatically. See [AGENTS.md](AGENTS.md) for the standard each exercise follows.

## Exercises

| Exercise | Pattern | Complexity |
|---|---|---|
| [`maximal_square`](exercises/maximal_square/maximal_square.h) | Dynamic programming on a grid | O(rows x cols) |
| [`count_fields`](exercises/count_fields/count_fields.h) | Flood fill, connected components | O(rows x cols) |
| [`longest_unique_run`](exercises/longest_unique_run/longest_unique_run.h) | Sliding window | O(n) |
| [`merge_schedules`](exercises/merge_schedules/merge_schedules.h) | Intervals: sort, then sweep | O(n log n) |
| [`min_harvest_speed`](exercises/min_harvest_speed/min_harvest_speed.h) | Binary search on the answer | O(n log M) |
| [`busiest_fields`](exercises/busiest_fields/busiest_fields.h) | Heap of size K | O(n + m log k) |
| [`lru_cache`](exercises/lru_cache/lru_cache.h) | Hash map + linked list | O(1) per operation |
| [`rate_limiter`](exercises/rate_limiter/rate_limiter.h) | Strategy pattern: 5 limiting algorithms | O(1) per request |
| [`consistent_hashing`](exercises/consistent_hashing/consistent_hashing.h) | Sorted map on a ring, virtual nodes | O(log N) lookup |
| [`house_robber_circle`](exercises/house_robber_circle/house_robber_circle.h) | DP with two variables, run twice (circular) | O(n) |
| [`word_break`](exercises/word_break/word_break.h) | DP over positions (stepping stones) | O(n x W x L) |
| [`max_depth_tree`](exercises/max_depth_tree/max_depth_tree.h) | Tree recursion + level-by-level queue | O(n) |
| [`max_path_sum`](exercises/max_path_sum/max_path_sum.h) | Tree recursion: return one arm, record the V | O(n) |

Container cheat sheet: [docs/containers.md](docs/containers.md).

## Sandbox

`exercises/sandbox/` is a private scratch pad for free experiments (a container, a lambda, a pointer trick, "what happens if...").

```bash
make sandbox                 # builds and runs exercises/sandbox/scratch.cpp (created from a template)
make sandbox SAN=1           # with AddressSanitizer + UBSan
```

Everything in that folder except its README is ignored by git, and it is not part of `make run` or CI.

## Working with Claude Code (or any AI assistant)

[AGENTS.md](AGENTS.md) describes how we work, and `CLAUDE.md` imports it. In short:

- **Learning mode:** give a problem; the assistant explains it in chat first (story, brute force, insight, pseudocode, a really-run trace) and only creates files after you say you understand it.
- **Sandbox mode:** "experiment with ..." works only inside `exercises/sandbox/`.
- Slash commands in `.claude/commands/`: `/walkthrough`, `/add-exercise`, `/verify`, `/sandbox`.
- The assistant never commits or pushes; you do.

## Requirements

- Any C++17 compiler (Apple clang from Xcode 10+, GCC 7+, Clang 5+).
- GNU Make 3.81 or newer.

## License

[MIT](LICENSE).
