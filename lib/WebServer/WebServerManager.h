#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "../../include/Config.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

class WebServerManager {
public:
  WebServerManager();
  void begin();
  void update(); // call in loop if needed
  void sendData(const char *data);

private:
  AsyncWebServer server;
  AsyncEventSource events;

  void setupWiFi();
  void setupRoutes();
};

#endif
