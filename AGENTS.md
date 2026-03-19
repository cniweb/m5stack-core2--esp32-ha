# AGENTS.md

Repository guidance for agentic coding assistants working in `m5stack-core2--esp32-ha`.

## Scope

This repository contains a custom `Arduino + PlatformIO` firmware for an `M5Stack Core2`.
The device acts as a touch display for Home Assistant energy data and reads values through the Home Assistant REST API.

The repository also contains a Home Assistant package that produces the stable `sensor.core2_*` entities consumed by the firmware.

## Current Stack

- Firmware: `Arduino` on `ESP32`
- Build system: `PlatformIO`
- Board: `m5stack-core2`
- Display and touch: `M5Unified`
- Backend integration: `Home Assistant REST API`
- Home Assistant package: `home-assistant/packages/core2_power_history.yaml`

## Source Of Truth Files

- `platformio.ini` - board config and pre-build hooks
- `src/main.cpp` - full device firmware and UI rendering
- `include/dashboard_config.h` - entity IDs, polling intervals, scales, timeouts
- `include/secrets.example.h` - required local secret structure
- `scripts/ha_rest_preflight.py` - mandatory local pre-build Home Assistant check
- `README.md`, `docs/setup.md`, `docs/architecture.md` - verified user and technical docs

## Cursor And Copilot Rules

No `.cursorrules`, `.cursor/rules/`, or `.github/copilot-instructions.md` files were present during the latest review.
If any are added later, treat them as higher-priority repo guidance and sync this file.

## Verified Commands

These commands were verified in this repository:

```sh
pio run
pio run -t upload --upload-port COM8
pio device monitor --port COM8 --baud 115200
```

Notes:

- `pio run` executes the Home Assistant REST preflight locally.
- Upload to `COM8` was verified for the connected `M5Stack Core2` in this workspace.
- The exact COM port may differ on another machine.

## Build, Lint, And Test Commands

- Build firmware: `pio run`
- Upload firmware: `pio run -t upload --upload-port <COMx>`
- Serial monitor: `pio device monitor --port <COMx> --baud 115200`
- Lint: not configured
- Format: not configured
- Static analysis: not configured
- Type-check: not configured

For this repo, `pio run` plus the REST preflight is the main verification step.

## Running A Single Test

No unit test framework is configured, so there is no verified single-test command.
If tests are added later, update this file with the framework-native single-test form.

## Required Home Assistant Entities

The firmware expects these entities to exist before builds and uploads are considered healthy:

- `sensor.core2_solar_live`
- `sensor.core2_house_live`
- `sensor.core2_solar_day_energy_kwh`
- `sensor.core2_house_day_energy_kwh`
- `sensor.core2_grid_import_power`
- `sensor.core2_grid_export_power`
- `sensor.core2_grid_import_day_energy_kwh`
- `sensor.core2_grid_export_day_energy_kwh`

These are created by `home-assistant/packages/core2_power_history.yaml`.

## Pre-Build REST Preflight

Before every local `pio run`, `scripts/ha_rest_preflight.py` validates:

- `include/secrets.h` exists
- `HA_BASE_URL` is set
- `HA_TOKEN` is not the placeholder value
- the required `sensor.core2_*` entities are reachable through `/api/states/<entity_id>`
- entity states are usable

Special case:

- fresh `utility_meter` sensors may temporarily report `state=unknown` with `status=collecting`
- this is treated as a warning, not a hard failure

GitHub Actions skips the online preflight because CI has no local secrets and no access to the user’s Home Assistant instance.

## Local Secrets Handling

- `include/secrets.h` is intentionally ignored by Git
- never commit real Wi-Fi credentials or Home Assistant tokens
- use `include/secrets.example.h` as the template
- if secret names change, update both code and docs

## Hardware Guidance

- The target device is an `M5Stack Core2` using the `m5stack-core2` PlatformIO board target.
- Uploads were verified over USB serial on Windows.
- Always confirm the actual port with `pio device list` before flashing.

## Code Style Guidance

### General

- Keep changes small, readable, and hardware-focused.
- Prefer explicit code over abstraction-heavy refactors.
- Preserve the current direct-drawing UI style unless a broader redesign is requested.
- Use ASCII unless the file already uses another encoding.
- When you need to search docs, use `context7` tools.

### Imports And Dependencies

- Prefer standard/library headers first, then framework headers, then local headers.
- Remove unused dependencies when possible.
- Avoid adding new libraries unless the current stack cannot reasonably support the task.
- Prefer the existing `M5Unified`, `HTTPClient`, `WiFi`, and `WiFiClientSecure` stack.

### Formatting

- Match the existing brace and spacing style in `src/main.cpp`.
- Keep helper functions compact and near related rendering or networking logic.
- Avoid unrelated formatting churn.

### Types And Naming

- Use fixed-width integer types where appropriate.
- Keep helper names descriptive, e.g. `draw_split_value`, `draw_history_row`, `refresh_data`.
- Keep page names and entity concepts consistent with the UI and docs.

