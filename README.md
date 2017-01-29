## home-automation
Implementation of a home automation project. Using a combination of Arduino with sensors, MQTT, node-red, websockets and some html5 code to create a simple dashboard for monitoring the home.
## Hardware dependencies
While you are free to use the code fragments here, in order to use the entire project you will need the following
- 1 or more Arduinos (I am using the Nano, but not using any Nano-specific features)
- Sensors to connect to the Arduinos (temperature, light, motion, etc.)
- ESP8266 WiFi chip, to allow the Arduinos to remotely connect over Wifi
Alternatively you can choose to simulate the events by directly publishing to the messaging queues (see below).

## Dependencies
You will need the following toosl/software.
- Arduino IDE for the Arduino sketches (to run the code that polls the sensors)
- Arduino IDE libraries for the ESP8266 (to program the ESP8266)
- PubSub client for the ESP8226 (to connect to MQTT from the ESP 8266)
- node-red (for collecting events from MQTT and rendering the dashboard)
- HTML5 supported browser (for the dashboards)
- Editor of your choice (Emacs maybe? :-) ).
