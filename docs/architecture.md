# Architecture

## Zielbild

Das Projekt stellt Energie-Daten aus Home Assistant auf einem `M5Stack Core2` dar.
Die Anzeige fokussiert sich auf:

- aktuelle Solarleistung
- aktuellen Hausverbrauch
- Netzbezug und Einspeisung
- Tagessummen fuer Energiefluesse

## Architekturuebersicht

Die aktuelle Architektur trennt Datenaufbereitung und Darstellung klar:

1. `Home Assistant` bleibt die zentrale Datenquelle.
2. `home-assistant/packages/core2_power_history.yaml` erzeugt abgeleitete `sensor.core2_*`-Entities.
3. Die Arduino-Firmware auf dem Core2 liest diese Werte ueber die REST-API.
4. Das Display rendert die Werte lokal und verwaltet Touch-Navigation sowie Session-Historie.

## Warum Arduino + REST

Diese Branch-Variante nutzt absichtlich keine ESPHome-Firmware auf dem Geraet.
Ziele dieser Entscheidung:

- volle Kontrolle ueber Rendering und Touch-Logik
- direkter Zugriff auf die Home-Assistant-REST-API
- geringere Abhaengigkeit von YAML-basierten Anzeige-Workflows
- gute Basis fuer spaetere UI- oder Protokoll-Experimente

## Rollen der Komponenten

### Home Assistant

- liefert Basissensoren fuer Solar und Hausverbrauch
- leitet Netzbezug und Einspeisung aus den Basissensoren ab
- integriert Leistung zu Energie
- bildet Tageswerte mit `utility_meter`
- stellt fertige `sensor.core2_*`-Entities fuer das Display bereit

### Arduino-Firmware auf dem Core2

- verbindet sich mit WLAN
- authentifiziert sich mit Bearer-Token an Home Assistant
- pollt REST-Endpunkte in festen Intervallen
- rendert die UI mit `M5Unified`
- verwaltet Touch-Reiter und lokale Session-Historie

## Datenfluss

### Eingangs-Sensoren in Home Assistant

- `sensor.house_energy_solar_total`
- `sensor.leistung_haushalt`

### Abgeleitete Sensoren im Paket

- `sensor.core2_solar_live`
- `sensor.core2_house_live`
- `sensor.core2_grid_import_power`
- `sensor.core2_grid_export_power`
- `sensor.core2_solar_day_energy_kwh`
- `sensor.core2_house_day_energy_kwh`
- `sensor.core2_grid_import_day_energy_kwh`
- `sensor.core2_grid_export_day_energy_kwh`

### REST-Abruf auf dem Core2

Die Firmware verwendet pro Entity einen Aufruf gegen:

- `/api/states/<entity_id>`

mit:

- `Authorization: Bearer <token>`
- `Content-Type: application/json`

## Firmware-Aufbau

### Konfiguration

- `include/dashboard_config.h` enthaelt Polling-Intervalle, Skalierung und Entity-IDs
- `include/secrets.h` enthaelt WLAN und Home-Assistant-Zugangsdaten

### Laufzeit

- Polling-Intervall fuer REST-Daten: `15s`
- lokales History-Sampling: `5min`
- lokale History-Laenge: `12` Werte

### Anzeige

- vier Seiten
  - `Ueber`
  - `Detail`
  - `Summen`
  - `Netz`
- Gauges und Karten werden direkt mit `M5.Display` gezeichnet
- die Historie auf der Uebersichtsseite ist nur eine lokale Session-Historie und kein echter Tagesverlauf aus Home Assistant

## Build-Sicherheit

Vor jedem Build wird ein Preflight ausgefuehrt.

Der Hook in `platformio.ini` startet `scripts/ha_rest_preflight.py` und prueft:

- ob `include/secrets.h` vorhanden ist
- ob `HA_TOKEN` kein Platzhalter mehr ist
- ob Home Assistant erreichbar ist
- ob alle benoetigten `sensor.core2_*`-Entities existieren und einen brauchbaren Zustand liefern

Dadurch scheitert ein Build frueh, wenn die Backend-Seite noch nicht korrekt vorbereitet ist.

## Designentscheidungen

### Warum abgeleitete Sensoren in Home Assistant statt direkt im Code

- weniger Logik im Mikrocontroller
- klarere Verantwortlichkeiten
- bessere Wiederverwendbarkeit in Home Assistant selbst
- einfacherer Austausch der Firmware, solange die `sensor.core2_*`-Schnittstelle stabil bleibt

### Warum lokale Session-Historie statt kompletter Tageshistorie per REST

- einfacher Startpunkt fuer den Prototyp
- geringe REST-Last
- wenig RAM- und Implementierungsaufwand auf dem ESP32

## Grenzen der aktuellen Version

- keine echte historische Tageskurve aus Home Assistant
- Polling statt push-basierter Updates
- Netzbezug und Einspeisung sind aus Solar und Hausverbrauch abgeleitet
- keine Beruecksichtigung eines Batteriespeichers in der Energiebilanz

## Sinnvolle Weiterentwicklungen

1. echte Stunden- oder 15-Minuten-Historie aus Home Assistant abrufen
2. REST durch WebSocket oder MQTT ergaenzen
3. Netzsensoren auf echte Zaehlerdaten umstellen
4. weitere Seiten fuer Woche, Batterie oder Wetter ergaenzen
