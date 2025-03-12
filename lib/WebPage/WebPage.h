/**
 * WebPage.h
 *
 * Header file for web server utility functions
 * Handles WiFi connection and loading HTML content
 */

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include <WebServer.h>

/**
 * Connect to WiFi network
 *
 * Attempts to connect to the specified WiFi network and
 * prints the IP address when connected.
 *
 * @param WIFI_SSID Network name
 * @param WIFI_PASS Network password
 */
void connectWifi(const char *WIFI_SSID, const char *WIFI_PASS);

/**
 * Read HTML content from SPIFFS
 *
 * Loads the main HTML file from flash and sets up static routes
 * for CSS and JavaScript files.
 *
 * @param htmlPage String to store the HTML content
 * @param server WebServer instance to configure static routes
 */
void readHtml(String &htmlPage, WebServer &server);

#endif