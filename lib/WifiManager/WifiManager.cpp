#include "WiFiManager.h"

WiFiManager::WiFiManager() : 
    configMode(false),
    portalRunning(false),
    portalStartTime(0),
    haPort(8123),
    haEnabled(false),
    dnsServer(nullptr),
    configServer(nullptr) {
}

WiFiManager::~WiFiManager() {
    if (dnsServer) {
        delete dnsServer;
    }
    if (configServer) {
        configServer->stop();
        delete configServer;
    }
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
            
            configMode = false;
            return true;
        } else {
            Serial.println("\nFailed to connect to WiFi. Starting configuration portal...");
        }
    } else {
        Serial.println("No WiFi configuration found.");
    }
    
    // If we reached here, we couldn't connect to WiFi
    // Start the configuration portal
    setupConfigServer();
    configMode = true;
    return false;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::setupConfigServer() {
    // Set up access point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_NAME, AP_PASSWORD);
    delay(500); // Give it time to start up
    
    Serial.print("Starting config portal AP at IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Start DNS server
    dnsServer = new DNSServer();
    dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
    
    // Start web server
    configServer = new WebServer(80);
    
    // Setup server routes
    configServer->on("/", HTTP_GET, [this]() { this->handleConfigRoot(); });
    configServer->on("/save", HTTP_POST, [this]() { this->handleConfigSave(); });
    configServer->on("/scan", HTTP_GET, [this]() { this->handleWifiScan(); });
    configServer->on("/board", HTTP_GET, [this]() { this->handleBoardControl(); });
    
    // Game control endpoints (needed for standalone mode)
    configServer->on("/setclassic", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/setextension", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/getboard", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/getnumber", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/startgame", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/endgame", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/selectNumber", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/rollDice", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/eightSixCanTouch", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/twoTwelveCanTouch", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/sameNumbersCanTouch", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/sameResourceCanTouch", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    configServer->on("/manualDice", HTTP_GET, [this]() { this->handleCatanBoardRequest(); });
    
    // Serve static files from SPIFFS
    configServer->serveStatic("/css/", SPIFFS, "/css/");
    configServer->serveStatic("/js/", SPIFFS, "/js/");
    configServer->serveStatic("/config/", SPIFFS, "/config/");
    
    // Capture other requests for captive portal
    configServer->onNotFound([this]() {
        Serial.print("Not found: ");
        Serial.println(configServer->uri());
        // Redirect to configuration portal only for web page requests
        if (configServer->header("User-Agent").indexOf("Mozilla") >= 0) {
            this->handleConfigRoot();
        } else {
            // For other requests (like API calls), send 404
            configServer->send(404, "text/plain", "Not found");
        }
    });
    
    configServer->begin();
    portalRunning = true;
    portalStartTime = millis();
}

void WiFiManager::process() {
    if (portalRunning) {
        // Process DNS requests
        if (dnsServer) {
            dnsServer->processNextRequest();
        }
        
        // Process web server requests
        if (configServer) {
            configServer->handleClient();
        }
        
        // Check for timeout
        if (CONFIG_PORTAL_TIMEOUT > 0 && 
            millis() - portalStartTime > CONFIG_PORTAL_TIMEOUT * 1000) {
            Serial.println("Config portal timeout. Restarting device...");
            ESP.restart();
        }
    }
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

void WiFiManager::handleConfigRoot() {
    // Check if the config portal HTML file exists in SPIFFS
    if (SPIFFS.exists("/portal.html")) {
        // Read the file content
        File file = SPIFFS.open("/portal.html", "r");
        if (!file) {
            configServer->send(500, "text/plain", "Failed to open config portal file");
            return;
        }
        
        String html = file.readString();
        file.close();
        
        // Replace placeholder values with actual config values
        html.replace("{{SSID}}", ssid);
        html.replace("{{PASSWORD}}", password);
        html.replace("{{HA_IP}}", haIp);
        html.replace("{{HA_PORT}}", String(haPort));
        html.replace("{{HA_TOKEN}}", haAccessToken);
        
        configServer->send(200, "text/html", html);
    } else {
        configServer->send(404, "text/plain", "Config portal file not found. Please upload the portal.html file to the /data/ directory in SPIFFS.");
    }
}

void WiFiManager::handleWifiScan() {
    Serial.println("Scanning for networks...");
    
    int n = WiFi.scanNetworks();
    JsonDocument doc;
    JsonArray networks = doc.createNestedArray("networks");
    
    for (int i = 0; i < n; ++i) {
        JsonObject network = networks.createNestedObject();
        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        network["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
    
    String response;
    serializeJson(doc, response);
    configServer->send(200, "application/json", response);
    
    // Clean up scan results
    WiFi.scanDelete();
}

void WiFiManager::handleConfigSave() {
    if (configServer->hasArg("ssid")) {
        ssid = configServer->arg("ssid");
        password = configServer->arg("password");
        haIp = configServer->arg("ha_ip");
        
        // Parse port with validation
        String portStr = configServer->arg("ha_port");
        haPort = portStr.length() > 0 ? portStr.toInt() : 8123;
        
        haAccessToken = configServer->arg("ha_token");
        
        // Determine if HA is enabled based on whether required fields are provided
        haEnabled = haIp.length() > 0 && haAccessToken.length() > 0;
        
        // Save configuration
        if (saveConfig()) {
            // Check if we have a success page in SPIFFS
            if (SPIFFS.exists("/success.html")) {
                File file = SPIFFS.open("/success.html", "r");
                if (file) {
                    String html = file.readString();
                    file.close();
                    configServer->send(200, "text/html", html);
                } else {
                    configServer->send(200, "text/plain", "Configuration saved! Restarting device...");
                }
            } else {
                configServer->send(200, "text/plain", "Configuration saved! Restarting device...");
            }
            
            // Give the browser time to receive the response before restarting
            delay(2000);
            
            // End the portal
            portalRunning = false;
            
            // Restart the ESP32 to apply new settings
            ESP.restart();
        } else {
            configServer->send(500, "text/plain", "Failed to save configuration");
        }
    } else {
        configServer->send(400, "text/plain", "Missing SSID parameter");
    }
}

void WiFiManager::handleBoardControl() {
    // Check if index.html exists in SPIFFS
    if (SPIFFS.exists("/index.html")) {
        // Load the file content
        File file = SPIFFS.open("/index.html", "r");
        if (file) {
            String html = file.readString();
            file.close();
            
            // Log the successful file load
            Serial.println("Loading board interface from index.html");
            
            // Modify paths if needed for standalone mode
            // This ensures CSS and JS files are loaded properly in AP mode
            html.replace("href=\"/css/", "href=\"/css/");
            html.replace("src=\"/js/", "src=\"/js/");
            
            // Send the HTML content
            configServer->send(200, "text/html", html);
            
            Serial.println("Board interface sent to client");
        } else {
            Serial.println("Failed to open index.html file");
            configServer->send(500, "text/plain", "Failed to load board interface");
        }
    } else {
        Serial.println("index.html not found in SPIFFS");
        configServer->send(404, "text/plain", "Board interface not found");
    }
}

/**
 * Handler for Catan board API requests in WiFiManager mode
 * This implements a simplified version of the board API for standalone mode
 */
void WiFiManager::handleCatanBoardRequest() {
    String uri = configServer->uri();
    Serial.print("Catan board API request: ");
    Serial.println(uri);
    
    // Simple JSON response for most endpoints
    if (uri == "/getboard") {
        // Simplified board data
        String response = "{\"resources\":[0,1,2,3,4,5,0,1,2,3,4,0,1,2,3,4,0,1,2],";
        response += "\"numbers\":[2,3,4,5,6,0,8,9,10,11,12,3,4,5,6,8,9,10,11],";
        response += "\"extension\":false,\"gameStarted\":false,";
        response += "\"eightSixCanTouch\":true,\"twoTwelveCanTouch\":true,";
        response += "\"sameNumbersCanTouch\":true,\"sameResourceCanTouch\":true,";
        response += "\"manualDice\":true,\"selectedNumber\":0}";
        
        configServer->send(200, "application/json", response);
    }
    else if (uri == "/getnumber") {
        configServer->send(200, "application/json", "0");
    }
    else if (uri == "/setclassic" || uri == "/setextension") {
        // Simple response for mode switching
        String response = "{\"resources\":[0,1,2,3,4,5,0,1,2,3,4,0,1,2,3,4,0,1,2],";
        response += "\"numbers\":[2,3,4,5,6,0,8,9,10,11,12,3,4,5,6,8,9,10,11],";
        response += "\"extension\":" + String(uri == "/setextension" ? "true" : "false") + ",";
        response += "\"gameStarted\":false,";
        response += "\"eightSixCanTouch\":true,\"twoTwelveCanTouch\":true,";
        response += "\"sameNumbersCanTouch\":true,\"sameResourceCanTouch\":true,";
        response += "\"manualDice\":true,\"selectedNumber\":0}";
        
        configServer->send(200, "application/json", response);
    }
    else if (uri == "/startgame" || uri == "/endgame") {
        // Simple response for game state changes
        String response = "{\"resources\":[0,1,2,3,4,5,0,1,2,3,4,0,1,2,3,4,0,1,2],";
        response += "\"numbers\":[2,3,4,5,6,0,8,9,10,11,12,3,4,5,6,8,9,10,11],";
        response += "\"extension\":false,";
        response += "\"gameStarted\":" + String(uri == "/startgame" ? "true" : "false") + ",";
        response += "\"eightSixCanTouch\":true,\"twoTwelveCanTouch\":true,";
        response += "\"sameNumbersCanTouch\":true,\"sameResourceCanTouch\":true,";
        response += "\"manualDice\":true,\"selectedNumber\":0}";
        
        configServer->send(200, "application/json", response);
    }
    else if (uri == "/selectNumber" || uri == "/rollDice") {
        // Return the selected number or a random number for dice
        String value;
        if (uri == "/selectNumber") {
            value = configServer->arg("value");
        } else {
            // For dice roll, generate random result (2-12)
            int die1 = random(1, 7);
            int die2 = random(1, 7);
            value = String(die1 + die2);
        }
        
        configServer->send(200, "text/plain", value);
    }
    else if (uri.indexOf("CanTouch") > 0 || uri == "/manualDice") {
        // Handle settings changes
        configServer->send(200, "text/plain", "Setting updated");
    }
    else {
        // Default response for other endpoints
        configServer->send(200, "text/plain", "OK");
    }
}