# Adafruit Feather IOT

My little test project with Adafruit Feather Huzzah

## Getting Started

Create a .h file in the project root called 'config.h'. Add the following declarations:

```
const char* password = "your_ssid_password";
const char* ssid = "your_ssid";
const char* iotHubDomain = "domain of your iot hub";
const char* deviceId = "iot hub id of the adafruit device";
const char* mqttUserId = "userId for mqtt topic";
const char* sasToken = "shared access signature token";

```

More information on connecting to Iot Hub via MQTT:
* https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support

IoT Hub itself needs to be setup, see also:
* https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-arduino-huzzah-esp8266-get-started

Firmware update proven to work with http blob endpoint:
* http://cocostore.blob.core.windows.net/$web/hello_world.bin
Working with https on the same endpoint gives read timeout. Does work without MQTT involved in sketch.

Now found that firmware update with https can work provided:
1. MQTT is not running
2. firmwareUrl and fingerprint must therefore be stored as strings as part of firmware

When only condition 2 is satisfied, the download fails (see branch https://github.com/fransvanderlek/fota/tree/failed_because_mqtt_running )

 
