#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "DDT_Motor.h"

const char* ssid = "Potato6";
const char* password = "50lanumtu6";

IPAddress serverIP(192, 168, 1, 25);
unsigned int serverPort = 5000;

WiFiUDP udp;
MotorHandler motor_handler(9, 8);

unsigned int localUdpPort = 6000;  // PCからの指令受信用ポート
char incomingPacket[128];
float targetAngleOffset = 0.0;
int turnRateOffset=0;

uint8_t Acce = 0;
uint8_t Brake_P = 0;
uint8_t Mode = 3;

Receiver receiv1;
Receiver receiv2;

unsigned long startTime = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Setup Start");

    pinMode(11, OUTPUT);
    digitalWrite(11, LOW);
    delay(100);

    motor_handler.begin(115200);
    motor_handler.Set_MotorMode(Mode, 1);
    motor_handler.Set_MotorMode(Mode, 1);
    motor_handler.Set_MotorMode(Mode, 1);

    // 最初は 0 の位置にする
    motor_handler.Control_Motor(0, 1, Mode, Acce, Brake_P, &receiv1);

    // Wi-Fi 接続
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");

    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());

    udp.begin(localUdpPort);
    Serial.printf("Listening for control commands on UDP port %d\n", localUdpPort);

    startTime = millis();
    delay(500);
    Serial.println("Setup End");
}

void send_combined_udp(double timestamp, float currentAngle, float gyroY, const Receiver& r1, const Receiver& r2, unsigned long loopTime) {
    char buf[250];
    snprintf(buf, sizeof(buf),
             "TS=%.6f,Angle=%.2f,Gyro=%.2f,ID1_Speed=%d,ID1_Position=%d,ID1_Current=%d,ID2_Speed=%d,ID2_Position=%d,ID2_Current=%d,LoopTime=%lu",
             timestamp, currentAngle, gyroY,
             r1.BSpeed, r1.Position, r1.ECurru,
             r2.BSpeed, r2.Position, r2.ECurru,
             loopTime);

    udp.beginPacket(serverIP, serverPort);
    udp.print(buf);
    udp.endPacket();
    Serial.println(buf);
}

void receiveControlCommand() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0) {
      incomingPacket[len] = 0;  // null終端
      String packetStr = String(incomingPacket);

      // 複数フィールドをカンマで分割
      int lastIndex = 0;
      while (lastIndex >= 0) {
        int commaIndex = packetStr.indexOf(',', lastIndex);
        String field = (commaIndex != -1) ? packetStr.substring(lastIndex, commaIndex) : packetStr.substring(lastIndex);
        lastIndex = (commaIndex != -1) ? commaIndex + 1 : -1;

        if (field.startsWith("TargetAngleOffset=")) {
          targetAngleOffset = field.substring(18).toFloat();
          Serial.print("[RECEIVED] TargetAngleOffset = ");
          Serial.println(targetAngleOffset);
        } else if (field.startsWith("TurnRateOffset=")) {
          turnRateOffset = field.substring(15).toFloat();
          Serial.print("[RECEIVED] TurnRateOffset = ");
          Serial.println(turnRateOffset);
        }
      }
    }
  }
}

//---------------------------------
// 0～32767 を往復するための変数
//---------------------------------
static int targetValue = 0;          // 現在指示する目標値(0～32767)
static int direction   = 1;          // 増減方向 (1:増やす, -1:減らす)
static int stepValue   = 1000;       // 1回のループで増減させる値（速度調整用）
static const int maxValue = 30000;   // 往復の最大値

//---------------------------------
// タイミング制御用の変数
//---------------------------------
static unsigned long prevMillis = 0;
static const unsigned long interval = 20; // 20msごとに値を更新

void loop() {
  // UDPコマンド受信チェック (必要に応じて)
  receiveControlCommand();
  
  bool success = false;

  while (!success){
    success = motor_handler.Control_Motor(0, 1, Mode, Acce, Brake_P, &receiv1);
  }
  delay(5000);  
  success = false;
  while (!success){
    success = motor_handler.Control_Motor(15000, 1, Mode, Acce, Brake_P, &receiv1);
  }
  delay(5000);
}
