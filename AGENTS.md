# AGENTS.md

Repository guidance for agentic coding assistants working in `m5stack-core2--esp32-ha`.

## Repository Scope

This repository contains a custom `Arduino + PlatformIO` firmware for an `M5Stack Core2`.
The device acts as a touch display for Home Assistant energy data and reads values through the Home Assistant REST API.

The repository also includes a Home Assistant package that creates the `sensor.core2_*` entities consumed by the device firmware.

## Current Stack

- Firmware: `Arduino` on `ESP32`
- Build system: `PlatformIO`
- Board: `m5stack-core2`
- Display and touch: `M5Unified`
- Backend integration: `Home Assistant REST API`
- Home Assistant sidecar config: `home-assistant/packages/core2_power_history.yaml`

## Source Of Truth Files

- `platformio.ini` - build target and pre-build hooks
- `src/main.cpp` - full device firmware and UI rendering
- `include/dashboard_config.h` - entity IDs, polling intervals, scales, history sizes
- `include/secrets.example.h` - required local secret structure
- `scripts/ha_rest_preflight.py` - mandatory pre-build Home Assistant connectivity check
- `home-assistant/packages/core2_power_history.yaml` - required HA package for derived sensors
- `README.md`, `docs/setup.md`, `docs/architecture.md` - verified project docs

## Cursor And Copilot Rules

No `.cursorrules`, `.cursor/rules/`, or `.github/copilot-instructions.md` files were present during the latest review.
If any are added later, agents must treat them as higher-priority instructions and sync this file.

## Verified Commands

These commands were verified in this repository:

```sh
pio run
pio run -t upload --upload-port COM8
pio device monitor --port COM8 --baud 115200
```

Notes:

- `pio run` executes a pre-build Home Assistant REST validation step.
- Upload to `COM8` was verified for the connected `M5Stack Core2` in this workspace.
- The exact COM port may differ on another machine.

## Build Commands

- Standard build: `pio run`
- Upload firmware: `pio run -t upload --upload-port <COMx>`
- Serial monitor: `pio device monitor --port <COMx> --baud 115200`

## Lint And Static Checks

- Lint command: not configured.
- Format command: not configured.
- Static analysis command: not configured.
- Type-check command: not configured.

For this repo, `pio run` plus the REST preflight is the main verification step.

## Test Commands

- Firmware verification: `pio run`
- Hardware smoke test: `pio run -t upload --upload-port <COMx>` then `pio device monitor --port <COMx> --baud 115200`
- Home Assistant backend verification: run the PlatformIO preflight through `pio run`

## Running A Single Test

No unit test framework is configured.
There is no verified single-test command.

If tests are added later, update this section with the exact framework-native form.

## Required Home Assistant Entities

The device firmware expects these Home Assistant entities to exist before builds and uploads are considered healthy:

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

Before every `pio run`, `scripts/ha_rest_preflight.py` validates:

- `include/secrets.h` exists
- `HA_BASE_URL` is set
- `HA_TOKEN` is not the placeholder value
- the required `sensor.core2_*` entities are reachable through `/api/states/<entity_id>`
- entity states are usable

Special case:

- fresh `utility_meter` sensors may temporarily report `state=unknown` with `status=collecting`
- that condition is treated as a warning, not a hard failure

Agents must not remove or bypass this preflight unless the user explicitly requests it.

## Local Secrets Handling

- `include/secrets.h` is intentionally ignored by Git
- never commit real Wi-Fi credentials or Home Assistant tokens
- use `include/secrets.example.h` as the template
- if changing secret names, update both the example file and any consuming code/docs

## Hardware And Upload Guidance

- The device is an `M5Stack Core2` using the `m5stack-core2` PlatformIO board target.
- Uploads were verified over USB serial on Windows.
- Always verify the actual COM port with `pio device list` or system serial port tools before upload.

## Code Style Guidance

### General

- Keep changes small and hardware-focused.
- Prefer explicit, readable code over abstraction-heavy refactors.
- Preserve the current direct-drawing UI style unless a broader redesign is requested.
- Use ASCII unless the file already uses another character set.

### Imports And Dependencies

- Prefer standard/library headers first, then framework headers, then local headers.
- Remove unused dependencies when implementation changes make them unnecessary.
- Avoid adding new libraries unless the current stack cannot reasonably support the task.
- Prefer the existing `M5Unified`, `HTTPClient`, `WiFi`, and `WiFiClientSecure` stack.

