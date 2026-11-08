/*
  Free Games ESP32 firmware  v1.2
  Made by Kamil Hassan

  - Open Wi-Fi "Free Games" (no password, on purpose)
  - Captive portal: phones open the games page automatically
  - Up to MAX_CLIENTS connected devices, live count at /api/status
  - Friendly error pages (404, missing games storage)
  - Tic Tac Toe rooms so two phones on this Wi-Fi can play each other (/api/ttt/...)
  Games are served from SPIFFS (upload the "data" folder).
  Works on ESP32, ESP32-S3, and ESP32-C3 (Arduino-ESP32).
*/
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <esp_system.h>
#include "RoomData.h"

#define AP_SSID      "Free Games"
#define AP_CHANNEL   6
#define MAX_CLIENTS  8      // ESP32 allows up to 10; 8 keeps it smooth
#define MAX_ROOMS    4      // 4 rooms = 8 players
#define SEEN_TIMEOUT 20000  // ms without a request = player left
#define ROOM_TIMEOUT 60000  // ms without the creator = room closed

WebServer server(80);
DNSServer dnsServer;
const IPAddress apIP(192, 168, 4, 1);
const char* HOME_URL = "http://192.168.4.1/";
bool fsOk = false;

// ---------------------------------------------------------------- helpers
String contentTypeFor(const String& p) {
  if (p.endsWith(".html") || p.endsWith(".htm")) return "text/html";
  if (p.endsWith(".css")) return "text/css";
  if (p.endsWith(".js")) return "application/javascript";
  if (p.endsWith(".json")) return "application/json";
  if (p.endsWith(".png")) return "image/png";
  if (p.endsWith(".jpg") || p.endsWith(".jpeg")) return "image/jpeg";
  if (p.endsWith(".svg")) return "image/svg+xml";
  if (p.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

void sendJson(const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", body);
}

void redirectHome() {
  server.sendHeader("Location", HOME_URL, true);
  server.sendHeader("Cache-Control", "no-store");
  server.send(302, "text/plain", "");
}

bool isApHost() {
  String h = server.hostHeader();
  return h.length() == 0 || h == "192.168.4.1" || h.startsWith("192.168.4.1:");
}

// A friendly page instead of a bare error message
void sendErrorPage(int code, const String& title, const String& message) {
  String html = F("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<title>Free Games</title><style>"
                  "body{margin:0;min-height:100vh;display:grid;place-items:center;padding:1.5rem;"
                  "background:#14152e;color:#eef0ff;font:16px/1.6 system-ui,sans-serif;text-align:center}"
                  ".c{max-width:420px}.e{font-size:3.5rem}h1{margin:.2em 0}p{color:#a2a6d0}"
                  "a{display:inline-block;margin-top:1rem;padding:.8rem 1.8rem;border-radius:999px;"
                  "background:#ffc93c;color:#1a1b3a;font-weight:700;text-decoration:none}"
                  "</style></head><body><div class='c'><div class='e'>&#127918;</div><h1>");
  html += title;
  html += F("</h1><p>");
  html += message;
  html += F("</p><a href='/'>Back to the games</a></div></body></html>");
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "text/html", html);
}

// ---------------------------------------------------------------- static files + captive portal
void handleRequest() {
  // Any address that is not ours (connectivity checks, typed websites) -> games page
  if (!isApHost()) { redirectHome(); return; }

  String path = server.uri();
  if (path == "/") path = "/index.html";
  if (path.indexOf("..") >= 0) { sendErrorPage(400, "Bad address", "That address doesn't look right."); return; }
  if (!fsOk) {
    sendErrorPage(503, "Games not found",
                  "The game storage on this board is empty or damaged. Re-flash it from the web flasher.");
    return;
  }
  if (!SPIFFS.exists(path)) {
    sendErrorPage(404, "Page not found", "We couldn't find that game. It may have been removed.");
    return;
  }
  File file = SPIFFS.open(path, "r");
  if (!file) { sendErrorPage(500, "Something went wrong", "The board couldn't open that file. Try again."); return; }
  server.streamFile(file, contentTypeFor(path));
  file.close();
}

// ---------------------------------------------------------------- status
void handlePing() { sendJson("{\"ok\":1,\"ttt\":1}"); }

void handleStatus() {
  String j = "{\"clients\":" + String(WiFi.softAPgetStationNum());
  j += ",\"max\":" + String(MAX_CLIENTS);
  j += ",\"uptime\":" + String(millis() / 1000);
  j += ",\"fs\":" + String(fsOk ? 1 : 0);
  if (fsOk) {
    j += ",\"used\":" + String((unsigned long)SPIFFS.usedBytes());
    j += ",\"total\":" + String((unsigned long)SPIFFS.totalBytes());
  }
  j += "}";
  sendJson(j);
}

// ---------------------------------------------------------------- Tic Tac Toe rooms

Room rooms[MAX_ROOMS];

const int LINES[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};

void randomId(char* out, int len, const char* alphabet) {
  int n = strlen(alphabet);
  for (int i = 0; i < len; i++) out[i] = alphabet[esp_random() % n];
  out[len] = 0;
}

// Keep names short and safe for JSON (UTF-8 letters are allowed)
String cleanName(String s) {
  s.trim();
  String out = "";
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\' || c == '<' || c == '>' || (uint8_t)c < 0x20) continue;
    out += c;
  }
  if (out.length() > 24) {
    out = out.substring(0, 24);
    while (out.length() > 0 && (((uint8_t)out[out.length() - 1]) & 0xC0) == 0x80) out.remove(out.length() - 1); // fix a cut-off UTF-8 letter
    if (out.length() > 0 && ((uint8_t)out[out.length() - 1]) >= 0xC0) out.remove(out.length() - 1);
  }
  if (out.length() == 0) out = "Player";
  return out;
}

