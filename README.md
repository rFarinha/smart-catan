# Catan Board Generator

A digital Catan board generator for ESP32 with physical LED visualization and web interface. This project allows you to generate randomized Catan game boards with configurable rules, display them on a physical LED board, and control everything through a web interface.

Go to [makerworld](https://makerworld.com/en/@user_1615041609) to get all the 3d models.

## Features

- Generates random Catan boards with customizable rules
- Supports both classic (19 hexes) and extension (30 hexes) boards
- Physical board visualization using WS2812B addressable LEDs
- Web interface for easy control from any device
- Configurable board generation rules:
  - Whether 6 & 8 tokens can be adjacent
  - Whether 2 & 12 tokens can be adjacent
  - Whether same resource types can be adjacent
  - Whether same number tokens can be adjacent
- Dice rolling simulation with LED animation
- Optional Home Assistant integration

## Hardware Requirements

- ESP32 development board (ESP32-WROOM or similar)
- WS2812B addressable LED strip:
  - 19 LEDs for classic Catan board
  - 30 LEDs for extension board
- Power supply for the LEDs (5V)
- USB connection for programming

## Software Requirements

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- Web browser for accessing the interface

## Getting Started

### Setting Up PlatformIO

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)
3. Clone this repository
4. Open the project folder in VS Code

### WiFi Configuration

There are two ways to set your WiFi credentials:

#### Option 1: Edit the placeholders in main.cpp (Recommended for users)

Look for the following section in `src/main.cpp` and simply replace the placeholder values with your actual WiFi credentials:

```cpp
// ------------- WIFI Credentials -------------
#ifndef PASSWORD_H
#define PASSWORD_H
const char *WIFI_SSID = "PlaceholderSSID";  // Change to your WiFi name
const char *WIFI_PASS = "PlaceholderPassword";  // Change to your WiFi password
```

#### Option 2: Create a separate password.h file (Recommended for contributors)

If you plan to contribute to this project or make multiple commits, creating a separate `include/password.h` file is better practice:

1. Create a file named `include/password.h` with the following content:
   ```cpp
   #ifndef PASSWORD_H
   #define PASSWORD_H
   
   const char *WIFI_SSID = "YourWifiName";
   const char *WIFI_PASS = "YourWifiPassword";
   
   #endif
   ```

2. This file is already listed in `.gitignore` to prevent accidentally committing your credentials.

Using option 2 is better for development as it keeps your personal credentials separate from the code you might commit.

### Building and Uploading the Firmware

1. Connect your ESP32 via USB
2. In VS Code with PlatformIO extension installed, click the PlatformIO icon in the sidebar
3. Under "PROJECT TASKS" > "esp32dev", click:
   - "Build" to compile the firmware
   - "Upload" to flash the firmware to the ESP32
   - "Upload Filesystem Image" to upload the web interface files to SPIFFS

Alternatively, you can use these commands from the terminal:

```bash
# Build the project
pio run

# Upload firmware to ESP32
pio run --target upload

# Upload filesystem image (web interface)
pio run --target uploadfs
```

### Wiring

Follow the makerworld associated document to build the board. Then:
- Connect the LED data line to GPIO 4 (default, can be changed in `main.cpp`)
- Connect the LED power (5V) and ground to appropriate power supply
- Make sure your power supply can handle the current required by the LEDs

### Using the Web Interface

1. After uploading and restarting, the ESP32 will connect to your WiFi
2. Open a web browser and go to smartcatan.local or check the Serial Monitor for the assigned IP address
3. Use the interface to:
   - Choose between classic or extension board
   - Configure board generation rules
   - Start/end games
   - Roll dice or manually select numbers

## Project Structure

- `src/main.cpp` - Main application code
- `data/` - Web interface files (HTML, CSS, JS)
- `lib/` - Project libraries:
  - `BoardGenerator/` - Board generation algorithms
  - `LedController/` - LED control and animations
  - `WebPage/` - Web server setup
  - `HomeAssistant/` - Optional Home Assistant integration

