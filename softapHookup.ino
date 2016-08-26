#include "softapHookup.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
#define RESET_PIN 5
SoftapHookup softaphookup("myespssid", "password", &server, RESET_PIN);

void setup() {
  Serial.begin(115200);
  softaphookup.start();
}

void loop() {
// Your code here
}
