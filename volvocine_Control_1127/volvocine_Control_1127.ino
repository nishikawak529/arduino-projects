#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>
#include "commands.h"
#include "datasave.h"
#include "dataprocess.h" // 追加
#include <Servo.h>
#include <cmath>
static Servo s_servo; 

const bool isRobot = true; // trueならロボット、falseなら測定装置
const short touch = isRobot ? 14 : 16;
const short servopin = isRobot ? 22 : 20;

void setup() {
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(servopin, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(touch, INPUT);
  analogWriteSetDefaultFreq(50);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(115200);  // シリアル通信の準備をする
  while (!Serial);                                // 準備が終わるのを待つ
  Serial.println("プログラム開始");  // シリアル通信でメッセージをPCに送信
  digitalWrite(26,HIGH);
  Flash.mkdir("dir/");
}

using namespace std;
int threshold = 0;
bool smode = 1;
long t = 0;
long millisnow = 0;
const int datalim = 15000;
short b2[datalim];
int b3[datalim];
bool b4[datalim];
short b5[datalim];
int counter = 0;
short snum = 0;
int gtime = 0;
int ltime = 0;
int datasize = 0;
int MaxAngle = 145;
int MinAngle = 45;
double Rangle = 40;
int freq = 1000/1.5;
int phasef = 0;
int phasei = 0;
double kappa = 0.2;
double frequ = 1000.0/freq;
double Ufrequ = 1000.0/freq;
double diff = 0;
double dff = 0;
double pdelay = 0.00;
const float pi = 3.14;
double light = 0;
const int LightWindowSize2 = 120;
int light2 = 0;
int countpoint = 0;
int LightWindowSize = 0;

String myTime;
String s;

void loop() {
  pdelay = 0.41;
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
      if (digitalRead(touch) == HIGH) {
          s_servo.detach();
        while (digitalRead(touch) == HIGH)
          ;
        if (smode == 0) {
          Serial.println("stop");
          smode = 1;
          if (snum > 5) {
            datasave(b5,b4,b3,b2,counter);
            Serial.println("Data saved");
          }
          counter = 0;
          snum = 0;
        } else {
          smode = 0;
          Serial.println("start");
        }
      }
      if (smode == 0) {
        if (snum == 0){
          s_servo.attach(servopin);
          t = millis();
        }
        phasei = t + freq/2;
        millisnow = millis();
        while (freq - (millisnow - t) > 0) {
          dff = millisnow-t;
          s_servo.write(int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2))));
          b2[counter] = analogRead(A3);
          b3[counter] = millisnow - ltime + gtime;
          if ((freq/2)-dff > 0){
            b4[counter] = 1;
            countpoint = counter;
            
          }
          else{
            b4[counter] = 0;
          }
          b5[counter] = analogRead(A2);
          //Serial.println(b4[counter]);
          //delay(16);
          //b2[counter] = light;
          //b2[counter] = Rangle;
          //b5[counter] = threshold;
          //[counter] = int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2)));
          //Serial.println((freq/2)-dff);
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter]);
            }
          }
          counter = counter + 1;
          millisnow = millis();
        }
        t = millis();
        snum = snum + 1;
        if (phasef != 0 && phasei != 0 && abs(phasef-phasei)<freq/2){
          diff = phasef-phasei;
          if (light > 10){
            frequ = Ufrequ;
          }
          else{
            frequ = Ufrequ - kappa*sin(2*pi* (diff/freq-   (pdelay+(light)*0.05)/2*frequ/1.5) );
          }
        }
        else{
          frequ = Ufrequ;
        }
        freq = round(1000.0/frequ);
        threshold = calculateDynamicThreshold(b2, counter);
        light = 0;
        LightWindowSize = counter - countpoint;
        for (int i = 0; i < LightWindowSize; ++i) {
          if (b5[counter-1-i] > 50){
            light += b5[counter-1-i] - 50;
          }          
        }
        //light = log(1+light/LightWindowSize);
        light = 0;
        //Rangle = 2*calculateAmplitude(frequ, 0.94*(0.01+0.99*(7-light)/7));
        Rangle = 100;
        //MinAngle = MaxAngle-Rangle;
        MaxAngle = MinAngle+Rangle;
        //Serial.println(light);
      }
    }
  }
}