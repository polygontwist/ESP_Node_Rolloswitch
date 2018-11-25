/*
  Rollowebswitcher
  433MHz
  5C:CF:7F:2D:29:EE
  115200 Baud

  NodeMCU ESP12-E, 115200,  4M (3M SPIFFS), 80MHz
                                  ->weniger einstellen? (OTA)

 
*/
/*
  Board: NodeMCU 1.0 (ESP-12E),80MHz, 115200, 4M
  Programmer: AVRISP mkll
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>//
#include <time.h>

#include <JeVe_EasyOTA.h>  // https://github.com/jeroenvermeulen/JeVe_EasyOTA/blob/master/JeVe_EasyOTA.h

#include "FS.h"   //http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html

#include "myNTP.h"
myNTP oNtp;

#include "data.h" //index.htm

const char* progversion  = "Rollo V1.00 ota fs ntp ti";


//----------------------------------------------------------------------------------------------
#define ARDUINO_HOSTNAME  "rollo" //http://rollo.wg/
//----------------------------------------------------------------------------------------------

EasyOTA OTA;

//web-time


#define actionheader "HTTP/1.1 303 OK\r\nLocation:/index.htm\r\nCache-Control: no-cache\r\n\r\n"

ESP8266WebServer server(80);

uint8_t MAC_array[6];
char MAC_char[18];
String macadresse="";

#define pin_led 5        //gpio05 D1
#define pin_ledinvert false

//433MHz
#define sender433PIN 4        //gpio04 D2 Sender

const char* rollo_KEY =   "Q00QFF0F0Q00Q10";
const char* rollo_UP =    "Q00QFF0F0Q00Q10F0F0F";//3x(oder 3x mehr) + 6x F0F1Q
const char* rollo_DOWN =  "Q00QFF0F0Q00Q10F0101";//3x(oder 3x mehr) + 6x F0110
const char* rollo_STOP =  "Q00QFF0F0Q00Q10FFFFF";//3x

#define WIFI_SSID         "yourWIFISSID"
#define WIFI_PASSWORD     "yourWLANPassword"

#include "wifisetup.h" //reale Daten 


#define rolloWiederholungen 6 //evtl. weniger? 10?
#define rolloPulseLength  280 //!

#define RolloBefehl_UP  1
#define RolloBefehl_DOWN  2
#define RolloBefehl_STOP  3

File fsUploadFile;                      //Hält den aktuellen Upload

#define TIMERtxt "/timer.txt"

unsigned long tim_zeitchecker= 15*1000;//alle 15sec Timer checken
unsigned long tim_previousMillis=0;
byte last_minute;

//---------------------------------------------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
//---------------------------------------------
void setLED(bool an){
  if(pin_ledinvert)an=!an;
  digitalWrite(pin_led, an);
}
void toogleLED(){
  digitalWrite(pin_led, !digitalRead(pin_led));
}

bool getLED(){
  if(pin_ledinvert){
     return digitalRead(pin_led)==LOW;
    }
    else{
     return digitalRead(pin_led)==HIGH;
    }
}
//---------------------------------------------




void setup() {
  //serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("-----------------------------");
  Serial.print ("  ");
  Serial.println(progversion);
  Serial.println("-----------------------------");
/*  Serial.print(ESP.getCpuFreqMHz());
  Serial.print("MHz CId:");
  Serial.println(ESP.getChipId());
*/
  //433MHz
  pinMode(sender433PIN, OUTPUT);

  pinMode(pin_led, OUTPUT);
  setLED(true);

  //OTA
  OTA.onMessage([](char *message, int line) {
    toogleLED();
    Serial.print(">");
    Serial.println(message);
  });
  OTA.setup(WIFI_SSID, WIFI_PASSWORD, ARDUINO_HOSTNAME);//connect to WLAN

  //SPIFFS
  SPIFFS.begin();
  

  //get MAC
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    if(i>0) macadresse+=":";
    macadresse+= String(MAC_array[i], HEX);
    //sprintf(MAC_char, "%s%02x:", MAC_char, MAC_array[i]);
  }
  Serial.print("MAC: ");
  Serial.println(macadresse);

  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*
    if (MDNS.begin("esp8266")) {//??
      Serial.println("MDNS responder started");
      //http://www.gtkdb.de/index_18_2858.html
      //https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266mDNS/examples/mDNS_Web_Server/mDNS_Web_Server.ino
    }
  */

  server.on("/action", handleAction);//daten&befehle
  
  server.on("/", handleIndex);//daten&befehle
  server.on("/index.htm", handleIndex);//Einstellungen
  server.on("/index.html", handleIndex);// -''-

  server.on("/data.json", handleData);

  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
