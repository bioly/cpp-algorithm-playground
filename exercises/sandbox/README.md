# sandbox

A free scratch pad for experiments: try a container, a lambda, a pointer trick, or a
half-formed algorithm idea without touching the real exercises.

```bash
make sandbox                 # builds and runs exercises/sandbox/scratch.cpp
make sandbox SAN=1           # the same with AddressSanitizer + UBSan
make sandbox SANDBOX_FILE=exercises/sandbox/lambdas.cpp     # another scratch file
make sandbox SANDBOX_LIBS=exercises/lru_cache/lru_cache.cpp # reuse an exercise's code
make sandbox-reset           # DESTRUCTIVE: replace scratch.cpp with the template
```

- `scratch.cpp` is created from `templates/sandbox.cpp.in` the first time you run `make sandbox`.
- It has its own `main()` and is **not** part of `make run`, the exercise table or CI.
- **Everything in this folder except this README is ignored by git.** Experiments stay on your machine.
- When an experiment turns into a real exercise, create it properly with `make new NAME=my_idea TITLE="My Idea"` and follow [AGENTS.md](../../AGENTS.md).

Using Claude Code in this repo? See the "Sandbox mode" section of [AGENTS.md](../../AGENTS.md).
