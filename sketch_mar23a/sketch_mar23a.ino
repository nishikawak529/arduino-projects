#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>
int b1,b2 = 0;

void setup() {
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(20, INPUT);
    analogWriteSetDefaultFreq(50);
    pinMode(A2,INPUT);
    pinMode(A3,INPUT);

    Serial.begin(115200);             // シリアル通信の準備をする
    while (!Serial);                  // 準備が終わるのを待つ
    Serial.println("プログラム開始");    // シリアル通信でメッセージをPCに送信
    //analogWrite(21, 14);
    //delay(1000);
}

using namespace std;

void loop() {
  b1 = analogRead(A3);
  b2 = analogRead(A2);
  if (b1 > 465){
    digitalWrite(LED0,HIGH);
  }
  else{
    digitalWrite(LED0,LOW);
  }
  if (b2 > 400){
    digitalWrite(LED1,HIGH);
  }
  else{
    digitalWrite(LED1,LOW);
  }
  Serial.print(b1);
  Serial.print("   ");
  Serial.println(b2);         // シリアル通信でカウンターの値をPCに送信

}
