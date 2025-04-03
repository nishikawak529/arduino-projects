#include <Arduino.h>
#include <WiFi.h>       // Pico WでのWi-Fi接続用
#include <WiFiUdp.h>    // UDP通信用
#include <Wire.h>       // I2C通信用
#include <MPU6050_light.h> // MPU6050_lightライブラリ

// --- Wi-Fi接続情報 ---
const char* ssid     = "Potato6";
const char* password = "50lanumtu6";

// --- 送信先 (PCサーバ等) の情報 ---
IPAddress serverIP(192, 168, 1, 18);  // サーバのIPアドレス
unsigned int serverPort = 5000;       // 送信先ポート

WiFiUDP udp;        // UDPインスタンス
MPU6050 mpu(Wire);  // MPU6050_lightライブラリのオブジェクト

const int agent_id = 1;  // 各エージェントごとに異なるIDを設定

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Program Start.");

  // --- Wi-Fi に接続 ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // --- I2C初期化 (Pico Wの場合)
  //   SDA: GP4,  SCL: GP5 （必要に応じて変更）
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // --- MPU6050 の初期化 ---
  byte status = mpu.begin();
  if (status != 0) {
    Serial.print("MPU6050 init failed with code: ");
    Serial.println(status);
    while (1) { /* 初期化失敗で停止 */ }
  }
  // センサーを水平・静止状態にしてキャリブレーション
  mpu.calcOffsets();
  Serial.println("MPU6050 offsets calculated.");

  // --- UDPソケット開始 ---
  udp.begin(serverPort);
}

void loop() {
  // センサ値の更新
  mpu.update();

  // 加速度（G単位）
  float accX = mpu.getAccX();
  float accY = mpu.getAccY();
  float accZ = mpu.getAccZ();
  // 角速度（°/秒）
  float gyroX = mpu.getGyroX();
  float gyroY = mpu.getGyroY();
  float gyroZ = mpu.getGyroZ();
  // 姿勢角（オイラー角）
  float angleX = mpu.getAngleX();
  float angleY = mpu.getAngleY();
  float angleZ = mpu.getAngleZ();

  // マイクロ秒タイマーから秒単位タイムスタンプを取得
  float ts = micros() / 1000000.0;

  // 送信用メッセージ作成（フォーマット例）
  char buf[256];
  snprintf(buf, sizeof(buf),
           "ID=%d,TS=%.6f,AccX=%.2f,AccY=%.2f,AccZ=%.2f,GyX=%.2f,GyY=%.2f,GyZ=%.2f,AngleX=%.2f,AngleY=%.2f,AngleZ=%.2f",
           agent_id, ts,
           accX, accY, accZ,
           gyroX, gyroY, gyroZ,
           angleX, angleY, angleZ);

  // UDP送信
  udp.beginPacket(serverIP, serverPort);
  udp.print(buf);
  udp.endPacket();

  // 送信周期調整（例: 100ms間隔）
  delay(0);
}
