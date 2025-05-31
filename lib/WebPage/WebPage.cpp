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
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("DNS Address: ");
    Serial.print(WiFi.dnsIP());
    Serial.println("Gateway Address: ");
    Serial.print(WiFi.gatewayIP());
    Serial.println("Subnet Mask: ");
    Serial.print(WiFi.subnetMask());
    Serial.println("Hostname: ");
    Serial.print(WiFi.getHostname());
    Serial.println(""); // Print a blank line for better readability
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
void readHtml(String &htmlPage, WebServer &server)
{
    // Initialize SPIFFS if not already mounted
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Open the index.html file
    File file = SPIFFS.open("/index.html", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Read the file contents into the htmlPage string
    while (file.available())
    {
        htmlPage += (char)file.read();
    }
    file.close();

    // Set up routes for static assets (CSS and JavaScript files)
    server.serveStatic("/css/", SPIFFS, "/css/");
    server.serveStatic("/js/", SPIFFS, "/js/");
}