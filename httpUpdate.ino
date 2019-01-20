/**
 * httpUpdate.ino
 *
 *  Created on: 27.11.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <MQTT.h>
#include <ArduinoJson.h>
#include "config.h" //must create one

#include <EEPROM.h>

#define USE_SERIAL Serial

int updateFrequencySec = 1;

long lastMessageSentTimestamp;

static const int firmwareVersion = 1; // in each new firmware release, this version should be upped so we know not to upgrade after startup

WiFiClientSecure espClient;
MQTTClient iotHubMqttClient = MQTTClient(512); //really important to set buffer to sufficient number

bool updating = false;
static const String firmwareUrl = "https://cocostore.blob.core.windows.net/$web/hello_world.bin";
static const String urlThumbprint = "6a 81 2e f9 92 86 c1 48 4e 03 37 47 3a 2d 7d e3 26 dd 62 8b";

int updateEEPROMAddr = 0;
static  const uint8_t UPDATE = 1 ;
static  const uint8_t OPERATE = 0 ;


void setup() {

	USE_SERIAL.begin(115200);
	// USE_SERIAL.setDebugOutput(true);

	EEPROM.begin(512);


	Serial.println("Checking update flag");
	uint8_t doUpdate = EEPROM.read(updateEEPROMAddr);

	if (doUpdate != NULL && doUpdate == UPDATE){
		updating = true;
		Serial.println("update = true");
	} else {
		Serial.println("update = false");
	}

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	USE_SERIAL.println("================");
	USE_SERIAL.println("=   ODIN       =");
	USE_SERIAL.println("================");

	for (uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFi.mode(WIFI_STA);

	WiFi.begin(secondary_ssid, secondary_password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.print("WiFi connected to ");
	Serial.println(secondary_ssid);
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	if (!updating){

		Serial.print("setup mqtt...");

		//setup iot hub MQTT transport
			bool connect = false;
			iotHubMqttClient.begin(iotHubDomain, 8883, espClient);
			iotHubMqttClient.onMessage(messageReceived);

			while (!connect) {
				connect = iotHubMqttClient.connect(deviceId, mqttUserId, sasToken);

				delay(1000);

				Serial.print("last error status: ");
				Serial.println(iotHubMqttClient.lastError());
			};

			//initTime();

			iotHubMqttClient.subscribe(deviceBoundTopic); //works through device explorer and iot hub gui

			//subscribe to properties update notifications

			iotHubMqttClient.subscribe("$iothub/twin/PATCH/properties/desired/#");

			//fetch initial state
			iotHubMqttClient.subscribe("$iothub/twin/res/#");

			iotHubMqttClient.publish("$iothub/twin/GET/?$rid={14545455}");
	} else {
		Serial.print("we are updating, skip setup mqtt.");

	}



}

void loop() {

	/**
	 * actually found out there is no need to introduce update in loop(),
	 * problem was more in downloading from azure. also the static website is not working.
	 *
	 */
	if (updating) {

		t_httpUpdate_return ret;

		if (urlThumbprint.length() > 0) {
			Serial.println(urlThumbprint);
			ret = ESPhttpUpdate.update(firmwareUrl, "", urlThumbprint);

		} else {
			ret = ESPhttpUpdate.update(firmwareUrl);

		}



		switch (ret) {
		case HTTP_UPDATE_FAILED:
			Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s",
					ESPhttpUpdate.getLastError(),
					ESPhttpUpdate.getLastErrorString().c_str());

			if (ESPhttpUpdate.getLastError() == 11) {
				delay(2000);
				//digitalWrite(resetPin, LOW);
			}
			break;

		case HTTP_UPDATE_NO_UPDATES:
			Serial.println("HTTP_UPDATE_NO_UPDATES");
			break;

		case HTTP_UPDATE_OK:
			Serial.println("HTTP_UPDATE_OK");
			ESP.restart();
			break;
		}

		Serial.println("clearing UPDATE flag of EEPROM");
		EEPROM.write( updateEEPROMAddr,OPERATE );
		EEPROM.commit();

		updating = false;

	} else {
		iotHubMqttClient.loop();

	}



	delay(1000);
	// wait for WiFi connection

}

void messageReceived(String &topic, String &payload) {
	Serial.println("incoming: " + topic + " - " + payload);

	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(payload);

	String output;
	root.printTo(output);

	Serial.println(output);

	int desiredUpdateFrequencySec;

	int desiredFirmwareVersion;

	if (topic.startsWith("$iothub/twin/res/")) {
		Serial.println(" initial properties received");
		Serial.println(
				" reading  : "
						+ root["desired"]["updateFrequencySec"].as<String>());
		desiredUpdateFrequencySec =
				root["desired"]["updateFrequencySec"].as<int>();

		desiredFirmwareVersion = root["desired"]["firmwareVersion"].as<int>();
		//firmwareUrl = root["desired"]["firmwareUrl"].as<String>();
		//urlThumbprint = root["desired"]["urlThumbPrint"].as<String>();

	} else if (topic.startsWith("$iothub/twin/PATCH/properties/desired/")) {
		Serial.println(" update properties received");
		Serial.println(
				" reading  : " + root["updateFrequencySec"].as<String>());
		desiredUpdateFrequencySec = root["updateFrequencySec"].as<int>();

		desiredFirmwareVersion = root["firmwareVersion"].as<int>();

		//firmwareUrl = root["firmwareUrl"].as<String>();
		//urlThumbprint = root["urlThumbPrint"].as<String>();
	}

	if (desiredFirmwareVersion > firmwareVersion) {

		Serial.println(
				" Updating firmware to version  : " + desiredFirmwareVersion);
		Serial.println(" download firmware from " + firmwareUrl);


		Serial.println("writing UPDATE flag to EEPROM");
		EEPROM.write(updateEEPROMAddr,UPDATE);
		EEPROM.commit();

		Serial.println("rebooting");
		ESP.restart();

		//ESPhttpUpdate.setLedPin(ledPin, LOW);

		//t_httpUpdate_return ret = ESPhttpUpdate.update(firmwareUrl, 0x00);

//		Serial.println("Disconnecting MQTT");
//		while (!iotHubMqttClient.disconnect()) {
//			Serial.print(".");
//
//			delay(1000);
//		}
//		Serial.println(" Done.");

	}

}

