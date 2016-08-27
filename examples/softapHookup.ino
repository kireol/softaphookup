#include "SoftapHookup.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
SoftapHookup softaphookup("myespssid", "password", &server);

void setup() {
  Serial.begin(115200);
  softaphookup.start();
}

void loop() {
// Your code here
}