## Optional: Home Assistant Integration

The Catan Board Generator can be integrated with Home Assistant to trigger automations when dice are rolled or numbers are selected. This allows for exciting possibilities such as changing your room lighting based on the selected number or triggering special effects when the robber (7) is rolled.

### Enabling Home Assistant Integration

1. Open `src/main.cpp` and uncomment the following line:
   ```cpp
   //#define ENABLE_HOME_ASSISTANT
   ```

2. Set your Home Assistant credentials by either:
   - Editing the placeholder values directly in `main.cpp`:
     ```cpp
     // Home Assistant credentials (only used if ENABLE_HOME_ASSISTANT is defined)
      const char *HA_IP = "homeassistant.local";
      const uint16_t HA_PORT = 8123;
      const char *HA_ACCESS_TOKEN = "your_long_lived_access_token";
      #endif
     ```
   - Or adding them to your `include/password.h` file if you're using that method
      ```cpp
      #ifndef PASSWORD_H
      #define PASSWORD_H
      
      const char *WIFI_SSID = "YourWifiName";
      const char *WIFI_PASS = "YourWifiPassword";

      const char *HA_IP = "homeassistant.local";
      const uint16_t HA_PORT = 8123;
      const char *HA_ACCESS_TOKEN = "your_long_lived_access_token";
      
      #endif
      ```


### Setting Up a Webhook in Home Assistant

1. In your Home Assistant installation, go to Settings → Automations & Scenes → Create Automation
2. Choose "Webhook" as the trigger
3. Set the webhook ID to `esp32_number`
4. Add conditions as needed
5. Set up your desired actions (e.g., control lights based on the selected number)

### Example Automation

Here's an example automation in YAML format that changes light colors based on the selected number:

```yaml
alias: Catan Board Dice Roll
description: "Change lights based on Catan dice number"
trigger:
  - platform: webhook
    webhook_id: esp32_number
condition: []
action:
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ trigger.json.selectedNumber == 7 }}"
        sequence:
          - service: light.turn_on
            target:
              entity_id: light.living_room
            data:
              rgb_color: [255, 0, 0]
              brightness_pct: 100
      - conditions:
          - condition: template
            value_template: "{{ trigger.json.selectedNumber == 6 or trigger.json.selectedNumber == 8 }}"
        sequence:
          - service: light.turn_on
            target:
              entity_id: light.living_room
            data:
              rgb_color: [0, 255, 0]
              brightness_pct: 100
    default:
      - service: light.turn_on
        target:
          entity_id: light.living_room
        data:
          rgb_color: [0, 0, 255]
          brightness_pct: 70
mode: single
```

This automation will:
- Turn the lights red when 7 (robber) is rolled
- Turn the lights green when 6 or 8 (highest probability) is rolled
- Turn the lights blue for all other numbers

