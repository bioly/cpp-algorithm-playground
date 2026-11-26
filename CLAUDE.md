# CLAUDE.md

@AGENTS.md

The file above applies in full (imported). If your client did not load it, open [AGENTS.md](AGENTS.md) first.

Specific to Claude Code:

- **Two modes.** A new problem means *Learning mode*: explain in chat, run a real example, wait for "create the code" before adding files. Words like "experiment", "try", "play", "sandbox" mean *Sandbox mode*: work only in `exercises/sandbox/` with `make sandbox`.
- **Never run real output by hand.** Traces and numbers in explanations come from code you actually ran.
- **Do not commit or push** unless asked. The user pushes to GitHub; tell them which files changed.
- Project slash commands (`.claude/commands/`):
  - `/walkthrough <problem>`: explain a problem in chat only, no files.
  - `/add-exercise <name>`: build the exercise after the user approved the walkthrough.
  - `/verify`: the full definition of done, including mutation checks.
  - `/sandbox <idea>`: experiment in the sandbox.
- Shared permissions are in `.claude/settings.json` (`make`, compilers and read-only git are allowed; `git push` is denied). Personal overrides go in `.claude/settings.local.json` (ignored by git).
