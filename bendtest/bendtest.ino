#include <Flash.h>
#include <iostream>
#include <string>

void setup() {
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  Serial.begin(115200);             // シリアル通信の準備をする
  while (!Serial);                  // 準備が終わるのを待つ
  Serial.println("プログラム開始");    // シリアル通信でメッセージをPCに送信
}

void loop() {
  Serial.println(analogRead(A3));

}
