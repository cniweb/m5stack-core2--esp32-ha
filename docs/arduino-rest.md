# Arduino REST Prototype

Diese Variante ersetzt ESPHome auf dem Geraet durch eine eigene Arduino-Firmware fuer den `M5Stack Core2`.
Die Daten kommen direkt aus der Home-Assistant-REST-API.

## Aktueller Stand

- Plattform: `PlatformIO`
- Board: `m5stack-core2`
- Framework: `arduino`
- Display/Touch: `M5Unified`
- REST/JSON: `HTTPClient`, `WiFiClientSecure`, `ArduinoJson`

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
#define HA_BASE_URL "http://homeassistant.local:8123"
#define HA_TOKEN "eyJ..."
```

## Anzeigeumfang

Die Firmware besitzt vier Touch-Seiten:

- `Ueber`: zwei Gauges plus lokale Session-Historie
- `Detail`: Live-Werte, Bilanz und REST-Status
- `Summen`: Tages-kWh fuer Solar und Verbrauch
- `Netz`: Netzbezug und Einspeisung live und als Tageswerte

## Wichtige Unterschiede zu ESPHome

- keine native Home-Assistant-API, sondern REST-Polling
- Token-Verwaltung liegt in der Firmware
- Historie auf der Uebersichtsseite ist lokal und nur fuer die laufende Session
- Tages-kWh kommen weiterhin aus Home Assistant, nicht aus lokaler Berechnung auf dem Display

## Build und Flash

Typische Befehle waeren:

```sh
pio run
pio run -t upload
pio device monitor
```

In dieser Arbeitsumgebung wurde `pio run` bereits erfolgreich ausgefuehrt.

Ergebnis des Builds:

- RAM: ca. `50 KB`
- Flash: ca. `1.09 MB`

Die Upload- und Monitor-Befehle wurden noch nicht ausgefuehrt.

## HTTPS

Die Vorlage unterstuetzt HTTP und HTTPS.

- fuer lokale Tests ist `HA_ALLOW_INSECURE_TLS 1` der einfachste Weg
- fuer produktive Nutzung sollte spaeter ein Root-CA-Zertifikat in `HA_ROOT_CA_PEM` hinterlegt werden

## Naechste sinnvolle Schritte

1. `include/secrets.h` anlegen
2. Home-Assistant-Paket aktivieren und die `sensor.core2_*`-Entities pruefen
3. Projekt lokal mit PlatformIO bauen
4. bei Bedarf echte Tages- oder Stundenhistorie ueber weitere REST-Endpunkte nachziehen
