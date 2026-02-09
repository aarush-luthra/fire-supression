#include "WebServerManager.h"

WebServerManager::WebServerManager()
    : server(WEB_SERVER_PORT), events("/events") {}

void WebServerManager::begin() {
  setupWiFi();

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    // Fallback or retry?
  }

  setupRoutes();
  server.begin();
}

void WebServerManager::setupWiFi() {
  // Check if we should use Access Point mode
  if (USE_AP_MODE || String(WIFI_SSID) == "YOUR_WIFI_SSID") {
    Serial.println("Starting Access Point...");
    WiFi.softAP(WIFI_SSID, WIFI_PASS);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  } else {
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.println("");

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("WiFi connection failed. Falling back to AP mode.");
      WiFi.softAP("ESP32_Fire_Fallback", "12345678");
      Serial.println(WiFi.softAPIP());
    }
  }
}

void WebServerManager::setupRoutes() {
  // Serve Static Files
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/app.js", "text/javascript");
  });

  server.on("/dashboard.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/dashboard.js", "text/javascript");
  });

  // Serve CSS files if any (assuming inline or CDN for now based on file view)
  // If user adds local css files, we can serve them here.

  // Event Source
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n",
                    client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
}

void WebServerManager::sendData(const char *data) {
  events.send(data, "message", millis());
}

void WebServerManager::update() {
  // cleanup clients if needed, but AsyncWebServer handles this mostly
}
