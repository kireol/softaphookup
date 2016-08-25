/*
 *  This sketch demonstrates how to scan WiFi networks. 
 *  The API is almost the same as with the WiFi Shield library, 
 *  the most obvious difference being the different file you need to include:
 */
#include "softapHookup.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
//WiFiServer server(80);
SoftapHookup softaphookup("myespssid", "password", &server);
void setup() {
  Serial.begin(115200);
  delay(20);
  softaphookup.setup();

  Serial.println("Setup done");
}

void loop() {
  softaphookup.start();
}