/*
  Serial.println("Dateien(SPIFFS):");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
  }

  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
    Serial.println("Info:");
    Serial.println(formatBytes(fs_info.totalBytes).c_str());//2949250 (2.8MB)
    Serial.println(formatBytes(fs_info.usedBytes).c_str());//502
    Serial.println(fs_info.blockSize);//8192
    Serial.println(fs_info.pageSize);//256
    Serial.println(fs_info.maxOpenFiles);//5
    Serial.println(fs_info.maxPathLength);//32
  }
*/

  setLED(false);
  Serial.println("ready.");

  //NTP start
  oNtp.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  OTA.loop();
  oNtp.update();

  unsigned long currentMillis = millis();//milliseconds since the Arduino board began running


  if(oNtp.hatTime() && currentMillis - tim_previousMillis > tim_zeitchecker){//Timer checken
      tim_previousMillis = currentMillis;
      if(last_minute!=oNtp.getminute()){//nur 1x pro min
        checktimer();
        last_minute=oNtp.getminute();
     }
   } 
}

//-------------------Timer---------------
const int line_BUFFER_SIZE = 255;
char line_buffer[line_BUFFER_SIZE];

void checktimer(){ 
  if (SPIFFS.exists(TIMERtxt)) {
   // time
    File file = SPIFFS.open(TIMERtxt, "r");
    String zeile;
    char tmp[line_BUFFER_SIZE];
    int tmpcounter;
    bool onoff;
    char zeit[5];
    byte t_st=0;
    byte t_min=0;
    byte tage=0;
    String befehl="";
    String id="";
    int anz=0;
    int i;
    int t;
    int sepcounter;
    
    if(file){
      //Serial.println("opend timer.txt");
      //Zeilenweise einlesen
      //on/off|hh:mm|mo-so als bit|befehl|id
      //on|07:05|31|UP|t1
      while (file.available()){
          zeile=file.readStringUntil('\n');
          //Serial.println(zeile);
          //Zeile zerlegen
          anz= zeile.length();
          zeile.toCharArray(line_buffer,anz);
          tmpcounter=0;
          sepcounter=0;
          onoff=false;
          for(i=0;i<anz;i++){
            if(line_buffer[i]=='|'){
              tmp[tmpcounter]='\0';              
              if(sepcounter==0){//on/off
                onoff=( tmp[0]=='o' && tmp[1]=='n');
              }
              if(sepcounter==1){//hh:mm
                zeit[0]=tmp[0];
                zeit[1]=tmp[1];
                zeit[2]='\0';
                t_st=String(zeit).toInt();
                zeit[0]=tmp[3];
                zeit[1]=tmp[4];
                t_min=String(zeit).toInt();
              }
              if(sepcounter==2){
                tage=String(tmp).toInt();
              }
              if(sepcounter==3){
                befehl=String(tmp);
              }
              if(sepcounter==4){
                id=String(tmp);
              }
              sepcounter++;
              tmpcounter=0;
            }
            else{
             tmp[tmpcounter]=line_buffer[i];
             tmpcounter++;
            }
          }
          //Zeilendaten auswerten
          if(onoff){
           /*Serial.print("check ");
            Serial.print(t_st);
            Serial.print(":");
            Serial.print(t_min);
            Serial.print(" ");
            Serial.print(tage, BIN);
            Serial.print(" "+befehl);
            Serial.print(" "+id);
*/
            byte maske=0;//uint8_t
            byte ntp_wochentag=oNtp.getwochentag();
            byte ntp_stunde=oNtp.getstunde();
            byte ntp_minute=oNtp.getminute();
            
            if(ntp_wochentag==0)maske=1;//Serial.print(" Mo");
            if(ntp_wochentag==1)maske=2;//Serial.print(" Di");
            if(ntp_wochentag==2)maske=4;//Serial.print(" Mi");
            if(ntp_wochentag==3)maske=8;//Serial.print(" Do");
            if(ntp_wochentag==4)maske=16;//Serial.print(" Fr");
            if(ntp_wochentag==5)maske=32;//Serial.print(" Sa");
            if(ntp_wochentag==6)maske=64;//Serial.print(" So");

            if(tage & maske ){Serial.print(" isday,");
              if(ntp_stunde==t_st && ntp_minute==t_min){
                  if(befehl=="UP"){Serial.println(" >UP");  
                      RolloBefehl(1, 1);
                    }
                    else
                  if(befehl=="STOP"){Serial.println(" >STOP");  
                      RolloBefehl(2, 1);
                    }
                    else
                  if(befehl=="DOWN"){Serial.println(" >DOWN");  
                      RolloBefehl(3, 1);
                    }
                    /*else{
                      Serial.print(befehl); 
                      Serial.println("  Befehl ungültig  "); 
                     }*/
               }/*else{
                  Serial.println(" but not time.");
                }*/
            }
           /* else
            Serial.println(" is not the day.");*/
             
          }
          
          
      }
    }
    file.close();
  }  
}







