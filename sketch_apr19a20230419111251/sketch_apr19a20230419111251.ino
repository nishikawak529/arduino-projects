#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>
#include "commands.h"
#include "datasave.h"

void setup() {
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(20, INPUT);
  analogWriteSetDefaultFreq(50);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(115200);  // シリアル通信の準備をする
  while (!Serial);                                // 準備が終わるのを待つ
  Serial.println("プログラム開始");  // シリアル通信でメッセージをPCに送信

  Flash.mkdir("dir/");
}

using namespace std;
int threshold = 0;
bool smode = 1;
long t = 0;
const int datalim = 10000;
int b2[datalim];
int b3[datalim];
bool b4[datalim];
int counter = 0;
int snum = 0;
int gtime = 0;
int ltime = 0;
int datasize = 0;
int freq = 910;
int phasef = 0;
int phasei = 0;
double kappa = 0.2;
double frequ = 1000.0/freq;
double Ufrequ = 1000.0/freq;
double diff = 0;
double pdelay = 0.25;
const float pi = 3.14;

String myTime;
String s;

void loop() {
  threshold = thresholdLoad();
  pdelay = loadBeta();
  while(true){
    if (Serial.available()) {
      s = Serial.readString();
      Serial.println(s.c_str());
      s.trim();
      if (Flash.exists(("dir/" + s + ".txt").c_str())) {
        fileopen(s);
      } else if (s == "delete") {
        filedelete(); 
      } else if (s == "set") {
        setThreshold();
        threshold = thresholdLoad();
      } else if (s == "setbeta") {
        setBeta(freq, threshold);
        pdelay = loadBeta();
      } else if (s == "freq") {
        freq = rewritevalue(s);
        frequ = 1.0/freq;
      } else if (s == "f") {
        frequ = rewritevalue(s);
        freq = 1.0/frequ;
      } else if (s == "kappa") {
        kappa = rewritevalue(s);
      }else if (s == "pdelay") {
        pdelay = rewritevalue(s);
      } else if (s == "ls") {
        ls();
      } else if (s == "time") {
        Serial.println("send the time");
        while (true) {
          myTime = Serial.readString();
          if (!(myTime.length() == 0)) {
            gtime = std::__1::stoi(myTime.c_str());
            ltime = millis();
            Serial.println(gtime);
            break;
          }
        }
        digitalWrite(LED0, HIGH);
      } else {
        String errorMessage = "No such file named " + String("dir/") + s + ".txt";
        Serial.println(errorMessage.c_str());
      }
    }else {
      if (digitalRead(20) == HIGH) {
        if (smode == 0) {
          analogWrite(21, 30);
          smode = 1;
          if (snum > 5) {
            datasave(b4,b3,b2,counter);
          }
          counter = 0;
          snum = 0;
        } else {
          smode = 0;
        }
        while (digitalRead(20) == HIGH)
          ;
      }
      if (smode == 0) {
        if (snum == 0){
          analogWrite(21, 30);
          t = millis();
        }
        while (millis() - t < freq/2) {
          b2[counter] = analogRead(A3);
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 1;
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter]);
            }
          }
          counter = counter + 1;
        }
        analogWrite(21, 14);
        delay(10000);
        t = millis();
        phasei = t;
        while (millis() - t < freq/2) {
          b2[counter] = analogRead(A3);
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 0;
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter])+ltime-gtime;
            }
          }
          counter = counter + 1;
        }
        analogWrite(21, 30);
        t = millis();
        snum = snum + 1;
        if (phasef != 0 && phasei != 0 && abs(phasef-phasei)<freq/2){
          diff = phasef-phasei;
          frequ = Ufrequ - kappa*sin(2*pi*(diff/freq-pdelay/2));
        }
        else{
          frequ = Ufrequ;
        }
        freq = round(1000.0/frequ);  
        phasef = 0;
        phasei = 0;
      } else {
      }
    }
  }
}
