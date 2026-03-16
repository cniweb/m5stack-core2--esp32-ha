# CONTRIBUTOR.md

Diese Datei beschreibt den Entwicklungs-Workflow fuer Beitraege zu diesem Projekt.
Fuer Endanwender-Setup und Home-Assistant-Installation ist `README.md` der Einstiegspunkt.

## Bevor Du Anfaengst

Bitte lies zuerst:

- `README.md` fuer den Nutzer- und Installationsworkflow
- `docs/setup.md` fuer technische Setup-Details
- `docs/architecture.md` fuer Architektur und Datenfluss
- `AGENTS.md` fuer projektspezifische Regeln und verifizierte Erkenntnisse

## Voraussetzungen Fuer Entwicklung

Du brauchst:

- `PlatformIO` oder eine funktionierende `pio.exe`
- einen `M5Stack Core2`
- Home Assistant im Netzwerk
- Schreibzugriff auf die Home-Assistant-Konfiguration
- einen Long-Lived Access Token fuer Home Assistant

## Lokales Setup

### Home Assistant

Die Home-Assistant-Vorbereitung ist in `README.md` und `docs/setup.md` beschrieben.
Wichtig fuer Entwicklung:

- `home-assistant/packages/core2_power_history.yaml` muss im echten HA-`packages`-Ordner liegen
- die `sensor.core2_*`-Entities muessen vor Builds verfuegbar sein

### Firmware-Secrets

Kopiere:

- `include/secrets.example.h`

nach:

- `include/secrets.h`

`include/secrets.h` ist absichtlich von Git ausgeschlossen.

## Build Und Hardware-Workflow

Verifizierte Befehle:

```sh
pio run
pio run -t upload --upload-port COM8
pio device monitor --port COM8 --baud 115200
```

Hinweis:

- `COM8` wurde in dieser Arbeitsumgebung erfolgreich genutzt
- auf anderen Rechnern kann der Port anders sein

## GitHub Actions

Dieses Repository nutzt einen einfachen GitHub-Actions-Build-Workflow unter:

- `.github/workflows/build.yml`

Der Workflow:

- startet bei Pushes auf `main`
- startet bei Pull Requests
- installiert PlatformIO
- fuehrt `pio run` als Build-Check aus

Wichtig:

- der lokale REST-Preflight bleibt fuer echte Entwicklungsmaschinen aktiv
- in GitHub Actions wird er uebersprungen, weil dort weder `include/secrets.h` noch Zugriff auf Home Assistant vorhanden sind

## Preflight

Vor jedem Build validiert `scripts/ha_rest_preflight.py` automatisch:

- `include/secrets.h` vorhanden
- `HA_BASE_URL` gesetzt
- `HA_TOKEN` kein Platzhalter
- alle benoetigten `sensor.core2_*`-Entities erreichbar
- keine unbrauchbaren States

Sonderfall:

- frische `utility_meter`-Tagessensoren mit `status=collecting` duerfen kurz `unknown` sein
- das wird nur als Warnung behandelt

## Entwicklungsregeln

### Firmware

- UI-Aenderungen immer gegen grosse Wattwerte testen
- auf Textueberlauf in schmalen Karten achten
- schmale Layouts lieber zweizeilig statt abgeschnitten gestalten
- serielle Logs fuer Debugging kurz und gezielt halten

### Home Assistant

- bevorzugt echte Zaehler und Basissensoren nutzen, wenn vorhanden
- die Firmware soll auf stabile `sensor.core2_*`-Namen zugreifen
- bei Aenderungen an upstream-Sensoren zuerst das HA-Paket anpassen

### REST

- REST-Probleme immer getrennt nach Home Assistant, Netzwerk und On-Device-Handling untersuchen
- in diesem Projekt ist die direkte `state`-Extraktion aus dem Response-Text auf dem Geraet robuster als die fruehere JSON-Filter-Variante gewesen

## Vor Einem Commit

Vor einem Commit solltest du moeglichst:

1. `pio run` ausfuehren
2. bei Firmware-Aenderungen auf Hardware flashen
3. den seriellen Monitor pruefen
4. bei HA-Aenderungen die `sensor.core2_*`-Entities gegen Home Assistant verifizieren

## Doku Pflegen

Aktualisiere bei relevanten Aenderungen auch:

- `README.md` fuer Nutzer-Workflow
- `docs/setup.md` fuer Setup-Details
- `docs/architecture.md` fuer Architektur- oder Datenfluss-Aenderungen
- `AGENTS.md` fuer agentische Regeln und wichtige Learnings
