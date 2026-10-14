// NFC-Leser (PN532): Tags erkennen, rohe Seiten lesen und schreiben.
// Die Station deutet den Inhalt nicht – das macht PrintDock.
#pragma once
#include <Arduino.h>

struct TagInfo {
  bool present = false;
  uint8_t uid[10];
  uint8_t uidLen = 0;
  String uidHex;       // z. B. "04A1B2C3D4E580"
  String kind;         // "ntag" (NTAG21x / MIFARE Ultralight) oder "mifare_classic"
  uint16_t capacity = 0; // nutzbare Bytes laut Capability Container (nur ntag)
};

namespace nfc {
bool begin();
bool ok();

/** Hat sich der Zustand des Lesers geändert? Einmal abholen: 0 nein, 1 wieder da, -1 ausgefallen,
 *  2 hing und wurde neu eingerichtet. Die Station prüft alle paar Sekunden, ob der Leser antwortet. */
int8_t takeChange();
String firmware();

/** Einmal je Durchlauf aufrufen. Liefert true, wenn ein Tag neu aufgelegt oder entfernt wurde. */
bool poll(TagInfo &tag);

/** Seiten 0..n als Hex lesen (nur NTAG). Liest bis zum Ende der NDEF-Nachricht bzw. des ACE-Blocks. */
bool readPages(const TagInfo &tag, String &hexOut);

/** Hex-Daten ab Seite `start` schreiben. Prüft UID und Speichergrenzen. */
bool writePages(const TagInfo &tag, uint8_t start, const String &hex, String &error);
}  // namespace nfc
