*English · [Deutsch](README.de.md)*

# PrintDock Station

A small ESP32 with an **NFC reader**, a **scale** and a **display** for [PrintDock](https://github.com/Codegulasch/printdock). Put a spool on it and PrintDock recognises it, books the remaining amount by weight, and assigns it to the slot you load it into next. On a Bambu AMS, PrintDock also sets material and colour on that slot.

> [!IMPORTANT]
> **Public beta.** The firmware works, but the station is still in public beta. The hardware build is still in the works: the printable case and the scale build (platform, load-cell mount) are not finished yet. Until then, build the station on a breadboard or perfboard – wiring in [docs/wiring.md](docs/wiring.md).
>
> Feedback, bugs and ideas are very welcome:
> - for the station: https://github.com/Codegulasch/printdock-station/issues
> - for PrintDock itself: https://github.com/Codegulasch/printdock/issues

## What it does

One workflow for spools, RFID tags and QR codes:

| Step | What happens |
|---|---|
| **New spool** | Put it on the station. A blank tag makes PrintDock ask which spool it is, or create the spool and write the tag. Anycubic and OpenSpool tags already carry material and colour. |
| **Label** | Print labels in PrintDock. The QR code opens the spool on any phone. |
| **Weigh** | A new, full spool teaches PrintDock its empty weight (gross − net). After that, every weighing gives the remaining amount. |
| **Load** | Take the spool off and put it into a printer. Within 3 minutes PrintDock assigns it to the slot that now reports a spool. On Bambu printers (X2D, A1, …) material and colour are sent to the AMS. |
| **Print** | Usage is booked per print, as before, and synced to Spoolman if you use it. |
| **Back to storage** | Put it on the station: current remaining amount, location becomes “storage”. |

The station is deliberately thin: it reads and writes raw tag pages, weighs and displays. Tag formats, spool lookup and slot assignment all live in PrintDock, so the firmware stays small and new formats arrive with a PrintDock update.

**PrintDock Sensor:** with only a humidity sensor (no NFC reader, no scale) the same firmware becomes the **PrintDock Sensor**, a wireless humidity meter for a dry box, AMS or ACE without its own measurement. It shows up in PrintDock under *Settings → Printers & filament → Humidity & drying*.

**Requires PrintDock 0.21 or newer.**

## What you need

- an ESP32 (WROOM devkit or D1 Mini ESP32) or a D1 Mini with ESP8266 (PN532 then in I²C mode)
- a PN532 NFC reader
- an HX711 with a 5 kg load cell
- a 0.96″ OLED display (SSD1306, I²C)
- optional: a WS2812 LED (shows the spool colour), a push button, a passive buzzer, a humidity sensor (SHT31/SHT40/AHT20/HTU21D)
- NTAG215 stickers for your own spools (NTAG213 is enough for “Link only” / “Anycubic ACE”)
- a breadboard or perfboard and jumper wires, a USB cable or 5 V power supply

Parts cost roughly €20–30. Full parts list with prices: [docs/wiring.md](docs/wiring.md#stückliste).

## Wiring

Pin tables, schematic and breadboard plan per board (ESP32 and D1 Mini ESP8266): **[docs/wiring.md](docs/wiring.md)**. All four plans on one page: [docs/wiring.html](docs/wiring.html) (download and open in the browser). The wiring docs are in German; the diagrams and pin tables are self-explanatory.

The case and the scale build (platform, load-cell mount) are still in the works – until then, build it on a breadboard or perfboard.

## Flash the firmware

1. Download the file for your board from the **[Releases page](https://github.com/Codegulasch/printdock-station/releases)**: `printdock-station-<board>-<version>.factory.bin`

   | `<board>` | Board |
   |---|---|
   | `esp32dev` | ESP32 DevKit (38 pin) |
   | `d1_mini32` | D1 Mini ESP32 |
   | `d1_mini` | D1 Mini ESP8266 |

2. Connect the board via USB and open **https://web.esphome.io** in Chrome or Edge.
3. *Connect* → choose the port → *Install* → choose the downloaded `.factory.bin` → *Install*.

If the board does not show up as a port, install the USB driver for its USB chip (**CP210x** or **CH340**) and try a different cable (some are charge-only).

Alternatively with esptool – the factory.bin goes to offset **0x0**:

```sh
esptool.py write_flash 0x0 printdock-station-esp32dev-<version>.factory.bin
```

## Set up

1. **Wi-Fi:** power up the station and join the Wi-Fi network **PrintDock-Station** with a phone or laptop (no password). The setup page usually opens by itself; otherwise open **http://192.168.4.1** in the browser (on a phone, turn mobile data off briefly).
2. Tap *Configure WiFi*, choose your home Wi-Fi (2.4 GHz only) and enter its password.
3. Put the PrintDock address **with port 3080** into *PrintDock-Adresse*, e.g. `192.168.1.20:3080` (with the Home Assistant add-on: the Home Assistant IP with port 3080), and tap *Save*. Step by step with troubleshooting: [docs/wiring.md](docs/wiring.md#wlan-einrichten-schritt-für-schritt).
4. **Pair:** the display shows a six-digit code. Confirm it in PrintDock under *Settings → PrintDock Station*.
5. **Tare** the scale with nothing on it, then **calibrate** it with a known weight.
6. Optional: set the station to stand **at a printer** – with a fixed slot, PrintDock puts scanned spools straight into it.

## Daily use

- **Place a spool** on the station: PrintDock recognises it by its tag and books the remaining amount by weight. Then load it into a printer, or leave it in storage.
- **Button:**
  - short press = report the spool again and book it
  - long press = tare
  - hold for 5 s at power-on = reset everything (Wi-Fi, address, pairing). On the D1 Mini ESP8266: power up first, then hold as soon as the display asks you to.

## Troubleshooting

All in [docs/wiring.md](docs/wiring.md) (German):

- **Flashing fails** (“Chip is ESP8266”, “Flash ID: 0”, no port): [Flashen klappt nicht?](docs/wiring.md#flashen-klappt-nicht)
- **The “PrintDock-Station” Wi-Fi keeps coming back / “PrintDock nicht erreichbar”:** wrong Wi-Fi password, 5 GHz network or wrong address/port – [WLAN einrichten](docs/wiring.md#wlan-einrichten-schritt-für-schritt)
- **“NFC-Leser nicht gefunden” / tags are not read:** check the PN532 switches (SPI on ESP32, I²C on D1 Mini ESP8266), then power off completely – [NFC-Leser reagiert nicht?](docs/wiring.md#nfc-leser-reagiert-nicht)
- **The scale shows negative values:** swap A+ and A− on the HX711.

## Tag formats

| Format | Content | Tag | Read by |
|---|---|---|---|
| PrintDock + OpenSpool | link to the spool + OpenSpool JSON | NTAG215/216 | phones (opens the spool), Snapmaker U1 with community firmware, OpenSpool readers |
| OpenSpool | OpenSpool JSON | NTAG215/216 | as above, without the link |
| Link only | `http://…/#spool=S-012` | NTAG213+ | phones |
| Anycubic ACE | Anycubic layout (pages 4–31) | NTAG213/215 | ACE Pro / Kobra S1 |
| Bambu, Snapmaker stock | encrypted / signed | – | recognised by UID only |

## For developers

Build it yourself with [PlatformIO](https://platformio.org):

```sh
pio run                          # all boards
pio run -e esp32dev -t upload    # or -e d1_mini32, -e d1_mini (ESP8266)
pio device monitor
```

The factory images end up in `.pio/build/<env>/printdock-station-<env>.factory.bin`.

The station talks to PrintDock over a WebSocket with JSON messages, described in **[docs/protocol.md](docs/protocol.md)**. To try it without hardware, run `scripts/station-sim.js` from the [PrintDock repo](https://github.com/Codegulasch/printdock).

## License

MIT. The Anycubic tag layout follows [DnG-Crafts/ACE-RFID](https://github.com/DnG-Crafts/ACE-RFID), and OpenSpool follows [OpenSpool](https://github.com/RobThree/OpenSpool).
