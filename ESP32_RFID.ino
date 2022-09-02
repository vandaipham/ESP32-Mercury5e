#include <WiFi.h>
#include <HTTPClient.h>


// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
const char* hostname = "123"; //id считывателя
byte tries = 50;  // Попыток подключения к точке доступа
unsigned long previousMillis = 0;
unsigned long interval = 30000;

// Define GPIO - Indicator
#define LED_BUILTIN 2
#define LED_EXTERNAL 4

#define InterruptButton 18

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.137.9/insert3.php";
//?reader=002&status=1&duration=11&metka=E20030980617020916606AEE&time1=2022-06-09&time2=15:12:50
//rd=002&st=1&dr=11&mk=E20030980617020916606AEE&tm1=2022-06-09&tm2=15:12:50


const String uhf_reader = "200";

uint8_t loadfw[] = {0xFF,0x00,0x04,0x1D,0x0B};
uint8_t gen2[] = {0xFF,0x02,0x93,0x00,0x05,0x51,0x7D};
uint8_t eu3[] = {0xFF,0x01,0x97,0x08,0x4B,0xB5};
uint8_t se[] = {0xFF,0x01,0x98,0x01,0x44,0xBC};
uint8_t scan[] = {0xFF,0x02,0x21,0x00,0xC8,0xD6,0x29};
uint8_t checkapp[] = {0xff,0x00,0x0c,0x1d,0x03};
uint8_t serial38400[] = {0xFF,0x04,0x06,0x00,0x00,0x96,0x00,0xE0,0x41};

uint8_t power23[] = {0xFF,0x02,0x92,0x08,0xFC,0x49,0xA5};
uint8_t power22[] = {0xFF,0x02,0x92,0x08,0x98,0x49,0xC1};
uint8_t power21[] = {0xFF,0x02,0x92,0x08,0x34,0x49,0x6D};
uint8_t power20[] = {0xFF,0x02,0x92,0x07,0xD0,0x46,0x89};
uint8_t power19[] = {0xFF,0x02,0x92,0x07,0x6C,0x46,0x35};
uint8_t power18[] = {0xFF,0x02,0x92,0x07,0x08,0x46,0x51};
uint8_t power17[] = {0xFF,0x02,0x92,0x06,0xA4,0x47,0xFD};
uint8_t power16[] = {0xFF,0x02,0x92,0x06,0x40,0x47,0x19};
uint8_t power15[] = {0xFF,0x02,0x92,0x05,0xDC,0x44,0x85};
uint8_t power14[] = {0xFF,0x02,0x92,0x05,0x78,0x44,0x21};
uint8_t power13[] = {0xFF,0x02,0x92,0x05,0x14,0x44,0x4D};
uint8_t power12[] = {0xFF,0x02,0x92,0x04,0xB0,0x45,0xE9};
uint8_t power11[] = {0xFF,0x02,0x92,0x04,0x4C,0x45,0x15};
uint8_t power10[] = {0xFF,0x02,0x92,0x03,0xE8,0x42,0xB1};


int stat0 = 0;
int stat1 = 1;
int count = 0;
int er_ = 3;

String tag = "";
String tag1 = "";


void setup() {
  delay(1000);
  // Begin Serial connected to Computer
  Serial.begin(115200);
  // Initialize GPIO2, GPIO4
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_EXTERNAL, OUTPUT);
  delay(1000);

  // Initialize WiFi
  initWiFi();
  delay(1000);

  // Time Sync with NTP Server.
  Serial.println();
  Serial.println("Синхронизация часов с NTP сервером");
  
  configTime(0, 10800, "192.168.137.1");
  Serial.print("Текущие дата и время: ");
  printLocalTime();
  delay(1000);

  // Begin Serial2 connected to RFID-Reader
  Serial.println();
  Serial.println("Инициализация RFID модуля");
  Serial2.begin(9600);
  SendBlob(loadfw, 5);
  SendBlob(checkapp, 5);
  SendBlob(serial38400, 9);
  Serial2.begin(38400);
  SendBlob(gen2, 7);
  SendBlob(eu3, 6);
  SendBlob(power20, 7);
  delay(1000);

  /*Initialize InterruptButton for accepting interrupts */
  attachInterrupt(InterruptButton, INT0_ISR, FALLING);
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
  Serial.printf("Попытка подключения к Wi-Fi сети %s", ssid);
  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (--tries && WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("Не удалось подключиться, переход в режим точки доступа");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname);
  } else {
    Serial.println();
    Serial.println("Подключение установлено");

    Serial.print("Имя хоста: ");
    Serial.println(WiFi.getHostname());
    
    Serial.print("IP-адрес: ");
    Serial.println(WiFi.localIP());

    Serial.print("MAC-адрес контроллера: ");
    Serial.println(WiFi.macAddress());

    Serial.print("Маска подсети: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("Шлюз: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.printf("Уровень сигнала: %d дБм\n", WiFi.RSSI()); 
    digitalWrite(LED_BUILTIN,HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN,LOW);
  }
  Serial.println();
}


void SendGET(String reciv){   //функция отправки get запроса на сервер
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
  tag = ScanTag();
  if(tag != "NONE"){ 
    stat1 = 1;
    Rd_tag_show(true);
  }else{
    stat1 = 0;
    Rd_tag_show(false);
  }
 
  
  if(stat0 == stat1) {
    count++;
    tag1 = tag;  
  } else {
    if(count > er_) {
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


void SendBlob(uint8_t * b, uint8_t l) { //отправка команды на считыватель
  Serial.print("Отправка ");
  for (uint8_t j = 0; j < l; j++) {
    Serial.print(b[j], HEX);
    Serial.print(" ");
    Serial2.write(b[j]);
  }
  Serial.println();
  
  // ожидание ответа
  while(!Serial2.available());

  // вывод ответа
  Serial.print("Получено ");
  while(Serial2.available()){
    Serial.print(Serial2.read(), HEX);
    delay(5);
    Serial.print(" ");
  }
  Serial.println();
}


String ScanTag() {
  byte income;
  String reciv = "";
  for (int i=0; i<7; i++) Serial2.write(scan[i]);
  delay(50);

  while(Serial2.available()){
    income = Serial2.read();
    String u = String(income, HEX);
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


void Rd_tag_show(boolean status){
  if (status) {
    digitalWrite(LED_EXTERNAL, 1);
  } else {
    digitalWrite(LED_EXTERNAL, 0);
  }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Не удалось получить время");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}