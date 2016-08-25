#include "SoftapHookup.h"

#include <ESP8266WiFi.h>

SoftapHookup::SoftapHookup(char *defaultssid, char *password, ESP8266WebServer *inServer) {
    softapssid = defaultssid;
    softappassword = password;
    currentMode = SH_MODE_SCAN;
    numberOfFoundNetworks = 0;
    server = inServer;
}

void SoftapHookup::start() {
    switch (currentMode) {
        case SH_MODE_SCAN:
            scanForNetworks();
            break;
        case SH_MODE_SOFTAPSETUP:
            setupWiFi();
            break;
        case SH_MODE_SOFTAPSERVER:
            softapServer();
            break;
        case SH_MODE_CONNECTING:
            break;
        case SH_MODE_CONNECTED:
            break;
        default:
            Serial.println("Idle");
    }
}

void SoftapHookup::softapServer() {
    server->handleClient();
}

void SoftapHookup::setupWiFi() {
    Serial.println("in setupWiFi");
    WiFi.mode(WIFI_AP);

    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = String(softapssid);

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++) {
        AP_NameChar[i] = AP_NameString.charAt(i);
    }
    WiFi.softAP(AP_NameChar, softappassword);
    currentMode = SH_MODE_SOFTAPSERVER;

    server->on("/", HTTP_GET, std::bind(&SoftapHookup::showNetworks, this));
    server->on("/refresh", HTTP_GET, std::bind(&SoftapHookup::refresh, this));
    server->on("/select", HTTP_POST, std::bind(&SoftapHookup::selectSsid, this));

    server->begin();
}

void SoftapHookup::refresh() {
    scanForNetworks();
    showNetworks();
}

String SoftapHookup::getHTMLHeader() {
    return "<!DOCTYPE HTML>\r\n<html>\r\n";
}

String SoftapHookup::getHTMLFooter() {
    return "</html>\n";
}

void SoftapHookup::selectSsid() {
    String hiddenPassword = server->arg("hiddenpassword");
    String hiddenSsid = server->arg("hiddenssid");
    int ssidNum = server->arg("ssid").toInt();
    String password = server->arg("password");
    boolean useHiddenNetwork = false;
    String s = "";
    
    s += getHTMLHeader();

    if(hiddenSsid.length() > 0 && hiddenPassword.length() > 0){
      useHiddenNetwork = true;
    }

    s += "Connecting to network <br>";
    s += (useHiddenNetwork? hiddenSsid: WiFi.SSID(ssidNum));

    s += "<br>";
    
    s += getHTMLFooter();
    
    server->send(200, "text/html", s);

}

void SoftapHookup::showNetworks() {
    String s = "";
    s += getHTMLHeader();
    if (numberOfFoundNetworks == 0) {
        s += "no networks found";
    } else {
        s += "Select a network for <b>" +
             String(softapssid) +
             "</b> to connect.  * denotes password protected network<br><br><form action=\"/select\" method=\"post\">";
        s += "<select name=\"ssid\">";
        for (int i = 0; i < numberOfFoundNetworks; ++i) {
            s += "<option value=\"" + String(i) + "\">";
            s += i + 1;
            s += ": ";
            s += WiFi.SSID(i);
            s += " (";
            s += WiFi.RSSI(i);
            s += ")";
            s += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *";
            s += "</option>";
        }
        s += "</select><br><br>";
        s += "Network Password: <input type=\"text\" name=\"password\"><br>";
        s += "<br>or<br><br>";
        s += "Hidden network SSID: <input type=\"text\" name=\"hiddenssid\"><br>";
        s += "Network Password: <input type=\"text\" name=\"hiddenpassword\"><br>";
        s += "<br>";
        s += "<input type=\"submit\" value=\"Select\">";
        s += "</form>";
    }
    s += "<a href=\"/refresh\">Refresh</a>";

    s += getHTMLFooter();

    server->send(200, "text/html", s);
    Serial.println("in index");
}

void SoftapHookup::scanForNetworks() {
    Serial.println("scan start");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    numberOfFoundNetworks = WiFi.scanNetworks();
    Serial.println("scan done");
    if (numberOfFoundNetworks == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(numberOfFoundNetworks);
        Serial.println(" networks found");
        for (int i = 0; i < numberOfFoundNetworks; ++i) {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");
    currentMode = SH_MODE_SOFTAPSETUP;
}
