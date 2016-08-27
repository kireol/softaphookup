# softaphookup

##Installation
To use softhookup as an Arduino IDE library, copy or move the SoftapHookup folder located in the library folder to your Arduino library folder.  e.g. on OSX
```
mv library/SoftapHookup ~/Documents/Arduino/libraries/
```

##Description 
Hard coding the SSID and password in your IOT wifi device can be a thing of the past with SoftapHookup.  This is a library for setting up an ESP8266WiFi compatible device and getting it on a local wifi network using a web interface.  SSID and password will be saved to the EEPROM so if your device loses power, it will automatically attempt to reconnect.  Simply by calling .start(), the library will create a wifi access point that you can connect to from any wifi enabled device.  Once running, you can connect to 192.168.4.1 on any client device connected to the access point.  It will then scan and list any wifi access points (router).  You can select a wifi connection from the dropdown or type one in.  Once selected, the library will attempt to connect to the specified access point.


##TODO:
*Add connection led option   
*Change debug mode for serial output to use #ifdef  
*Remove .ino from source and check if it works in the arduino library dir 

##Example useage
```   
#include "softapHookup.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
SoftapHookup softaphookup("mySoftapEsp8266", "mysoftappassword", &server);

void setup() {
  Serial.begin(115200);
  softaphookup.start();
}

void loop() {
// Your code here
}
```   

#Directions
Run example.  Connect wifi enabled device(e.g. laptop) to soft access point of device.  Select the access point you would like your IOT device to connect to and hit submit.

#SDK
```   
SoftapHookup(char *softapssid, char *password, ESP8266WebServer *server);    
```   
Default contructor.  Send the ssid(Max 32 ASCII) and password(Max 32 ASCII) of the softap you will temporarily connect to.  Send an instance of ESP8266WebServer as the 3rd paramenter.   
```   
SoftapHookup(char *softapssid, char *password, ESP8266WebServer *server, int clearNetworkFromEepromPin);   
```   
Optional constructor.  All parameters are identical as the default contstructor with the addition of the last parameter.  This parameter is a digital pin.  If the pin is HIGH
```   
void start();
```   
You need to call this method to make all of the magic happen.
```   
void clearEeprom();
```   
This is a software method for clearing the eeprom contents.  This will only clear the section of the eeprom used by this library.
```   
int  getEepromSizeUsed();
```   
This will return the number of bytes used by the library in the EEPROM.
```   
void setEepromStartingByte(int);
```   
If you wish the library to have an offset where it starts storing the ssid and password, you can send it the byte offset with this method.
```   
void setShouldWriteEeprom(boolean shouldWrite);
```   
This will skip the writing of SSID and password.  Read and clear will still happen either way.
```   
void ignoreEeprom(boolean shouldIgnore);
```   
The library will not read, write, or clear from the EEPROM is you send true.  False will do the default which is to read, write, and clear the EEPROM space used for writing SSID and password.
```   
void hideSoftapSsid(boolean);
```   
The default operation is false.  If you call this method with true, the softap will not braodcast itself.  It will still be active, but will not appear in known wifi lists.
```   
void setSsidChannel(int idChannel);
```   
If you wish to change which channel the softap starts under, send the value here.  The default is the channel that the ESP8266WiFi library uses.
