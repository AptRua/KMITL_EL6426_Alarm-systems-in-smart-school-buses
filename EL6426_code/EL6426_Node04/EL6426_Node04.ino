#define BLYNK_TEMPLATE_ID "TEMPLATE ID"
#define BLYNK_DEVICE_NAME "PROJECT NAME"
#define BLYNK_AUTH_TOKEN "BLYNK TOKEN"

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN "LINE TOKEN"

char auth[] = "BLYNK TOKEN";
char ssid[] = "WiFi ssid";
char pass[] = "WiFi password";

#include "HX711.h"
#include <Wire.h> 

#define DOUT  D5
#define CLK   D6
#define DEC_POINT  2
#define LED  D0
#define PIR  D2
#define BUZZER D3
bool Status = 0;

float calibration_factor =46763; 
#define zero_factor -90836
float offset=0.860;
float get_units_kg();
String data;
float weigh;
uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0x13, 0xC5, 0xDC};

#define BOARD_ID 4

typedef struct struct_message {
    int id;
    float A;
    int B;
} struct_message;

struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

HX711 scale(DOUT, CLK);
WidgetLED LED4(V4);WidgetLED LED8(V8);WidgetLED LED9(V9);
WidgetLCD LCD1(V15);

void setup() 
{
  Serial.begin(115200);
  pinMode(PIR,INPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER,HIGH);
  digitalWrite(LED,LOW);
  scale.set_scale(calibration_factor);
  WiFi.mode(WIFI_STA); 
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  Blynk.begin(auth, ssid, pass); LED4.off(); LED8.off(); LED9.off(); LCD1.clear();Blynk.virtualWrite(V14,0);
  Blynk.begin(auth, ssid, pass);
  LINE.setToken(LINE_TOKEN);
}
void loop() 
{ 
  //Blynk.run();
  if ((millis() - lastTime) > timerDelay) {
    read_weigh_off();
    PIR_SENSOR_off();
    myData.id = BOARD_ID;
    myData.A = data.toFloat();
    myData.B = digitalRead(PIR);
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
}
BLYNK_CONNECTED(){
  Blynk.syncVirtual(V0);
}
BLYNK_WRITE(V0){
while(param.asInt()){
  if ((millis() - lastTime) > timerDelay) {
    read_weigh();
    PIR_SENSOR();
    myData.id = BOARD_ID;
    myData.A = data.toFloat();
    myData.B = digitalRead(PIR);
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
    } 
  }
}
void alert()
{
  Serial.println("SOMEONE IN CAR!");
  Serial.println("PLEASE CHECK INSIDE!");
  Serial.println("PIR status : "); Serial.println(digitalRead(PIR));
  analogWrite(BUZZER,100);
  digitalWrite(LED,LOW);
  LCD1.clear();
  LED4.on(); LED8.on(); LED9.on();Blynk.virtualWrite(V14,data);
  delay(500);
  analogWrite(BUZZER,1000);
  digitalWrite(LED,HIGH);
  delay(500);
  LCD1.print(0,0,"SOMEONE IN CAR!");
  LED4.off(); LED8.off(); LED9.off();
  LINE.notify("SOMEONE IN CAR!");
  LINE.notify("PLEASE CHECK INSIDE!");
}
void read_weigh()
{
  Serial.print("Reading: ");
  data = String(get_units_kg()+offset, DEC_POINT);
  weigh = data.toFloat();
  if(weigh>1.00){
    check_weigh();
    Serial.print(data);
  Serial.print(" kg  ");
  Blynk.virtualWrite(V14,data);LED8.on();
  Serial.print("\"OBJECT\"");
  Serial.println("PIR status : ");
  Serial.println(digitalRead(PIR));
  delay(1000);
  }
  else if(weigh<=0.00){
    LED8.off(); Blynk.virtualWrite(V14,0);
    Serial.print("Reading: ");
    Serial.print(" 0.00 kg  ");
    Serial.println("\"nothing\"");
    Serial.print("PIR status : ");
    Serial.println(digitalRead(PIR));
  }
  else{
  Serial.print(data);
  Serial.print(" kg  ");
  Blynk.virtualWrite(V14,data);LED8.on();
  Serial.println("\"OBJECT\"");
  Serial.print("PIR status : ");
  Serial.println(digitalRead(PIR));
  delay(1000);
  }
}
void check_weigh()
{
  int i;
  for(i=10;i>=1;i--){
    data = String(get_units_kg()+offset, DEC_POINT);
    weigh = data.toFloat();
    if(weigh<1.00){
      read_weigh();
    }
    else{
      Serial.println();
      Serial.print("Alarm in ");
      Serial.print(i);
      Serial.println(" s");
      LCD1.clear();
      LCD1.print(4,0,"Alarm in");
      LCD1.print(7,1,i);
      Blynk.virtualWrite(V14,data);
    delay(1000);
    }
  while(i == 1){
  alert();}
  }
}
float get_units_kg()
{
  return(scale.get_units()*0.453592);
}
void PIR_SENSOR()
{
  if(digitalRead(PIR))
  {
    if(!Status)
    {
      Status = 1;
    }
    delay(1000);
    while(Status == 1){
    alert();}
  }
   else
   {
    digitalWrite(BUZZER,HIGH);
    digitalWrite(LED,LOW);
    Status = 0;
   }
}
void read_weigh_off()
{
  Serial.print("Reading: ");
  data = String(get_units_kg()+offset, DEC_POINT);
  weigh = data.toFloat();
  if(weigh<=0.00){
    LED8.off(); Blynk.virtualWrite(V14,0);
    Serial.print("Reading: ");
    Serial.print(" 0.00 kg  ");
    Serial.println("\"nothing\"");
    Serial.print("PIR status : ");
    Serial.println(digitalRead(PIR));
  }
  else{
  Serial.print(data);
  Serial.print(" kg  ");
  Blynk.virtualWrite(V14,data);LED8.on();
  Serial.println("\"OBJECT\"");
  delay(1000);
  }
}
void PIR_SENSOR_off()
{
  if(digitalRead(PIR))
  {
    if(!Status)
    {
      Status = 1;
    }
    LED4.on();
    Serial.println("PIR status : ");
    Serial.println(digitalRead(PIR));
    delay(1000);
  }
   else
   {
    LED4.off();
    Serial.println("PIR status : ");
    Serial.println(digitalRead(PIR));
   }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
