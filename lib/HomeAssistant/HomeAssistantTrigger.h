#ifndef HOME_ASSISTANT_TRIGGER_H
#define HOME_ASSISTANT_TRIGGER_H

#include <Arduino.h>

// When ENABLE_HOME_ASSISTANT is defined, include the real implementation.
// Otherwise, provide dummy (inline) functions.
#ifdef ENABLE_HOME_ASSISTANT

/**
 * @brief Initializes the Home Assistant connection settings.
 *
 * @param host           The hostname or IP of your Home Assistant instance.
 * @param port           The port number (typically 8123).
 * @param apiKey         The long-lived access token for authentication.
 * @param scriptEndpoint The API endpoint for triggering your script.
 */
void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint);

/**
 * @brief Triggers the Home Assistant script with the provided number.
 *
 * @param selectedNumber The number that has changed.
 */
void triggerHomeAssistantScript(int selectedNumber);

#else // If Home Assistant integration is disabled, provide stubs.

inline void initHomeAssistant(const char *host, uint16_t port, const char *apiKey, const char *scriptEndpoint)
{
    // Do nothing.
}

inline void triggerHomeAssistantScript(int selectedNumber)
{
    // Do nothing.
}

#endif // ENABLE_HOME_ASSISTANT

#endif // HOME_ASSISTANT_TRIGGER_H
