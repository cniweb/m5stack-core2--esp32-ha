# Setup

## Voraussetzungen

- Home Assistant laeuft bereits
- ESPHome ist in Home Assistant installiert oder separat verfuegbar
- der `M5Stack Core2 V1.1` ist per USB anschliessbar
- die benoetigten Home-Assistant-Sensoren fuer Solar und Verbrauch existieren bereits

## Empfohlene Home-Assistant-Entitaeten

Passe diese Beispielnamen an dein System an:

- `sensor.house_energy_solar_total`
- `sensor.leistung_haushalt`
- `sensor.stromverbrauch_haushalt_stundlich` optional fuer spaetere Buckets

Falls du andere Entitaetsnamen hast, muessen nur die Referenzen in `esphome/core2-dashboard.yaml` angepasst werden.
Die fehlenden Tagesenergie-Sensoren werden im Home-Assistant-Paket jetzt aus den Leistungssensoren abgeleitet.

## Historie vorbereiten

Fuer die erste Version gibt es zwei sinnvolle Wege:

### Einfacher Start

Nutze 24 Stunden-Buckets fuer den aktuellen Tag.

Vorteile:

- leicht zu verstehen
- wenig UI-Komplexitaet
- gut auf 320x240 darstellbar

### Feinerer Verlauf

Nutze 96 Buckets mit 15 Minuten.

Vorteile:

- genauerer Tagesverlauf
- bessere Sichtbarkeit von Lastspitzen und PV-Schwankungen

Nachteil:

- hoeherer Pflegeaufwand in Home Assistant und in der UI

## ESPHome-Datei verwenden

Die Datei `esphome/core2-dashboard.yaml` ist als Startpunkt gedacht.

Vor dem ersten Flashen musst du mindestens diese Werte anpassen:

- WLAN-Zugangsdaten
- optional spaeter `!secret`-Referenzen einfuehren
- optional `api`- und `ota`-Schluessel ergaenzen
- die verwendeten Home-Assistant-Entity-IDs

Die aktuelle Vorlage nutzt absichtlich einfache Platzhalter-Strings statt `!secret`, damit die Datei ohne YAML-Tag-Sonderfaelle direkt lesbar und leicht anpassbar bleibt.

## Flashen

Wenn ESPHome lokal installiert ist, sind typische Befehle:

```sh
"C:\Users\Admin\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.13_qbz5n2kfra8p0\LocalCache\local-packages\Python313\Scripts\esphome.exe" config esphome/core2-dashboard.yaml
"C:\Users\Admin\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.13_qbz5n2kfra8p0\LocalCache\local-packages\Python313\Scripts\esphome.exe" run esphome/core2-dashboard.yaml
```

Die Konfiguration wurde mit deiner lokalen ESPHome-Installation bereits erfolgreich validiert.

Dabei wurden nur diese hardwaretypischen Hinweise ausgegeben:

- `GPIO5` ist ein Strapping-Pin
- `GPIO15` ist ein Strapping-Pin

Das ist beim Core2 erwartbar, weil diese Pins laut Board-Layout fuer das Display genutzt werden.

## Home Assistant anbinden

Die Live-Werte kommen ueber `sensor.homeassistant` in das Display.
Fuer die Historie gibt es zwei Startoptionen:

- mehrere numerische Helper-Sensoren, je einer pro Bucket
- ein Textsensor mit kommagetrennter oder JSON-codierter Liste, die spaeter vom ESP verarbeitet wird

Fuer einen schnellen Start ist je Bucket ein Sensor am unkompliziertesten.

Fuer deine aktuelle Installation gilt jetzt als Default:

- Solar live aus `sensor.house_energy_solar_total`
- Hausverbrauch live aus `sensor.leistung_haushalt`
- Netzbezug und Einspeisung werden aus Solar minus Hausverbrauch abgeleitet
- Tages-kWh werden per `integration` plus `utility_meter` in Home Assistant erzeugt

Wichtig: Diese Netz-Ableitung ist korrekt fuer eine einfache Bilanz ohne separaten Batteriespeicher. Falls spaeter Batterie oder getrennte Zaehler dazukommen, sollten die Netzsensoren auf echte Zaehlerwerte umgestellt werden.

## Touch-Seiten

Das Display besitzt jetzt vier Touch-Reiter im unteren Bereich:

- `Uebersicht`: Gauges plus kompakte Tagesbalken
- `Details`: Live-Werte, aktuelle Bilanz, Peaks und Systemstatus
- `Summen`: Tages-kWh fuer Solar und Verbrauch sowie Spitzenwerte
- `Netz`: aktueller Netzbezug, aktuelle Einspeisung und Tageswerte fuer beide Richtungen

Die Navigation erfolgt ueber die vier Touch-Flaechen am unteren Rand des Displays.

## Naechster sinnvoller Schritt

1. echte Entity-IDs eintragen
2. zuerst nur Live-Gauges testen
3. danach Summensensoren fuer `kWh` pruefen
4. danach Netzbezug- und Einspeise-Sensoren pruefen
5. anschliessend Stundenbalken aus Home Assistant nachziehen
6. spaeter auf 15-Minuten-Buckets erweitern
