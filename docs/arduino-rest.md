# Arduino REST Prototype

Diese Variante nutzt eine eigene Arduino-Firmware auf dem `M5Stack Core2`.
Die Daten kommen direkt aus der Home-Assistant-REST-API.

## Aktueller Stand

- Plattform: `PlatformIO`
- Board: `m5stack-core2`
- Framework: `arduino`
- Display/Touch: `M5Unified`
- REST/JSON: `HTTPClient`, `WiFiClientSecure`

## Projektdateien

- `platformio.ini`
- `src/main.cpp`
- `include/dashboard_config.h`
- `include/secrets.example.h`

## Voraussetzungen

1. In Home Assistant muss das Paket `home-assistant/packages/core2_power_history.yaml` geladen sein.
2. Dadurch entstehen die vom Arduino-Client abgefragten Entities:
   - `sensor.core2_solar_live`
   - `sensor.core2_house_live`
   - `sensor.core2_solar_day_energy_kwh`
   - `sensor.core2_house_day_energy_kwh`
   - `sensor.core2_grid_import_power`
   - `sensor.core2_grid_export_power`
   - `sensor.core2_grid_import_day_energy_kwh`
   - `sensor.core2_grid_export_day_energy_kwh`

## Secrets anlegen

Kopiere `include/secrets.example.h` nach `include/secrets.h` und trage ein:

- WLAN-SSID
- WLAN-Passwort
- Home-Assistant-URL
- Long-Lived Access Token

Beispiel:

```cpp
#define WIFI_SSID "MeinWLAN"
#define WIFI_PASSWORD "geheim"
#define HA_BASE_URL "http://homeassistant:8123"
#define HA_TOKEN "eyJ..."
```

## Anzeigeumfang

Die Firmware besitzt vier Touch-Seiten:

- `Ueber`: zwei Gauges plus lokale Session-Historie
- `Detail`: Live-Werte, Bilanz und REST-Status
- `Summen`: Tages-kWh fuer Solar und Verbrauch
- `Netz`: Netzbezug und Einspeisung live und als Tageswerte

Die ausfuehrliche Einrichtung ist in `docs/SETUP.md` beschrieben.

## Architekturhinweise

- keine native Home-Assistant-API, sondern REST-Polling
- Token-Verwaltung liegt in der Firmware
- Historie auf der Uebersichtsseite ist lokal und nur fuer die laufende Session
- Tages-kWh kommen aus Home Assistant und nicht aus lokaler Berechnung auf dem Display
- Netzbezug und Einspeisung werden aktuell im HA-Paket aus Solar- und Hausleistung abgeleitet

Die ausfuehrliche Architektur steht in `docs/ARCHITECTURE.md`.

## Build und Flash

Vor jedem Build fuehrt PlatformIO jetzt automatisch einen kleinen REST-Preflight aus.

Geprueft werden:

- `HA_BASE_URL` und `HA_TOKEN` aus `include/secrets.h`
- die acht erwarteten `sensor.core2_*`-Entities aus `include/dashboard_config.h`
- ob jeder Sensor per `/api/states/<entity_id>` erreichbar ist
- ob der Rueckgabewert nicht `unknown` oder `unavailable` ist

Wenn einer dieser Checks fehlschlaegt, bricht der Build vor dem Kompilieren ab.

Typische Befehle waeren:

```sh
pio run
pio run -t upload
pio device monitor
```

In dieser Arbeitsumgebung wurden `pio run`, Upload auf den Core2 und der serielle Monitor bereits erfolgreich verwendet.

Ergebnis des Builds:

- RAM: ca. `50 KB`
- Flash: ca. `1.09 MB`

Die Firmware wurde erfolgreich auf den Core2 geflasht und mit seriellen Diagnosemeldungen geprueft.

## HTTPS

Die Vorlage unterstuetzt HTTP und HTTPS.

- fuer lokale Tests ist `HA_ALLOW_INSECURE_TLS 1` der einfachste Weg
- fuer produktive Nutzung sollte spaeter ein Root-CA-Zertifikat in `HA_ROOT_CA_PEM` hinterlegt werden

## Naechste sinnvolle Schritte

1. `include/secrets.h` anlegen
2. Home-Assistant-Paket aktivieren und die `sensor.core2_*`-Entities pruefen
3. Projekt lokal mit PlatformIO bauen
4. danach auf den Core2 flashen
5. bei Bedarf echte Tages- oder Stundenhistorie ueber weitere REST-Endpunkte nachziehen
