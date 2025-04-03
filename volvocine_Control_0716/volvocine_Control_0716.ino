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
// 係数を定義a
const float C1 = -0.00316;
const float C2 = -0.12949;
const float C3 = 0.00002;
const float C4 = 0.00313;
const float C5 = 0.03227;
const float C6 = 0.09246;
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
int MaxAngle = 130;
int MinAngle = 50;
double Rangle = 40;
short freq = 1000/1.5;
int phasef = 0;
int phasei = 0;
double kappa = 0.4;
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
  pdelay = 0.37;
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
          //s_servo.write(160);
          t = millis();
        }
        countpoint = counter;
        while (millis() - t < freq/2) {
          dff = millis()-t;
          s_servo.write(int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2))));
          b2[counter] = analogRead(A3);
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 1;
          b5[counter] = analogRead(A2);
          //delay(16);
          //b2[counter] = light;
          //b2[counter] = Rangle;
          //b5[counter] = threshold;
          //[counter] = int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2)));
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold && b2[counter] != 0 && b2[counter-1] != 0){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter]);
            }
          }
          counter = counter + 1;
        }
        t = millis();
        
        light = 0;
        LightWindowSize = counter - countpoint;
        for (int i = 0; i < LightWindowSize; ++i) {
          if (b5[counter-1-i] > 30){
            light += b5[counter-1-i] - 30;
          }          
        }
        light = log(1+light/LightWindowSize);

        //Rangle = 120*(13.0-light)/13.0;
        Rangle = 120;
        MinAngle = 160-Rangle;
        //double solution_max = calculateAmplitude(frequ, 0.10);
        //double solution_min = calculateAmplitude(frequ, 0);
        //double difference = solution_max - solution_min;
        //double lambda = (1022.0-static_cast<double>(light))/1022.0; 

        //Rangle = difference*lambda;
        //MaxAngle = 115+solution_min+Rangle;
        //MinAngle = 115-solution_min-Rangle;
 
        phasei = t;
        while (millis() - t < freq/2) {
          dff = millis()-t;
          s_servo.write(int((MaxAngle+MinAngle)/2+(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2))));
          b2[counter] = analogRead(A3);
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 0;
          b5[counter] = analogRead(A2);
          //delay(16);
          //b2[counter] = light;
          //b2[counter] = Rangle;
          //b5[counter] = threshold;
          //b5[counter] = int((MaxAngle+MinAngle)/2+(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2)));
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold && b2[counter] != 0 && b2[counter-1] != 0){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter])+ltime-gtime;
            }
          }
          counter = counter + 1;
        }
        t = millis();
        snum = snum + 1;

        if (phasef != 0 && phasei != 0 && abs(phasef-phasei)<freq/2){
          diff = phasef-phasei;
          //frequ = Ufrequ - kappa*sin(2*pi*(diff/freq-pdelay/2*(1+0.01*(solution_max-solution_min))*frequ));
          light2 = 0;
          //if (counter > LightWindowSize2+1){
          //  for (int i = 0; i < LightWindowSize2; ++i) {
          //    if (b5[counter-1-i] >900);{ 
          //      light2 += b5[counter-1-i];
          //    }          
          //  }
          //  light2 = light2/LightWindowSize2;
          //}
          if (Rangle < 80){
            frequ = Ufrequ;
          }
          else{
            frequ = Ufrequ - kappa*sin(2*pi*(diff/freq-pdelay/2*(1+0.4/3000*(105-Rangle)*(105-Rangle))*frequ/1.5));
          }
        }
        else{
          frequ = Ufrequ;
        }
        //Rangle = 120*(2100-light)/2100;
        //MinAngle = 160-Rangle;
        freq = round(1000.0/frequ);  
        phasef = 0;
        phasei = 0;
        threshold = calculateDynamicThreshold(b2, counter);
      }
    }
  }
}