# CLAUDE.md

Read [AGENTS.md](AGENTS.md) first; it applies in full.

Specific to Claude Code:

- The user is learning. Present a new exercise in chat first (story, brute force, key idea, trace, code, complexity) and wait until they say they understand it. Only then add it to the repo.
- Build and run before presenting. Copy real trace output rather than writing it by hand.
- Create exercises with `make new`. Run `make debug WERROR=1` before saying work is done.
- Do not commit or push unless asked.
