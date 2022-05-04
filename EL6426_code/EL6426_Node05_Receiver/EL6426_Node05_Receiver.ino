/*
#1 C4:5B:BE:64:59:EE A,B C45BBE6459EE
#2 A4:E5:7C:B6:58:4E C,D A4E57CB6584E
#3 40:F5:20:3E:4F:9B E,F A40F5203E4F9B
#4 94:B9:7E:13:C4:54 G,H A94B97E13C454
#5 94:B9:7E:13:C5:DC I,J
 */
#include <ESP8266WiFi.h>
#include <espnow.h>
char ssid[] = "WiFi ssid";
char pass[] = "WiFi password";

typedef struct struct_message {
    int id;
    float A;
    int B;
} struct_message;

struct_message myData;

struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;

struct_message boardsStruct[4] = {board1, board2, board3, board4};

void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u\n", myData.id);
  boardsStruct[myData.id-1].A = myData.A;
  boardsStruct[myData.id-1].B = myData.B;
  Serial.printf("Weigh value: %.2f \r\n", boardsStruct[myData.id-1].A);
  Serial.printf("PIR status: %d \r\n", boardsStruct[myData.id-1].B);
  Serial.println();
  
}
 
void setup() {
 Serial.begin(115200);
  WiFi.begin(ssid,pass);
  if (esp_now_init() != 0) {
   Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
}
