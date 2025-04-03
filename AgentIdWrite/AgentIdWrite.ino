#include <Arduino.h>
#include <LittleFS.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // LittleFS をマウント(初期化)
  if(!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
    while(true) {
      // エラー停止
    }
  }

  // config.txt を書き込みモードで開く
  File f = LittleFS.open("/config.txt", "w");
  if(!f) {
    Serial.println("Failed to open /config.txt for writing!");
  } else {
    // エージェントIDとして42を書き込む
    f.println("2");
    f.close();
    Serial.println("Wrote to /config.txt successfully!");
  }

  // ---- 読み込みテスト (オプション) ----
  File f2 = LittleFS.open("/config.txt", "r");
  if(!f2) {
    Serial.println("Failed to open /config.txt for reading!");
  } else {
    String line = f2.readStringUntil('\n');
    f2.close();
    Serial.print("Read value: ");
    Serial.println(line);
  }
}

void loop() {
  // 何もしない
}
