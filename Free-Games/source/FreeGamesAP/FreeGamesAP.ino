
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>

WebServer server(80);
DNSServer dnsServer;

const char* apSSID = "Free Games";

void handleFileRequest() {
  String path = server.uri();

  if (path == "/") {
    path = "/index.html";
  }

  // Reject path traversal
  if (path.indexOf("..") >= 0) {
    server.send(400, "text/plain", "Bad request");
    return;
  }

  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "File not found: " + path);
    return;
  }

  String contentType = "text/html";
  if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".json")) contentType = "application/json";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
    contentType = "image/jpeg";
  else if (path.endsWith(".svg")) contentType = "image/svg+xml";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";

  File file = SPIFFS.open(path, "r");
  if (!file) {
    server.send(500, "text/plain", "Could not open file");
    return;
  }

  server.streamFile(file, contentType);
  file.close();
}

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(false)) {
    Serial.println("SPIFFS mount failed!");
    Serial.println("Check partition scheme and filesystem upload.");
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID);

  IPAddress ip = WiFi.softAPIP();

  dnsServer.start(53, "*", ip);

  server.onNotFound(handleFileRequest);
  server.begin();

  Serial.println("Free Games server started.");
  Serial.print("Open: http://");
  Serial.println(ip);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
