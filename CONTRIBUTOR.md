# CONTRIBUTOR.md

Diese Datei beschreibt den Entwicklungs-Workflow fuer Beitraege zu diesem Projekt.
Fuer Endanwender-Setup und Home-Assistant-Installation ist `README.md` der Einstiegspunkt.

## Bevor Du Anfaengst

Bitte lies zuerst:

- [README.md](README.md) fuer den Nutzer- und Installationsworkflow
- [docs/setup.md](docs/setup.md) fuer technische Setup-Details
- [docs/architecture.md](docs/architecture.md) fuer Architektur und Datenfluss
- [AGENTS.md](AGENTS.md) fuer projektspezifische Regeln und verifizierte Erkenntnisse

## Voraussetzungen Fuer Entwicklung

Du brauchst:

- `PlatformIO` oder eine funktionierende `pio.exe`
- einen `M5Stack Core2`
- Home Assistant im Netzwerk
- Schreibzugriff auf die Home-Assistant-Konfiguration
- einen Long-Lived Access Token fuer Home Assistant

## Lokales Setup

Das vollstaendige lokale Setup fuer Home Assistant (Paket-Installation) und die Firmware (WLAN und HA-Tokens in `secrets.h`) ist detailliert in der **[Setup-Anleitung](docs/setup.md)** beschrieben.
Bitte fuehre diese Schritte aus, bevor du mit der Entwicklung beginnst.

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

Vor jedem lokalen Build validiert `scripts/ha_rest_preflight.py` automatisch deine Konfiguration gegen Home Assistant.
Details dazu, was genau geprueft wird und wie Fehler behoben werden koennen, findest du in der [Setup-Anleitung unter REST-Preflight](docs/setup.md#rest-preflight).

## Entwicklungsregeln

Alle verbindlichen Entwicklungsrichtlinien zu Layouts, Home Assistant Integration, REST-Handling und Code-Style werden zentral in **[AGENTS.md](AGENTS.md)** gepflegt.
Dies stellt sicher, dass sowohl menschliche Entwickler als auch KI-Assistenten nach denselben verifizierten Regeln arbeiten.

## Vor Einem Commit

Vor einem Commit solltest du moeglichst:

1. `pio run` ausfuehren
2. bei Firmware-Aenderungen auf Hardware flashen
3. den seriellen Monitor pruefen
4. bei HA-Aenderungen die `sensor.core2_*`-Entities gegen Home Assistant verifizieren

## Doku Pflegen

Aktualisiere bei relevanten Aenderungen auch:

- [README.md](README.md) fuer Nutzer-Workflow
- [docs/setup.md](docs/setup.md) fuer Setup-Details
- [docs/architecture.md](docs/architecture.md) fuer Architektur- oder Datenfluss-Aenderungen
- [AGENTS.md](AGENTS.md) fuer agentische Regeln und wichtige Learnings
