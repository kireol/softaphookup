#ifndef _SoftapHookup_H_
#define _SoftapHookup_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

extern String queryString;

enum currentMode {
    SH_MODE_RESET_CHECK = 1, SH_MODE_EEPROM_CONNECT = 2, SH_MODE_SCAN = 3, SH_MODE_SOFTAPSETUP = 4, SH_MODE_SOFTAPSERVER = 5, SH_MODE_CONNECTING = 6, SH_MODE_CONNECTED = 7
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
    void connectToRemoteWifi();
    void saveToEeprom();
    void checkForReset();
    void readFromEeprom();
    void init();

    ESP8266WebServer *server;

    char* softapssid;
    char* softappassword;
    char remoteSsid[33];
    char remotePassword[33];
    int currentMode;
    int numberOfFoundNetworks;
    unsigned long timeoutMillis;
    boolean lastConnectAttemptFailed;
    int clearNetworkFromEepromPin;
    int eepromStartingByte;
    boolean shouldWriteToEeprom;
    boolean shouldIgnoreEeprom;

public:

    SoftapHookup(char *softapssid, char *password, ESP8266WebServer *server);
    SoftapHookup(char *softapssid, char *password, ESP8266WebServer *server, int clearNetworkFromEepromPin);

    void start();
    void clearEeprom();
    int  getEepromSizeUsed();
    void setEepromStartingByte(int);
    void setShouldWriteEeprom(boolean shouldWrite);
    void ignoreEeprom(boolean shouldIgnore);
};

#endif
