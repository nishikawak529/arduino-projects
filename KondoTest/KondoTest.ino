#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi設定
const char* ssid = "Potato6";
const char* password = "50lanumtu6";

// サーバー（PC側）のIPアドレスとUDPポート（状態管理用）
IPAddress serverIP(192, 168, 1, 18);
unsigned int serverPort = 6000;

// UDP通信オブジェクト
WiFiUDP udp;

// システム状態の列挙型
enum SystemState {
  STATE_INIT,           // 初期状態
  STATE_SERVO_POWERED,  // サーボ電源接続完了（PC側でキー入力済み）
  STATE_UART_STARTED,   // UART初期化完了
  STATE_OPERATION       // 本来のサーボ動作開始（ID読み出し処理を実施）
};

SystemState currentState = STATE_INIT;
bool operationCompleted = false;  // 操作処理が完了したか

const int EN_IN_PIN = 2;  // サーボ用信号線のEN_INピン

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
  if (packetSize) {
    char incomingPacket[100];
    int len = udp.read(incomingPacket, 99);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    return String(incomingPacket);
  }
  return "";
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting state management...");

  // Wi-Fi接続開始
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // UDP用ローカルポート（例：5002）を開始
  udp.begin(5002);

  // EN_INピンを出力に設定し、HIGHにする
  pinMode(EN_IN_PIN, OUTPUT);
  digitalWrite(EN_IN_PIN, LOW);

  // この時点でユーザが外部でサーボ電源を接続したと仮定し、
  // 状態「STATE: SERVO_POWERED」をサーバーへ送信
  currentState = STATE_SERVO_POWERED;
  sendState("STATE: SERVO_POWERED");
}

void loop() {
  // UDPからのコマンドを確認
  String udpMsg = readUDPMessage();
  if (udpMsg.length() > 0) {
    Serial.print("Received UDP command: ");
    Serial.println(udpMsg);

    if (currentState == STATE_SERVO_POWERED && udpMsg.startsWith("COMMAND: START_UART")) {
      // UART初期化（シリアルサーボ用UART）
      Serial1.begin(115200, SERIAL_8E1);
      Serial.println("UART (Serial1) initialized for servo.");
      currentState = STATE_UART_STARTED;
      sendState("STATE: UART_STARTED");
    }
    else if (currentState == STATE_UART_STARTED && udpMsg.startsWith("COMMAND: START_OPERATION")) {
      Serial.println("Transitioning to OPERATION state...");
      currentState = STATE_OPERATION;
      sendState("STATE: OPERATION_STARTED");
    }
  }

  // 本来の動作開始状態：ID読み出しコマンドの送信と受信処理を実行（1回だけ）
  if (currentState == STATE_OPERATION && !operationCompleted) {
    delay(500);
    uint8_t idCommand[4] = { 0xF4, 0x01, 0x01, 0x01 };

    // ID読み出しコマンドの送信
    digitalWrite(EN_IN_PIN, HIGH);
    sleep_us(200);
    Serial1.write(idCommand, 4);
    Serial1.flush();
    digitalWrite(EN_IN_PIN, LOW);
    sleep_us(200);
    uint8_t receivedByte = Serial1.read();
    Serial.print("【デバッグ】受信データ: 0x");
    Serial.println(receivedByte, HEX);

    while (true){
      Serial.println("【OPERATION】Sending ID read command...");
      // ID読み出しコマンドの準備
      // コマンド構成：CMDに0xFF、その後SCとして0x00を3回送信
      uint8_t idCommand[4] = { 0xFF, 0x00, 0x00, 0x00 };

      // ID読み出しコマンドの送信
      digitalWrite(EN_IN_PIN, HIGH);
      sleep_us(200);
      Serial1.write(idCommand, 4);
      Serial1.flush();
      // コマンド送信直後、EN_INピンをLOWにして受信モードに切り替え
      digitalWrite(EN_IN_PIN, LOW);
      sleep_us(200);
      uint8_t receivedByte = Serial1.read();
      Serial.print("【デバッグ】受信データ: 0x");
      Serial.println(receivedByte, HEX);
      delay(1000);
    }
    operationCompleted = true;
    Serial.println("【OPERATION】ID read operation completed.");
  }

  // STATE_OPERATIONになってから他の処理を実施する場合は、ここに追加してください
}
