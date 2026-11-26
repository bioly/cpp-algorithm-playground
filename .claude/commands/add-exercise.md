---
description: Build the approved exercise in the repo (header, source, table, README)
argument-hint: <snake_case_name> ["Title"]
---
Create the exercise: $ARGUMENTS

Only do this after the user approved the walkthrough. Follow AGENTS.md exactly:

1. `make new NAME=<name> TITLE="<Title>"`, then fill in the header (STORY, HOW THE INTERVIEW GOES, TRAPS) and the source (solution first, hand-built tests next, narrated trace in a "Bonus" section at the bottom).
2. Register it in `src/main.cpp` (include in alphabetical order, table row at the end) and add the README table row.
3. Run `/verify`. Do not say "done" until it passes.
4. Do not commit or push. List the files created or changed.
