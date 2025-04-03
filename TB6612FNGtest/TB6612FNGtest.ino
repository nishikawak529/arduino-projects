#include "TB6612FNG.h"

// ピンの定義
#define STBY_PIN 10
#define AIN1_PIN 11
#define AIN2_PIN 12
#define PWMA_PIN 13
#define BIN1_PIN 14
#define BIN2_PIN 15
#define PWMB_PIN 16

// TB6612FNG のインスタンス生成
TB6612FNG motorDriver(STBY_PIN, AIN1_PIN, AIN2_PIN, PWMA_PIN, BIN1_PIN, BIN2_PIN, PWMB_PIN);

void setup() {
  motorDriver.init();
}

void loop() {
  // モータA: 50%速度正転、モータB: 100%速度逆転
  motorDriver.motorAForward(64);
  motorDriver.motorBBackward(64);
  delay(2000);

  // 停止
  motorDriver.motorAStop();
  motorDriver.motorBStop();
  delay(1000);

  // モータA: 100%速度逆転、モータB: 50%速度正転
  motorDriver.motorABackward(64);
  motorDriver.motorBForward(64);
  delay(2000);

  // 再度停止
  motorDriver.motorAStop();
  motorDriver.motorBStop();
  delay(1000);
}