Room* findRoom(String token) {
  token.toUpperCase();
  for (int i = 0; i < MAX_ROOMS; i++)
    if (rooms[i].used && token.equals(rooms[i].token)) return &rooms[i];
  return nullptr;
}

// Returns the room and sets role to 'X' or 'O' if the player id matches
Room* authRoom(const String& token, const String& pid, char& role) {
  Room* r = findRoom(token);
  if (!r) return nullptr;
  if (pid.equals(r->pidX)) { role = 'X'; r->seenX = millis(); return r; }
  if (r->hasO && pid.equals(r->pidO)) { role = 'O'; r->seenO = millis(); return r; }
  return nullptr;
}

void newBoard(Room& r) {
  for (int i = 0; i < 9; i++) r.board[i] = '-';
  r.board[9] = 0;
  r.turn = r.starter;
  r.winner = 0;
  r.line[0] = r.line[1] = r.line[2] = -1;
  r.v++;
}

void checkWinner(Room& r) {
  for (int i = 0; i < 8; i++) {
    char a = r.board[LINES[i][0]];
    if (a != '-' && a == r.board[LINES[i][1]] && a == r.board[LINES[i][2]]) {
      r.winner = a;
      r.line[0] = LINES[i][0]; r.line[1] = LINES[i][1]; r.line[2] = LINES[i][2];
      if (a == 'X') r.sx++; else r.so++;
      return;
    }
  }
  for (int i = 0; i < 9; i++) if (r.board[i] == '-') return;
  r.winner = 'D';
  r.d++;
}

String stateJson(Room& r, char role) {
  String st = !r.hasO ? "wait" : (r.winner ? "over" : "play");
  String j = "{\"v\":" + String(r.v);
  j += ",\"nx\":\"" + r.nameX + "\",\"no\":\"" + (r.hasO ? r.nameO : String("")) + "\"";
  j += ",\"b\":\"" + String(r.board) + "\"";
  j += ",\"t\":\"" + String(r.turn) + "\"";
  j += ",\"w\":\"" + (r.winner ? String(r.winner) : String("")) + "\"";
  j += ",\"l\":[" + String(r.line[0]) + "," + String(r.line[1]) + "," + String(r.line[2]) + "]";
  j += ",\"sx\":" + String(r.sx) + ",\"so\":" + String(r.so) + ",\"d\":" + String(r.d);
  j += ",\"you\":\"" + String(role) + "\"";
  j += ",\"opp\":" + String(r.hasO ? 1 : 0);
  j += ",\"st\":\"" + st + "\"}";
  return j;
}

