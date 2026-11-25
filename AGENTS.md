# AGENTS.md

Instructions for any AI assistant (Claude, Copilot, Codex, ...) working in this repository.

## What this is

A C++ refresher for a former C++ developer, built from interview-style problems. **Teaching clarity beats cleverness.** Each exercise should take 10 to 20 minutes to study. The goal is the algorithm AND the STL AND modern C++.

## Hard constraints

- **No dependencies.** Only the standard library, a **C++17** compiler and GNU Make **3.81**. Target the C++11 to C++17 feature set; do not use C++20 (concepts, ranges, `<format>`, `span`). Do not add CMake, test libraries or third-party code.
- Makefile: use only Make 3.81 features. Never use `.ONESHELL`, `$(file ...)`, `!=`, grouped targets or `.RECIPEPREFIX`. Shell scripts must work with macOS bash 3.2 and BSD sed.
- Do not push, force-push or rewrite git history unless the user asks.

## Commands

```bash
make run                  # all exercises in the table
make run ONLY="name" V=1  # one exercise, with its trace
make debug WERROR=1       # sanitizers + warnings as errors: run before saying "done"
make new NAME=foo TITLE="Foo"
```

## One exercise = exactly one `.h` + one `.cpp`

Folder `exercises/<name>/` (snake_case, no number), files `<name>.h` and `<name>.cpp`, namespace `refresher::<name>` (nested `namespace refresher { namespace <name> {`), entry point `bool Run();`. Include the header as `"<name>/<name>.h"`. Register it in `src/main.cpp` (include + table row). Keep it simple and compact: about 150 to 250 lines for the pair, comments included. Concentrate on the story, the task and the C++ constructs; avoid helper machinery and boilerplate.

### Header must contain (in this order)
1. Title, pattern, level.
2. **STORY**: a short scenario with the operations and a small example. Plain and self-explanatory, with fresh themes.
3. **HOW THE INTERVIEW GOES**: four short steps written as a conversation you could have with an interviewer: clarify (questions plus assumed answers), brute force (and why it is too slow), insight (key idea, with a small ASCII drawing), complexity (time and space, with the reason).
4. The declarations, each with a short contract comment, then `bool Run();`.

Keep the class or function as plain as possible. Prefer simple code that reads like the explanation over production-grade machinery. If something (such as deleting copy operations) is a real concern but would distract, leave a one-line NOTE and mention it as a follow-up instead of implementing it.

### Source must contain
1. The solution, with a **`C++ NOTE:`** comment at EVERY interesting construct (lambda, `auto`, `optional`, move, `= delete`, `explicit`, `const`/`noexcept`, `if` with initializer, STL algorithms, ...): what it is, why it is used here, and the old way it replaces.
2. More solution comments. Comments explain **why**, not what: container choice, types (`int` vs `size_t`), the trap each tricky line avoids, overflow, iterator invalidation.
3. Tests inside `Run()` using `CHECK(...)` from `src/check.h`: empty or minimal input, typical cases, the trap the exercise teaches, and a large input if performance matters. Add a randomized comparison against a slow obviously-correct version (fixed seed) only when it stays short.
4. A trace printed only when `Verbose()` is true, ideally using the real code.
5. `return AllChecksPassed();`

Work out expected values independently. A wrong expected value is a test bug.

## Style

C++17, no extensions. Warnings clean under `-Wall -Wextra -Wpedantic -Wshadow -Werror`. Prefer the standard library and common algorithms (`sort`, `find_if`, `lower_bound`, `accumulate`, `rotate`, `partition`...). Use `std::cout` streams for output, not `printf`. Format with `clang-format` (style in `.clang-format`).

## Definition of done

- [ ] `make debug WERROR=1` passes.
- [ ] The exercise is in the `src/main.cpp` table and in the README table.
- [ ] The header and the source follow the structure above.
- [ ] No build output or editor files are staged.
