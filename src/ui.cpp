#include "ui.h"

#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "config.h"

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
static Adafruit_NeoPixel led(LED_COUNT, PIN_LED, NEO_GRB + NEO_KHZ800);
static String text[4];
static uint8_t count = 0;
static bool dirty = true, wifiOk = false, linkOk = false, weightOn = false, weightStable = false;
static float weight = 0, humidity = NAN;
static bool envOn = false;
static uint32_t color = 0, blinkUntil = 0, lastDraw = 0;

static void buzz(uint16_t hz, uint16_t ms) {
#if defined(PIN_BUZZER) && defined(ESP8266)
  tone(PIN_BUZZER, hz, ms);
  delay(ms);
#elif defined(PIN_BUZZER)
  ledcAttachPin(PIN_BUZZER, 7);
  ledcWriteTone(7, hz);
  delay(ms);
  ledcWriteTone(7, 0);
#endif
}

static void applyLed() {
  uint32_t c = color;
  if (blinkUntil && millis() < blinkUntil) c = (millis() / 200) % 2 ? 0xF59E0B : 0;
  // gedimmt, damit die LED nicht blendet
  led.setPixelColor(0, led.Color(((c >> 16) & 255) / 4, ((c >> 8) & 255) / 4, (c & 255) / 4));
  led.show();
}

namespace ui {

void begin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  oled.begin();
  oled.enableUTF8Print();
  led.begin();
  led.clear();
  led.show();
  const String hello[] = {"PrintDock", "Station " FW_VERSION, BOARD_NAME};
  show(hello, 3);
  loop();
}

void show(const String lines[], uint8_t n) {
  count = min<uint8_t>(n, 4);
  for (uint8_t i = 0; i < 4; i++) text[i] = i < count ? lines[i] : "";
  dirty = true;
}

void setColor(uint32_t rgb) {
  color = rgb;
  applyLed();
}

void beep(const String &kind) {
  if (kind == "ok") buzz(2200, 70);
  else if (kind == "warn") { buzz(1400, 90); delay(60); buzz(1400, 90); }
  else if (kind == "error") buzz(400, 350);
}

void setStatus(bool wifi, bool linked) {
  if (wifi != wifiOk || linked != linkOk) dirty = true;
  wifiOk = wifi;
  linkOk = linked;
}

void setWeight(bool on, float grams, bool stable) {
  if (on != weightOn || fabsf(grams - weight) >= 0.5f || stable != weightStable) dirty = true;
  weightOn = on;
  weight = grams;
  weightStable = stable;
}

void setEnv(bool on, float h) {
  if (on != envOn || fabsf(h - humidity) >= 0.5f) dirty = true;
  envOn = on;
  humidity = h;
}

void blink() {
  blinkUntil = millis() + 3000;
}

void loop() {
  if (blinkUntil) {
    applyLed();
    if (millis() > blinkUntil) {
      blinkUntil = 0;
      applyLed();
    }
  }
  if (!dirty || millis() - lastDraw < 80) return;
  dirty = false;
  lastDraw = millis();
  oled.clearBuffer();
  oled.setFont(u8g2_font_7x14B_tf);
  oled.drawUTF8(0, 12, text[0].c_str());
  oled.setFont(u8g2_font_6x12_tf);
  for (uint8_t i = 1; i < 4; i++) oled.drawUTF8(0, 12 + i * 12, text[i].c_str());
  // Fußzeile: Gewicht links, Verbindung rechts
  oled.drawHLine(0, 51, 128);
  if (weightOn) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%s%.0f g", weightStable ? "" : "~", weight);
    oled.setFont(u8g2_font_7x14B_tf);
    oled.drawUTF8(0, 64, buf);
  }
  char hb[12];
  if (envOn && !isnan(humidity)) snprintf(hb, sizeof(hb), "%.0f%% rF", humidity);
  if (envOn && !weightOn) {
    // nur Feuchtesensor: Messwert groß
    oled.setFont(u8g2_font_7x14B_tf);
    oled.drawUTF8(0, 64, hb);
  } else if (envOn) {
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawUTF8(56, 62, hb);
  }
  oled.setFont(u8g2_font_5x8_tf);
  oled.drawUTF8(92, 62, linkOk ? "online" : wifiOk ? "WLAN" : "offline");
  oled.sendBuffer();
}

}  // namespace ui
