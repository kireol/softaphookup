# softaphookup

##Description 
Hard coding the SSID and password in your IOT wifi device can be a thing of the past with SoftapHookup.  This is a library for setting up an ESP8266 device or Arduino with ESP8266 and getting it on a wifi network using a web interface.  Simply by calling .start(), the library will create a wifi access point that you can connect to from any wifi enabled device.  Once running, you can connect to 192.168.4.1 on any client device connected to the access point.  It will then scan and list any wifi access points (router).  You can select a wifi connection from the dropdown or type one in.  Once selected, the library will attempt to connect to the specified access point.


##TODO:
*Add connection led option   
*Change debug mode for serial output to use #ifdef
