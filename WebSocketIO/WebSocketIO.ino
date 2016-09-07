#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "index.h"
int LEDpin=13, SWpin=14, Aval=0, LEDState, SWState;
static const char ssid[]="pr500m-a7895d-1", password[]="ec0919a940339";
MDNSResponder mdns;
ESP8266WebServer server=ESP8266WebServer(80);
WebSocketsServer ws    =WebSocketsServer(81);
static void writeLED(int onoff){ LEDState=onoff; digitalWrite(LEDpin, onoff);}
void wsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length){
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type){
    case WStype_DISCONNECTED: Serial.printf("[%u] Disconnected!\r\n", num); break;
    case WStype_CONNECTED:{
        IPAddress ip=ws.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0],ip[1],ip[2],ip[3], payload);
        if(LEDState) ws.sendTXT(num, "ledon",  5);    // Send the current LED status
        else         ws.sendTXT(num, "ledoff", 6);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      if(strcmp("ledon", (const char *)payload)==0)       writeLED(1);
      else if(strcmp("ledoff", (const char *)payload)==0) writeLED(0);
      else analogWrite(LEDpin, atoi((const char *)payload)*10);
      ws.broadcastTXT(payload, length);      // send data to all connected clients
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);
      ws.sendBIN(num, payload, length);      // echo data back to browser
      break;
    default: Serial.printf("Invalid WStype [%d]\r\n", type); break;
  }
}
void handleRoot(){  server.send(200, "text/html", INDEX_HTML);}
void handleNotFound(){
  String message="File Not Found\n\n";
  message+="URI: "+server.uri();
  message+="\nMethod: "+(server.method()==HTTP_GET)?"GET":"POST";
  message+="\nArguments: ";  message+=server.args()+"\n";
  for(uint8_t i=0; i<server.args(); i++) message+=" "+server.argName(i)+": "+server.arg(i)+"\n";
  server.send(404, "text/plain", message);
}

void setup(){
  pinMode(SWpin,INPUT_PULLUP); pinMode(LEDpin,OUTPUT); writeLED(0);
  Serial.begin(115200); delay(10);
  WiFi.begin(ssid, password);
  Serial.println(""); while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println(""); Serial.print("Connected to "); Serial.println(ssid);
  IPAddress ip=WiFi.localIP();
  Serial.print("IP address: ");  Serial.println(ip);
  if(mdns.begin("esp8266", ip)){
    Serial.println("MDNS responder started");
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws",   "tcp", 81);
  }else Serial.println("MDNS.begin failed");
  Serial.println("Connect to http://esp8266.local or http://"+WiFi.localIP());
  server.on("/", handleRoot); server.onNotFound(handleNotFound);
  server.begin();
  ws.begin(); ws.onEvent(wsEvent);
}
void loop(){
  ws.loop();
  server.handleClient();
  if(!digitalRead(SWpin)){
    if(SWState){ SWState=0; writeLED(1); ws.broadcastTXT("ledon", 5); }
  }else SWState=1;
  int val=analogRead(17)/10; delay(3);
  if(val>100)  val=100;
  if(val!=Aval && val%5==0){
    char s[5];
    itoa(val, s, 10);
    Serial.print("VR: "); Serial.println(s);
    analogWrite(LEDpin, val*10);
    ws.broadcastTXT(s, strlen(s));
    Aval=val;
  }
}