void handleCreate() {
  Room* r = nullptr;
  for (int i = 0; i < MAX_ROOMS; i++) if (!rooms[i].used) { r = &rooms[i]; break; }
  if (!r) { sendJson("{\"err\":\"All game slots are busy. Try again in a minute.\"}"); return; }
  r->used = true;
  // 4 letters/numbers that are easy to read out loud; make sure no other room has the same code
  bool clash;
  do {
    randomId(r->token, 4, "ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
    clash = false;
    for (int i = 0; i < MAX_ROOMS; i++) if (&rooms[i] != r && rooms[i].used && String(rooms[i].token).equals(r->token)) clash = true;
  } while (clash);
  randomId(r->pidX, 8, "abcdefghijklmnopqrstuvwxyz0123456789");
  r->pidO[0] = 0;
  r->nameX = cleanName(server.arg("name"));
  r->nameO = "";
  r->hasO = false;
  r->sx = r->so = r->d = 0;
  r->starter = 'X';
  r->seenX = millis(); r->seenO = 0;
  r->v = 0;
  newBoard(*r);
  sendJson("{\"token\":\"" + String(r->token) + "\",\"pid\":\"" + String(r->pidX) + "\",\"you\":\"X\"}");
}

void handleJoin() {
  Room* r = findRoom(server.arg("token"));
  if (!r) { sendJson("{\"err\":\"Code not found. Check it and try again.\"}"); return; }
  if (r->hasO) { sendJson("{\"err\":\"That game is already full.\"}"); return; }
  randomId(r->pidO, 8, "abcdefghijklmnopqrstuvwxyz0123456789");
  r->nameO = cleanName(server.arg("name"));
  r->hasO = true;
  r->seenO = millis();
  r->sx = r->so = r->d = 0;
  r->starter = 'X';
  newBoard(*r);
  sendJson("{\"pid\":\"" + String(r->pidO) + "\",\"you\":\"O\"}");
}

void handleState() {
  char role = 0;
  Room* r = authRoom(server.arg("token"), server.arg("pid"), role);
  if (!r) { sendJson("{\"err\":\"gone\"}"); return; }
  sendJson(stateJson(*r, role));
}

void handleMove() {
  char role = 0;
  Room* r = authRoom(server.arg("token"), server.arg("pid"), role);
  if (!r) { sendJson("{\"err\":\"gone\"}"); return; }
  int cell = server.arg("cell").toInt();
  if (!r->hasO || r->winner || r->turn != role || cell < 0 || cell > 8 || r->board[cell] != '-') {
    sendJson("{\"err\":\"Move not allowed\"}");
    return;
  }
  r->board[cell] = role;
  r->turn = (role == 'X') ? 'O' : 'X';
  checkWinner(*r);
  r->v++;
  sendJson("{\"ok\":1}");
}

void handleRematch() {
  char role = 0;
  Room* r = authRoom(server.arg("token"), server.arg("pid"), role);
  if (!r) { sendJson("{\"err\":\"gone\"}"); return; }
  if (r->winner && r->hasO) {
    r->starter = (r->starter == 'X') ? 'O' : 'X';
    newBoard(*r);
  }
  sendJson("{\"ok\":1}");
}

void handleLeave() {
  char role = 0;
  Room* r = authRoom(server.arg("token"), server.arg("pid"), role);
  if (r) {
    if (role == 'X') r->used = false;               // the creator closes the room
    else { r->hasO = false; r->nameO = ""; r->v++; } // the guest leaves, the seat is free again
  }
  sendJson("{\"ok\":1}");
}

// Called every second: drop players and rooms that went quiet
void housekeeping() {
  uint32_t now = millis();
  for (int i = 0; i < MAX_ROOMS; i++) {
    Room& r = rooms[i];
    if (!r.used) continue;
    if (now - r.seenX > ROOM_TIMEOUT) { r.used = false; continue; }
    if (r.hasO && now - r.seenO > SEEN_TIMEOUT) { r.hasO = false; r.nameO = ""; r.v++; }
  }
}

// ---------------------------------------------------------------- setup / loop
void setup() {
  Serial.begin(115200);

  fsOk = SPIFFS.begin(false);
  if (!fsOk) Serial.println("SPIFFS mount failed. Upload the data folder or re-flash the board.");

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, NULL, AP_CHANNEL, 0, MAX_CLIENTS);   // NULL = open network
  WiFi.setSleep(false);
#if CONFIG_IDF_TARGET_ESP32C3
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // some C3 boards need a lower power to stay stable
#endif

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  server.on("/api/ping", handlePing);
  server.on("/api/status", handleStatus);
  server.on("/api/ttt/create", handleCreate);
  server.on("/api/ttt/join", handleJoin);
  server.on("/api/ttt/state", handleState);
  server.on("/api/ttt/move", handleMove);
  server.on("/api/ttt/rematch", handleRematch);
  server.on("/api/ttt/leave", handleLeave);
  // connectivity checks used by Android, iPhone, Windows and Firefox
  const char* probes[] = {"/generate_204", "/gen_204", "/hotspot-detect.html", "/library/test/success.html",
                          "/connecttest.txt", "/ncsi.txt", "/redirect", "/success.txt", "/canonical.html"};
  for (unsigned int i = 0; i < sizeof(probes) / sizeof(probes[0]); i++) server.on(probes[i], redirectHome);
  server.onNotFound(handleRequest);
  server.begin();

  Serial.println("Free Games v1.2 started");
  Serial.print("Open: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  static uint32_t lastTick = 0;
  if (millis() - lastTick >= 1000) { lastTick = millis(); housekeeping(); }
}
