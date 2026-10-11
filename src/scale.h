// Waage (HX711 + Wägezelle). Kalibrierung liegt im Flash (Preferences).
#pragma once
#include <Arduino.h>

namespace scale {
void begin();
bool ok();
/** Regelmäßig aufrufen; liefert true, wenn ein neuer Messwert vorliegt. */
bool update();
float grams();
bool stable();
void tare();
/** Mit bekanntem Gewicht kalibrieren (vorher tarieren). */
bool calibrate(float knownGrams);
}  // namespace scale
