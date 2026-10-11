// Pinbelegung und Grundeinstellungen.
// Gleiche GPIOs auf ESP32-WROOM DevKit und D1 Mini ESP32 – siehe docs/wiring.md.
#pragma once

#ifndef FW_VERSION
#define FW_VERSION "dev"
#endif
#ifndef BOARD_NAME
#if defined(ARDUINO_D1_MINI32)
#define BOARD_NAME "D1 Mini ESP32"
#elif defined(ARDUINO_ESP32_DEV)
#define BOARD_NAME "ESP32-WROOM DevKit"
#else
#define BOARD_NAME "ESP32"
#endif
#endif

// PN532 über SPI (am Modul: SPI-Modus einstellen)
#define PIN_PN532_SCK 18
#define PIN_PN532_MISO 19
#define PIN_PN532_MOSI 23
#define PIN_PN532_SS 5

// HX711 (Wägezelle)
#define PIN_HX711_DOUT 32
#define PIN_HX711_SCK 33

// OLED SSD1306 128×64 über I²C
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// WS2812-LED (eine LED oder ein kurzer Ring)
#define PIN_LED 4
#define LED_COUNT 1

// Taster gegen GND (kurz: Spule erneut melden, lang: Tara, beim Einschalten 5 s halten: WLAN zurücksetzen)
#define PIN_BUTTON 27

// Passiver Summer (optional)
#define PIN_BUZZER 25

// Allgemein
#define AP_NAME "PrintDock-Station"
#define DEFAULT_PORT 3080
#define WS_PATH "/api/station/ws"
#define NFC_POLL_MS 150      // so oft nach einem Tag fragen
#define NFC_GONE_MISSES 4    // so viele Fehlversuche, bis der Tag als entfernt gilt
#define SCALE_SAMPLE_MS 100  // Messabstand der Waage
#define SCALE_WINDOW 8       // gleitender Mittelwert über so viele Messungen
#define SCALE_STABLE_G 1.5f  // Schwankung, unter der das Gewicht als ruhig gilt
#define WEIGHT_HEARTBEAT_MS 5000
