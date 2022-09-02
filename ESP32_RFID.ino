#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SD.h>


// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
unsigned long previousMillis = 0;
unsigned long interval = 30000;

// Define GPIO - Indicator
#define LED_1 2
#define LED_2 4

// Define filename
#define char* file_name = "testFile";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.137.9/insert3.php";
//?reader=002&status=1&duration=11&metka=E20030980617020916606AEE&time1=2022-06-09&time2=15:12:50
//rd=002&st=1&dr=11&mk=E20030980617020916606AEE&tm1=2022-06-09&tm2=15:12:50

#define RD_TAG 7

const String uhf_reader = "200";


HardwareSerial port_M5e(2); // use UART2

byte go_fw[] = {0xFF,0x00,0x04,0x1D,0x0B};
byte setting_gen2[] = {0xFF,0x02,0x93,0x00,0x05,0x51,0x7D};
byte setting_reg[] = {0xFF,0x01,0x97,0x08,0x4B,0xB5};   //EU3
byte se_mode[] = {0xFF,0x01,0x98,0x01,0x44,0xBC};       //min power saving mode

byte power_rx[] = {0xFF,0x02,0x92,0x07,0xD0,0x46,0x89}; //20 dbm
byte setting_38400[] = {0xFF,0x04,0x06,0x00,0x00,0x96,0x00,0xE0,0x41};

int stat0 = 0;
int stat1 = 1;
int count = 0;
int er_ = 3;

String tag = "";
String tag1 = "";


void setup() {
  delay(1000);

  // Initialize WiFi
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  delay(1000);

  // Initialize GPIO2, GPIO4
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  delay(1000);

  // Begin Serial connected to Computer
  Serial.begin(38400);

  // Begin Serial2 connected to RFID-Reader
  port_M5e.begin(9600, SERIAL_8N1, 16, 17); // потом поднять скорость 38400
  Cmd_exec(go_fw, 5);
  Cmd_exec(setting_gen2, 7);
  Cmd_exec(setting_reg, 6);
  Cmd_exec(power_rx, 7);
  Cmd_exec(se_mode, 6);
  Cmd_exec(setting_38400, 9);
  port_M5e.begin(38400, SERIAL_8N1, 16, 17);
  delay(200);    
     
  /*Initialize INT0 for accepting interrupts */
  attachInterrupt(0, INT0_ISR, FALLING);
}


void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}


bool SendGET(String reciv){   //функция отправки get запроса на сервер
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    String serverPath = serverName + reciv;
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
      
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi disconnected!");
  }
}


void INT0_ISR(){ // Scan and send request

  String sender = "rd=" + uhf_reader;
  tag = Scan_tag();
  if(tag != "NONE"){ 
    stat1 = 1;
    Rd_tag_show(true);
  }else{
    stat1 = 0;
    Rd_tag_show(false);
  }
 
  
  if(stat0 == stat1){
    count++;
    tag1 = tag;  
  }else{
    if(count > er_){
      sender += "&st=" + String(stat0) + "&dr=" + String(count) + "&mk=";  
      if(stat0 == 0) sender += tag;
      else if(stat0 == 1) sender += tag1;

      //отправка на Slave
      //sender = "insert(\"" + sender + "\")";
      //sender = "insert(\"" + sender;

      // Send GET request
      SendGET(sender);

      
      Serial.print(sender);
           
      stat0 = stat1;
      count = 0;  
    }
  }
}

void Cmd_exec(byte arr[], int n){
  byte i;
  for(i=0;i<n;i++) port_M5e.write(arr[i]);
  delay(500);
  port_M5e.flush();
}

String Scan_tag(){
  byte incom;
  byte scantag[] = {0xFF,0x02,0x21,0x00,0xC8,0xD6,0x29};
  String reciv = "";
  byte i;
  
  for(i=0;i<7;i++) port_M5e.write(scantag[i]);
  delay(50); 
  while(port_M5e.available()){
    incom = port_M5e.read();
    String u = String(incom, HEX);
    if(u.length()<2) u = '0'+ u;
    reciv += u;
  }

  if(reciv.length()>14){
    reciv = reciv.substring(10,34);
    reciv.toUpperCase();
  } else {
    reciv = "NONE";
  }
  return reciv;
}


void Rd_tag_show(boolean stat){
  if(stat){
    digitalWrite(RD_TAG,1); 
  }else{
    digitalWrite(RD_TAG,0); 
  }  
}





