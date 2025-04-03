#include <Arduino.h>
#include <Servo.h>

static Servo s_servo; 

void setup() {
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(14, INPUT);
  pinMode(15, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  s_servo.attach(PIN_D22);
  //s_servo.write(90);
  Serial.begin(115200);  // シリアル通信の準備をする
  while (!Serial);                                // 準備が終わるのを待つ
  Serial.println("プログラム開始");  // シリアル通信でメッセージをPCに送信
  // put your setup code here, to run once:
}

int t = 0;
int freq = 1000;

void light(){
  if (analogRead(A2) < 900){
    digitalWrite(LED1, HIGH);
  }else{
    digitalWrite(LED1, LOW);
  }
  if (analogRead(A3) < 450){
    digitalWrite(LED2, HIGH);
  }else{
    digitalWrite(LED2, LOW);
  }
  if (digitalRead(14) == 1){
    digitalWrite(LED3, HIGH);
  }else{
    digitalWrite(LED3, LOW);
  }
}
void dispValue(){
  Serial.println(analogRead(A3));
}

void loop() {
  digitalWrite(15, HIGH);
  //digitalWrite(LED0, HIGH);
  s_servo.write(160);
  t = millis();
  while (millis() - t < freq/2){
    light();
    dispValue();
  }  
  digitalWrite(15, LOW);
  //digitalWrite(LED0, LOW);
  s_servo.write(30);
  t = millis();
  while (millis() - t < freq/2){
    light();
    dispValue();
  }
}
