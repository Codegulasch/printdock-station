#include "scale.h"

#include <HX711.h>
#include <Preferences.h>

#include "config.h"

static HX711 hx;
static Preferences prefs;
static bool present = false;
static long offset = 0;
static float factor = 420.0f;  // Rohwert je Gramm – Startwert für eine typische 5-kg-Zelle
static float window[SCALE_WINDOW];
static uint8_t filled = 0, pos = 0;
static uint32_t lastSample = 0;
static float current = 0;
static bool isStable = false;

static long rawAverage(uint8_t n) {
  long sum = 0;
  uint8_t got = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (hx.wait_ready_timeout(200)) {
      sum += hx.read();
      got++;
    }
  }
  return got ? sum / got : 0;
}

namespace scale {

void begin() {
  hx.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
  present = hx.wait_ready_timeout(1000);
  prefs.begin("scale", false);
  offset = prefs.getLong("offset", 0);
  factor = prefs.getFloat("factor", factor);
  if (present && !prefs.isKey("offset")) tare();
}

bool ok() { return present; }

bool update() {
  if (!present || millis() - lastSample < SCALE_SAMPLE_MS) return false;
  if (!hx.is_ready()) return false;
  lastSample = millis();
  float g = (hx.read() - offset) / factor;
  window[pos] = g;
  pos = (pos + 1) % SCALE_WINDOW;
  if (filled < SCALE_WINDOW) filled++;
  float sum = 0, lo = 1e9, hi = -1e9;
  for (uint8_t i = 0; i < filled; i++) {
    sum += window[i];
    lo = min(lo, window[i]);
    hi = max(hi, window[i]);
  }
  current = sum / filled;
  if (fabsf(current) < 0.5f) current = 0;
  isStable = filled == SCALE_WINDOW && (hi - lo) < SCALE_STABLE_G;
  return true;
}

float grams() { return current; }
bool stable() { return isStable; }

void tare() {
  if (!present) return;
  offset = rawAverage(15);
  prefs.putLong("offset", offset);
  filled = 0;
}

bool calibrate(float knownGrams) {
  if (!present || knownGrams <= 0) return false;
  long raw = rawAverage(20);
  float f = (raw - offset) / knownGrams;
  if (fabsf(f) < 1.0f) return false;  // nichts aufgelegt?
  factor = f;
  prefs.putFloat("factor", factor);
  filled = 0;
  return true;
}

}  // namespace scale