//--------------------Server--------------------------------


//------------Data IO--------------------

/*
void handleSysJS() {
  server.send(200, "application/javascript", systemjs);
  Serial.println("send system.js");
}

void handleFavicon() { // sendFavIcon();
  server.send_P(200, "image/x-icon", favicon256, sizeof(favicon256));
  Serial.println("send favicon");
}
*/

void handleData(){// data.json
  String message = "{\r\n";
  String aktionen = "";

  //übergabeparameter?
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "settimekorr") {
       oNtp.setTimeDiff(server.arg(i).toInt());
       aktionen +="set_timekorr ";
    }
    if (server.argName(i) == "led") {
       if (server.arg(i) == "on" ){
            setLED(true);
            aktionen +="LED_ON ";
        }
       if (server.arg(i) == "off"){
            setLED(false);
            aktionen +="LED_OFF ";
       }
    }
  }
  message +="\"aktionen\":\""+aktionen+"\",\r\n";
  message +="\"progversion\":\""+String(progversion)+"\",\r\n";
  message +="\"cpu_freq\":\""+String(ESP.getCpuFreqMHz())+"\",\r\n";
  message +="\"chip_id\":\""+String(ESP.getChipId())+"\",\r\n";
  
  
  byte ntp_stunde   =oNtp.getstunde();
  byte ntp_minute   =oNtp.getminute();
  byte ntp_secunde  =oNtp.getsecunde();
 