### Extended Automation
```yaml
alias: Catan
description: Change lights based on Catan dice number and resources selected.
triggers:
  - webhook_id: esp32_number
    trigger: webhook
    allowed_methods:
      - POST
      - PUT
    local_only: true
conditions: []
actions:
  - action: scene.create
    metadata: {}
    data:
      snapshot_entities:
        - media_player.sonos_keuken
        - light.ledpaneel_woonkamer_achter
      scene_id: catan_entities_restore
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ trigger.json.selectedNumber != 7 }}"
            alias: Not Robber
        sequence:
          - repeat:
              sequence:
                - choose:
                    - conditions:
                        - condition: template
                          value_template: "{{repeat.item == \"sheep\"}}"
                      sequence:
                        - data:
                            brightness_pct: 65
                            rgbww_color:
                              - 185
                              - 255
                              - 35
                              - 100
                              - 50
                          action: light.turn_on
                          target:
                            entity_id: light.ledpaneel_woonkamer_achter
                        - action: media_player.volume_set
                          metadata: {}
                          data:
                            volume_level: 0.4
                          target:
                            entity_id: media_player.sonos_keuken
                        - action: media_player.play_media
                          target:
                            entity_id: media_player.sonos_keuken
                          data:
                            media_content_id: media-source://media_source/local/catan-sheep.mp3
                            media_content_type: audio/mpeg
                          metadata:
                            title: catan-sheep.mp3
                            thumbnail: null
                            media_class: music
                            children_media_class: null
                            navigateIds:
                              - {}
                              - media_content_type: app
                                media_content_id: media-source://media_source
                        - delay:
                            hours: 0
                            minutes: 0
                            seconds: 2
                            milliseconds: 0
                      alias: Sheep
                    - conditions:
                        - condition: template
                          value_template: "{{repeat.item == \"wood\"}}"
                      sequence:
                        - data:
                            rgbww_color:
                              - 0
                              - 255
                              - 0
                              - 0
                              - 0
                            brightness_pct: 35
                          action: light.turn_on
                          target:
                            entity_id: light.ledpaneel_woonkamer_achter
                        - action: media_player.volume_set
                          metadata: {}
                          data:
                            volume_level: 0.4
                          target:
                            entity_id: media_player.sonos_keuken
                        - action: media_player.play_media
                          target:
                            entity_id: media_player.sonos_keuken
                          data:
                            media_content_id: media-source://media_source/local/catan-wood.mp3
                            media_content_type: audio/mpeg
                          metadata:
                            title: catan-wood.mp3
                            thumbnail: null
                            media_class: music
                            children_media_class: null
                            navigateIds:
                              - {}
                              - media_content_type: app
                                media_content_id: media-source://media_source
                        - delay:
                            hours: 0
                            minutes: 0
                            seconds: 2
                            milliseconds: 0
                      alias: Wood
                    - conditions:
                        - condition: template
                          value_template: "{{repeat.item == \"wheat\"}}"
                      sequence:
                        - data:
                            brightness_pct: 100
                            rgbww_color:
                              - 255
                              - 170
                              - 0
                              - 0
                              - 0
                          action: light.turn_on
                          target:
                            entity_id: light.ledpaneel_woonkamer_achter
                        - action: media_player.volume_set
                          metadata: {}
                          data:
                            volume_level: 0.4
                          target:
                            entity_id: media_player.sonos_keuken
                        - action: media_player.play_media
                          target:
                            entity_id: media_player.sonos_keuken
                          data:
                            media_content_id: media-source://media_source/local/catan-cow.mp3
                            media_content_type: audio/mpeg
                          metadata:
                            title: catan-cow.mp3
                            thumbnail: null
                            media_class: music
                            children_media_class: null
                            navigateIds:
                              - {}
                              - media_content_type: app
                                media_content_id: media-source://media_source
                        - delay:
                            hours: 0
                            minutes: 0
                            seconds: 5
                            milliseconds: 0
                      alias: Wheat
                    - conditions:
                        - condition: template
                          value_template: "{{repeat.item == \"brick\"}}"
                      sequence:
                        - data:
                            rgb_color:
                              - 230
                              - 97
                              - 0
                            brightness_pct: 100
                          action: light.turn_on
                          target:
                            entity_id: light.ledpaneel_woonkamer_achter
                        - action: media_player.volume_set
                          metadata: {}
                          data:
                            volume_level: 0.9
                          target:
                            entity_id: media_player.sonos_keuken
                        - action: media_player.play_media
                          target:
                            entity_id: media_player.sonos_keuken
                          data:
                            media_content_id: media-source://media_source/local/catan-brick2.mp3
                            media_content_type: audio/mpeg
                          metadata:
                            title: catan-brick2.mp3
                            thumbnail: null
                            media_class: music
                            children_media_class: null
                            navigateIds:
                              - {}
                              - media_content_type: app
                                media_content_id: media-source://media_source
                        - delay:
                            hours: 0
                            minutes: 0
                            seconds: 6
                            milliseconds: 0
                      alias: Brick
                    - conditions:
                        - condition: template
                          value_template: "{{repeat.item == \"ore\"}}"
                      sequence:
                        - data:
                            brightness_pct: 80
                            rgbww_color:
                              - 147
                              - 50
                              - 255
                              - 100
                              - 50
                          action: light.turn_on
                          target:
                            entity_id: light.ledpaneel_woonkamer_achter
                        - action: media_player.volume_set
                          metadata: {}
                          data:
                            volume_level: 0.4
                          target:
                            entity_id: media_player.sonos_keuken
                        - action: media_player.play_media
                          target:
                            entity_id: media_player.sonos_keuken
                          data:
                            media_content_id: media-source://media_source/local/catan-ore.mp3
                            media_content_type: audio/mpeg
                          metadata:
                            title: catan-ore.mp3
                            thumbnail: null
                            media_class: music
                            children_media_class: null
                            navigateIds:
                              - {}
                              - media_content_type: app
                                media_content_id: media-source://media_source
                        - delay:
                            hours: 0
                            minutes: 0
                            seconds: 2
                            milliseconds: 0
                      alias: Ore
              for_each: "{{ trigger.json.resourceTypes }}"
        alias: Not Robber
      - conditions:
          - condition: template
            value_template: "{{ trigger.json.selectedNumber == 7 }}"
            alias: Robber
        sequence:
          - data:
              rgb_color:
                - 255
                - 0
                - 0
              brightness_pct: 100
            action: light.turn_on
            target:
              entity_id: light.ledpaneel_woonkamer_achter
          - action: media_player.volume_set
            metadata: {}
            data:
              volume_level: 0.4
            target:
              entity_id: media_player.sonos_keuken
          - action: media_player.play_media
            target:
              entity_id: media_player.sonos_keuken
            data:
              media_content_id: media-source://media_source/local/dragon-growl-37570.mp3
              media_content_type: audio/mpeg
            metadata:
              title: dragon-growl-37570.mp3
              thumbnail: null
              media_class: music
              children_media_class: null
              navigateIds:
                - {}
                - media_content_type: app
                  media_content_id: media-source://media_source
          - delay:
              hours: 0
              minutes: 0
              seconds: 2
              milliseconds: 0
        alias: Robber
  - delay:
      hours: 0
      minutes: 0
      seconds: 8
      milliseconds: 0
  - action: scene.turn_on
    metadata: {}
    data:
      entity_id: scene.catan_entities_restore
mode: single
```
This automation will:
- Save the current state of the lights and media player to a temporary scene
- Change the lights and play sounds based on the resource types selected. For example, it will turn the lights light green for sheep, dark green for wood, yellow for wheat, orange for brick, and purple for ore.
- If the robber (7) is rolled, it will turn the lights red and play a dragon growl sound.

Tip: I found sounds and music on [freesound.org](https://freesound.org/) and [zapsplat.com](https://www.zapsplat.com/). You can use any sound you like, just make sure to update the file paths in the automation.


## Customization

- To change the GPIO pin for the LED strip, modify `LED_STRIP_PIN` in `main.cpp`
- To modify the board generation rules, update the default settings in `main.cpp`
- To customize the web interface, edit the files in the `data/` directory

## Troubleshooting

### Common Issues

- **Can't upload firmware**: Make sure the ESP32 is connected and the correct port is selected, and you have the correct drivers.
- **LEDs not lighting up**: Check power supply and data connection. Check all the connections with a voltimeter.
- **Web interface not showing**: Make sure SPIFFS filesystem was uploaded
- **Generating boards takes too long**: Adjust the board generation options, particularly avoid disabling too many adjacency rules at once

### Serial Monitor

You can monitor debug output by connecting to the ESP32's serial port at 115200 baud.

## Future Plans V2.0
- Dice tower with ESP32 camera that checks the dice values
- Based on the dice from the camera trigger the correct leds
- Create tests
- Build pipeline


## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Based on the game Catan by Klaus Teuber
- Uses the Adafruit NeoPixel library for LED control
