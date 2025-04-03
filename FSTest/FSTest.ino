#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const int digitalInputPin = 3;
const int analogPin0 = 26;
const int analogPin1 = 27;
const int analogPin2 = 28;

const char* ssid = "Potato6";
const char* password = "50lanumtu6";
IPAddress serverIP(192, 168, 1, 43);
unsigned int serverPort = 5000;
WiFiUDP udp;

const char* logFileName = "/log.bin";

struct LogData {
  uint32_t micros;
  uint16_t analog0;
  uint16_t analog1;
  uint16_t analog2;
  uint8_t digital;
};

unsigned long lastMicros = 0;
const unsigned long interval = 1000; // 1000Hz = 1ms周期

bool recording = true;
bool sent = false;
File logFile;

// ボタン状態＆チャタリング除去用
bool buttonPressed = false;
unsigned long lastButtonChangeTime = 0;
const unsigned long debounceDelay = 200; // ms

void setup() {
  pinMode(digitalInputPin, INPUT);
  Serial.begin(115200);
  analogReadResolution(12);

  // LittleFS（Pico用：戻り値なし）
  LittleFS.begin();
  Serial.println("[INFO] LittleFS initialized");

  // WiFi接続
  WiFi.begin(ssid, password);
  Serial.print("[INFO] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n[INFO] WiFi connected. IP=");
  Serial.println(WiFi.localIP());

  // ログファイル初期化
  LittleFS.remove(logFileName);  // 前回分削除
  logFile = LittleFS.open(logFileName, "w");
  if (!logFile) {
    Serial.println("[ERROR] Failed to create log file");
    while (1);
  }

  Serial.println("[INFO] Logging started");
}

void loop() {
  // ボタン状態チェック＋チャタリング除去
  bool currButton = digitalRead(digitalInputPin);
  if (currButton != buttonPressed && (millis() - lastButtonChangeTime > debounceDelay)) {
    lastButtonChangeTime = millis();
    buttonPressed = currButton;

    if (buttonPressed && recording) {
      Serial.println("[INFO] Logging stopped. Sending log...");
      recording = false;

      logFile.close();
      delay(100);
      File readFile = LittleFS.open(logFileName, "r");
      if (!readFile) {
        Serial.println("[ERROR] Failed to open log file for reading");
        return;
      }

      size_t count = 0;
      LogData entry;

      const int maxPacketSize = 512;
      char packet[maxPacketSize];
      size_t offset = 0;
      int perPacketCount = 0;

      while (readFile.read((uint8_t*)&entry, sizeof(entry)) == sizeof(entry)) {
        // データを1行の文字列に変換
        char line[64];
        int len = snprintf(line, sizeof(line),
          "t=%lu,a0=%u,a1=%u,a2=%u,d=%u\n",
          entry.micros, entry.analog0, entry.analog1, entry.analog2, entry.digital);

        // パケットに追加（入らなければ送信）
        if (offset + len >= maxPacketSize || perPacketCount >= 10) {
          if (WiFi.status() == WL_CONNECTED) {
            udp.beginPacket(serverIP, serverPort);
            udp.write(packet, offset);
            udp.endPacket();
          }
          offset = 0;
          perPacketCount = 0;
        }

        memcpy(packet + offset, line, len);
        offset += len;
        perPacketCount++;
        count++;
      }

      // 残りがあれば送信
      if (offset > 0) {
        if (WiFi.status() == WL_CONNECTED) {
          udp.beginPacket(serverIP, serverPort);
          udp.write(packet, offset);
          udp.endPacket();
        }
      }

      readFile.close();
      Serial.printf("[INFO] Sent %d records in grouped packets\n", count);
      LittleFS.remove(logFileName);
      sent = true;
    }
  }

  // 記録処理（1000Hz）
  if (recording && (micros() - lastMicros >= interval)) {
    lastMicros = micros();

    LogData entry;
    entry.micros = lastMicros;
    entry.analog0 = analogRead(analogPin0);
    entry.analog1 = analogRead(analogPin1);
    entry.analog2 = analogRead(analogPin2);
    entry.digital = digitalRead(digitalInputPin);

    if (logFile) {
      size_t written = logFile.write((uint8_t*)&entry, sizeof(entry));
      if (written != sizeof(entry)) {
        Serial.println("[ERROR] Failed to write to log file");
      }
    }

    // シリアルデバッグ
    Serial.printf("[LOG] t=%lu a0=%u a1=%u a2=%u d=%u\n", entry.micros, entry.analog0, entry.analog1, entry.analog2, entry.digital);
  }

  // 送信完了後に停止
  if (sent) {
    Serial.println("[INFO] Send complete. Halting.");
    while (1);  // 完全停止（必要なら削除してもOK）
  }
}
