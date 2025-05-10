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