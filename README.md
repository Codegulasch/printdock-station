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
- an ESP32 (WROOM devkit or D1 Mini ESP32)
- a PN532 NFC reader
- an HX711 with a 5 kg load cell
- a 0.96″ OLED display
- optionally a WS2812 LED, a push button and a buzzer

Parts cost roughly €20–30. Wiring and parts list: [docs/wiring.md](docs/wiring.md).

## Firmware

**Flash a release:** download the `…factory.bin` for your board from *Releases*, then flash it to address 0x0 with either:
- https://web.esphome.io in the browser, or
- `esptool.py write_flash 0x0 <file>`

**Build it yourself** with PlatformIO:

```sh
pio run                      # both boards
pio run -e d1_mini32 -t upload
```

## Setup

1. Start the station and join the Wi-Fi network **PrintDock-Station**. Enter your home Wi-Fi and the PrintDock address, e.g. `192.168.1.20:3080`.
2. In PrintDock, confirm the code shown on the display under *Settings → Filament → Stations*.
3. Tare the scale with nothing on it, then calibrate it with a known weight.

The protocol is described in [docs/protocol.md](docs/protocol.md). To try it without hardware, run `scripts/station-sim.js` from PrintDock. This needs PrintDock **0.9.0** or newer.

## License

MIT. The Anycubic tag layout follows [DnG-Crafts/ACE-RFID](https://github.com/DnG-Crafts/ACE-RFID), and OpenSpool follows [OpenSpool](https://github.com/RobThree/OpenSpool).