### Formatting

- Match the existing brace and spacing style in `src/main.cpp`.
- Keep helper functions compact and near related rendering or networking logic.
- Avoid unrelated formatting churn.

### Naming

- Use `g_` prefix for persistent globals, matching the existing firmware style.
- Use descriptive helper names such as `draw_split_value`, `draw_history_row`, `refresh_data`.
- Keep page names and entity concepts consistent with the UI and docs.

### Error Handling

- Fail clearly in pre-build scripts.
- For firmware runtime issues, prefer concise serial diagnostics over silent failure.
- Do not hide network or REST failures if they affect displayed values.
- Preserve safe fallbacks like `n/v` for unavailable sensor values.

### State Management

- Keep Home Assistant derived values in Home Assistant where practical.
- Keep the device focused on polling, rendering, and limited local interaction.
- Avoid moving business logic from HA into the ESP32 unless explicitly requested.

## Project-Specific Best Practices

### Home Assistant Integration

- Prefer real Home Assistant source sensors over derived approximations when available.
- In this project, real grid energy totals are preferred to locally reconstructed totals.
- Keep the firmware bound to stable `sensor.core2_*` names, not raw upstream entity IDs.
- If the user changes backend entities, update the HA package first, then keep firmware IDs stable if possible.

### REST Handling

- Hostname resolution may differ between desktop tooling and the Core2.
- If REST works on the workstation but not on-device, investigate request/response handling before assuming the host is wrong.
- The firmware currently extracts the `state` value directly from the response body text because that proved more reliable on-device than the earlier JSON parsing attempt.
- Do not replace this with a more abstract parser without re-verifying on the actual hardware.

### Utility Meter And Energy Precision

- Tiny solar values can keep Home Assistant energy integrations at `0.000` too long if precision is too low.
- The solar total integration uses higher precision so `sensor.core2_solar_day_energy_kwh` initializes correctly.
- If a day-energy sensor stays `unknown`, inspect the upstream integration sensor precision before changing the firmware.

### Display Layout Work

- Test layout changes against large watt values, not only normal values.
- Use split-line layouts for narrow cards when labels and values can overflow.
- The overview page intentionally keeps labels short: `Solar`, `Haus`.
- Hide non-essential status text on the overview page when it would waste display space.

### Logging

- Keep serial logs concise in the normal branch state.
- Temporary raw-response logging is acceptable for debugging but should be removed or reduced before finalizing a PR.

## Agent Workflow Expectations

- Read `README.md`, `docs/setup.md`, and `docs/architecture.md` before making architectural claims.
- Verify builds with `pio run` after firmware changes.
- If UI or networking behavior changes, prefer reflashing the real Core2 when available.
- When Home Assistant configuration changes are involved, re-check the relevant `sensor.core2_*` entities over REST.
- Do not claim the device works unless both build-time checks and at least one runtime validation path were exercised.
- When a firmware change has been successfully built and flashed to the real Core2, prefer this follow-up workflow unless the user says otherwise:
  1. commit the change with a meaningful message
  2. push the current branch
  3. check the latest GitHub Actions status and report whether the remote build passed

## When Touching Home Assistant Package Files

- Preserve the contract that firmware consumes `sensor.core2_*` entities.
- Avoid renaming those entities unless the firmware and docs are updated together.
- Prefer accurate units and correct `device_class` / `state_class` metadata.
- Be careful with `utility_meter` initialization behavior right after restart.

## When Touching Firmware UI Files

- Check card widths before adding longer text.
- Consider how values behave at `0`, high wattage, negative balances, and unavailable states.
- Keep first-page content visually simple; move detail text to secondary pages.

## Documentation Expectations

- Update `README.md` for user-visible workflow changes.
- Update `docs/setup.md` when setup, flashing, or preflight behavior changes.
- Update `docs/architecture.md` when data flow or responsibilities change.
- Keep docs aligned with verified behavior, not aspirations.

## SKILLS.md Note

No repository-local `SKILLS.md` is currently required.
The project is better served by this repo-specific `AGENTS.md` plus the checked-in docs.
If a future `SKILLS.md` is added, use it for reusable workflows only, not for replacing concrete repo facts stored here.

## Bottom Line

This is a real, non-empty Arduino + Home Assistant project with verified build and hardware workflow.
Agents should treat the checked-in PlatformIO config, Home Assistant package, and documentation as the source of truth and should validate changes against the actual Core2 and Home Assistant REST integration whenever practical.
