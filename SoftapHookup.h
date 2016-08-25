#ifndef _SoftapHookup_H_
#define _SoftapHookup_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

extern String queryString;

enum currentMode {
    SH_MODE_SCAN = 1, SH_MODE_SOFTAPSETUP = 2, SH_MODE_SOFTAPSERVER = 3, SH_MODE_CONNECTING = 4, SH_MODE_CONNECTED = 5
};

class SoftapHookup {
protected:
    void scanForNetworks();
    void softapServer();
    void setupWiFi();
    void showNetworks();
    void selectSsid();
    void refresh();
    String getHTMLHeader();
    String getHTMLFooter();

    ESP8266WebServer *server;

    char *softapssid;
    char *softappassword;
    int currentMode;
    int numberOfFoundNetworks;

public:

    SoftapHookup(char *softapssid, char *password, ESP8266WebServer *server);

    void start();
};

#endif
