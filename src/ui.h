// Anzeige (OLED), LED und Summer.
#pragma once
#include <Arduino.h>

namespace ui {
void begin();
/** Bis zu vier Zeilen; die erste fett. Gewicht und Status zeichnet ui::loop() unten dazu. */
void show(const String lines[], uint8_t n);
void setColor(uint32_t rgb);  // 0 = aus
void beep(const String &kind); // "ok", "warn", "error"
void setStatus(bool wifi, bool linked);
void setWeight(bool on, float grams, bool stable);
void blink();
void loop();
}  // namespace ui
