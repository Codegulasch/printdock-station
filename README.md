# PrintDock Station

A small ESP32 with an **NFC reader**, a **scale** and a **display** for [PrintDock](https://github.com/Codegulasch/printdock). Put a spool on it and PrintDock recognises it, books the remaining amount by weight, and assigns it to the slot you load it into next. On a Bambu AMS, PrintDock also sets material and colour on that slot.

[Deutsch](README.de.md)

## One workflow for spools, RFID and QR codes

- **New spool:** put it on the station.
  - A blank tag makes PrintDock ask which spool it is, or create the spool and write the tag.
  - Anycubic and OpenSpool tags already carry material and colour.
- **Label:** print labels in PrintDock. The QR code opens the spool on any phone.
- **Weigh:** a new, full spool teaches PrintDock its empty weight (gross − net). After that, every weighing gives the remaining amount.
- **Load:** take the spool off and put it into a printer. Within 3 minutes PrintDock assigns it to the slot that now reports a spool. On Bambu printers the material and colour are sent to the AMS.
- **Print:** usage is booked per print, and synced to Spoolman if you use it.

The station is deliberately thin: it reads and writes raw tag pages, weighs and displays. Tag formats, spool lookup and slot assignment all live in PrintDock, so new formats arrive with a PrintDock update.

## Tag formats

| Format | Content | Tag | Read by |
|---|---|---|---|
| PrintDock + OpenSpool | link to the spool + OpenSpool JSON | NTAG215/216 | phones (opens the spool), Snapmaker U1 with community firmware, OpenSpool readers |
| OpenSpool | OpenSpool JSON | NTAG215/216 | as above, without the link |
| Link only | `http://…/#spool=S-012` | NTAG213+ | phones |
| Anycubic ACE | Anycubic layout (pages 4–31) | NTAG213/215 | ACE Pro / Kobra S1 |
| Bambu, Snapmaker stock | encrypted / signed | – | recognised by UID only |

## Hardware

You need:
- an ESP32 (WROOM devkit or D1 Mini ESP32) or a D1 Mini with ESP8266 (PN532 then in I²C mode)
- a PN532 NFC reader
- an HX711 with a 5 kg load cell
- a 0.96″ OLED display
- optionally a WS2812 LED, a push button, a buzzer and a humidity sensor (SHT31/SHT40/AHT20/HTU21D)

With only the humidity sensor it becomes the **PrintDock Sensor**, a wireless humidity meter for a dry box, AMS or ACE without its own measurement. It uses the same firmware and shows up in PrintDock under *Settings → Printers & filament → Humidity & drying*.

Parts cost roughly €20–30. A printable case (no supports, only 4 screws) is in [case/](case/README.md). Wiring and parts list: [docs/wiring.md](docs/wiring.md).

## Firmware

**Flash a release:** download the `…factory.bin` for your board from *Releases*, then flash it to address 0x0 with either:
- https://web.esphome.io in the browser, or
- `esptool.py write_flash 0x0 <file>`

**Build it yourself** with PlatformIO:

```sh
pio run                      # all boards
pio run -e d1_mini32 -t upload   # or -e d1_mini for the ESP8266
```

## Setup

1. **Wi-Fi:** start the station and join the Wi-Fi network **PrintDock-Station** with a phone or laptop (no password). The setup page usually opens by itself; otherwise open **http://192.168.4.1** in the browser (on a phone, turn mobile data off briefly). Tap *Configure WiFi*, choose your home Wi-Fi (2.4 GHz only), enter its password, put the PrintDock address, e.g. `192.168.1.20:3080`, into *PrintDock-Adresse* and tap *Save*. Step by step with troubleshooting (German): [docs/wiring.md](docs/wiring.md#wlan-einrichten-schritt-für-schritt).
2. In PrintDock, confirm the code shown on the display under *Settings → PrintDock Station*.
3. Tare the scale with nothing on it, then calibrate it with a known weight.

The protocol is described in [docs/protocol.md](docs/protocol.md). To try it without hardware, run `scripts/station-sim.js` from PrintDock. This needs PrintDock **0.9.0** or newer.

## License

MIT. The Anycubic tag layout follows [DnG-Crafts/ACE-RFID](https://github.com/DnG-Crafts/ACE-RFID), and OpenSpool follows [OpenSpool](https://github.com/RobThree/OpenSpool).
