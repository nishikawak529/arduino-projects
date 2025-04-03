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
MPU6050 mpu(Wire);
MotorHandler motor_handler(9, 8);

unsigned int localUdpPort = 6000;  // PCからの指令受信用ポート
char incomingPacket[128];
float targetAngleOffset = 0.0;
int turnRateOffset=0;
bool overrideZeroInput = false;  // ← 追加
int RLoffset = 0;

uint8_t Acce = 0;
uint8_t Brake_P = 0;
uint8_t Mode = 1;

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
    motor_handler.Control_Motor(0, 1, Mode, Acce, Brake_P, &receiv1);
    delay(100);
    motor_handler.Set_MotorMode(Mode, 2);
    motor_handler.Control_Motor(0, 2, Mode, Acce, Brake_P, &receiv2);

    RLoffset = receiv1.Position + receiv2.Position;

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");

    // IPアドレス取得＆表示
    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());

    udp.begin(localUdpPort);
    Serial.printf("Listening for control commands on UDP port %d\n", localUdpPort);

    Wire.setSDA(4);
    Wire.setSCL(5);
    Wire.begin();

    byte status = mpu.begin();
    if (status != 0) {
      Serial.print("MPU6050 init failed with code: ");
      Serial.println(status);
      while (1) {}
    }
    //mpu.calcOffsets();
    //Serial.println("MPU6050 offsets calculated.");

    //udp.begin(serverPort);

    startTime = millis();
    delay(500);
    Serial.println("Setup End");
}

void send_combined_udp(double timestamp, float currentAngle, float gyroY, float gyroZ, int cmd1, int cmd2,
                       const Receiver& r1, const Receiver& r2, unsigned long loopTime) {
    char buf[300];
    snprintf(buf, sizeof(buf),
             "TS=%.6f,Angle=%.2f,GyroY=%.2f,GyroZ=%.2f,CMD1=%d,CMD2=%d,ID1_Speed=%d,ID1_Position=%d,ID1_Current=%d,ID2_Speed=%d,ID2_Position=%d,ID2_Current=%d,LoopTime=%lu",
             timestamp, currentAngle, gyroY, gyroZ, cmd1, cmd2,
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
        } else if (field.startsWith("OverrideZero=")) {
          String val = field.substring(13);
          overrideZeroInput = (val == "True") || (val == "true");
          Serial.print("[RECEIVED] OverrideZero = ");
          Serial.println(overrideZeroInput);
        }
      }
    }
  }
}


// PI制御パラメータ
float Kp = 450.0;    // 比例ゲイン（最初は5くらいから調整）
float Ki = 1.0;    // 積分ゲイン（最初は0.05〜0.2くらいで試行）
float Kd = 20.0;    // 積分ゲイン（最初は0.05〜0.2くらいで試行）
float integral = 0.0;

float targetSpeed = 0;
float KpSpeed = 0.06;
float KiSpeed = 0.0005;
float KdSpeed = 1.0;
float integralSpeed = 0.0;
float previousSpeed = 0.0;

float integralRL = 0.0;
float KpRL = 20.0;
float KiRL = 0.01;
float KdRL = 0.0;
float RLerror = 0.0;

void loop() {
    receiveControlCommand();  // 毎ループで受信確認
    mpu.update();

    targetSpeed = targetAngleOffset;
    float currentAngle = mpu.getAngleY() + 4.5;  // オフセットを反映;
    float gyroY = mpu.getGyroY()-2.7;
    float gyroZ = mpu.getGyroZ()+2.45;

    unsigned long elapsedTime = millis() - startTime;
    startTime = millis();
    double timestamp = micros() / 1000000.0;

    float errorSpeed = targetSpeed - (receiv1.BSpeed-receiv2.BSpeed)/2;

    // 積分項を更新
    integralSpeed += errorSpeed*elapsedTime;
    integralSpeed = constrain(integralSpeed, -1500, 1500);  // 大きくなり過ぎないように制限

    float derivativeSpeed = ((receiv1.BSpeed-receiv2.BSpeed)/2 - previousSpeed)/elapsedTime;
    previousSpeed = (receiv1.BSpeed-receiv2.BSpeed)/2;

    // 誤差（目標角度を目標速度から算出）
    float error = currentAngle  + KpSpeed*(errorSpeed) + KiSpeed*integralSpeed + KdSpeed*derivativeSpeed;

    // 積分項を更新
    integral += error*elapsedTime;
    integral = constrain(integral, -500, 500);  // 大きくなり過ぎないように制限

    // PID制御出力
    int output = static_cast<int>(Kp * error + Ki * integral + Kd * gyroY);
    output = output + output/abs(output)*0;

    int output1 = 0;
    if (turnRateOffset == 0){
        //output1  = KpRL * RLerror + KiRL * integralRL;
        output1  = - KpRL * gyroZ;
    }else{
        RLoffset = receiv1.Position + receiv2.Position;
    }

        // デッドバンド処理（微小誤差で無駄に動かさない）
    if (abs(currentAngle) < 0.2) {  
        output = 0;
        output1 = 0;
    }

    int cmd1 = -output + turnRateOffset - output1;
    int cmd2 = output + turnRateOffset - output1;
    
    // overrideZeroInput が True の場合は強制停止
    if (overrideZeroInput) {
        cmd1 = 0;
        cmd2 = 0;
        integral = 0.0;
        integralSpeed = 0.0;
        previousSpeed = 0.0;
        Serial.println("[INFO] OverrideZero active: motors stopped.");
    }
    
    bool success1 = motor_handler.Control_Motor(cmd1, 1, Mode, Acce, Brake_P, &receiv1);
    bool success2 = motor_handler.Control_Motor(cmd2, 2, Mode, Acce, Brake_P, &receiv2);
    
    if (success1 && success2) {
        send_combined_udp(timestamp, currentAngle, gyroY, gyroZ, cmd1, cmd2, receiv1, receiv2, elapsedTime);
    }
    else {
        if (!success1) Serial.println("Receive failed for ID=1");
        if (!success2) Serial.println("Receive failed for ID=2");
    }
    RLerror = receiv1.BSpeed + receiv2.BSpeed;
}
