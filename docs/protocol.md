# PrintDock Station protocol

The station talks to PrintDock over a single WebSocket connection:

```
ws://<printdock-host>:<port>/api/station/ws
```

The default port is 3080. As a Home Assistant add-on, use the add-on's direct port, not the ingress URL. All messages are JSON text frames with a `t` field for the type. The station is deliberately thin: it reports raw tag pages, weight and button presses, and shows whatever PrintDock sends. Tag formats, spool lookup, tare and slot assignment all live in PrintDock (`lib/tags.js`, `lib/stations.js`).

## Pairing

1. The station connects and sends `hello`. A paired station includes its `token`.
2. If the token is unknown or missing, PrintDock replies with `pair {code}`. The station shows the six-digit code, and the same code appears in PrintDock under *Settings → PrintDock Station*.
3. When the user clicks *Pair*, PrintDock sends `welcome {id, name, token}`. The station stores the token in flash.
4. When a station is removed in PrintDock, it gets `forget` and drops its token.

## Station → PrintDock

| `t` | Fields | When |
|---|---|---|
| `hello` | `id` (efuse MAC, hex), `name`, `fw`, `hw`, `caps {nfc, scale, display, led, humidity}`, `token?` | after connecting |
| `tag` | `uid` (hex), `kind` (`ntag` \| `mifare_classic`), `pages?` (hex, from page 0) | tag placed (once per tag) |
| `tag_gone` | – | tag removed |
| `weight` | `g` (number), `stable` (bool) | on change (at most every 300 ms) and every 5 s |
| `button` | `press` (`short` \| `long`) | button; long press also tares locally |
| `written` | `job`, `ok`, `uid`, `error?`, `pages?` (read back after writing) | after a `write` |
| `calibrated` | `ok`, `grams` | after `calibrate` |
| `env` | `h` (% RH), `temp?` (°C), `sensor` | every 30 s if a humidity sensor is fitted (`caps.humidity`) |
| `log` | `msg` | optional diagnostics |

`pages` covers pages 0–3 (UID, lock bits, Capability Container) and as much user memory as needed. That means up to the end of the NDEF TLV, or pages 4–31 for an Anycubic ACE block.

## PrintDock → station

| `t` | Fields | Meaning |
|---|---|---|
| `pair` | `code` | show the pairing code |
| `welcome` | `id`, `name`, `token` | paired / reconnected |
| `pair_rejected`, `forget` | – | drop the token |
| `show` | `lines` (up to 4 strings, UTF-8), `color` (`#RRGGBB` \| null), `beep` (`ok` \| `warn` \| `error` \| null) | display text, LED colour, sound |
| `write` | `job`, `uid`, `start` (page), `data` (hex) | write pages to the tag with that UID; reply `written` |
| `tare` | – | zero the scale |
| `calibrate` | `grams` | set the scale factor from a known weight (tare first) |
| `blink` | – | flash LED (identify) |
| `ota` | `url` (`http://…` firmware .bin) | firmware update |
| `reboot` | – | restart |

The firmware refuses to write below page 4 or past the user memory that the Capability Container announces. Lock and config pages are never touched.

## Typical flow

```
station                         PrintDock
  hello ───────────────────────▶
        ◀─────────────────────── welcome
        ◀─────────────────────── show ["Regal","Spule auflegen"]
  tag {uid,pages} ─────────────▶  decode → spool S-021, arm for loading
        ◀─────────────────────── show ["S-021 PETG","Blau · Sunlu","685 g von 1000 g","jetzt einlegen"]
  weight {g:900,stable} ───────▶  remaining = 900 − tare
        ◀─────────────────────── show [... "Rest 685 g" ...]
  tag_gone ────────────────────▶
                                  printer reports a spool in AMS 1 · 3 → S-021 goes there,
                                  material/colour are sent to the AMS (Bambu)
        ◀─────────────────────── show ["S-021 eingelegt","Bambu A1","AMS lite · 3"]
```
