#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>

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
File myFile;
bool smode = 1;
long t = 0;
const int datalim = 11000;
unsigned int b2[datalim];
unsigned int b3[datalim];
bool b4[datalim];
int counter = 0;
int fnum = 1;
int snum = 0;
int gtime = 0;
int ltime = 0;
int datasize = 0;
int freq = 910;
int phasef = 0;
int phasei = 0;
double kappa = 1;
double frequ = 1000.0/freq;
double diff = 0;
double pdelay = 0.25;

String myTime;
String s;
String f;
String file;
String temp;

void loop() {
  if (Serial.available()) {
    s = Serial.readString();
    Serial.println(s.c_str());
    s.trim();
    if (Flash.exists(("dir/" + s + ".txt").c_str())) {
      myFile = Flash.open(("dir/" + s + ".txt").c_str());
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      myFile.close();
      Flash.remove(("dir/" + s + ".txt").c_str());
    } else if (s == "delete") {
      Serial.println("which file do you delete?");
      while (true) {
        f = Serial.readString();
        if (!(f.length() == 0)) {
          Serial.println(f.c_str());
          f.trim();
        }
        file = ("dir/" + f + ".txt").c_str();
        if (Flash.exists(file)) {
          Flash.remove(file);
          Serial.println((file + " is deleted").c_str());
          break;
        } else if (f == "all") {
          for (int i = 1; i < 100; i++) {
            if (Flash.exists(("dir/servo" + to_string(i) + ".txt").c_str())) {
              Flash.remove(("dir/servo" + to_string(i) + ".txt").c_str());
              Serial.print(i);
            }if (Flash.exists(("dir/free" + to_string(i) + ".txt").c_str())) {
              Flash.remove(("dir/free" + to_string(i) + ".txt").c_str());
            }
          }
          Serial.println("all files are deleted");
          break;
        } else if (f == "short") {
          for (int i = 1; i < 100; i++) {
            Serial.print(i);
            Serial.print("    ");
            if (Flash.exists(("dir/servo" + to_string(i) + ".txt").c_str())) {
              datasize = 0;
              myFile = Flash.open(("dir/servo" + to_string(i) + ".txt").c_str());
              while (myFile.available()) {
                temp = myFile.read();
                datasize = datasize + 1;
              }
              datasize = datasize/5;
              myFile.close();
              Serial.println(datasize);
              if (datasize <= 650) {
                Flash.remove(("dir/servo" + to_string(i) + ".txt").c_str());
              }
            }
          }
          Serial.println("short files are deleted");
          break;
        } else if (f == "exit") {
          break;
        } else if (!(f.length() == 0)) {
          Serial.println("No such file.");
        }
      } 
    } else if (s == "freq") {
      Serial.println("New value of freq");
      while (true) {
        f = Serial.readString();
        if (!(f.length() == 0)) {
          freq = stoi(f.c_str());
          Serial.println(freq);
          frequ = 1.0/freq;
          break;
        }
      }
    } else if (s == "f") {
      Serial.println("New value of frequency");
      while (true) {
        f = Serial.readString();
        if (!(f.length() == 0)) {
          frequ = std::stod(f.c_str());
          Serial.println(frequ);
          freq = round(1.0/frequ);
          break;
        }
      }
    } else if (s == "kappa") {
      Serial.println("New value of kappa");
      while (true) {
        f = Serial.readString();
        if (!(f.length() == 0)) {
          kappa = std::stod(f.c_str());
          Serial.println(kappa);
          break;
        }
      }
    }else if (s == "pdelay") {
      Serial.println("New value of delay");
      while (true) {
        f = Serial.readString();
        if (!(f.length() == 0)) {
          pdelay = std::stod(f.c_str());
          Serial.println(pdelay);
          break;
        }
      }
    } else if (s == "ls") {
      for (int i = 0; i < 100; i++) {
        if (Flash.exists(("dir/servo" + to_string(i) + ".txt").c_str())) {
          Serial.println(("dir/servo" + to_string(i) + ".txt").c_str());
        }if (Flash.exists(("dir/free" + to_string(i) + ".txt").c_str())) {
          Serial.println(("dir/free" + to_string(i) + ".txt").c_str());
        }
      }
    } else if (s == "time") {
      Serial.println("send the time");
      while (true) {
        myTime = Serial.readString();
        if (!(myTime.length() == 0)) {
          gtime = stoi(myTime.c_str());
          digitalWrite(LED0, HIGH);
          digitalWrite(LED1, HIGH);
          digitalWrite(LED2, HIGH);
          digitalWrite(LED3, HIGH);
          ltime = millis();
          Serial.println(gtime);
          break;
        }
      }
    } else {
      String errorMessage = "No such file named " + String("dir/") + s + ".txt";
      Serial.println(errorMessage.c_str());
    }
  } else {
    if (digitalRead(20) == HIGH) {
      if (smode == 0) {
        analogWrite(21, 0);
        smode = 1;
        if (snum > 5) {
          while (Flash.exists(("dir/servo" + to_string(fnum) + ".txt").c_str())) {
            fnum = fnum + 1;
          }
          myFile = Flash.open(("dir/servo" + to_string(fnum) + ".txt").c_str(), FILE_WRITE);
          for (int i = 0; i < counter; i++) {
            myFile.println((String(b4[i])+" "+String(b3[i])+" "+String(b2[i])).c_str());
          }
          myFile.close();
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
          if (b2[counter] <=400 && b2[counter-1] > 400){
            phasef = millis();
          }
        }
        counter = counter + 1;
      }
      phasei = millis();
      t = millis();
      analogWrite(21, 14);
      while (millis() - t < freq/2) {
        b2[counter] = analogRead(A3);
        b3[counter] = millis()-ltime+gtime;
        b4[counter] = 0;
        if (counter >= 2){
          if (b2[counter] <=400 && b2[counter-1] > 400){
            phasef = millis();
          }
        }
        counter = counter + 1;
      }
      analogWrite(21, 30);
      t = millis();
      snum = snum + 1;
      Serial.print(snum);
      if (phasef != 0 && phasei != 0 && abs(phasef-phasei)<freq/2){
        diff = phasef-phasei;
        frequ = 1.1-kappa*(diff/freq-pdelay/2);
      }
      else{
        frequ = 1.1;
      }
      freq = round(1000.0/frequ);
      phasef = 0;
      phasei = 0;
    } else {
    }
  }
}

