//Libraries needed for all the Network Protocols
#include <Arduino.h>

//Will have to setup Arduino to add the ESP8266 Core libraries
//Good tutorial -- https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

//Libraries needed for the GenSetMotorController
//address for library download https://www.dimensionengineering.com/info/arduino
#include <Sabertooth.h>
#include <Wire.h>

//Some debug and other variables
#define DEBUG Serial
#define debug true
#define debugLoop false
#define PRODUCTION false
String inStr = "";
String inStr2 = "";

Sabertooth ST(128);

//New values for the App
const int appFullForward = 0;
const int appMinForward = 9;
const int appMinReverse = -9;
const int appFullReverse = 0;

//New values for the App
const int appFullLeft = -45;
const int appMinLeft = -15;
const int appMinRight = 15;
const int appFullRight = 45;

int FNR = 0;
int steer = 0;
int yyy = 0;
int xxx = 0;

//Setting define for easier debug to serial
const char* mDNSid   = "CEP";//Hostname to be entered in app as CEP.local
const char* ssid     = "caterpillar-cep";//The WiFi AP to connect phone to
const char* password = "coolbeans";//Password for the WiFi AP *must be at least 8 characters

//Inititating the websocket that will recieve data from phone
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
 
            if (debug == true && PRODUCTION == false) {DEBUG.printf("[%u] Disconnected!\n", num);}
            
            break;
            
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);            
            if (debug == true && PRODUCTION == false) {DEBUG.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);}
        
            break;
        }
        
        case WStype_TEXT:
          {

            if (payload[0] == '#') {

              //Steer Value section
              inStr = (char *)payload;
              String newStr = inStr.substring(1);
              if (debug == true && PRODUCTION == false) {DEBUG.println(newStr);}
              float steerVal = newStr.toFloat();
              if (debug == true && PRODUCTION == false) {DEBUG.printf("%f", steerVal);}
              xxx = steerVal;

              break;
             
            }
          
            
            else if (payload[0] == '>') {

              inStr2 = (char *)payload;
              String newStr2 = inStr2.substring(1);
              if (debug == true && PRODUCTION == false) {DEBUG.println(newStr2);}
              int goVal = newStr2.toInt();
              yyy = goVal;
              
              break;
            }
          }
        }
    }

void setup() {
  
  if (debug == true && PRODUCTION == false) {
    DEBUG.begin(115200);
    DEBUG.println();
    DEBUG.println();
    DEBUG.println();

    for(uint8_t t = 4; t > 0; t--) {
        DEBUG.printf("[SETUP] BOOT WAIT %d...\n", t);
        DEBUG.flush();
        delay(1000);
    }
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  if (debug == true && PRODUCTION == false) {DEBUG.printf("AP IP Address: ");}
  if (debug == true && PRODUCTION == false) {DEBUG.println(myIP);}

  if (debug == true && PRODUCTION == false) {DEBUG.println("");}
  if (debug == true && PRODUCTION == false) {DEBUG.print("Connected! IP address: ");}
  if (debug == true && PRODUCTION == false) {DEBUG.println(WiFi.softAPIP());}

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  // Set up mDNS responder:
  if (!MDNS.begin(mDNSid)) {
    if (debug == true && PRODUCTION == false) {DEBUG.println("Error setting up MDNS responder!");}
    while(1) { 
      delay(1000);
    }
  }
    if (debug == true && PRODUCTION == false) {DEBUG.println("mDNS responder started");}

      // Add service to MDNS
  MDNS.addService("ws", "tcp", 81);

  //Init for the Sabertooth board
  if (PRODUCTION == true && debug == false) {SabertoothTXPinSerial.begin(9600);}

  //Init instance of Sabertooth drive and turn to zero
  if (PRODUCTION == true && debug == false) {ST.drive(0);}
  if (PRODUCTION == true && debug == false) {ST.turn(0);}



}

void loop() {
  
    webSocket.loop();
    
    if (yyy > appMinForward) {
      if (debugLoop) {DEBUG.println("FNR going forward");}
      if (debugLoop) {DEBUG.println("Yep, its forward");}
      FNR = map(yyy, appMinForward, appFullForward, 0, 127);
    }
    else if (yyy < appMinReverse) {
      if (debugLoop) {DEBUG.println("FNR going reverse");}
      if (debugLoop) {DEBUG.println("Yep, its Reverse");}
      FNR = map(yyy, appMinReverse, appFullReverse, 0, -127);
    }
    else {
      if (debugLoop) {DEBUG.println("FNR going nowhere");}
      if (debugLoop) {DEBUG.println("Damn, Im sitting still");}
      FNR = 0;
    }
    


    if (xxx > appMinRight) {
      if (debugLoop) {DEBUG.println("Steer right");}
      if (debugLoop) {DEBUG.println("STEER RIGHT");}
      steer = map(xxx, appMinRight, appFullRight, 0, -127);
      
    }
    else if (xxx < appMinRight) {
      if (debugLoop) {DEBUG.println("Steer left");}
      if (debugLoop) {DEBUG.println("STEER LEFT");}
      steer = map(xxx, appMinLeft, appFullLeft, 0, 127);
    }
    else {
      if (debugLoop) {DEBUG.println("Steer nowhere");}
      if (debugLoop) {DEBUG.println("STOPPED");}
      steer = 0;
    }

    if (PRODUCTION == true && debug == false) {ST.drive(FNR);}
   if (PRODUCTION == true && debug == false) {ST.turn(steer);}
    
}