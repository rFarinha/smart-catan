#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// Configuration constants
#define AP_NAME "Smart Catan"       // Name of the WiFi network created by the ESP32
#define AP_PASSWORD ""              // Empty for open network, or set a password
#define CONFIG_FILE "/wifi_config.json"
#define CONNECT_TIMEOUT 10          // Wait 10 seconds to connect to WiFi

class WiFiManager {
private:
    bool apMode;
    
    // Config values
    String ssid;
    String password;
    String haIp;
    uint16_t haPort;
    String haAccessToken;
    bool haEnabled;
    
    // Load config from file
    bool loadConfig();
    
    // Save config to file
    bool saveConfig();
    
public:
    WiFiManager();
    
    // Initialize and try to connect to stored WiFi
    // Returns true if connected, false if AP mode started
    bool begin();
    
    // Check if connected to a WiFi network
    bool isConnected() const;
    
    // Check if in AP mode
    bool isInApMode() const { return apMode; }
    
    // Start AP mode
    void startApMode();
    
    // Save WiFi credentials and return success status
    bool saveCredentials(const String &ssid, const String &password);
    
    // Save Home Assistant configuration and return success status
    bool saveHAConfig(const String &ip, uint16_t port, const String &token);
    
    // Get Home Assistant config properties
    bool isHaEnabled() const { return haEnabled; }
    String getHaIp() const { return haIp; }
    uint16_t getHaPort() const { return haPort; }
    String getHaAccessToken() const { return haAccessToken; }
};

#endif // WIFI_MANAGER_H