#include "WebPage.h"

void connectWifi(const char *WIFI_SSID, const char *WIFI_PASS)
{
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void readHtml(String &htmlPage, WebServer &server)
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    File file = SPIFFS.open("/index.html", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    while (file.available())
    {
        htmlPage += (char)file.read();
    }
    file.close();

    // Serve static files for CSS and JavaScript
    server.serveStatic("/css/", SPIFFS, "/css/");
    server.serveStatic("/js/", SPIFFS, "/js/");
}