//Welcome to E for Engineer---SUBSCRIBE Now
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>


const char* ssid     = "DorePita";//Replace with your Wifi Name
const char* password = "ahihihihihi";// Replace with your wifi Password

//Server information
//unsigned long channel = 2107461;
String apiKey = "V0RIGZE8AV21UOXM";
const char* server = "api.thingspeak.com";

WiFiClient  client;

#include <SoftwareSerial.h>
#define RX D4
#define TX D3
SoftwareSerial simSerial(TX,RX);

String dataReturn = "";

void setup() {
  Serial.begin(115200);
  delay(10);
  simSerial.begin(115200);
  delay(10);
//SIM7020E
  Serial.println("Connecting to SIM7020E...");
  while(1){
    dataReturn = "";
    Serial.println("AT");
    simSerial.print("AT\r\n");
    delay(1000);
    while(simSerial.available()){
      dataReturn += simSerial.readString();
    }
    if(dataReturn.indexOf("OK") != -1){
      Serial.println(dataReturn);
      break;
    }
  }    
  
//WIFI
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
/*  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());*/
  ThingSpeak.begin(client);
}

String parseData(String str){
  
  return  str;
}

void loop() {
 
  //get the last data of the fields
  /*
  int lte_rsrp = ThingSpeak.readFloatField(channel, rsrp);
  int lte_rsrq = ThingSpeak.readFloatField(channel, rsrq);
  int lte_pci = ThingSpeak.readFloatField(channel, pci);
  */
  
//DATA
  int rsrp;
  int rsrq;
  int pci;
  String mcc = "";
  
  String response = ""; //chuỗi dữ liệu trả về từ lệnh AT+CENG?
  int count = 0; //biến đếm
  bool success = false; //biến kiểm tra kết quả

  for (int i=0; i<10; i++) { //gọi lại lệnh AT+CENG tối đa 10 lần
    Serial.println("AT+CENG?");
    simSerial.println("AT+CENG?\r\n");
    while (simSerial.available() <= 0) { delay(100); }
//    response = simSerial.readString();
    response = "AT+CENG?+CENG: 1791,1,223,\"0509E017\",-109,-21,-88,-11,3,\"A924\",0,,-98+CENG: 1791,1,81,-108OK";
    //kiểm tra xem có kết quả hay không
    if (response.indexOf("+CENG:") != -1) {
      success = true;
      Serial.println(response);
      break;
    }
    delay(1000); //nghỉ 1 giây trước khi gọi lại lệnh
  }

  if (success) { //nếu có kết quả
    //tách và parse dữ liệu
    char *token = strtok(&response[0], "\n");
    while (token != NULL) {
      count++;
      if (count == 1) { // parse RSRP, RSRQ, PCI, MCC
        char *tok = strtok(token, ",");
        int i = 0;
        while (tok != NULL) {
          switch(i) {
            case 4: // RSRP
              Serial.print("RSRP: ");
              Serial.println(tok);
              rsrp = atoi(tok);
              break;
            case 5: // RSRQ
              Serial.print("RSRQ: ");
              Serial.println(tok);
              rsrq = atoi(tok);
              break;
            case 2: // PCI
              Serial.print("PCI: ");
              Serial.println(tok);
              pci = atoi(tok);
              break;
            case 9: // MCC
              Serial.print("MCC: ");
              mcc = tok;
              mcc.replace("\"", "");
              Serial.println(mcc);
              break;
          }
          i++;
          tok = strtok(NULL, ",");
        }
      }
      token = strtok(NULL, "\n");
    }
  } else {
    Serial.println("Failed to get CENG info, reseting SIM Module");
    simSerial.println("AT+CRESET\r\n");
    delay(2000);
Serial.println("Reconnecting...");
  }
//CONNECT
  if ((success) && (client.connect(server,80))){
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(rsrp);
    postStr +="&field2=";
    postStr += String(rsrq);
    postStr +="&field3=";
    postStr += String(pci);
    postStr +="&field4=";
    postStr += mcc;

    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

//Serial notification
    Serial.println("Sent to ThingSpeak:");
    Serial.print("RSRP: ");
    Serial.print(rsrp);
    Serial.println(" dBm.");
    Serial.print("RSRQ: ");
    Serial.print(rsrq);
    Serial.println(" dB.");
    Serial.print("PCI: ");
    Serial.print(pci);
    Serial.println(".");
    Serial.print("MCC: ");
    Serial.println(mcc);
    Serial.println("............");
    client.stop();
    Serial.println("Waiting for next data...");
    delay(15000);
  }else{
    Serial.println("Can't connect to server!");
    client.stop();
    delay(2000);
  }
}