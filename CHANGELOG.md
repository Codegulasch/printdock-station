# Changelog

## 0.3.0

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
