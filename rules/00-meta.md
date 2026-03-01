# Rule 00: Meta — How Rules and the Knowledge DB Work

## The Knowledge Database Is a Lookup Table, Not a Document Store

The MCP knowledge database (`.claude/knowledge.db`) exists for **persistent inter-session pointers and short findings**. It is NOT the place to store full rules, documentation, or lengthy content.

- **Rules** live as markdown files in `./rules/` — humans can read and edit them.
- **Knowledge DB entries** point to rules files and store short factual findings.
- When you learn something important, decide: is it a **rule** (put it in `./rules/`) or a **fact** (put it in the DB with `add_knowledge`)?

## How to Use Rules

1. At session start, call `search_knowledge()` for the topic you're working on.
2. Knowledge entries will reference rules files by path (e.g., `rules/02-coding-style.md`).
3. **Read the rules file** before proceeding — don't rely on memory or summaries.
4. If a rule is wrong or outdated, update the file and the corresponding DB entry.

## Adding New Rules

- Use the next available number prefix: `NN-short-name.md`
- Keep rules concise and actionable — no essays.
- Add a knowledge DB entry pointing to the new file.
