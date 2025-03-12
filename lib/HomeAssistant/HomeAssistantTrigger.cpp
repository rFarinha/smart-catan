/**
 * HomeAssistantTrigger.cpp
 *
 * Implementation of the Home Assistant integration for the Catan board generator.
 * This file provides functionality to notify a Home Assistant instance
 * about dice rolls and number selections during Catan gameplay.
 *
 * The integration uses the Home Assistant webhook API to send notifications
 * when numbers are selected, enabling home automation actions
 * to be triggered by game events.
 */

#include "HomeAssistantTrigger.h"

// Only compile this implementation if Home Assistant integration is enabled
#ifdef ENABLE_HOME_ASSISTANT

#include <WiFi.h>
#include <HTTPClient.h>

// Static variables to store Home Assistant connection configuration
static String _haHost;           // Hostname or IP address of Home Assistant instance
static uint16_t _haPort;         // Port number (typically 8123)
static String _haApiKey;         // Long-lived access token for authentication
static String _haScriptEndpoint; // API endpoint for triggering automation scripts

/**
 * Initialize the Home Assistant connection settings
 *
 * Stores the provided configuration parameters for later use
 * when triggering automations. This function must be called before
 * any Home Assistant triggers can be sent.
 *
 * @param host           Hostname or IP address of the Home Assistant instance
 * @param port           Port number (typically 8123 for Home Assistant)
 * @param apiKey         Long-lived access token for API authentication
 * @param scriptEndpoint API endpoint path for triggering automation scripts
 */
void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint)
{
    // Store the configuration in static variables for later use
    _haHost = host;
    _haPort = port;
    _haApiKey = apiKey;
    _haScriptEndpoint = scriptEndpoint;
}

/**
 * Trigger a Home Assistant automation when a number is selected
 *
 * Sends an HTTP POST request to the configured Home Assistant instance
 * with the selected dice number. This can be used to trigger automations
 * like lighting effects corresponding to different game events.
 *
 * Uses a webhook endpoint format: http://{host}:{port}/api/webhook/esp32_number
 * with a JSON payload containing the selected number.
 *
 * @param selectedNumber The dice number that was selected (2-12, or 7 for robber)
 */
void triggerHomeAssistantScript(int selectedNumber)
{
    HTTPClient http;

    // Construct the URL for the Home Assistant webhook
    // This uses a webhook endpoint called "esp32_number"
    String url = "http://" + _haHost + ":" + String(_haPort) + "/api/webhook/esp32_number";

    // Initialize the HTTP request
    http.begin(url);

    // Set required headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _haApiKey);

    // Create a simple JSON payload with the selected number
    String payload = "{\"selectedNumber\": " + String(selectedNumber) + "}";

    // Send the POST request
    int httpResponseCode = http.POST(payload);

    // Log the result
    if (httpResponseCode > 0)
    {
        Serial.printf("HomeAssistant trigger response code: %d\n", httpResponseCode);
    }
    else
    {
        Serial.printf("Error triggering HomeAssistant: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Clean up HTTP resources
    http.end();
}

#endif // ENABLE_HOME_ASSISTANT