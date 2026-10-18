*[English](README.md) · Deutsch*

# PrintDock Station

Ein kleiner ESP32 mit **NFC-Leser**, **Waage** und **Display** für [PrintDock](https://github.com/Codegulasch/printdock). Spule auflegen – PrintDock erkennt sie, bucht die Restmenge nach Gewicht und ordnet sie dem Fach zu, in das du sie danach einlegst. Bei Bambu-AMS trägt PrintDock Material und Farbe gleich am Fach ein.

> [!IMPORTANT]
> **Öffentliche Beta.** Die Firmware läuft, die Station ist aber noch in der öffentlichen Beta. Am Hardware-Aufbau wird noch gearbeitet: Das Gehäuse zum Drucken und der Waagen-Aufbau (Teller, Halterung der Wägezelle) sind noch nicht fertig. Bis dahin baust du die Station auf einem Steckbrett oder einer Lochrasterplatine auf – Verdrahtung in [docs/wiring.md](docs/wiring.md).
>
> Rückmeldungen, Fehler und Ideen sind sehr willkommen:
> - zur Station: https://github.com/Codegulasch/printdock-station/issues
> - zu PrintDock selbst: https://github.com/Codegulasch/printdock/issues

## Was sie kann

Ein Ablauf für Spulen, RFID und QR-Code:

| Schritt | Was passiert |
|---|---|
| **Neue Spule** | Auf die Station legen. Leerer Tag → PrintDock fragt „welche Spule?“ oder legt sie an und beschreibt den Tag. Anycubic-Tags und OpenSpool-Tags bringen Material und Farbe schon mit. |
| **Etikett** | In PrintDock *Etiketten* drucken – der QR-Code öffnet die Spule auf jedem Handy. |
| **Wiegen** | Volle, neue Spule auflegen → PrintDock lernt das Leergewicht (brutto − netto). Danach ergibt jedes Auflegen die Restmenge. |
| **Einlegen** | Spule von der Station nehmen und in einen Drucker legen. Innerhalb von 3 Minuten ordnet PrintDock sie dem Fach zu, in dem jetzt eine Spule steckt. Bei Bambu (X2D, A1, …) werden Material und Farbe ans AMS übertragen. |
| **Drucken** | PrintDock bucht den Verbrauch je Druck ab (wie bisher). Mit Spoolman geht alles auch dorthin. |
| **Zurück ins Lager** | Auf die Station legen → aktuelle Restmenge, Ort wird „Lager“. |

Die Station selbst ist bewusst „dumm“: Sie liest und schreibt rohe Tag-Seiten, wiegt und zeigt an. Welches Tag-Format, welche Spule und wohin sie gehört, entscheidet allein PrintDock. So bleibt die Firmware klein, und neue Formate kommen mit einem PrintDock-Update.

**PrintDock Sensor:** Nur mit Feuchtesensor (ohne NFC-Leser und Waage) wird aus derselben Firmware der **PrintDock Sensor**, ein Funk-Feuchtemesser für Trockenbox, AMS oder ACE ohne eigene Messung. Er meldet sich in PrintDock unter *Einstellungen → Drucker & Filament → Feuchte & Trocknen*.

**Voraussetzung: PrintDock 0.21 oder neuer.**

## Was du brauchst

- ESP32 (WROOM-DevKit oder D1 Mini ESP32) oder D1 Mini mit ESP8266 (PN532 dann im I²C-Modus)
- PN532-NFC-Leser
- HX711 mit 5-kg-Wägezelle
- 0,96″-OLED (SSD1306, I²C)
- optional: WS2812-LED (zeigt die Spulenfarbe), Taster, passiver Summer, Feuchtesensor (SHT31/SHT40/AHT20/HTU21D)
- NTAG215-Sticker für eigene Spulen (NTAG213 reicht für „Nur Link“ / „Anycubic ACE“)
- Steckbrett oder Lochrasterplatine mit Kabeln, USB-Kabel oder 5-V-Netzteil

Materialkosten etwa 20–30 €. Komplette Stückliste mit Preisen: [docs/wiring.md](docs/wiring.md#stückliste).

## Verdrahtung

Pin-Tabellen, Schaltplan und Steckbrett-Plan je Board (ESP32 und D1 Mini ESP8266): **[docs/wiring.md](docs/wiring.md)**. Alle vier Pläne auf einer Seite: [docs/wiring.html](docs/wiring.html) (herunterladen und im Browser öffnen).

Gehäuse und Waagen-Aufbau (Teller, Halterung der Wägezelle) sind noch in Arbeit – bis dahin baust du die Station auf einem Steckbrett oder einer Lochrasterplatine auf.

## Firmware flashen

1. Auf der **[Releases-Seite](https://github.com/Codegulasch/printdock-station/releases)** die Datei für dein Board herunterladen: `printdock-station-<board>-<version>.factory.bin`

   | `<board>` | Board |
   |---|---|
   | `esp32dev` | ESP32 DevKit (38 Pin) |
   | `d1_mini32` | D1 Mini ESP32 |
   | `d1_mini` | D1 Mini ESP8266 |

2. Board per USB anschließen und **https://web.esphome.io** in Chrome oder Edge öffnen.
3. *Connect* → Port wählen → *Install* → die heruntergeladene `.factory.bin` wählen → *Install*.

Taucht das Board nicht als Port auf, den USB-Treiber für seinen USB-Chip installieren (**CP210x** oder **CH340**) und ein anderes Kabel probieren (manche können nur laden).

Alternativ mit esptool – die factory.bin gehört an Adresse **0x0**:

```sh
esptool.py write_flash 0x0 printdock-station-esp32dev-<version>.factory.bin
```

## Einrichten

1. **WLAN:** Station einschalten und mit Handy oder Laptop das WLAN **PrintDock-Station** wählen (ohne Passwort). Die Einrichtungsseite öffnet sich meist selbst – sonst im Browser **http://192.168.4.1** aufrufen (am Handy mobile Daten kurz aus).
2. *Configure WiFi* tippen, Heim-WLAN wählen (nur 2,4 GHz) und das Passwort eingeben.
3. Ins Feld *PrintDock-Adresse* die Adresse von PrintDock **mit Port 3080** eintragen, z. B. `192.168.1.20:3080` (beim Home-Assistant-Add-on: die IP von Home Assistant mit Port 3080) → *Save*. Schritt für Schritt mit Fehlerhilfe: [docs/wiring.md](docs/wiring.md#wlan-einrichten-schritt-für-schritt).
4. **Koppeln:** Das Display zeigt einen sechsstelligen Code. Bestätige ihn in PrintDock unter *Einstellungen → PrintDock Station*.
5. *Tara* mit leerer Waage, dann mit bekanntem Gewicht *Kalibrieren*.
6. Optional bei der Station einstellen, dass sie **an einem Drucker** steht – mit festem Fach legt PrintDock gescannte Spulen sofort dort hinein.

## Im Alltag

- **Spule auflegen:** PrintDock erkennt sie am Tag und bucht die Restmenge nach Gewicht. Danach in einen Drucker einlegen oder im Lager lassen.
- **Taster:**
  - kurz = Spule erneut melden und buchen
  - lang = Tara
  - beim Einschalten 5 s halten = alles zurücksetzen (WLAN, Adresse, Kopplung). Beim D1 Mini ESP8266: erst einschalten, dann halten, sobald das Display dazu auffordert.

## Fehlersuche

Alles in [docs/wiring.md](docs/wiring.md):

- **Flashen klappt nicht** („Chip is ESP8266“, „Flash ID: 0“, kein Port): [Flashen klappt nicht?](docs/wiring.md#flashen-klappt-nicht)
- **WLAN „PrintDock-Station“ taucht wieder auf / „PrintDock nicht erreichbar“:** WLAN-Passwort falsch, 5-GHz-Netz oder Adresse/Port falsch – [WLAN einrichten](docs/wiring.md#wlan-einrichten-schritt-für-schritt)
- **„NFC-Leser nicht gefunden“ / Tags werden nicht gelesen:** Schalter am PN532 prüfen (ESP32: SPI, D1 Mini ESP8266: I²C), danach ganz stromlos machen – [NFC-Leser reagiert nicht?](docs/wiring.md#nfc-leser-reagiert-nicht)
- **Waage zeigt negative Werte:** A+ und A− am HX711 tauschen.

## Tag-Formate

| Format | Inhalt | Tag | Erkannt von |
|---|---|---|---|
| PrintDock + OpenSpool | Link zur Spule und OpenSpool-JSON | NTAG215/216 | Handy (öffnet die Spule), Snapmaker U1 mit Community-Firmware, OpenSpool-Leser |
| OpenSpool | OpenSpool-JSON | NTAG215/216 | wie oben, ohne Link |
| Nur Link | `http://…/#spool=S-012` | NTAG213+ | Handy |
| Anycubic ACE | Anycubic-Format (Seiten 4–31) | NTAG213/215 | ACE Pro / Kobra S1 |
| Bambu, Snapmaker-Original | (verschlüsselt/signiert) | – | nur an der Kennung (UID) |

## Für Entwickler

Selbst bauen mit [PlatformIO](https://platformio.org):

```sh
pio run                          # alle Boards
pio run -e esp32dev -t upload    # bzw. -e d1_mini32, -e d1_mini (ESP8266)
pio device monitor
```

Die Factory-Images landen in `.pio/build/<env>/printdock-station-<env>.factory.bin`.

Die Station spricht mit PrintDock per WebSocket mit JSON-Nachrichten, beschrieben in **[docs/protocol.md](docs/protocol.md)**. Zum Ausprobieren ohne Hardware liegt im [PrintDock-Repo](https://github.com/Codegulasch/printdock) `scripts/station-sim.js`.

## Lizenz

MIT. Das Anycubic-Tag-Layout stammt aus [DnG-Crafts/ACE-RFID](https://github.com/DnG-Crafts/ACE-RFID), OpenSpool aus [OpenSpool](https://github.com/RobThree/OpenSpool).
