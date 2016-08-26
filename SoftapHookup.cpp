#include "SoftapHookup.h"

#include <ESP8266WiFi.h>
#include <EEPROM.h>

//eeprom offset method?
//couldnt connect message from eeprom
//connect led
//looper
//dont reset
//allow to specify ip range
//clean up eeprom . . . . repeats

SoftapHookup::SoftapHookup(char *defaultssid, char *password, ESP8266WebServer *inServer) {
    softapssid = defaultssid;
    softappassword = password;
    server = inServer;
    init();
}

SoftapHookup::SoftapHookup(char *defaultssid, char *password, ESP8266WebServer *inServer, int inClearNetworkFromEepromPin) {
    softapssid = defaultssid;
    softappassword = password;
    server = inServer;
    clearNetworkFromEepromPin = inClearNetworkFromEepromPin;
    init();
}

void SoftapHookup::init(){
    currentMode = SH_MODE_RESET_CHECK;
    numberOfFoundNetworks = 0;
    timeoutMillis = 10000;
    lastConnectAttemptFailed = false;  //load this from eeprom
    clearNetworkFromEepromPin = -1;
    eepromStartingByte = 0;

}
void SoftapHookup::start() {
    switch (currentMode) {
        case SH_MODE_RESET_CHECK:
            checkForReset();
            break;
        case SH_MODE_EEPROM_CONNECT:
            readFromEeprom();
            break;
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
            connectToRemoteWifi();
            break;
        case SH_MODE_CONNECTED:
            break;
        default:
            Serial.println("Idle");
    }
}

void SoftapHookup::checkForReset(){
  Serial.println("Checking eeprom reset pin");
  currentMode = SH_MODE_EEPROM_CONNECT;

  if(  clearNetworkFromEepromPin == -1){
    Serial.println("No reset pin specified.  Skipping eeprom reset");
    return;
  }

  pinMode(clearNetworkFromEepromPin, INPUT);

  if(digitalRead(clearNetworkFromEepromPin) == HIGH){
    Serial.println("Pin detected.  Clearing eeprom");
    clearEeprom();
  }else{
    Serial.println("Skipping eeprom clearing.  Pin not detected.");    
  }
}

void SoftapHookup::connectToRemoteWifi(){
  Serial.print("Connecting to network ");
  Serial.print( String(remoteSsid));
  Serial.print(" with password ");
  Serial.print(String(remotePassword));
  Serial.println(".");

  if(strlen(remotePassword) > 0){
    WiFi.begin(remoteSsid, remotePassword);    
  }else{
    WiFi.begin(remoteSsid);    
  }

  unsigned long previousMillis = millis();
  unsigned long currentMillis;
  
  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if((currentMillis - previousMillis) >= timeoutMillis){
      Serial.println("Connection Timeout");
      currentMode = SH_MODE_SCAN;
      lastConnectAttemptFailed = true;  //write to eeprom here
      ESP.reset();
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  saveToEeprom();
  currentMode = SH_MODE_CONNECTED;
}

void SoftapHookup::readFromEeprom(){
  Serial.println("Checking Eeprom");
  
  EEPROM.begin(512);

  for (int i = 0; i < sizeof(remoteSsid); ++i)
  {
    remoteSsid[i] = char(EEPROM.read(eepromStartingByte + i));
  }
  Serial.print("SSID: ");
  Serial.println(remoteSsid);
  
  Serial.println("Reading EEPROM pass");

  for (int i = 0; i < sizeof(remotePassword); ++i)
  {
    remotePassword[i] = char(EEPROM.read(eepromStartingByte + sizeof(remoteSsid) + i));
  }
  
  Serial.print("Password: ");
  Serial.println(remotePassword);

  if(strlen(remoteSsid) > 0){
    currentMode = SH_MODE_CONNECTING;
  }else{
    currentMode = SH_MODE_SCAN;
  }
}

void SoftapHookup::saveToEeprom(){
  EEPROM.begin(512);

  clearEeprom();
  
  for (int i = 0; i < sizeof(remoteSsid); ++i)
  {
    EEPROM.write(eepromStartingByte + i, remoteSsid[i]);
    Serial.print("Wrote: ");
    Serial.println(remoteSsid[i]); 
  }


  for (int i = 0; i < sizeof(remotePassword); ++i)
  {
    EEPROM.write(eepromStartingByte + sizeof(remoteSsid) + i, remotePassword[i]);
    Serial.print("Wrote: ");
    Serial.println(remotePassword[i]); 
  }    
  EEPROM.commit();

}
void SoftapHookup::clearEeprom() {
  int length = getEepromSizeUsed();

  EEPROM.begin(512);

  for (int i = 0; i < length; ++i) { 
    EEPROM.write(eepromStartingByte + i, 0); 
  }
  EEPROM.commit();

}


void SoftapHookup::softapServer() {
    server->handleClient();
}

void SoftapHookup::setupWiFi() {
    Serial.println("in setup WiFi");

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
    Serial.println("User selection detected");
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
    if(useHiddenNetwork){
      hiddenSsid.toCharArray(remoteSsid, sizeof(remoteSsid));
      hiddenPassword.toCharArray(remotePassword, sizeof(remotePassword));
    }
    else{
      WiFi.SSID(ssidNum).toCharArray(remoteSsid, sizeof(remoteSsid));
      password.toCharArray(remotePassword, sizeof(remotePassword));
    }
    currentMode = SH_MODE_CONNECTING;
}

void SoftapHookup::showNetworks() {
    String s = "";
    s += getHTMLHeader();
    if (numberOfFoundNetworks == 0) {
        s += "no networks found";
    } else {
        if(lastConnectAttemptFailed == true){
          s += "<b>Last connection failed</b><br>";
        }

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

int SoftapHookup::getEepromSizeUsed(){
  return sizeof(remoteSsid) + sizeof(remotePassword);
}

void  SoftapHookup::setEepromStartingByte(int startByte){
  eepromStartingByte = startByte;
}
