#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include <WebServer.h>

void connectWifi(const char *WIFI_SSID, const char *WIFI_PASS);

void readHtml(String &htmlPage, WebServer &server);

#endif