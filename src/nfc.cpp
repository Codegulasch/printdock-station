#include "nfc.h"

#include <Adafruit_PN532.h>
#include <SPI.h>
#include <Wire.h>

#include "config.h"

#if defined(PN532_I2C)
static Adafruit_PN532 pn532(255, 255, &Wire);  // I²C ohne IRQ- und Reset-Leitung
#else
static Adafruit_PN532 pn532(PIN_PN532_SS, &SPI);  // Hardware-SPI (VSPI)
#endif
static bool ready = false;
static uint32_t fwVersion = 0;
static uint8_t misses = 0;
static uint32_t lastInit = 0;
// Überwachung: Hängt der Leser (Wackelkontakt, Störung), richtet die Station ihn selbst neu ein
static bool everOk = false;      // seit dem Start schon einmal geantwortet
static uint8_t fails = 0;        // Lebenszeichen in Folge ausgeblieben
static uint32_t lastCheck = 0;   // letztes Lebenszeichen (Antwort oder gelesener Tag)
static int8_t change = 0;        // für takeChange(): +1 wieder da, -1 weg, 2 neu gestartet

static String toHex(const uint8_t *b, size_t n) {
  static const char *H = "0123456789ABCDEF";
  String s;
  s.reserve(n * 2);
  for (size_t i = 0; i < n; i++) {
    s += H[b[i] >> 4];
    s += H[b[i] & 15];
  }
  return s;
}

static int hexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

namespace nfc {

// Einmal einrichten: Bus, Aufwecken, Firmware abfragen, SAM-Konfiguration
static bool init() {
#if defined(PN532_I2C)
  Wire.setClockStretchLimit(150000);  // PN532 hält den Takt beim Antworten an
#else
  SPI.begin(PIN_PN532_SCK, PIN_PN532_MISO, PIN_PN532_MOSI, PIN_PN532_SS);
#endif
  pn532.begin();
  fwVersion = pn532.getFirmwareVersion();
  if (!fwVersion) {
    ready = false;
    return false;
  }
  pn532.SAMConfig();
  pn532.setPassiveActivationRetries(0x01);  // nicht blockieren, wenn kein Tag da ist
  ready = true;
  everOk = true;
  fails = 0;
  lastCheck = millis();
  return true;
}

bool begin() {
  lastInit = millis();
  return init();
}

int8_t takeChange() {
  int8_t c = change;
  change = 0;
  return c;
}

bool ok() { return ready; }

String firmware() {
  if (!fwVersion) return "";
  return String((fwVersion >> 16) & 0xFF, HEX) + "." + String((fwVersion >> 8) & 0xFF);
}

/** 16 Byte (4 Seiten) ab `page` lesen – ein READ-Befehl statt vier. */
static bool read4(uint8_t page, uint8_t *out16) {
  uint8_t cmd[2] = {0x30, page};
  uint8_t resp[32];
  uint8_t len = sizeof(resp);
  if (!pn532.inDataExchange(cmd, 2, resp, &len) || len < 16) return false;
  memcpy(out16, resp, 16);
  return true;
}

bool poll(TagInfo &tag) {
  if (!ready) {
    // Leser nicht da: alle 10 s neu versuchen, nach einem Ausfall alle 2 s
    if (millis() - lastInit > (everOk ? 2000UL : 10000UL)) {
      lastInit = millis();
      if (init()) change = 1;
    }
    if (tag.present) {
      tag = TagInfo();
      return true;
    }
    return false;
  }
  uint8_t uid[10];
  uint8_t len = 0;
  bool found = pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &len, 60);
  if (!found) {
    if (tag.present && ++misses >= NFC_GONE_MISSES) {
      tag = TagInfo();
      misses = 0;
      return true;
    }
    // Lebenszeichen: antwortet der Leser noch? Zweimal nichts → neu einrichten
    if (millis() - lastCheck > NFC_CHECK_MS) {
      lastCheck = millis();
      if (pn532.getFirmwareVersion()) {
        fails = 0;
      } else if (++fails >= 2) {
        lastInit = millis();
        if (init()) {
          change = 2;  // hing, läuft wieder
        } else {
          ready = false;
          change = -1;
        }
        if (tag.present) {
          tag = TagInfo();
          return true;
        }
      }
    }
    return false;
  }
  misses = 0;
  fails = 0;
  lastCheck = millis();
  if (tag.present && len == tag.uidLen && memcmp(uid, tag.uid, len) == 0) return false;
  tag = TagInfo();
  tag.present = true;
  tag.uidLen = len;
  memcpy(tag.uid, uid, len);
  tag.uidHex = toHex(uid, len);
  tag.kind = len == 7 ? "ntag" : "mifare_classic";
  if (len == 7) {
    uint8_t buf[16];
    // Seite 3 = Capability Container: E1 10 <Größe/8> 00
    if (read4(0, buf) && buf[12] == 0xE1) tag.capacity = buf[14] * 8;
  }
  return true;
}

