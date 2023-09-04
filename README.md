# Projekt: Wifi-433 Rollo Schalter

Hier habe ich ein elektrischem Rollo mit 433Mhz Fernbedienung über ein NodeMCU mit ESP12-E bedienbar gemacht.


![](https://raw.githubusercontent.com/polygontwist/ESP_Node_Rolloswitch/master/bilder/screenshot.png)

# Setup
das Projekt ist wie https://github.com/polygontwist/ESP_sonoff_Schaltuhr aufgebaut.

Bei WIFI_SSID  WIFI_PASSWORD  müssen die eigenen Werte eingetragen werden, dabei #include "wifisetup.h" auskommentieren.
Die Werte rollo_KEY, rollo_UP, rollo_DOWN, rollo_STOP müssen vorher auch selbst aus der Fernbedienung des Rollos ausgelesen werden.
Damit das ganze funktioniert muss in der system.js ebenfalls der gleiche rollo_KEY eingestellt werden (Zeile 102).
Das Teil ist dann unter dem Namen der bei ARDUINO_HOSTNAME im Netzwerk ansprechbar.

# Quellen und Inspirationen
folgende Bibliothecken wurden verwendet:
* JeVe_EasyOTA (Version 2.2.0) https://github.com/jeroenvermeulen/JeVe_EasyOTA/
* ESP8266WiFi
* WiFiClient
* ESP8266WebServer
* time
* FS (SPIFFS)  http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system

# Hardware
![Steckplatine](https://github.com/polygontwist/ESP_Node_Rolloswitch/blob/master/bilder/steckplatine_rollo.jpg)
