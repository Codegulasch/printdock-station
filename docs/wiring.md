# Verdrahtung und Stückliste

Gleiche GPIOs für **ESP32-WROOM-32 DevKit** (30/38 Pin) und **D1 Mini ESP32** (Wemos/LOLIN, 2 × 20 Pins). Alle Module laufen mit 3,3 V, nur die WS2812-LED braucht 5 V.

## Stückliste

| Teil | Hinweis | ca. Preis |
|---|---|---|
| ESP32-WROOM DevKit **oder** D1 Mini ESP32 | jeder ESP32 mit 4 MB Flash | 4–8 € |
| PN532-NFC-Modul (rot, „V3“) | liest/schreibt NTAG213/215/216, liest MIFARE Classic | 4–8 € |
| HX711 + Wägezelle 5 kg | Balken-Wägezelle (Aluminium) | 4–7 € |
| OLED 0,96″ 128 × 64, SSD1306, I²C | 4 Pins (GND, VCC, SCL, SDA) | 3–5 € |
| WS2812B-LED (einzeln oder Mini-Ring) | optional – zeigt die Spulenfarbe | 1–3 € |
| Taster | optional – kurz: erneut melden, lang: Tara | <1 € |
| Passiver Summer | optional | <1 € |
| Feuchtesensor SHT31, SHT40, AHT20 oder HTU21D (I²C-Modul) | optional – misst Feuchte und Temperatur, z. B. in der Trockenbox | 2–6 € |
| NTAG215-Sticker (25 mm) | für eigene Spulen; NTAG213 reicht für „Nur Link“ / „Anycubic ACE“ | 0,20–0,40 €/Stk |

## Anschlüsse

| Modul | Pin am Modul | ESP32-GPIO |
|---|---|---|
| **PN532** (SPI) | SCK | 18 |
| | MISO | 19 |
| | MOSI | 23 |
| | SS / NSS | 5 |
| | VCC / GND | 3V3 / GND |
| **HX711** | DT / DOUT | 32 |
| | SCK | 33 |
| | VCC / GND | 3V3 / GND |
| **OLED** | SDA | 21 |
| | SCL | 22 |
| | VCC / GND | 3V3 / GND |
| **WS2812** | DIN | 4 |
| | 5V / GND | 5V (VIN) / GND |
| **Taster** | – | 27 ↔ GND |
| **Summer** | + | 25 (− an GND) |
| **Feuchtesensor** | SDA / SCL | 21 / 22 (parallel zum OLED) |
| | VCC / GND | 3V3 / GND |

### PN532 auf SPI stellen

Auf dem roten PN532-Modul (Elechouse V3 und Nachbauten) sitzen zwei kleine Schalter:

| Modus | Schalter 1 | Schalter 2 |
|---|---|---|
| **SPI** (verwendet) | OFF | ON |
| I²C | ON | OFF |
| HSU (UART) | OFF | OFF |

Findet die Station den Leser nicht, zeigt das Display „NFC-Leser nicht gefunden – SPI-Modus prüfen“.

### Wägezelle an den HX711

| Ader | HX711 |
|---|---|
| rot | E+ |
| schwarz | E− |
| weiß | A− |
| grün | A+ |

Zeigt die Waage negative Werte, A+ und A− tauschen. Die Wägezelle braucht eine feste Unterlage auf der einen Seite und den Teller auf der anderen. Eine Spule (Ø 200 mm) liegt auf einem Teller von etwa 120–150 mm sicher auf.

### Nur als Feuchtemesser (PrintDock Sensor)

Ohne PN532 und HX711 – nur ESP32, Feuchtesensor und optional das OLED – wird die Station zum **PrintDock Sensor**, einem Funk-Feuchtemesser, etwa für eine Trockenbox ohne eigene Anzeige oder ein AMS/ACE ohne Messung. In PrintDock erscheint er unter *Einstellungen → Drucker & Filament → Feuchte & Trocknen* zum Koppeln und wird dort einer Messstelle zugeordnet. Der Sensor sollte in der Box sitzen; das ESP32 kann außen bleiben (vier Adern, bis etwa 1 m).

## Erster Start

1. Firmware flashen:
   - Im Browser über https://web.esphome.io → *Connect* → `printdock-station-<board>-<version>.factory.bin`.
   - Oder mit esptool: `esptool.py write_flash 0x0 printdock-station-esp32dev-0.1.0.factory.bin`.
2. Die Station öffnet das WLAN **PrintDock-Station**. Damit verbinden: das Einrichtungsfenster öffnet sich, sonst http://192.168.4.1 aufrufen.
3. Heim-WLAN wählen und die **PrintDock-Adresse** eintragen, z. B. `192.168.1.20:3080`. Beim Home-Assistant-Add-on ist das die IP von Home Assistant mit Port 3080. PrintDock zeigt die passende Adresse unter *Einstellungen → PrintDock Station → Station einrichten*.
4. Das Display zeigt einen sechsstelligen Code. In PrintDock unter *Einstellungen → PrintDock Station* denselben Code bestätigen.
5. Waage kalibrieren: leer *Tara*, dann ein bekanntes Gewicht auflegen und *Kalibrieren*.

**Zurücksetzen:** Taster beim Einschalten 5 Sekunden halten. Das löscht WLAN, Adresse und Kopplung.
