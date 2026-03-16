# AGENTS.md

Repository guidance for agentic coding assistants working in `m5stack-core2--esp32-ha`.

## Current Repository State

- The workspace root is currently empty.
- This directory is not a Git repository at the moment.
- No source files, config files, manifests, or test files were present during analysis.
- No existing root `AGENTS.md` was found.
- No `.cursorrules` file was found.
- No `.cursor/rules/` directory was found.
- No `.github/copilot-instructions.md` file was found.
- No `README`, `package.json`, `pyproject.toml`, or `platformio.ini` was found.

Because there is no code yet, the commands and style rules below are best-effort guidance based on the current empty state.
If the repository is later populated, agents should re-check the repo before relying on this file.

## Working Assumptions

- Treat this repo as uninitialized until build/test tooling is added.
- Do not invent project-specific commands and present them as verified facts.
- Prefer reporting `not detected` over guessing.
- When tooling appears later, update this file with exact commands from checked-in config.

## Verified Commands

The following commands were checked during analysis:

```sh
git rev-parse --show-toplevel   # fails: not a git repository
ls -la                          # shows an empty directory
```

## Build Commands

- Build command: not detected.
- Firmware build command: not detected.
- Web/app build command: not detected.
- Package manager: not detected.
- Build output directory: not detected.

Use this workflow once the repo contains files:

1. Check for `package.json`, `pyproject.toml`, `platformio.ini`, `Cargo.toml`, `Makefile`, or `CMakeLists.txt`.
2. Read `README` and CI config for the canonical build command.
3. Prefer repo-local scripts over globally invented commands.
4. Update this file with the exact verified command.

## Lint Commands

- Lint command: not detected.
- Format command: not detected.
- Static analysis command: not detected.
- Type-check command: not detected.

When tooling is added, look for these common sources of truth:

- `package.json` scripts such as `lint`, `format`, `typecheck`.
- Python tools in `pyproject.toml` such as `ruff`, `black`, `mypy`, `pytest`.
- Embedded tooling in `platformio.ini`, `CMakeLists.txt`, or helper scripts.
- CI workflows under `.github/workflows/`.

## Test Commands

- Test command: not detected.
- Unit test command: not detected.
- Integration test command: not detected.
- Hardware-in-the-loop test command: not detected.
- Coverage command: not detected.

## Running a Single Test

No test framework is present, so a verified single-test command is not available.

When test tooling appears, prefer the framework's native single-test form:

- `pytest path/to/test_file.py::test_name`
- `npm test -- path/to/test.file`
- `vitest run path/to/test.ts -t "test name"`
- `jest path/to/test.file -t "test name"`
- `ctest -R test_name`
- `pio test -e <env> -f <filter>`

Do not assume any of the commands above work here until confirmed in the repo.

## Cursor And Copilot Rules

No Cursor or Copilot instruction files were found during analysis.

- `.cursorrules`: not present
- `.cursor/rules/`: not present
- `.github/copilot-instructions.md`: not present

If any of those files are later added, agents should treat them as higher-priority repo guidance and sync this file accordingly.

## Code Style Guidance

There is no existing code to infer conventions from, so use conservative defaults until the repo establishes project-specific patterns.

### Imports And Dependencies

- Prefer standard library or platform headers first, third-party next, local modules last.
- Keep imports grouped and stable.
- Avoid unused imports.
- Do not add new dependencies unless required for the task.
- Prefer existing dependencies over introducing alternatives.

### Formatting

- Follow any formatter already configured in the repo once present.
- Until then, keep formatting consistent within each file.
- Use spaces, not tabs, unless an existing file clearly uses tabs.
- Keep lines readable; favor clarity over packing logic densely.
- Avoid unrelated formatting churn in files you touch.

### Types And Interfaces

- Prefer explicit types at public boundaries.
- Prefer narrow, concrete types over vague catch-all types.
- Avoid `any`-style escape hatches unless unavoidable and documented.
- Keep function signatures small and intention-revealing.
- Model error cases in the type system where the language supports it.

### Naming

- Match the dominant language convention once code exists.
- Use descriptive names over abbreviations.
- Keep acronyms consistent.
- Name functions after behavior, variables after data, and types after concepts.
- Avoid one-letter names except for tight local loops.

### Functions And Structure

- Keep functions focused on one responsibility.
- Prefer small helpers over deeply nested logic.
- Favor composition over duplication.
- Avoid premature abstraction.
- Make side effects obvious.

### Error Handling

- Fail loudly with actionable messages when a task cannot proceed safely.
- Do not swallow exceptions or status codes silently.
- Preserve original error context when rethrowing or wrapping errors.
- Return structured errors where the language/framework expects them.
- Handle cleanup paths explicitly for I/O, hardware, and network work.

### State And Side Effects

- Minimize mutable global state.
- Keep state ownership clear.
- Prefer pure transformations before imperative updates.
- Document assumptions around hardware state, time, retries, and connectivity.
- Make retries bounded and explicit.

### Testing Expectations

- Add or update tests when behavior changes and a test framework exists.
- Keep tests close to the changed behavior.
- Prefer deterministic tests over timing-sensitive tests.
- Cover happy path, edge cases, and failure paths.
- For hardware-dependent code, separate logic from device interaction when possible.

### Comments And Documentation

- Add comments only when the code is not self-explanatory.
- Prefer explaining why over restating what.
- Update docs when commands, setup, or behavior changes.
- Remove stale comments instead of working around them.

## Agent Behavior In This Repo

- Re-check the repository before making claims about tooling.
- If the repo is still empty, say so clearly.
- If new config files appear, treat them as the source of truth.
- Prefer minimal, precise changes over broad scaffolding.
- Do not create speculative infrastructure unless the user requests it.

## How To Update This File Later

When the repo gains real code, replace `not detected` sections with:

1. Exact build, lint, format, type-check, and test commands.
2. Exact single-test commands for the active framework.
3. Language-specific naming and typing rules inferred from the codebase.
4. Any rules from `.cursorrules`, `.cursor/rules/`, or `.github/copilot-instructions.md`.
5. Any CI-only constraints that agents must honor locally.

## Bottom Line

This repository currently contains no code or tooling to analyze.
Agents should treat all project-specific commands as unknown until files are added and verified.
