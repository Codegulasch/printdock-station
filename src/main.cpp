// PrintDock Station – NFC-Leser, Waage und Display für PrintDock.
// Protokoll: docs/protocol.md · Verdrahtung: docs/wiring.md
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HTTPUpdate.h>
#include <Preferences.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config.h"
#include "nfc.h"
#include "scale.h"
#include "ui.h"

static Preferences prefs;
static WebSocketsClient ws;
static TagInfo tag;
static String pdHost, token, deviceId;
static uint16_t pdPort = DEFAULT_PORT;
static bool linked = false;   // gekoppelt und von PrintDock begrüßt
static bool wsUp = false;     // WebSocket offen
static float lastSentG = -1e9;
static bool lastSentStable = false;
static uint32_t lastWeightSent = 0, lastNfcPoll = 0;

// ---------------------------------------------------------------- Hilfen

static void send(JsonDocument &doc) {
  if (!wsUp) return;
  String out;
  serializeJson(doc, out);
  ws.sendTXT(out);
}

static void showLines(std::initializer_list<String> l) {
  String a[4];
  uint8_t n = 0;
  for (auto &s : l) if (n < 4) a[n++] = s;
  ui::show(a, n);
}

static uint32_t parseColor(const char *hex) {
  if (!hex || hex[0] != '#' || strlen(hex) < 7) return 0;
  return strtoul(hex + 1, nullptr, 16);
}

/** "http://192.168.1.20:3080/" → Host und Port */
static void parseAddress(String a) {
  a.trim();
  a.replace("http://", "");
  a.replace("https://", "");
  a.replace("ws://", "");
  int slash = a.indexOf('/');
  if (slash >= 0) a = a.substring(0, slash);
  int colon = a.lastIndexOf(':');
  pdPort = DEFAULT_PORT;
  if (colon > 0) {
    pdPort = a.substring(colon + 1).toInt() ?: DEFAULT_PORT;
    a = a.substring(0, colon);
  }
  pdHost = a;
}

/** homeassistant.local per mDNS auflösen – der normale DNS kennt .local nicht */
static String resolveHost(const String &h) {
  if (!h.endsWith(".local")) return h;
  IPAddress ip = MDNS.queryHost(h.substring(0, h.length() - 6), 2000);
  return ip == IPAddress() ? h : ip.toString();
}

// ---------------------------------------------------------------- Meldungen an PrintDock

static void sendHello() {
  JsonDocument d;
  d["t"] = "hello";
  d["id"] = deviceId;
  d["name"] = "PrintDock Station " + deviceId.substring(deviceId.length() - 5);
  d["fw"] = FW_VERSION;
  d["hw"] = BOARD_NAME;
  JsonObject caps = d["caps"].to<JsonObject>();
  caps["nfc"] = nfc::ok();
  caps["scale"] = scale::ok();
  caps["display"] = true;
  caps["led"] = true;
  if (token.length()) d["token"] = token;
  send(d);
}

static void sendTag() {
  JsonDocument d;
  d["t"] = "tag";
  d["uid"] = tag.uidHex;
  d["kind"] = tag.kind;
  String pages;
  if (tag.kind == "ntag" && nfc::readPages(tag, pages)) d["pages"] = pages;
  send(d);
}

static void sendWeight(bool force = false) {
  if (!scale::ok()) return;
  float g = scale::grams();
  bool st = scale::stable();
  bool changed = fabsf(g - lastSentG) >= 1.0f || st != lastSentStable;
  if (!force && !(changed && millis() - lastWeightSent > 300) && millis() - lastWeightSent < WEIGHT_HEARTBEAT_MS) return;
  JsonDocument d;
  d["t"] = "weight";
  d["g"] = roundf(g * 10) / 10;
  d["stable"] = st;
  send(d);
  lastSentG = g;
  lastSentStable = st;
  lastWeightSent = millis();
}

static void sendButton(const char *press) {
  JsonDocument d;
  d["t"] = "button";
  d["press"] = press;
  send(d);
}

