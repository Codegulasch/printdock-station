#include "env.h"

#include <Wire.h>

enum Kind { NONE, SHT4X, SHT3X, AHTX0, HTU21 };
static Kind kind = NONE;
static uint8_t addr = 0;

// Sensirion-CRC (SHT3x/SHT4x, AHT20): Polynom 0x31, Start 0xFF
static uint8_t crc8(const uint8_t *d, uint8_t n, uint8_t init = 0xFF) {
  uint8_t c = init;
  for (uint8_t i = 0; i < n; i++) {
    c ^= d[i];
    for (uint8_t b = 0; b < 8; b++) c = c & 0x80 ? (c << 1) ^ 0x31 : c << 1;
  }
  return c;
}

static bool present(uint8_t a) {
  Wire.beginTransmission(a);
  return Wire.endTransmission() == 0;
}

static bool cmd(uint8_t a, const uint8_t *bytes, uint8_t n) {
  Wire.beginTransmission(a);
  Wire.write(bytes, n);
  return Wire.endTransmission() == 0;
}

static bool readN(uint8_t a, uint8_t *buf, uint8_t n) {
  if (Wire.requestFrom(a, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

static bool readSht4x(uint8_t a, float &h, float &t) {
  const uint8_t c[] = {0xFD};  // hohe Genauigkeit
  uint8_t b[6];
  if (!cmd(a, c, 1)) return false;
  delay(10);
  if (!readN(a, b, 6) || crc8(b, 2) != b[2] || crc8(b + 3, 2) != b[5]) return false;
  t = -45 + 175.0f * (b[0] << 8 | b[1]) / 65535.0f;
  h = constrain(-6 + 125.0f * (b[3] << 8 | b[4]) / 65535.0f, 0.0f, 100.0f);
  return true;
}

static bool readSht3x(uint8_t a, float &h, float &t) {
  const uint8_t c[] = {0x24, 0x00};  // Einzelmessung, hohe Wiederholgenauigkeit
  uint8_t b[6];
  if (!cmd(a, c, 2)) return false;
  delay(20);
  if (!readN(a, b, 6) || crc8(b, 2) != b[2] || crc8(b + 3, 2) != b[5]) return false;
  t = -45 + 175.0f * (b[0] << 8 | b[1]) / 65535.0f;
  h = constrain(100.0f * (b[3] << 8 | b[4]) / 65535.0f, 0.0f, 100.0f);
  return true;
}

static bool readAht(float &h, float &t) {
  const uint8_t c[] = {0xAC, 0x33, 0x00};
  uint8_t b[7];
  if (!cmd(0x38, c, 3)) return false;
  delay(80);
  if (!readN(0x38, b, 7) || (b[0] & 0x80)) return false;  // noch beschäftigt
  uint32_t rh = (uint32_t(b[1]) << 12) | (uint32_t(b[2]) << 4) | (b[3] >> 4);
  uint32_t tr = (uint32_t(b[3] & 0x0F) << 16) | (uint32_t(b[4]) << 8) | b[5];
  h = rh * 100.0f / 1048576.0f;
  t = tr * 200.0f / 1048576.0f - 50;
  return h >= 0 && h <= 100;
}

static bool readHtu(float &h, float &t) {
  uint8_t b[3];
  const uint8_t ch[] = {0xF5}, ct[] = {0xF3};  // ohne Clock-Stretching
  if (!cmd(0x40, ch, 1)) return false;
  delay(20);
  if (!readN(0x40, b, 3)) return false;
  uint16_t rh = (b[0] << 8 | b[1]) & ~0x3;
  if (!cmd(0x40, ct, 1)) return false;
  delay(55);
  if (!readN(0x40, b, 3)) return false;
  uint16_t tr = (b[0] << 8 | b[1]) & ~0x3;
  h = constrain(-6 + 125.0f * rh / 65536.0f, 0.0f, 100.0f);
  t = -46.85f + 175.72f * tr / 65536.0f;
  return true;
}

namespace env {

bool begin() {
  float h, t;
  for (uint8_t a : {0x44, 0x45}) {
    if (!present(a)) continue;
    if (readSht4x(a, h, t)) { kind = SHT4X; addr = a; return true; }
    delay(5);
    if (readSht3x(a, h, t)) { kind = SHT3X; addr = a; return true; }
  }
  if (present(0x38)) {
    // AHT20 initialisieren (AHT10 ignoriert das)
    const uint8_t init[] = {0xBE, 0x08, 0x00};
    cmd(0x38, init, 3);
    delay(20);
    if (readAht(h, t)) { kind = AHTX0; addr = 0x38; return true; }
  }
  if (present(0x40) && readHtu(h, t)) { kind = HTU21; addr = 0x40; return true; }
  kind = NONE;
  return false;
}

bool ok() { return kind != NONE; }

const char *sensor() {
  switch (kind) {
    case SHT4X: return "SHT4x";
    case SHT3X: return "SHT3x";
    case AHTX0: return "AHT20";
    case HTU21: return "HTU21D";
    default: return "";
  }
}

bool read(float &h, float &t) {
  switch (kind) {
    case SHT4X: return readSht4x(addr, h, t);
    case SHT3X: return readSht3x(addr, h, t);
    case AHTX0: return readAht(h, t);
    case HTU21: return readHtu(h, t);
    default: return false;
  }
}

}  // namespace env
