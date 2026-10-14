# Changelog

## 0.3.1

- **NFC-Leser überwacht:** Die Station prüft alle 3 Sekunden, ob der PN532 noch antwortet. Hängt er (Wackelkontakt, Störung), richtet sie ihn selbst neu ein – vorher half nur Strom aus. Ist er ganz weg, zeigt das Display „NFC-Leser antwortet nicht – Kabel prüfen“ und es wird alle 2 Sekunden neu versucht. PrintDock bekommt den Ausfall ins Protokoll.
- Fix: Wurde der Leser erst nach dem Start gefunden (später eingesteckt, Wackler beim Einschalten), meldete die Station das PrintDock nicht – dort stand weiter „ohne NFC-Leser“.
- I²C-Bus wird beim Start freigegeben, falls ein Teilnehmer ihn blockiert (D1 Mini ESP8266: PN532 und Display am selben Bus).
- Doku: Verdrahtungspläne für ESP32 (38 Pin) und D1 Mini – je Schaltplan und Steckbrett, als Bilder in `docs/wiring.md` und zusammen auf einer Seite (`docs/wiring.html`). Fehlersuche NFC-Leser, Hinweis zu G23 auf ESP32-Boards mit 38 Pins.

## 0.3.0

- Gehäuse zum Drucken (`case/`): Unterteil, Deckel mit PN532-Schienen, Teller – für D1 Mini und ESP32, ohne Stützen, nur die 4 Schrauben der Wägezelle.
- Doku: WLAN-Einrichtung Schritt für Schritt (http://192.168.4.1, *Configure WiFi*, Fehlerhilfe).
- Neues Board: **D1 Mini mit ESP8266**. PN532, OLED und Feuchtesensor teilen sich dort den I²C-Bus (PN532 im I²C-Modus), Belegung in docs/wiring.md. Zurücksetzen per Taste an D3 nach dem Einschalten (D3 ist der Flash-Pin).

## 0.2.0

- Feuchtesensor (optional) am I²C-Bus: SHT4x, SHT3x, AHT10/AHT20, HTU21D/SHT21/Si7021 werden erkannt. Die Station meldet Feuchte und Temperatur alle 30 s an PrintDock (`env`) und zeigt sie an. Ohne NFC-Leser und Waage arbeitet sie als reiner Feuchtemesser, z. B. in der Trockenbox.

## 0.1.0

- Erste Version: PN532 (SPI) liest NTAG21x/MIFARE Ultralight inkl. Seiten und erkennt MIFARE Classic an der UID, schreibt Seiten im Auftrag von PrintDock und prüft sie nach dem Schreiben.
- Waage mit HX711: gleitender Mittelwert, „ruhig“-Erkennung, Tara und Kalibrierung (im Flash gespeichert).
- OLED-Anzeige (vier Zeilen von PrintDock, Gewicht und Verbindungsstatus), WS2812-LED in Spulenfarbe, Summer.
- WLAN-Einrichtung über eigenes WLAN „PrintDock-Station“, PrintDock-Adresse inkl. `.local`-Namen (mDNS).
- Kopplung per sechsstelligem Code, WebSocket-Verbindung zu PrintDock mit automatischem Wiederverbinden.
- Firmware-Update über PrintDock vorbereitet (`ota`).
- Boards: ESP32-WROOM DevKit, D1 Mini ESP32.