// ---------------------------------------------------------------- Befehle von PrintDock

static void onMessage(const char *payload, size_t len) {
  JsonDocument m;
  if (deserializeJson(m, payload, len)) return;
  const char *t = m["t"] | "";

  if (!strcmp(t, "pair")) {
    String code = m["code"] | "";
    showLines({"Koppeln", "Code " + code, "in PrintDock", "bestätigen"});
    ui::setColor(0x3B82F6);
  } else if (!strcmp(t, "welcome")) {
    String tk = m["token"] | "";
    if (tk.length() && tk != token) {
      token = tk;
      prefs.putString("token", token);
    }
    linked = true;
    if (tag.present) sendTag();
    sendWeight(true);
  } else if (!strcmp(t, "forget") || !strcmp(t, "pair_rejected")) {
    token = "";
    prefs.remove("token");
    linked = false;
    showLines({"Nicht gekoppelt", "in PrintDock", "neu koppeln"});
  } else if (!strcmp(t, "show")) {
    String a[4];
    uint8_t n = 0;
    for (JsonVariant v : m["lines"].as<JsonArray>()) if (n < 4) a[n++] = v.as<String>();
    ui::show(a, n);
    ui::setColor(parseColor(m["color"] | ""));
    const char *b = m["beep"] | "";
    if (*b) ui::beep(b);
  } else if (!strcmp(t, "write")) {
    JsonDocument r;
    r["t"] = "written";
    r["job"] = m["job"];
    String err;
    String uid = m["uid"] | "";
    if (!tag.present || uid != tag.uidHex) err = "tag removed";
    else nfc::writePages(tag, m["start"] | 4, m["data"] | "", err);
    r["ok"] = err.length() == 0;
    r["uid"] = tag.uidHex;
    if (err.length()) r["error"] = err;
    else {
      String pages;
      if (nfc::readPages(tag, pages)) r["pages"] = pages;  // zur Kontrolle neu lesen
    }
    send(r);
  } else if (!strcmp(t, "tare")) {
    scale::tare();
    showLines({"Tara", "Waage auf 0"});
    sendWeight(true);
  } else if (!strcmp(t, "calibrate")) {
    float g = m["grams"] | 0.0f;
    bool ok = scale::calibrate(g);
    JsonDocument r;
    r["t"] = "calibrated";
    r["ok"] = ok;
    r["grams"] = g;
    send(r);
    if (!ok) showLines({"Kalibrieren", "fehlgeschlagen", "Gewicht aufgelegt?"});
  } else if (!strcmp(t, "blink")) {
    ui::blink();
  } else if (!strcmp(t, "ota")) {
    // Firmware-Update von einer Adresse (z. B. PrintDock oder GitHub-Release)
    String url = m["url"] | "";
    if (url.startsWith("http://")) {
      showLines({"Update", "wird geladen …"});
      ui::loop();
      WiFiClient c;
      httpUpdate.rebootOnUpdate(true);
      t_httpUpdate_return r = httpUpdate.update(c, url);
      if (r != HTTP_UPDATE_OK) showLines({"Update", "fehlgeschlagen", httpUpdate.getLastErrorString().substring(0, 20)});
    }
  } else if (!strcmp(t, "reboot")) {
    ESP.restart();
  }
}