bool readPages(const TagInfo &tag, String &hexOut) {
  if (tag.kind != "ntag") return false;
  // Höchstens den Nutzspeicher (NTAG216: 888 Byte) – meist reicht viel weniger
  const uint16_t userBytes = tag.capacity ? tag.capacity : 144;
  const uint16_t lastPage = 3 + userBytes / 4;
  uint8_t mem[16 + 888];
  uint16_t have = 0;  // gelesene Bytes ab Seite 0
  uint16_t want = 16 + 16;
  for (uint16_t page = 0; page <= lastPage && have < want; page += 4) {
    uint8_t buf[16];
    if (!read4(page, buf)) return false;
    memcpy(mem + have, buf, 16);
    have += 16;
    if (have >= 20) {
      const uint8_t *u = mem + 16;  // ab Seite 4
      if (u[0] == 0x7B) {
        want = 16 + 28 * 4;  // Anycubic-ACE-Block: Seiten 4..31
      } else if (u[0] == 0x03) {
        // NDEF-TLV: 03 <len> oder 03 FF <hi> <lo>
        uint16_t ndefLen = u[1] == 0xFF ? (uint16_t(u[2]) << 8 | u[3]) + 4 : u[1] + 2;
        want = 16 + ndefLen + 1;
      } else {
        want = 16 + 16;  // leer oder unbekannt: nur der Anfang
      }
      if (want > 16 + userBytes) want = 16 + userBytes;
    }
  }
  hexOut = toHex(mem, have);
  return true;
}

bool writePages(const TagInfo &tag, uint8_t start, const String &hex, String &error) {
  if (tag.kind != "ntag") {
    error = "not writable";
    return false;
  }
  // Tag erneut auswählen und prüfen, dass es noch derselbe ist
  uint8_t uid[10];
  uint8_t len = 0;
  if (!pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &len, 200) || len != tag.uidLen || memcmp(uid, tag.uid, len) != 0) {
    error = "tag removed";
    return false;
  }
  const size_t bytes = hex.length() / 2;
  const uint16_t pages = (bytes + 3) / 4;
  const uint16_t userBytes = tag.capacity ? tag.capacity : 144;
  if (start < 4 || start + pages > 4 + userBytes / 4) {
    error = "out of range";
    return false;
  }
  for (uint16_t p = 0; p < pages; p++) {
    uint8_t data[4] = {0, 0, 0, 0};
    for (uint8_t i = 0; i < 4; i++) {
      size_t at = (p * 4 + i) * 2;
      if (at + 1 < hex.length()) {
        int hi = hexVal(hex[at]);
        int lo = hexVal(hex[at + 1]);
        if (hi < 0 || lo < 0) {
          error = "bad data";
          return false;
        }
        data[i] = hi << 4 | lo;
      }
    }
    if (!pn532.ntag2xx_WritePage(start + p, data)) {
      error = "write failed at page " + String(start + p);
      return false;
    }
  }
  return true;
}

}  // namespace nfc
