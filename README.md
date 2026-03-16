# M5Stack Core2 Home Assistant REST Dashboard

Dieses Projekt nutzt einen `M5Stack Core2` als Touch-Display fuer Energie-Daten aus Home Assistant.
Die Firmware laeuft als eigene Arduino-Anwendung auf dem ESP32 und liest die Werte ueber die Home-Assistant-REST-API.

## Funktionen

- Live-Anzeige fuer Solarleistung und Hausverbrauch
- Tagessummen fuer Solar, Verbrauch, Netzbezug und Einspeisung
- vier Touch-Seiten auf dem Core2
- REST-Preflight vor jedem Build, damit fehlende Home-Assistant-Entities frueh erkannt werden
- Home-Assistant-Paket zur Erzeugung der benoetigten `sensor.core2_*`-Entities

## Seiten auf dem Display

- `Ueber`: zwei Gauge-artige Anzeigen plus lokale Session-Historie
- `Detail`: Live-Werte, Bilanz und Verbindungsstatus
- `Summen`: Tages-kWh fuer Solar und Hausverbrauch
- `Netz`: Netzbezug und Einspeisung live plus Tageswerte

## Projektstruktur

- `src/main.cpp` - Arduino-Firmware fuer den Core2
- `include/dashboard_config.h` - verwendete Home-Assistant-Entity-IDs
- `include/secrets.example.h` - Vorlage fuer WLAN und API-Token
- `scripts/ha_rest_preflight.py` - REST-Check vor dem Build
- `home-assistant/packages/core2_power_history.yaml` - Home-Assistant-Paket fuer abgeleitete Sensoren
- `docs/SETUP.md` - Schritt-fuer-Schritt-Einrichtung
- `docs/ARCHITECTURE.md` - Architektur und Designentscheidungen
- `docs/arduino-rest.md` - kompakte Projektdoku fuer die Arduino-Variante

## Voraussetzungen

- `M5Stack Core2`
- Home Assistant mit Zugriff auf die Basissensoren
  - `sensor.house_energy_solar_total`
  - `sensor.leistung_haushalt`
- PlatformIO oder `pio.exe`
- ein Long-Lived Access Token fuer Home Assistant

## Schnellstart

1. Home-Assistant-Paket aus `home-assistant/packages/core2_power_history.yaml` einbinden.
2. In Home Assistant pruefen, dass die `sensor.core2_*`-Entities vorhanden sind.
3. `include/secrets.example.h` nach `include/secrets.h` kopieren und anpassen.
4. Firmware mit `pio run` bauen.
5. Auf den Core2 flashen und mit `pio device monitor` beobachten.

## Build

Typische Befehle:

```sh
pio run
pio run -t upload
pio device monitor
```

Vor `pio run` wird automatisch ein REST-Preflight ausgefuehrt. Der Build bricht ab, wenn:

- `include/secrets.h` fehlt
- `HA_TOKEN` noch ein Platzhalter ist
- Home Assistant nicht erreichbar ist
- benoetigte `sensor.core2_*`-Entities nicht existieren

## Home Assistant

Die Firmware liest nicht direkt deine Ursprungs-Sensoren, sondern die im Paket erzeugten `sensor.core2_*`-Entities.
Das Paket leitet fehlende Groessen wie Netzbezug, Einspeisung und Tagesenergie in Home Assistant ab.

## Sicherheit

- `include/secrets.h` ist durch `.gitignore` ausgeschlossen
- der REST-Zugriff nutzt einen Bearer-Token
- fuer lokale Installationen ist HTTP moeglich; bei HTTPS sollte spaeter ein CA-Zertifikat verwendet werden

## Dokumentation

- Setup: `docs/SETUP.md`
- Architektur: `docs/ARCHITECTURE.md`
- Arduino-REST-Details: `docs/arduino-rest.md`
