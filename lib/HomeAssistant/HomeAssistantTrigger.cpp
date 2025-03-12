#include "HomeAssistantTrigger.h"

#ifdef ENABLE_HOME_ASSISTANT

#include <WiFi.h>
#include <HTTPClient.h>

// Internal variables to hold the configuration.
static String _haHost;
static uint16_t _haPort;
static String _haApiKey;
static String _haScriptEndpoint;

void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint)
{
    _haHost = host;
    _haPort = port;
    _haApiKey = apiKey;
    _haScriptEndpoint = scriptEndpoint;
}

void triggerHomeAssistantScript(int selectedNumber)
{
    HTTPClient http;
    // Construct the URL for Home Assistant.
    String url = "http://" + _haHost + ":" + String(_haPort) + "/api/webhook/esp32_number";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _haApiKey);

    // Create a JSON payload with the selected number.
    String payload = "{\"selectedNumber\": " + String(selectedNumber) + "}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0)
    {
        Serial.printf("HomeAssistant trigger response code: %d\n", httpResponseCode);
    }
    else
    {
        Serial.printf("Error triggering HomeAssistant: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}

#endif // ENABLE_HOME_ASSISTANT