//  ntp_stunde
  message +="\"lokalzeit\":\"";
  if(ntp_stunde<10)message+="0";
  message+=String(ntp_stunde)+":";
  if(ntp_minute<10)message+="0";
  message+= String(ntp_minute)+":";
  if(ntp_secunde<10)message+="0";
  message+=String(ntp_secunde);
  message +="\",\r\n";
  
  message +="\"datum\":{\r\n";
  message +=" \"tag\":"+String(oNtp.getwochentag())+",\r\n";
  message +=" \"year\":"+String(oNtp.getyear())+",\r\n";
  message +=" \"month\":"+String(oNtp.getmonth())+",\r\n";
  message +=" \"day\":"+String(oNtp.getday())+",\r\n";
  message +=" \"timekorr\":"+String(oNtp.getUTCtimediff())+",\r\n";
  if(oNtp.isSummertime())
    message +=" \"summertime\":true\r\n";
  else
    message +=" \"summertime\":false\r\n";
  message +="},\r\n";
 
  message +="\"macadresse\":\""+macadresse+"\",\r\n";

  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
      message +="\"fstotalBytes\":"+String(fs_info.totalBytes)+",\r\n";
      message +="\"fsusedBytes\":"+String(fs_info.usedBytes)+",\r\n";

      message +="\"fsused\":\"";
      message +=float(int(100.0/fs_info.totalBytes*fs_info.usedBytes*100.0)/100.0);
      message +="%\",\r\n";
   }
  //files
  message +="\"files\":[\r\n";
  String fileName;
  Dir dir = SPIFFS.openDir("/");
  uint8_t counter=0;
  while (dir.next()) {
      fileName = dir.fileName(); 
      if(counter>0)  message +=",\r\n";
      message +=" {";
      message +="\"fileName\":\""+fileName+"\", ";
      message +="\"fileSize\":"+String(dir.fileSize());
      message +="}";
     counter++;
  };
  message +="\r\n]\r\n";

//--
  message +="\r\n}";

  server.sendHeader("Access-Control-Allow-Origin", "*");//wenn vom HTTPS-Seiten aufgerufen wird!
  server.send(200, "text/plain", message );
  Serial.println("send data.json");
}


void handleIndex() { // index.htm(l)
  //$h1gtag $info
  int pos1 = 0;
  int pos2 = 0;
  String s;
  String tmp;

  String message = "";

  while (indexHTM.indexOf("\r\n", pos2) > 0) {
    pos1 = pos2;
    pos2 = indexHTM.indexOf("\r\n", pos2) + 2;
    s = indexHTM.substring(pos1, pos2);

    if (s.indexOf("$h1gtag") != -1) {
      s.replace("$h1gtag", progversion);
    }

  /* if (s.indexOf("$sysinfo") != -1) {     
      tmp = "MAC: <span style=\"text-transform: uppercase;\">";
      tmp+= macadresse+"</span>"; 

      tmp+= "<span class=\"ltime\">lokaltime ";
      if(ntp_stunde<10)tmp+="0";
      tmp+=String(ntp_stunde)+":";
      if(ntp_minute<10)tmp+="0";
      tmp+= String(ntp_minute)+":";
      if(ntp_secunde<10)tmp+="0";
      tmp+=String(ntp_secunde)+"</span>";
   
      
      s.replace("$sysinfo", tmp);
    }*/
    if(s.indexOf("$filelist") != -1){
        
        tmp="<table class=\"files\">\n";
        String fileName;
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            fileName = dir.fileName(); 
            Serial.print("getfilelist: ");
            Serial.println(fileName);
            tmp+="<tr>\n";
            tmp+="\t<td><a target=\"_blank\" href =\"" + fileName + "\"" ;
            if(isdownload(fileName)){
              tmp+= " download=\"" + fileName+ "\"" ;
              tmp+= " class=\"dl\"";
              tmp+= " title=\"Download\"";
            }else{
               tmp+= " title=\"show\"";
            }
            tmp+= " >" + fileName.substring(1) + "</a></td>\n\t<td class=\"size\">" + formatBytes(dir.fileSize())+"</td>\n\t<td class=\"action\">";
            tmp+="<a href =\"" + fileName + "?delete=" + fileName + "\" class=\"fl_del\"> löschen </a>\n";
            tmp+="\t</td>\n</tr>\n";
        };

        FSInfo fs_info;
        tmp += "<tr><td colspan=\"3\">";
        if (SPIFFS.info(fs_info)) {
          tmp += formatBytes(fs_info.usedBytes).c_str(); //502
          tmp += " von ";
          tmp += formatBytes(fs_info.totalBytes).c_str(); //2949250 (2.8MB)   formatBytes(fileSize).c_str()
          tmp += " (";
          tmp += float(int(100.0/fs_info.totalBytes*fs_info.usedBytes*100.0)/100.0);
          tmp += "%)";
          /*tmp += "<br>\nblockSize:";
          tmp += fs_info.blockSize; //8192
          tmp += "<br>\npageSize:";
          tmp += fs_info.pageSize; //256
          tmp += "<br>\nmaxOpenFiles:";
          tmp += fs_info.maxOpenFiles; //5
          tmp += "<br>\nmaxPathLength:";
          tmp += fs_info.maxPathLength; //32*/
        }
        tmp+="</td></tr></table>\n";
        s.replace("$filelist", tmp);
    }
    /* if(s.indexOf("$info") != -1){
       s.replace("$info", ":-)");
      }*/
    message += s;
  }

  server.send(200, "text/html", message );
  //server.send_P(200, "text/html", indexHTM, sizeof(indexHTM));
  Serial.println("send indexHTM");
}

