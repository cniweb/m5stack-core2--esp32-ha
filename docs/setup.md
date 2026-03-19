# Setup

## Ziel

Diese Anleitung beschreibt die Einrichtung der Arduino-REST-Variante fuer das `M5Stack Core2` Display.
Die Firmware liest Daten direkt aus Home Assistant ueber die REST-API.

## Voraussetzungen

- Home Assistant laeuft im Netzwerk
- `M5Stack Core2` ist per USB anschliessbar
- PlatformIO ist installiert
- die Basissensoren in Home Assistant existieren
  - `sensor.house_energy_solar_total`
  - `sensor.leistung_haushalt`
- optional fuer spaetere Erweiterungen
  - `sensor.stromverbrauch_haushalt_stundlich`

## Home Assistant vorbereiten

Kopiere `home-assistant/packages/core2_power_history.yaml` in dein Home-Assistant-Konfigurationsverzeichnis nach `packages/core2_power_history.yaml`.

Aktiviere in `configuration.yaml` die Paket-Unterstuetzung:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

Danach Home Assistant neu starten.

## Erwartete Entities

Nach dem Neustart sollten diese Entities vorhanden sein:

- `sensor.core2_solar_live`
- `sensor.core2_house_live`
- `sensor.core2_solar_day_energy_kwh`
- `sensor.core2_house_day_energy_kwh`
- `sensor.core2_grid_import_power`
- `sensor.core2_grid_export_power`
- `sensor.core2_grid_import_energy_total`
- `sensor.core2_grid_export_energy_total`
- `sensor.core2_grid_import_day_energy_kwh`
- `sensor.core2_grid_export_day_energy_kwh`

Wenn diese Entities fehlen, wird der Build spaeter durch den REST-Preflight absichtlich gestoppt.

## Secrets anlegen

Kopiere `include/secrets.example.h` nach `include/secrets.h`.

Trage mindestens diese Werte ein:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `HA_BASE_URL`
- `HA_TOKEN`

Beispiel:

```cpp
#pragma once

#define WIFI_SSID "MeinWLAN"
#define WIFI_PASSWORD "geheim"
#define HA_BASE_URL "http://homeassistant:8123"
#define HA_TOKEN "eyJ..."

#define HA_ALLOW_INSECURE_TLS 1
#define HA_ROOT_CA_PEM ""
```

`include/secrets.h` ist in `.gitignore` eingetragen und wird nicht committed.

## Build

Typische Befehle:

```sh
pio run
pio run -t upload
pio device monitor
```

## REST-Preflight

Vor jedem Build fuehrt `scripts/ha_rest_preflight.py` automatisch einen Test gegen Home Assistant aus.

Geprueft werden:

- Erreichbarkeit von `HA_BASE_URL`
- Gueltigkeit des Bearer-Tokens
- Erreichbarkeit aller benoetigten `sensor.core2_*`-Entities
- gueltiger `state` statt `unknown` oder `unavailable`

Wenn ein Check fehlschlaegt, stoppt `pio run` vor dem Kompilieren.

Ausnahme:

- frisch initialisierte `utility_meter`-Sensoren wie `sensor.core2_*_day_energy_kwh` duerfen voruebergehend `unknown` sein, solange ihr Status in Home Assistant auf `collecting` steht
- in diesem Fall gibt der Preflight nur eine Warnung aus und laesst den Build weiterlaufen

## Firmware flashen

Sobald `pio run` erfolgreich ist, kannst du flashen:

```sh
pio run -t upload
```

Danach den seriellen Monitor oeffnen:

```sh
pio device monitor
```

Der Upload auf einen angeschlossenen Core2 ueber `COM8` wurde in dieser Arbeitsumgebung bereits erfolgreich getestet.

## Touch-Seiten

Die Firmware besitzt vier Touch-Reiter unten auf dem Display:

- `Ueber`
- `Detail`
- `Summen`
- `Netz`

## Fehlersuche

- `404` im Preflight bedeutet meist: die `sensor.core2_*`-Entities existieren noch nicht in Home Assistant
- `401` bedeutet meist: der Token in `include/secrets.h` ist ungueltig
- keine Verbindung bedeutet meist: `HA_BASE_URL` oder Netzwerk stimmen nicht
- wenn Home Assistant per HTTPS arbeitet, sollte spaeter `HA_ROOT_CA_PEM` statt `HA_ALLOW_INSECURE_TLS` verwendet werden

## Naechste Ausbaustufen

1. echte stunden- oder viertelstundenbasierte Historie ueber REST nachziehen
2. lokale Caching-Strategie verbessern
3. zusaetzliche Seiten fuer Woche, Monatswerte oder Batterie ergaenzen
