#include "WebPage.h"

/**
 * Connect to WiFi network
 *
 * Attempts to connect to the specified WiFi network and waits until
 * the connection is established. Prints connection status to Serial.
 *
 * @param WIFI_SSID Network name
 * @param WIFI_PASS Network password
 */
void connectWifi(const char *WIFI_SSID, const char *WIFI_PASS)
{
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait for connection to be established
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Print success message and IP address
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

/**
 * Read HTML content from SPIFFS
 *
 * Loads index.html from the SPIFFS filesystem and sets up
 * routes for static files (CSS, JavaScript).
 *
 * @param htmlPage String reference to store the loaded HTML content
 * @param server WebServer instance to configure static routes
 */
void readHtml(String &htmlPage, WebServer &server) {
    // Initialize SPIFFS if not already mounted
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Open the index.html file
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Reserve space in the string for better performance
    // Estimate file size and reserve accordingly
    htmlPage.reserve(file.size() + 100);
    
    // Read the file contents into the htmlPage string
    while (file.available()) {
        htmlPage += (char)file.read();
    }
    file.close();

    // Set cache headers for static assets to improve performance
    server.serveStatic("/css/", SPIFFS, "/css/", "max-age=86400");
    server.serveStatic("/js/", SPIFFS, "/js/", "max-age=86400");
    server.serveStatic("/fonts/", SPIFFS, "/fonts/", "max-age=86400");
}