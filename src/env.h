// Feuchte- und Temperatursensor (optional) am I²C-Bus des Displays.
// Erkannt werden SHT4x (0x44), SHT3x (0x44/0x45), AHT10/AHT20 (0x38) und HTU21D/SHT21/Si7021 (0x40).
#pragma once
#include <Arduino.h>

namespace env {
/** Nach Wire.begin() aufrufen – sucht den Sensor. */
bool begin();
bool ok();
const char *sensor();  // z. B. "SHT31"
/** Misst neu (dauert bis ~80 ms). Liefert false bei Lesefehler. */
bool read(float &humidity, float &temperature);
}  // namespace env