void handleAction() {
  String message = "{\n";
  message += "\"Arguments\":[\n";

  uint8_t rollobefehl = 0;
  uint8_t rollokeyOK = 0;
  uint8_t rolloRE = 0;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (i > 0) message += ",\n";
    message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"}";

    if (server.argName(i) == "rollo") {
      if (server.arg(i) == "UP")   rollobefehl = 1;
      if (server.arg(i) == "STOP") rollobefehl = 2;
      if (server.arg(i) == "DOWN") rollobefehl = 3;
    }
    if (server.argName(i) == "rkey") {
      if (server.arg(i) == rollo_KEY)rollokeyOK = 1;
    }
  }
  message += "\n ]";

  rolloRE = RolloBefehl(rollobefehl, rollokeyOK);
  if (rolloRE > 0)
    message += ",\n\"Rollo\":\"OK\"";
  else
    message += ",\n\"Rollo\":\"ERR\"";

  message += "\n}";
  server.sendHeader("Access-Control-Allow-Origin", "*");//wenn vom HTTPS-Seiten aufgerufen wird!
  server.send(200, "text/plain", message );

}


String getContentType(String filename) {              // ContentType für den Browser
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
bool isdownload(String filename) {     // download oder anzeigen im Browser
  if (filename.endsWith(".htm")) return false;
  else if (filename.endsWith(".html")) return false;
  else if (filename.endsWith(".css")) return false;
  else if (filename.endsWith(".js")) return false;
  else if (filename.endsWith(".xml")) return false;
  else if (filename.endsWith(".txt")) return false;
  else if (filename.endsWith(".png")) return false;
  else if (filename.endsWith(".gif")) return false;
  else if (filename.endsWith(".jpg")) return false;
  else if (filename.endsWith(".svg")) return false;
  else if (filename.endsWith(".ico")) return false;
  else if (filename.endsWith(".pdf")) return false;
  return true;
}

void handleFileUpload() {          // Dateien ins SPIFFS schreiben
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    if (filename.length() > 30) {
      int x = filename.length() - 30;
      filename = filename.substring(x, 30 + x);
    }
    filename = server.urlDecode(filename);
    filename = "/" + filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    if(!fsUploadFile) Serial.println("!! file open failed !!");
   
    //filename = String();
    
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile){
        Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile){
      fsUploadFile.close();
      Serial.println("close");
    }
    yield();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
    server.sendContent(actionheader);
  }
}

