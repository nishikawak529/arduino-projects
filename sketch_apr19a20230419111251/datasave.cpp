#include <string>
#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>

File saveFile;
File loadFile;
String thresholds;
String betas;
int thresholdparam = 0;

int fnum = 1;
void datasave(bool b1[], int b3[], int b2[], int counter[]){
  while (Flash.exists(("dir/servo" + std::__1::to_string(fnum) + ".txt").c_str())) {
    fnum = fnum + 1;
  }
  saveFile = Flash.open(("dir/servo" + std::__1::to_string(fnum) + ".txt").c_str(), FILE_WRITE);
  for (int i = 0; i < counter; i++) {
    saveFile.println((String(b1[i])+" "+String(b3[i])+" "+String(b2[i])).c_str());
  }
  saveFile.close();
  return;
}

void thresholdSave(int threshold){
  if (Flash.exists("dir/threshold.txt")){
    Flash.remove("dir/threshold.txt");
  }
  saveFile = Flash.open("dir/threshold.txt", FILE_WRITE);
  saveFile.print(String(threshold));
  saveFile.close();
  Serial.println("Saved threshold is "+String(threshold));
  return;
}

int thresholdLoad(){
  thresholds = "";  // 初期化する
  if (Flash.exists("dir/threshold.txt")) {
    loadFile = Flash.open("dir/threshold.txt");
    while (loadFile.available()) {
      //Serial.write(loadFile.read());
      thresholds += char(loadFile.read());
    }
    //thresholds = loadFile.read();
    loadFile.close();
  }
  else{
    Serial.println("There is no file named threshold.txt");
    thresholds = "350";
  }
  Serial.println(("Threshold is set as "+thresholds).c_str());
  return std::__1::stoi(thresholds.c_str());
}

void saveBeta(double beta){
  if (Flash.exists("dir/beta.txt")){
    Flash.remove("dir/beta.txt");
  }
  saveFile = Flash.open("dir/beta.txt", FILE_WRITE);
  saveFile.print(String(beta));
  saveFile.close();
  Serial.println("Saved beta is "+String(beta));
  return;
}

int loadBeta(){
  betas = "";  // 初期化する
  if (Flash.exists("dir/beta.txt")) {
    loadFile = Flash.open("dir/beta.txt");
    while (loadFile.available()) {
      betas += char(loadFile.read());
    }
    loadFile.close();
  }
  else{
    Serial.println("There is no file named beta.txt");
    betas = "0.25";
  }
  Serial.println(("Beta is set as "+betas).c_str());
  return std::__1::stoi(betas.c_str());
}