### Error Handling

- Fail clearly in pre-build scripts.
- For runtime issues, prefer concise serial diagnostics over silent failure.
- Do not hide network or REST failures if they affect displayed values.
- Preserve safe fallbacks like `n/v` for unavailable sensor values.

## Project-Specific Best Practices

### Home Assistant Integration

- Prefer real Home Assistant source sensors over rough local approximations when available.
- In this project, real grid energy totals are preferred to locally reconstructed totals.
- Keep the firmware bound to stable `sensor.core2_*` names, not raw upstream IDs.
- If the user changes backend entities, update the HA package first and keep firmware IDs stable if possible.

### REST Handling

- Hostname resolution may differ between desktop tooling and the Core2.
- If REST works on the workstation but not on-device, investigate request/response handling before assuming the host is wrong.
- The firmware currently extracts the `state` value directly from the response body text because that proved more reliable on-device than the earlier JSON parsing approach.
- Do not replace this parser without re-verifying on the actual Core2.
- Keep HTTP timeouts short; long blocking waits can break touch responsiveness.

### Utility Meter And Energy Precision

- Tiny solar values can keep HA energy integrations at `0.000` too long if precision is too low.
- The solar total integration uses higher precision so `sensor.core2_solar_day_energy_kwh` initializes correctly.
- If a day-energy sensor stays `unknown`, inspect integration precision before changing firmware behavior.

### Display Layout Work

- Test layout changes against large watt values, not only normal values.
- Use split-line layouts for narrow cards when labels and values can overflow.
- For the `Detail`, `Summen`, and `Netz` pages, keep a shared 12 px spacing grid:
  - outer page margin `12`
  - gap between stacked card rows `12`
  - gap above the bottom navigation `12`
  - gap between the two columns `12`
  - shared row anchors should stay aligned unless the user explicitly asks for an intentional exception
- Prefer deriving card rectangles from shared layout constants in `src/main.cpp` instead of hand-tuning each page independently.
- Keep card content anchored from shared inset constants in `src/main.cpp` so label/value spacing is driven by one set of rules instead of per-page cursor tweaks.
- When changing card sizes, keep the inner text padding visually consistent with the card border and re-check all sibling cards on the same page.
- For paired cards on the same row, align text baselines and value blocks so the perceived top and bottom padding inside each card matches as closely as possible.
- If a row mixes different card heights, adjust content anchors per row so middle-row content can sit slightly higher and bottom-row content slightly lower when that improves visual centering inside the frame.
- On the `Netz` page, keep the middle-row progress bars slightly higher than strict geometric center when that makes the label-plus-bar block look visually centered inside the card.
- If a label/value block risks overflow, first adjust the shared inset constants, then switch to a split-line layout, and only then change the card size.
- If the user gives a visual spacing preference during review, carry that preference forward into `AGENTS.md` when it should become an ongoing layout rule instead of a one-off tweak.
- The overview page intentionally keeps labels short: `Solar`, `Verbrauch`.
- Hide non-essential status text on the overview page when it wastes space.
- Display idle mode currently uses a very dim backlight instead of full off because that is more reliable for touch wake on the real device.

### Touch And Idle Behavior

- Wake logic must tolerate held touches, not only one-frame `wasPressed()` events.
- The first touch after wake should only wake the screen and must not accidentally trigger navigation.
- Keep `M5.update()` running even during blocking Wi-Fi or REST phases when working on touch responsiveness.

### Logging

- Keep serial logs concise in the normal branch state.
- Temporary raw-response logging is acceptable for debugging but should be removed or reduced before finalizing a PR.

## Agent Workflow Expectations

- Read `README.md`, `docs/setup.md`, and `docs/architecture.md` before making architectural claims.
- Verify builds with `pio run` after firmware changes.
- If UI or networking behavior changes, prefer reflashing the real Core2 when available.
- Re-check relevant `sensor.core2_*` entities over REST after Home Assistant changes.
- Do not claim the device works unless both build-time checks and at least one runtime validation path were exercised.
- After a firmware change is successfully built and flashed to the real Core2, prefer this workflow unless the user says otherwise:
  1. commit with a meaningful message
  2. push the current branch
  3. check the latest GitHub Actions status and report the result

## Documentation Expectations

- Update `README.md` for user-visible workflow or UI changes.
- Update `docs/setup.md` when setup, flashing, or preflight behavior changes.
- Update `docs/architecture.md` when data flow or responsibilities change.
- Keep docs aligned with verified behavior, not aspirations.

## SKILLS.md Note

No repository-local `SKILLS.md` is currently required.
This project is better served by this repo-specific `AGENTS.md` plus the checked-in docs.

## Bottom Line

This is a real Arduino + Home Assistant project with verified build, flash, monitor, and CI workflow.
Treat the checked-in PlatformIO config, HA package, and docs as the source of truth, and validate behavior on the actual Core2 whenever practical.
