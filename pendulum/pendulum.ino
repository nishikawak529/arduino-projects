#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include "TB6612FNG.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "RP2040_PWM.h"

// ----- PID 制御用ゲイン -----
float Kp = 6.5;
float Kp2 = 20.0;
float KpF = 4.0;
float Ki = 80.0;
float Kd = 0.0;
float Kg = 0.40;

// 目標角度 (倒立状態＝0°)
float setpoint = 0.0;  

// PID計算用変数
float error = 0.0;
float lastError = 0.0;
float integral = 0.0;
float nowintegral = 0.0;
unsigned long lastTime;

// MPU6050 オブジェクト (I2C経由)
MPU6050 mpu(Wire);

// TB6612FNG モータドライバオブジェクト
TB6612FNG motorDriver(10, 11, 12, 13, 14, 15, 16);

// ----- WiFi/UDP 設定 -----
const char* ssid     = "Potato6";
const char* password = "50lanumtu6";
IPAddress serverIP(192, 168, 1, 18); // 送信先PCのIPアドレス
unsigned int serverPort = 5000;      // UDP送信用ポート
WiFiUDP udp;

// ----- ジャイロ移動平均用バッファ設定 -----
#define GYRO_BUFF_SIZE 50
float gyroBuffer[GYRO_BUFF_SIZE]; // 直近5サンプルのジャイロ値を保持
float gyroSum = 0.0;              // バッファ内の合計値
int gyroIndex = 0;                // 書き込み位置 (0～4)

// ----- ジャイロ移動平均用バッファ設定 -----
#define D_BUFF_SIZE 2
float dBuffer[D_BUFF_SIZE]; // 直近5サンプルのジャイロ値を保持
float dSum = 0.0;              // バッファ内の合計値
int dIndex = 0;                // 書き込み位置 (0～4)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(300);
  Serial.println("倒立振子プログラム開始");

  // モータドライバの初期化
  motorDriver.init();

  // I2C 初期化
  Wire.begin();

  // MPU6050 の初期化
  byte status = mpu.begin();
  if (status != 0) {
    Serial.print("MPU6050 初期化失敗, コード: ");
    Serial.println(status);
    while (1);  // 初期化失敗時は停止
  }

  lastTime = millis();

  // WiFi 接続
  //WiFi.begin(ssid, password);
  //Serial.print("WiFi 接続中");
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("\nWiFi 接続完了");

  // ジャイロバッファの初期化 (0 で埋める)
  for (int i = 0; i < GYRO_BUFF_SIZE; i++) {
    gyroBuffer[i] = 0.0;
  }
}

void loop() {
  // センサ更新
  mpu.update();

  // 倒立振子の前後傾き・ジャイロ
  float currentAngle = mpu.getAngleY();  // [deg]
  float gyroY = mpu.getGyroY();         // [deg/s]

  // -------------------------------------------------------

  // PID 制御計算
  // ジャイロ項を使う際に「-Kd * (smoothedGyro)」のようにする
  error = setpoint - currentAngle-13.0;
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;
  // -------------- 移動平均処理 (直近5データ) --------------
  if (abs(error) < 30){
    nowintegral = error * dt;  
  } else{
    nowintegral = 0;
  }
  // 1. 古い値を合計から引く
  gyroSum -= gyroBuffer[gyroIndex];
  // 2. 新しい値をバッファに格納
  gyroBuffer[gyroIndex] = nowintegral;
  // 3. 新しい値を合計に足す
  gyroSum += nowintegral;
  // 4. インデックスを進める(リングバッファ)
  gyroIndex++;
  if (gyroIndex >= GYRO_BUFF_SIZE) {
    gyroIndex = 0;
  }
  // 5. 平均を算出
  float integral = gyroSum;


  float nowderivative = (error - lastError) / dt;
  // 1. 古い値を合計から引く
  dSum -= dBuffer[dIndex];
  // 2. 新しい値をバッファに格納
  dBuffer[dIndex] = nowderivative;
  // 3. 新しい値を合計に足す
  dSum += nowderivative;
  // 4. インデックスを進める(リングバッファ)
  dIndex++;
  if (dIndex >= D_BUFF_SIZE) {
    dIndex = 0;
  }
  // 5. 平均を算出
  float derivative = dSum / D_BUFF_SIZE;
  //float output = Kp1 * error * abs(error) + Ki * integral + Kd * derivative;
  float output = Kp * error + Ki * integral + Kd * derivative - Kg * gyroY;
  if (abs(error) > 3){
    digitalWrite(LED_BUILTIN, HIGH);
    output += Kp2 * (error - error/abs(error) * 3);
  }else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  lastError = error;

  // PID 制御出力をモータ駆動の PWM 値 (0～255) に変換
  int pwm = 0;
  if (abs(error) > 30) {
    // ある程度大きく傾いたらモーター出力を0にする等の条件 (暫定)
    output = 0;
  }

  if (output == 0) {
    motorDriver.motorAStop();
    motorDriver.motorBStop(); 
  } else if (output < 0) {
    output += KpF * error;
    pwm = 40 + 1.1 * constrain(abs((int)output), 0, 200);
    pwm = pwm * 100/255;
    motorDriver.motorAForward(20000,5 + pwm);
    motorDriver.motorBForward(20000,pwm);
  } else {
    pwm = 40 + 1.1*constrain(abs((int)output), 0, 200);
    pwm = pwm * 100/255;
    motorDriver.motorABackward(20000,5 + pwm);
    motorDriver.motorBBackward(20000,pwm);
  }
  if (abs(error) < 0){
    motorDriver.motorAStop();
  }

  // 送信用文字列（TIME, ANGLE, Gyro, ERROR, OUTPUT, PWM）
  char buf[150];
  snprintf(buf, sizeof(buf),
           "TIME=%lu,ANGLE=%.2f,GYROY=%.2f,ERROR=%.2f,OUTPUT=%.2f,PWM=%2f",
           now, currentAngle, derivative, Kp * error, Kg * gyroY, Ki * integral);

  // UDP送信
  //udp.beginPacket(serverIP, serverPort);
  //udp.print(buf);
  //udp.endPacket();

  //delay(1);  // 10ms周期
}
