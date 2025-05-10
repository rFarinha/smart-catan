#include "WiFiManager.h"

WiFiManager::WiFiManager() : 
    apMode(false),
    haPort(8123),
    haEnabled(false) {
}

bool WiFiManager::begin() {
    // Load saved configuration
    if (loadConfig()) {
        Serial.println("WiFi configuration found, attempting to connect...");
        
        // Try to connect to the stored WiFi
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        
        // Wait for connection for CONNECT_TIMEOUT seconds
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startTime < CONNECT_TIMEOUT * 1000) {
            delay(500);
            Serial.print(".");
        }
        
        // Check if connected
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to WiFi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            
            // Start mDNS responder
            if (!MDNS.begin("smartcatan")) {
                Serial.println("Error setting up MDNS responder!");
            } else {
                Serial.println("mDNS responder started");
            }
            
            apMode = false;
            return true;
        } else {
            Serial.println("\nFailed to connect to WiFi. Starting AP mode...");
        }
    } else {
        Serial.println("No WiFi configuration found. Starting AP mode...");
    }
    
    // If we couldn't connect to WiFi, start AP mode
    startApMode();
    return false;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startApMode() {
    // Set up access point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_NAME, AP_PASSWORD);
    delay(500); // Give it time to start up
    
    // Get the AP IP (usually 192.168.4.1)
    IPAddress apIP = WiFi.softAPIP();
    
    Serial.print("Started AP mode. Connect to ");
    Serial.print(AP_NAME);
    Serial.print(" WiFi network and navigate to http://");
    Serial.println(apIP);
    
    apMode = true;
}

bool WiFiManager::loadConfig() {
    if (!SPIFFS.exists(CONFIG_FILE)) {
        return false;
    }
    
    File configFile = SPIFFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.f_str());
        return false;
    }
    
    // Read values
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    haIp = doc["ha_ip"].as<String>();
    haPort = doc["ha_port"].as<uint16_t>();
    haAccessToken = doc["ha_token"].as<String>();
    haEnabled = doc["ha_enabled"].as<bool>();
    
    // Check if WiFi settings are valid
    if (ssid.length() == 0) {
        return false;
    }
    
    return true;
}

bool WiFiManager::saveConfig() {
    JsonDocument doc;
    
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["ha_ip"] = haIp;
    doc["ha_port"] = haPort;
    doc["ha_token"] = haAccessToken;
    doc["ha_enabled"] = haEnabled;
    
    File configFile = SPIFFS.open(CONFIG_FILE, "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }
    
    serializeJson(doc, configFile);
    configFile.close();
    return true;
}

bool WiFiManager::saveCredentials(const String &newSsid, const String &newPassword) {
    ssid = newSsid;
    password = newPassword;
    return saveConfig();
}

bool WiFiManager::saveHAConfig(const String &ip, uint16_t port, const String &token) {
    haIp = ip;
    haPort = port;
    haAccessToken = token;
    haEnabled = (ip.length() > 0 && token.length() > 0);
    return saveConfig();
}