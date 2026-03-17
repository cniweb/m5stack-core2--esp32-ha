Import("env")

import json
import os
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(env["PROJECT_DIR"])
SECRETS_PATH = ROOT / "include" / "secrets.h"
SECRETS_EXAMPLE_PATH = ROOT / "include" / "secrets.example.h"
CONFIG_PATH = ROOT / "include" / "dashboard_config.h"

ALLOWED_COLLECTING_UNKNOWN = {
    "sensor.core2_solar_day_energy_kwh",
    "sensor.core2_house_day_energy_kwh",
    "sensor.core2_grid_import_day_energy_kwh",
    "sensor.core2_grid_export_day_energy_kwh",
}


def parse_define(text, name):
    pattern = rf'^\s*#define\s+{re.escape(name)}\s+"([^"]*)"'
    match = re.search(pattern, text, re.MULTILINE)
    return match.group(1) if match else None


def parse_entities(text):
    return re.findall(r'constexpr const char \*k\w+Entity = "([^"]+)";', text)


def fail(message):
    print(f"HA REST preflight failed: {message}")
    env.Exit(1)


def run_preflight():
    github_actions = str(os.environ.get("GITHUB_ACTIONS", "")).lower() == "true"
    ci_mode = str(os.environ.get("PLATFORMIO_CI", "")).lower() == "true"
    if github_actions or ci_mode:
        print("HA REST preflight: skipped in GitHub Actions")
        return

    if not SECRETS_PATH.exists():
        if SECRETS_EXAMPLE_PATH.exists():
            print("HA REST preflight: include/secrets.h not found, using secrets.example.h for local placeholder validation")
            secrets_text = SECRETS_EXAMPLE_PATH.read_text(encoding="utf-8")
        else:
            fail("include/secrets.h not found")
    else:
        secrets_text = SECRETS_PATH.read_text(encoding="utf-8")

    config_text = CONFIG_PATH.read_text(encoding="utf-8")

    base_url = parse_define(secrets_text, "HA_BASE_URL")
    token = parse_define(secrets_text, "HA_TOKEN")
    entities = parse_entities(config_text)

    if not base_url:
        fail("HA_BASE_URL missing in include/secrets.h")
    if not token:
        fail("HA_TOKEN missing in include/secrets.h")
    if token == "CHANGE_ME_LONG_LIVED_ACCESS_TOKEN":
        fail("HA_TOKEN still uses placeholder value")
    if not entities:
        fail("No Home Assistant entities found in include/dashboard_config.h")

    normalized_base = base_url.rstrip("/")
    headers = [
        "-H",
        f"Authorization: Bearer {token}",
        "-H",
        "Content-Type: application/json",
    ]

    print(f"HA REST preflight: checking {len(entities)} entities against {normalized_base}")

    errors = []
    warnings = []
    for entity_id in entities:
        url = f"{normalized_base}/api/states/{entity_id}"
        try:
            result = subprocess.run(
                ["curl", "-sS", "--fail", *headers, url],
                check=False,
                capture_output=True,
                text=True,
                timeout=15,
            )
        except FileNotFoundError:
            fail("curl not found in PATH")
        except subprocess.TimeoutExpired:
            errors.append(f"{entity_id}: timeout")
            continue

        if result.returncode != 0:
            stderr = (result.stderr or "request failed").strip()
            errors.append(f"{entity_id}: {stderr}")
            continue

        try:
            payload = json.loads(result.stdout)
        except json.JSONDecodeError:
            errors.append(f"{entity_id}: invalid JSON")
            continue

        state = payload.get("state")
        attrs = payload.get("attributes", {})
        if state in (None, "unknown", "unavailable"):
            if (
                entity_id in ALLOWED_COLLECTING_UNKNOWN
                and state == "unknown"
                and attrs.get("status") == "collecting"
            ):
                warnings.append(f"{entity_id}: state=unknown (collecting)")
                print(f"  WARN {entity_id} -> unknown (collecting)")
                continue
            errors.append(f"{entity_id}: state={state}")
            continue

        print(f"  OK  {entity_id} -> {state}")

    if errors:
        print("HA REST preflight found problems:")
        for error in errors:
            print(f"  - {error}")
        env.Exit(1)

    if warnings:
        print("HA REST preflight warnings:")
        for warning in warnings:
            print(f"  - {warning}")

    print("HA REST preflight passed.")


run_preflight()
