# Changelog

## 0.1.0

- Erste Version: PN532 (SPI) liest NTAG21x/MIFARE Ultralight inkl. Seiten und erkennt MIFARE Classic an der UID, schreibt Seiten im Auftrag von PrintDock und prüft sie nach dem Schreiben.
- Waage mit HX711: gleitender Mittelwert, „ruhig“-Erkennung, Tara und Kalibrierung (im Flash gespeichert).
- OLED-Anzeige (vier Zeilen von PrintDock, Gewicht und Verbindungsstatus), WS2812-LED in Spulenfarbe, Summer.
- WLAN-Einrichtung über eigenes WLAN „PrintDock-Station“, PrintDock-Adresse inkl. `.local`-Namen (mDNS).
- Kopplung per sechsstelligem Code, WebSocket-Verbindung zu PrintDock mit automatischem Wiederverbinden.
- Firmware-Update über PrintDock vorbereitet (`ota`).
- Boards: ESP32-WROOM DevKit, D1 Mini ESP32.
