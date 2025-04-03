#include <string>
#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include "datasave.h"

String f;
String file;
String temp;
String snewvalue;
double newvalue;
int sampledatasize = 100;
double beta = 0;
int samplenumber = 10;
int ttemp = 0;
int btemp[1000];
int ctemp = 0;
int ctemp2 = 0;
int phaseftemp = 0;
int phaseitemp = 0;
double difftemp = 0;
int benddata;
int ubenddata;
int lbenddata;
File myFile;

void fileopen(String s){
  myFile = Flash.open(("dir/" + s + ".txt").c_str());
  while (myFile.available()) {
    Serial.write(myFile.read());
  }
  myFile.close();
  Flash.remove(("dir/" + s + ".txt").c_str());
}

void filedelete(){
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
        if (Flash.exists(("dir/servo" +  std::__1::to_string(i) + ".txt").c_str())) {
          Flash.remove(("dir/servo" +  std::__1::to_string(i) + ".txt").c_str());
          Serial.print(i);
        }if (Flash.exists(("dir/free" +  std::__1::to_string(i) + ".txt").c_str())) {
          Flash.remove(("dir/free" +  std::__1::to_string(i) + ".txt").c_str());
        }
      }
      Serial.println("all files are deleted");
      break;
    } else if (f == "exit") {
      break;
    } else if (!(f.length() == 0)) {
      Serial.println("No such file.");
    }
  }
}

void setThreshold(){
  Serial.println("Straighten the freejoint and type ok");
  while (true) {
    temp = Serial.readString();
    if (!(temp.length() == 0)) {
      Serial.println(temp.c_str());
      temp.trim();
    }
    if (temp == "ok") {
      benddata = 0;
      Serial.println("Getting data");
      for (int i = 0; i < sampledatasize; i++) {
        benddata = benddata+analogRead(A3);
      }
      ubenddata = benddata/sampledatasize;
      Serial.println(("Upper data = "+std::__1::to_string(ubenddata)).c_str());
      break;
    }
  }
  Serial.println("Bend the freejoint and type ok");
  while (true) {
    temp = Serial.readString();
    if (!(temp.length() == 0)) {
      Serial.println(temp.c_str());
      temp.trim();
    }
    if (temp == "ok") {
      benddata = 0;
      Serial.println("Getting data");
      for (int i = 0; i < sampledatasize; i++) {
        benddata = benddata + analogRead(A3);
      }
      lbenddata = benddata/sampledatasize;
      Serial.println(("Lower data = "+std::__1::to_string(lbenddata)).c_str());
      break;
    }
  }
  benddata = (lbenddata+ubenddata)/2;
  thresholdSave(benddata);
  return;
}

void setBeta(int freq, int threshold){
  analogWrite(21, 14);
  Serial.println("Type ok to start");
  while (true) {
    temp = Serial.readString();
    if (!(temp.length() == 0)) {
      Serial.println(temp.c_str());
      temp.trim();
    }
    if (temp == "ok") {
      beta = 0;
      ctemp = 1;
      ctemp2 = 0;
      Serial.println("Getting data");
      ttemp = millis();
      analogWrite(21, 30);
      for (int i = 0; i < samplenumber; i++) {
        while (millis() - ttemp < freq/2) {
          btemp[ctemp] = analogRead(A3);
          if (ctemp >= 2){
            if (btemp[ctemp] <= threshold && btemp[ctemp-1] > threshold){
              phaseftemp = millis();
            }
          }
          ctemp = ctemp + 1;
        }
        analogWrite(21, 14);
        ttemp = millis();
        phaseitemp = millis();
        while (millis() - ttemp < freq/2) {
          btemp[ctemp] = analogRead(A3);
          if (ctemp >= 2){
            if (btemp[ctemp] <= threshold && btemp[ctemp-1] > threshold){
              phaseftemp = millis();
            }
          }
          ctemp = ctemp + 1;
        }
        analogWrite(21, 30);
        ttemp = millis();
        if (phaseftemp != 0 && phaseitemp != 0 && abs(phaseftemp-phaseitemp)<freq/2){
          difftemp = phaseftemp-phaseitemp;
          beta = beta + difftemp/freq*2;
          ctemp2 += 1;
        }
        phaseftemp = 0;
        phaseitemp = 0;
      }
      break;
    }
  }
  analogWrite(21, 0);
  beta = beta/ctemp2;
  Serial.println(("beta = "+std::__1::to_string(beta)).c_str());
  saveBeta(beta);
  return;
}

double rewritevalue(String s){
  Serial.println("New value of " + s);
  while (true) {
    snewvalue = Serial.readString();
    if (!(snewvalue.length() == 0)) {
      newvalue = std::__1::stoi(snewvalue.c_str());
      Serial.println(newvalue);
      break;
    }
  }
  return newvalue;
}

void ls(){  
  if (Flash.exists("dir/threshold.txt")) {
     Serial.println("dir/threshold.txt");
  }
  for (int i = 0; i < 100; i++) {
    if (Flash.exists(("dir/servo" +  std::__1::to_string(i) + ".txt").c_str())) {
      Serial.println(("dir/servo" +  std::__1::to_string(i) + ".txt").c_str());
    }if (Flash.exists(("dir/free" +  std::__1::to_string(i) + ".txt").c_str())) {
      Serial.println(("dir/free" +  std::__1::to_string(i) + ".txt").c_str());
    }
  }
  return;
}

