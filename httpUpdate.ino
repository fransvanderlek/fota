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

#define USE_SERIAL Serial
namespace {

const int resetPin = 16;

}

ESP8266WiFiMulti WiFiMulti;

void setup() {

	USE_SERIAL.begin(115200);
	// USE_SERIAL.setDebugOutput(true);

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
	  WiFiMulti.addAP("koetjeloe", "ilhabela");

	  pinMode(16, OUTPUT);
	  digitalWrite(16, HIGH);

}

void loop() {
	// wait for WiFi connection
	if ((WiFiMulti.run() == WL_CONNECTED)) {

		Serial.println("Downloading firmware from https://cocostore.blob.core.windows.net/$web/hello_world.bin");
		t_httpUpdate_return  ret = ESPhttpUpdate.update("https://cocostore.blob.core.windows.net/$web/hello_world.bin", "", "6a 81 2e f9 92 86 c1 48 4e 03 37 47 3a 2d 7d e3 26 dd 62 8b");

		//t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

		switch (ret) {
		case HTTP_UPDATE_FAILED:
			USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s",
					ESPhttpUpdate.getLastError(),
					ESPhttpUpdate.getLastErrorString().c_str());

			if (ESPhttpUpdate.getLastError() == 11) {
				delay(2000);
				digitalWrite(resetPin, LOW);
			}

			break;

		case HTTP_UPDATE_NO_UPDATES:
			USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
			break;

		case HTTP_UPDATE_OK:
			USE_SERIAL.println("HTTP_UPDATE_OK");
			break;
		}
	}
}

