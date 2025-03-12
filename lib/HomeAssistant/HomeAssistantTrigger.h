/**
 * HomeAssistantTrigger.h
 *
 * This header defines the interface for integrating with Home Assistant,
 * a popular open-source home automation platform. The integration allows
 * the Catan board to notify Home Assistant when dice numbers are selected,
 * enabling home automation responses to game events.
 *
 * The library is conditionally compiled based on the ENABLE_HOME_ASSISTANT
 * definition, allowing users to disable this feature if not needed.
 */

#ifndef HOME_ASSISTANT_TRIGGER_H
#define HOME_ASSISTANT_TRIGGER_H

#include <Arduino.h>

// When ENABLE_HOME_ASSISTANT is defined, include the real implementation.
// Otherwise, provide dummy (inline) functions to maintain API compatibility.
#ifdef ENABLE_HOME_ASSISTANT

/**
 * Initialize the Home Assistant connection settings
 *
 * Sets up the configuration needed to communicate with a Home Assistant
 * instance via its REST API. This must be called before any trigger events.
 *
 * @param host           Hostname or IP address of the Home Assistant instance
 * @param port           Port number (typically 8123 for Home Assistant)
 * @param apiKey         Long-lived access token for API authentication
 * @param scriptEndpoint API endpoint path for triggering automation scripts
 */
void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint);

/**
 * Trigger a Home Assistant automation when a number is selected
 *
 * Sends an HTTP POST request to the configured Home Assistant instance
 * with the selected dice number, which can trigger automations like
 * lighting effects corresponding to different game events.
 *
 * @param selectedNumber The dice number that was selected (2-12, or 7 for robber)
 */
void triggerHomeAssistantScript(int selectedNumber);

#else // If Home Assistant integration is disabled, provide stub implementations

/**
 * Stub implementation when Home Assistant is disabled
 *
 * This empty implementation maintains API compatibility when
 * the ENABLE_HOME_ASSISTANT flag is not defined.
 */
inline void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint)
{
    // Do nothing when Home Assistant integration is disabled
}

/**
 * Stub implementation when Home Assistant is disabled
 *
 * This empty implementation maintains API compatibility when
 * the ENABLE_HOME_ASSISTANT flag is not defined.
 */
inline void triggerHomeAssistantScript(int selectedNumber)
{
    // Do nothing when Home Assistant integration is disabled
}

#endif // ENABLE_HOME_ASSISTANT

#endif // HOME_ASSISTANT_TRIGGER_H