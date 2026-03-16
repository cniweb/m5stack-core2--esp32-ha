# Architektur

## Ziel

Dieses Projekt nutzt einen `M5Stack Core2 V1.1` als kompaktes Wand- oder Tischdisplay fuer Energiedaten aus Home Assistant.
Die Anzeige soll zwei Dinge gleichzeitig leisten:

- aktuelle Solarleistung und aktuellen Hausverbrauch klar und schnell erfassbar zeigen
- den Tagesverlauf beider Werte als einfache historische Visualisierung darstellen

## Empfohlene Architektur

Die robusteste Startarchitektur ist:

1. `Home Assistant` bleibt die zentrale Datenquelle.
2. `ESPHome` laeuft auf dem Core2 und rendert nur die Anzeige.
3. Historische Tageswerte werden in `Home Assistant` in feste Zeit-Buckets aufgeteilt.
4. Der Core2 liest nur aktuelle Werte plus vorberechnete Buckets und zeichnet daraus Gauge und Balken.

Das ist fuer dieses Projekt besser als lokale Historie auf dem ESP32, weil:

- der Verlauf nach einem Neustart des Core2 nicht verloren geht
- die Zeitlogik nicht auf dem Mikrocontroller gepflegt werden muss
- Home Assistant bereits Recorder, Statistik und Automations-Logik mitbringt
- das Display dadurch schlank und reaktiv bleibt

## Anzeigenkonzept

Geplant ist eine Hauptseite mit zwei Bereichen:

- oben zwei Gauge-artige Anzeigen fuer aktuelle Werte
  - Solarleistung in Watt
  - Hausverbrauch in Watt
- unten zwei kompakte Balkenreihen fuer den aktuellen Tag
  - Solar-Tagesverlauf
  - Verbrauchs-Tagesverlauf

Fuer ESPHome/LVGL ist die praktischste Umsetzung:

- `meter` plus `label` fuer aktuelle Werte
- mehrere vertikale `bar`-Widgets fuer den Verlauf

ESPHome bietet in der dokumentierten Form keinen echten Balkendiagramm-Widget mit Achsen und Spaltenserie fuer diesen Anwendungsfall. Daher ist eine Reihe einzelner LVGL-Bars fuer den Tagesverlauf die verlässlichste Loesung.

## Datenmodell

### Live-Werte

Diese Sensoren kommen direkt aus Home Assistant via `sensor.homeassistant`:

- aktuelle Solarleistung, z. B. `sensor.solar_power`
- aktueller Hausverbrauch, z. B. `sensor.house_power`

### Historie

Empfohlen sind `15-Minuten-Buckets` fuer den aktuellen Tag:

- 96 Werte pro Tag fuer Solar
- 96 Werte pro Tag fuer Verbrauch

Fuer eine erste Version kann man das auf 24 Balken vereinfachen:

- 24 Buckets mit je einer Stunde

Das reduziert UI-Aufwand und ist auf 320x240 sehr gut lesbar.

## Verantwortlichkeiten

### Home Assistant

- stellt Live-Sensoren bereit
- aggregiert Tageswerte in feste Zeitfenster
- setzt die Tageshistorie um Mitternacht zurueck
- stellt vorberechnete Werte als Sensoren oder Text-Payload bereit

### ESPHome auf dem Core2

- verbindet sich per WLAN und API mit Home Assistant
- initialisiert Stromversorgung, Display und Touch
- aktualisiert Gauges fuer Live-Werte
- zeichnet Tagesbalken aus vorberechneten Bucket-Werten
- stellt optional Touch-Navigation fuer weitere Seiten bereit

## Hardware-relevante Punkte

Aus der recherchierten Core2/ESPHome-Dokumentation ergeben sich diese wichtigen Punkte:

- Displaymodell fuer ESPHome: `mipi_spi` mit `model: M5CORE2`
- Touchscreen: `ft63x6`
- der Power-Management-Baustein `AXP2101` ist fuer Core2 V1.1 wesentlich
- fuer den AXP2101 wird in der verfuegbaren ESPHome-Loesung der `arduino`-Framework benoetigt
- PSRAM sollte aktiviert werden

Wichtige Pins:

- SPI CLK `GPIO18`
- SPI MOSI `GPIO23`
- SPI MISO `GPIO38`
- Display CS `GPIO5`
- Display DC `GPIO15`
- I2C SDA `GPIO21`
- I2C SCL `GPIO22`
- Touch Interrupt `GPIO39`

## UI-Richtlinien

- Solar in warmen Gelb-/Orange-Toenen
- Hausverbrauch in kuehlen Blau-/Cyan-Toenen
- grosse Zahlenwerte in der oberen Haelfte
- Balken unten mit gleicher Skala pro Reihe
- keine Ueberfrachtung auf der ersten Seite

## Ausbaustufen

### V1

- WLAN, API, OTA
- zwei Live-Gauges
- zwei Tagesbalkenreihen
- Touch-Reiter fuer Uebersicht, Details, Summen und Netz
- Statusanzeige fuer Verbindung und Uhrzeit

### V2

- Touch-Seiten fuer Details
- Batterie- und WLAN-Anzeige
- Nachtmodus oder adaptive Helligkeit
- Tagesertrag und Tagesverbrauch als Summenwerte

### V3

- mehrere Ansichten wie Woche oder Netzbezug
- Alarme bei hoher Last oder geringer PV-Leistung
- optionale lokale Touch-Bedienung fuer Seitenwechsel
