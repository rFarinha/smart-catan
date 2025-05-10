#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// Configuration constants
#define AP_NAME "Smart Catan"       // Name of the WiFi network created by the ESP32
#define AP_PASSWORD ""              // Empty for open network, or set a password
#define CONFIG_FILE "/wifi_config.json"
#define DNS_PORT 53                 // Standard DNS port
#define CONFIG_PORTAL_TIMEOUT 180   // Timeout after 3 minutes of inactivity
#define CONNECT_TIMEOUT 10          // Wait 10 seconds to connect to WiFi

class WiFiManager {
private:
    bool configMode;
    bool portalRunning;
    unsigned long portalStartTime;
    
    // Config values
    String ssid;
    String password;
    String haIp;
    uint16_t haPort;
    String haAccessToken;
    bool haEnabled;
    
    // DNS Server for captive portal
    DNSServer* dnsServer;
    WebServer* configServer;
    
    // Load config from file
    bool loadConfig();
    
    // Save config to file
    bool saveConfig();
    
    // Setup configuration web server
    void setupConfigServer();
    
    // Handle the config server root page (showing the form)
    void handleConfigRoot();
    
    // Handle submission of config form
    void handleConfigSave();
    
    // Handle WiFi scan request
    void handleWifiScan();
    
    // Serve the regular Catan board interface
    void handleBoardControl();
    
    // Handler for Catan board API requests in standalone mode
    void handleCatanBoardRequest();
    
public:
    WiFiManager();
    ~WiFiManager();
    
    // Initialize and try to connect to stored WiFi
    // Returns true if connected, false if AP mode started
    bool begin();
    
    // Check if connected to a WiFi network
    bool isConnected() const;
    
    // Check if in configuration mode
    bool isInConfigMode() const { return configMode; }
    
    // Process portal requests (call in loop)
    void process();
    
    // Get Home Assistant config properties
    bool isHaEnabled() const { return haEnabled; }
    String getHaIp() const { return haIp; }
    uint16_t getHaPort() const { return haPort; }
    String getHaAccessToken() const { return haAccessToken; }
    
    // Allow main web server to check if portal is running
    bool isPortalActive() const { return portalRunning; }
    
    // Get the config server instance
    WebServer* getConfigServer() { return configServer; }
    
    // List all files in SPIFFS
    void listSPIFFSFiles();
};

#endif // WIFI_MANAGER_H