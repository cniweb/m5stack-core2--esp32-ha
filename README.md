# M5Stack Core2 Home Assistant REST Dashboard

Ein `M5Stack Core2` dient in diesem Projekt als eigenstaendiges Touch-Display fuer Energie-Daten aus Home Assistant.
Die Firmware laeuft als Arduino-Anwendung auf dem ESP32 und liest die benoetigten Werte direkt ueber die Home-Assistant-REST-API.

## Kurzueberblick

- Live-Anzeige fuer Solarleistung und Hausverbrauch
- Tageswerte fuer Solar, Verbrauch, Netzbezug und Einspeisung
- vier Touch-Seiten auf dem Core2
- Home-Assistant-Paket fuer stabile `sensor.core2_*`-Entities
- REST-Preflight vor jedem Build
- GitHub Actions Build-Check fuer Pushes und Pull Requests

## Display-Seiten

- `Ueber`: Gauges fuer Solar und Verbrauch, Akku oben rechts, kompakte Balkenhistorie mit Labels links neben den Balken
- `Detail`: Live-Werte, Bilanz, Peaks und Status
- `Summen`: Tages-kWh fuer Solar und Hausverbrauch
- `Netz`: Netzbezug und Einspeisung live plus Tageswerte

## UI-Stand

Der aktuell verifizierte UI-Stand auf dem Core2:

- die `Ueber`-Seite ist absichtlich kompakt gehalten
- der Akkustand erscheint nur auf der `Ueber`-Seite oben rechts
- das WLAN-Signal wird nur eingeblendet, wenn es schwach ist
- die Balkenlabels `Solar` und `Haus` stehen links vor den jeweiligen Balkendiagrammen
- schmale Karten nutzen an mehreren Stellen zweizeilige Werte, damit grosse Wattzahlen nicht abgeschnitten werden

Aktuell sind keine Screenshots im Repository eingecheckt.
Die textliche Beschreibung in dieser README bildet daher den verifizierten Ist-Zustand ab.

## Voraussetzungen

Fuer die Nutzung brauchst du:

- `M5Stack Core2`
- Home Assistant im selben Netzwerk
- Zugriff auf die Home-Assistant-Konfiguration
- einen `Long-Lived Access Token` fuer Home Assistant
- PlatformIO oder eine vorhandene `pio.exe`

## Installation Und Nutzung

### 1. Home Assistant vorbereiten

In deiner `configuration.yaml` muss Paket-Unterstuetzung aktiv sein:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

Danach die Paketdatei

- `home-assistant/packages/core2_power_history.yaml`

nach

- `packages/core2_power_history.yaml`

in dein echtes Home-Assistant-Konfigurationsverzeichnis kopieren und Home Assistant neu starten.

### 2. Erwartete Entities pruefen

Nach dem Neustart sollten diese Entities vorhanden sein:

- `sensor.core2_solar_live`
- `sensor.core2_house_live`
- `sensor.core2_solar_day_energy_kwh`
- `sensor.core2_house_day_energy_kwh`
- `sensor.core2_grid_import_power`
- `sensor.core2_grid_export_power`
- `sensor.core2_grid_import_day_energy_kwh`
- `sensor.core2_grid_export_day_energy_kwh`

Hinweis:

- frische `utility_meter`-Tagessensoren koennen kurz `unknown` mit `status=collecting` sein
- das ist direkt nach einem Neustart normal

### 3. Firmware konfigurieren

Kopiere:

- `include/secrets.example.h`

nach:

- `include/secrets.h`

Trage dort ein:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `HA_BASE_URL`
- `HA_TOKEN`

### 4. Bauen und flashen

Typische Befehle:

```sh
pio run
pio run -t upload --upload-port COM8
pio device monitor --port COM8 --baud 115200
```

Vor jedem Build wird automatisch ein REST-Preflight ausgefuehrt.

In GitHub Actions wird der Firmware-Build ebenfalls automatisch geprueft.
Der Online-REST-Preflight wird dort bewusst uebersprungen, weil keine lokalen Secrets und kein Zugriff auf deine Home-Assistant-Instanz vorhanden sind.

### 5. Im Alltag nutzen

- Der Core2 verbindet sich nach dem Start mit dem WLAN.
- Anschliessend werden die `sensor.core2_*`-Entities per REST gepollt.
- Die unteren Touch-Reiter wechseln zwischen den vier Ansichten.
- Die `Ueber`-Seite zeigt den Akku rechtsbuendig oben an und blendet WLAN nur bei schwachem Signal ein.

## Fehlersuche

- `404` im Preflight: Paket nicht geladen oder `sensor.core2_*` fehlt
- `401` im Preflight: Token in `include/secrets.h` ungueltig
- `n/v` auf dem Display: zuerst seriellen Monitor pruefen
- WLAN verbindet sich nicht: SSID, Passwort und 2.4-GHz-Verfuegbarkeit pruefen

## Projektstruktur

- `src/main.cpp` - komplette Arduino-Firmware und UI
- `include/dashboard_config.h` - Entity-IDs, Polling und Anzeigeparameter
- `include/secrets.example.h` - Vorlage fuer lokale Secrets
- `scripts/ha_rest_preflight.py` - Build-Preflight fuer Home Assistant REST
- `home-assistant/packages/core2_power_history.yaml` - Home-Assistant-Paket fuer die benoetigten `sensor.core2_*`

## Weiterfuehrende Doku

- Setup im Detail: `docs/setup.md`
- Architektur und Datenfluss: `docs/architecture.md`
- Arduino-REST-Ueberblick: `docs/arduino-rest.md`
- Hinweise fuer Mitwirkende: `CONTRIBUTOR.md`

## CI

Fuer Pushes auf `main` und fuer Pull Requests laeuft ein GitHub-Actions-Workflow:

- `.github/workflows/build.yml`

Der Workflow prueft, ob die Firmware mit PlatformIO erfolgreich gebaut werden kann.

## Mitwirken

Wenn du am Projekt mitarbeiten willst, lies bitte zuerst:

- `CONTRIBUTOR.md`

Die Datei verweist auf die technische Doku und beschreibt den verifizierten Entwicklungs-Workflow.
