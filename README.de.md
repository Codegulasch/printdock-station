# PrintDock Station

Ein kleiner ESP32 mit **NFC-Leser**, **Waage** und **Display** für [PrintDock](https://github.com/Codegulasch/printdock). Spule auflegen – PrintDock erkennt sie, bucht die Restmenge nach Gewicht und ordnet sie dem Fach zu, in das du sie danach einlegst. Bei Bambu-AMS trägt PrintDock Material und Farbe gleich am Fach ein.

[English](README.md)

## Ein Ablauf für Spulen, RFID und QR-Code

| Schritt | Was passiert |
|---|---|
| **Neue Spule** | Auf die Station legen. Leerer Tag → PrintDock fragt „welche Spule?“ oder legt sie an und beschreibt den Tag. Anycubic-Tags und OpenSpool-Tags bringen Material und Farbe schon mit. |
| **Etikett** | In PrintDock *Etiketten* drucken – der QR-Code öffnet die Spule auf jedem Handy. |
| **Wiegen** | Volle, neue Spule auflegen → PrintDock lernt das Leergewicht (brutto − netto). Danach ergibt jedes Auflegen die Restmenge. |
| **Einlegen** | Spule von der Station nehmen und in einen Drucker legen. Innerhalb von 3 Minuten ordnet PrintDock sie dem Fach zu, in dem jetzt eine Spule steckt. Bei Bambu (X2D, A1, …) werden Material und Farbe ans AMS übertragen. |
| **Drucken** | PrintDock bucht den Verbrauch je Druck ab (wie bisher). Mit Spoolman geht alles auch dorthin. |
| **Zurück ins Lager** | Auf die Station legen → aktuelle Restmenge, Ort wird „Lager“. |

Die Station selbst ist bewusst „dumm“: Sie liest und schreibt rohe Tag-Seiten, wiegt und zeigt an. Welches Tag-Format, welche Spule und wohin sie gehört, entscheidet allein PrintDock. So bleibt die Firmware klein, und neue Formate kommen mit einem PrintDock-Update.

## Tag-Formate

| Format | Inhalt | Tag | Erkannt von |
|---|---|---|---|
| PrintDock + OpenSpool | Link zur Spule und OpenSpool-JSON | NTAG215/216 | Handy (öffnet die Spule), Snapmaker U1 mit Community-Firmware, OpenSpool-Leser |
| OpenSpool | OpenSpool-JSON | NTAG215/216 | wie oben, ohne Link |
| Nur Link | `http://…/#spool=S-012` | NTAG213+ | Handy |
| Anycubic ACE | Anycubic-Format (Seiten 4–31) | NTAG213/215 | ACE Pro / Kobra S1 |
| Bambu, Snapmaker-Original | (verschlüsselt/signiert) | – | nur an der Kennung (UID) |

## Hardware

ESP32 (WROOM-DevKit oder D1 Mini ESP32), PN532, HX711 mit 5-kg-Wägezelle, 0,96″-OLED, optional WS2812-LED, Taster, Summer und ein Feuchtesensor (SHT31/SHT40/AHT20/HTU21D). Nur mit Feuchtesensor wird daraus der **PrintDock Sensor**, ein Funk-Feuchtemesser für Trockenbox, AMS oder ACE ohne eigene Messung – gleiche Firmware, er meldet sich in PrintDock unter *Einstellungen → Drucker & Filament → Feuchte & Trocknen*. Materialkosten etwa 20–30 €. Verdrahtung und Stückliste: **[docs/wiring.md](docs/wiring.md)**.

## Firmware

**Fertige Datei flashen:** Unter *Releases* liegt je Board eine `…factory.bin`.
- Im Browser: https://web.esphome.io → *Connect* → Datei wählen.
- Oder mit esptool: `esptool.py write_flash 0x0 <datei>`.

**Selbst bauen** mit [PlatformIO](https://platformio.org):

```sh
pio run                      # beide Boards
pio run -e d1_mini32 -t upload
pio device monitor
```

Die Factory-Images landen in `.pio/build/<env>/printdock-station-<env>.factory.bin`.

## Einrichten

1. Station starten → WLAN **PrintDock-Station** → Heim-WLAN und PrintDock-Adresse (z. B. `192.168.1.20:3080`) eintragen.
2. Code vom Display in PrintDock unter *Einstellungen → PrintDock Station* bestätigen.
3. *Tara* mit leerer Waage, dann mit bekanntem Gewicht *Kalibrieren*.
4. Optional bei der Station einstellen, dass sie **an einem Drucker** steht – mit festem Fach legt PrintDock gescannte Spulen sofort dort hinein.

**Taster:** kurz = Spule erneut melden und buchen, lang = Tara, beim Einschalten 5 s halten = alles zurücksetzen.

## Protokoll

WebSocket mit JSON-Nachrichten, beschrieben in **[docs/protocol.md](docs/protocol.md)**. Zum Ausprobieren ohne Hardware liegt in PrintDock `scripts/station-sim.js`.

Voraussetzung: PrintDock **0.9.0** oder neuer.

## Lizenz

MIT. Das Anycubic-Tag-Layout stammt aus [DnG-Crafts/ACE-RFID](https://github.com/DnG-Crafts/ACE-RFID), OpenSpool aus [OpenSpool](https://github.com/RobThree/OpenSpool).
