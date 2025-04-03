#include <WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

Servo servo1;

const char* ssid = "Potato6";
const char* password = "50lanumtu6";

// サーバー（PC側）のIPアドレスとUDPポート
IPAddress serverIP(192, 168, 1, 18);
unsigned int serverPort = 6000;

WiFiUDP udp;

// システム状態の定義
enum SystemState {
  STATE_INIT,
  STATE_WAIT_FOR_SERVER, // 初期状態：サーバーからの指示待ち
  STATE_PWM_OPERATION    // PWM操作開始
};

SystemState currentState = STATE_INIT;
const int servoPin = 8;  // GP8（PWM出力ピン）

// 状態メッセージをサーバーへ送信する関数
void sendState(const char* stateMsg) {
  udp.beginPacket(serverIP, serverPort);
  udp.print(stateMsg);
  udp.endPacket();
  Serial.print("Sent state: ");
  Serial.println(stateMsg);
}

// UDPからのメッセージを受信する関数
String readUDPMessage() {
  int packetSize = udp.parsePacket();
  if(packetSize) {
    char incomingPacket[100];
    int len = udp.read(incomingPacket, 99);
    if(len > 0) {
      incomingPacket[len] = '\0';
    }
    return String(incomingPacket);
  }
  return "";
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting PWM state management...");

  // Wi-Fi接続開始
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // UDP用ローカルポート（例：5002）を開始
  udp.begin(5002);

  // 初期状態：servoPinを出力に設定し、LOWにしてPWM出力無効状態にする
  pinMode(servoPin, OUTPUT);
  digitalWrite(servoPin, LOW);
  Serial.println("servoPin set LOW (PWM disabled).");

  // 状態「STATE: WAIT_FOR_SERVER」をサーバーへ送信
  currentState = STATE_WAIT_FOR_SERVER;
  sendState("STATE: WAIT_FOR_SERVER");
}

void loop() {
  // UDPからのコマンドをチェック
  String udpMsg = readUDPMessage();
  if (udpMsg.length() > 0) {
    Serial.print("Received UDP command: ");
    Serial.println(udpMsg);

    if (currentState == STATE_WAIT_FOR_SERVER && udpMsg.startsWith("COMMAND: START_PWM")) {
      // サーバーからの指示を受け、PWM制御開始（Servoライブラリでattach）
      Serial.println("Starting PWM operation...");
      servo1.attach(servoPin, 1000, 2000);
      currentState = STATE_PWM_OPERATION;
      sendState("STATE: PWM_STARTED");
    }
  }

  // PWM操作状態なら、サーボの角度を順に変化させる
  if (currentState == STATE_PWM_OPERATION) {
    servo1.write(0);    // 0°（1msパルス）
    delay(1000);
    servo1.write(90);   // 90°（約1.5msパルス）
    delay(1000);
    servo1.write(180);  // 180°（2msパルス）
    delay(1000);
  }
}
