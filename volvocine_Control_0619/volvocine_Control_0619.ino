#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>
#include "commands.h"
#include "datasave.h"
#include <Servo.h>
// 係数を定義a
const float C1 = -0.00316;
const float C2 = -0.12949;
const float C3 = 0.00002;
const float C4 = 0.00313;
const float C5 = 0.03227;
const float C6 = 0.09246;

static Servo s_servo; 
const short touch = 16;
const short servopin = 20;

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
const int datalim = 10000;
short b2[datalim];
int b3[datalim];
bool b4[datalim];
short b5[datalim];
int counter = 0;
short snum = 0;
int gtime = 0;
int ltime = 0;
int datasize = 0;
int MaxAngle = 160;
int MinAngle = 30;
double Rangle = 40;
short freq = 1000/1.5;
int phasef = 0;
int phasei = 0;
double kappa = 0.25;
double frequ = 1000.0/freq;
double Ufrequ = 1000.0/freq;
double diff = 0;
double dff = 0;
double pdelay = 0.31;
const float pi = 3.14;
int light = 0;

String myTime;
String s;

void loop() {
  threshold = thresholdLoad();
  //pdelay = loadBeta();
  pdelay = 0.31;
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
        while (millis() - t < freq/2) {
          dff = millis()-t;
          s_servo.write(int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2))));
          b2[counter] = analogRead(A3);
          //b2[counter] = int((MaxAngle+MinAngle)/2-(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2)));
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 1;
          b5[counter] = analogRead(A2);
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
        if (counter > 65){
          for (int i = 0; i < 60; ++i) {
            light += b5[counter-1-i];
          }
          light = light/60;
        }
        
        //double solution_max = calculateAmplitude(frequ, 0.10);
        //double solution_min = calculateAmplitude(frequ, 0);
        //double difference = solution_max - solution_min;
        //double lambda = (1022.0-static_cast<double>(light))/1022.0; 

        //Rangle = difference*lambda;
        //MaxAngle = 115+solution_min+Rangle;
        //MinAngle = 115-solution_min-Rangle;
        MaxAngle = 115+65;
        MinAngle = 115-65;
        //Serial.print("frequ = ");
        //Serial.print(frequ);
        //Serial.print(": solution_max = ");
        //Serial.print(solution_max);
        //Serial.print(": solution_min = ");
        //Serial.print(solution_min);
        //Serial.print(": light = ");
        //Serial.print(light);
        //Serial.print(": lambda = ");
        //Serial.print(lambda);
        //Serial.print(": solution_min+Rangle = ");
        //Serial.println(solution_min+Rangle);

        //Rangle = 25+40*(1022-light)/1022; //ここを変更
        //MaxAngle = 90+Rangle;
        //MinAngle = 90-Rangle;

        phasei = t;
        while (millis() - t < freq/2) {
          dff = millis()-t;
          s_servo.write(int((MaxAngle+MinAngle)/2+(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2))));
          b2[counter] = analogRead(A3);
          //b2[counter] = int((MaxAngle+MinAngle)/2+(MaxAngle-MinAngle)/2*cos(pi*dff/(freq/2)));
          b3[counter] = millis()-ltime+gtime;
          b4[counter] = 0;
          b5[counter] = analogRead(A2);
          if (counter >= 2){
            if (b2[counter] <= threshold && b2[counter-1] > threshold && b2[counter] > 300 && b2[counter-1] > 300){
              diff = b2[counter-1]-threshold;
              phasef = b3[counter-1]+(b3[counter]-b3[counter-1])*(diff)/(b2[counter-1]-b2[counter])+ltime-gtime;
            }
          }
          counter = counter + 1;
        }
        t = millis();
        snum = snum + 1;
        //light = 0;
        //if (counter > 70){
        //  for (int i = 0; i < 60; ++i) {
        //    light += b5[counter-1-i];
        //  }
        //  light = light/60;
        //}
        //if (phasef != 0 && phasei != 0 && abs(phasef-phasei)<freq/2){
        //  diff = phasef-phasei;
        //  frequ = Ufrequ - kappa*sin(2*pi*(diff/freq-pdelay/2*(1+0.01*(solution_max-solution_min))*frequ));
        //  //frequ = Ufrequ - kappa*sin(2*pi*(diff/freq));
        //}
        //else{
        //  frequ = Ufrequ;
        //}
        //Rangle = 130*(2100-light)/2100;
        //MaxAngle = 170;
        //MinAngle = 170-Rangle;
        //frequ = Ufrequ*(2044-light)/2044;
        //Serial.println(light);
        freq = round(1000.0/frequ);  
        phasef = 0;
        phasei = 0;
      }
    }
  }
}
// 振幅の解を計算する関数
float calculateAmplitude(float f, float Fz) {
  // 解を求める範囲
  const float minA = 0;
  const float maxA = 80.0;
  const float step = 0.1;

  // 最良の解を探す
  float bestA = minA;
  float smallestError = abs(C1 * minA + C2 * f + C3 * minA * minA + C4 * f * minA + C5 * f * f + C6 - Fz);

  for (float A = minA; A <= maxA; A += step) {
    float Fz_eqn = C1 * A + C2 * f + C3 * A * A + C4 * f * A + C5 * f * f + C6;
    float error = abs(Fz_eqn - Fz);
    if (error < smallestError) {
      bestA = A;
      smallestError = error;
    }
  }

  // 解が範囲外であれば調整
  if (bestA < minA) {
    return minA;
  } else if (bestA > maxA) {
    return maxA;
  } else {
    return bestA;
  }
}