bool handleFileRead(String path) {
  if(server.hasArg("delete")) {
    Serial.print("delete: ");
    Serial.print(server.arg("delete"));
    if(SPIFFS.remove(server.arg("delete")))                    //hier wir gelöscht
        Serial.println(" OK");
        else
        Serial.println(" ERR");
    server.sendContent(actionheader);
    return true;
  }
  //if (path.endsWith("/")) path += "index.html";
  path = server.urlDecode(path);
  String pathWithGz = path + ".gz";
  Serial.print("GET ");
  Serial.print(path);
  if (SPIFFS.exists(pathWithGz)) path += ".gz";
  if (SPIFFS.exists(path)) {
    Serial.print(" send ");
    Serial.println(path);
    //   Serial.println("handleFileRead: " + path);
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  Serial.println(" 404");
 /* if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) path += ".gz";
    //   Serial.println("handleFileRead: " + path);
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }*/
  return false;
}


void handleNotFound() {
  //--check Dateien im SPIFFS--
 if(!handleFileRead(server.uri())){ 
    //--404 als JSON--
    String message = "{\n \"error\":\"File Not Found\", \n\n";
    message += " \"URI\": \"";
    message += server.uri();
    message += "\",\n \"Method\":\"";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\",\n";
    message += " \"Arguments\":[\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      if (i > 0) message += ",\n";
      message += "  {\"" + server.argName(i) + "\" : \"" + server.arg(i) + "\"}";
    }
    message += "\n ]\n}";  
    server.sendHeader("Access-Control-Allow-Origin", "*");//wenn vom HTTPS-Seiten aufgerufen wird!
    server.send(404, "text/plain", message);  
    /*
      String sHTML = F("<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this webserver.</p></body></html>");
      webserver.send(404, "text/html", sHTML);
    */  
    String message2 = "404,";
    message2 += server.uri();
    message2 += ",";
    message2 += (server.method() == HTTP_GET) ? "GET" : "POST";
    Serial.println( message); 
  }
}



//----------------------433MHz-----------------------------
uint8_t RolloBefehl(uint8_t befehl, uint8_t key) {
  uint8_t re = 0;
  Serial.print("Rollo:");
  Serial.print(befehl);
  Serial.print(",");
  Serial.println(key);
  if (key == 1) {
    if (befehl == 1) {
      sendQuadState(rollo_UP, rolloWiederholungen);
      re = 1;
    }
    if (befehl == 2) {
      sendQuadState(rollo_STOP, rolloWiederholungen); //3x
      re = 2;
    }
    if (befehl == 3) {
      sendQuadState(rollo_DOWN, rolloWiederholungen);
      re = 3;
    }
  }
  return re;
}


void transmit(uint8_t nHighPulses, uint8_t nLowPulses) {
  if (sender433PIN != -1) {
    digitalWrite(sender433PIN, HIGH);
    delayMicroseconds(rolloPulseLength * nHighPulses);
    digitalWrite(sender433PIN, LOW);
    delayMicroseconds(rolloPulseLength * nLowPulses);
  }
}

void sendSync() {
  transmit(18, 6); //4500µs hi,1500µs lo //6ms
}
/**
   Sends a Quad-State "Q" Bit
              ___   _
   Waveform: |   |_| |___
*/
void sendQQ() {
  transmit(3, 1);
  transmit(1, 3);
}
/**
   Sends a Tri-State "0" Bit
              _     _
   Waveform: | |___| |___
*/
void sendT0() {
  transmit(1, 3);
  transmit(1, 3);
}
/**
   Sends a Tri-State "1" Bit
              ___   ___
   Waveform: |   |_|   |_
*/
void sendT1() {
  transmit(3, 1);
  transmit(3, 1);
}
/**
   Sends a Tri-State "F" Bit
              _     ___
   Waveform: | |___|   |_
*/
void sendTF() {
  transmit(1, 3);
  transmit(3, 1);
}

void sendQuadState(const char* sCodeWord, uint8_t nRepeatTransmit) {
  setLED(true);
  for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
    //Syn-Bit wird hier VOR Übertragung gesendet!
    sendSync();
    int i = 0;
    while (sCodeWord[i] != '\0') {
      switch (sCodeWord[i]) {
        case '0':
          sendT0();
          break;
        case 'F':
          sendTF();
          break;
        case '1':
          sendT1();
          break;
        case 'Q':
          sendQQ();
          break;
      }
      i++;
    }
  }
  setLED(false);
}