static void onWs(WStype_t type, uint8_t *payload, size_t len) {
  switch (type) {
    case WStype_CONNECTED:
      wsUp = true;
      sendHello();
      break;
    case WStype_DISCONNECTED:
      if (wsUp) showLines({"PrintDock", "nicht erreichbar", pdHost + ":" + String(pdPort)});
      wsUp = false;
      linked = false;
      break;
    case WStype_TEXT:
      onMessage((const char *)payload, len);
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------- Einrichtung

static void setupWifi(bool forcePortal) {
  WiFiManager wm;
  String saved = prefs.getString("address", "");
  WiFiManagerParameter addr("pd", "PrintDock-Adresse (z. B. 192.168.1.20:3080)", saved.c_str(), 80);
  wm.addParameter(&addr);
  wm.setTitle("PrintDock Station");
  wm.setConfigPortalTimeout(300);
  wm.setSaveParamsCallback([&]() { prefs.putString("address", addr.getValue()); });
  wm.setAPCallback([](WiFiManager *) { showLines({"Einrichten", "WLAN verbinden:", AP_NAME, "192.168.4.1"}); ui::loop(); });
  bool ok = forcePortal ? wm.startConfigPortal(AP_NAME) : wm.autoConnect(AP_NAME);
  if (strlen(addr.getValue())) prefs.putString("address", addr.getValue());
  if (!ok) ESP.restart();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  ui::begin();
  prefs.begin("pd", false);
  char idBuf[13];
  uint64_t mac = ESP.getEfuseMac();  // ab Werk eindeutig
  snprintf(idBuf, sizeof(idBuf), "%02X%02X%02X%02X%02X%02X", uint8_t(mac), uint8_t(mac >> 8), uint8_t(mac >> 16), uint8_t(mac >> 24), uint8_t(mac >> 32), uint8_t(mac >> 40));
  deviceId = idBuf;

  // Taste beim Einschalten 5 s halten → WLAN und Adresse neu einrichten, Kopplung vergessen
  bool reset = false;
  if (digitalRead(PIN_BUTTON) == LOW) {
    showLines({"Taste halten …", "zum Zurücksetzen"});
    ui::loop();
    uint32_t t0 = millis();
    while (digitalRead(PIN_BUTTON) == LOW && millis() - t0 < 5000) delay(20);
    reset = millis() - t0 >= 5000;
    if (reset) {
      prefs.remove("token");
      WiFiManager().resetSettings();
    }
  }

  if (!nfc::begin()) showLines({"NFC-Leser", "nicht gefunden", "SPI-Modus prüfen"});
  scale::begin();
  ui::loop();
  delay(nfc::ok() ? 300 : 1500);

  setupWifi(reset || prefs.getString("address", "").isEmpty());
  WiFi.setSleep(false);
  MDNS.begin(("printdock-station-" + deviceId.substring(6)).c_str());
  parseAddress(prefs.getString("address", ""));
  token = prefs.getString("token", "");
  showLines({"Verbinde …", pdHost + ":" + String(pdPort)});

  ws.begin(resolveHost(pdHost), pdPort, WS_PATH);
  ws.onEvent(onWs);
  ws.setReconnectInterval(4000);
  ws.enableHeartbeat(15000, 4000, 2);
}

// ---------------------------------------------------------------- Hauptschleife

static void handleButton() {
  static bool down = false, longSent = false;
  static uint32_t since = 0;
  bool pressed = digitalRead(PIN_BUTTON) == LOW;
  if (pressed && !down) {
    down = true;
    longSent = false;
    since = millis();
  } else if (pressed && down && !longSent && millis() - since > 1500) {
    longSent = true;
    scale::tare();  // lang drücken = Tara (auch ohne Verbindung)
    showLines({"Tara", "Waage auf 0"});
    ui::beep("ok");
    sendButton("long");
    sendWeight(true);
  } else if (!pressed && down) {
    down = false;
    if (!longSent && millis() - since > 40) sendButton("short");
  }
}

void loop() {
  ws.loop();
  handleButton();

  if (millis() - lastNfcPoll > NFC_POLL_MS) {
    lastNfcPoll = millis();
    if (nfc::poll(tag)) {
      if (tag.present) {
        ui::beep("ok");
        showLines({"Tag erkannt", tag.uidHex});
        if (linked) sendTag();
      } else if (linked) {
        JsonDocument d;
        d["t"] = "tag_gone";
        send(d);
      }
    }
  }

  if (scale::update()) {
    ui::setWeight(true, scale::grams(), scale::stable());
    if (linked) sendWeight();
  }

  ui::setStatus(WiFi.status() == WL_CONNECTED, linked);
  ui::loop();
}
