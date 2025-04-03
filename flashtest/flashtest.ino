#include <Arduino.h>
#include <File.h>
#include <Flash.h>
#include <iostream>
#include <string>


using namespace std;
File myFile; /**< File object */ 

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Flash.mkdir("dir/");  
}

String s;
String f;
String file;

void loop() {
  while (Serial.available()) {
    s = Serial.readString();
    Serial.write(s.c_str());
    s.trim();
    if (Flash.exists(("dir/" + s + ".txt").c_str())){
      myFile = Flash.open(("dir/" + s + ".txt").c_str());
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      myFile.close();
    }
    else if (s == "delete"){
      Serial.write("which file do you delete?");
      f = Serial.readString();
      Serial.write(f.c_str());
      f.trim();
      while (true){
        f = Serial.readString();
        Serial.write(f.c_str());
        f.trim();
        file = ("dir/" + f + ".txt").c_str();
        if (Flash.exists(file)){
          Flash.remove(file);
          Serial.write((file + " is deleted").c_str());
        }
        else if (f == "all"){
          for (int i=0; i<100; i++){
            if(Flash.exists(("dir/servo" + to_string(i) + ".txt").c_str())) {
              Flash.remove(("dir/servo" + to_string(i) + ".txt").c_str());
            }  
          }  
          for (int i=0; i<100; i++){
            if(Flash.exists(("dir/free" + to_string(i) + ".txt").c_str())) {
              Flash.remove(("dir/free" + to_string(i) + ".txt").c_str());
            }  
          }
          Serial.write("all files are deleted");
        }
        else if (f == "exit"){
          break;
        }        
        else if (!(f.length() == 0)){
          Serial.write("No such file.");
        }
      }
      Serial.write("delete done");      
    }
    else if (s == "ls"){
      for (int i=0; i<100; i++){
        if(Flash.exists(("dir/servo" + to_string(i) + ".txt").c_str())) {
          Serial.println(("dir/servo" + to_string(i) + ".txt").c_str());
        }  
      }  
      for (int i=0; i<100; i++){
        if(Flash.exists(("dir/free" + to_string(i) + ".txt").c_str())) {
          Serial.println(("dir/free" + to_string(i) + ".txt").c_str());
        }  
      }
    }
    else{
      String errorMessage = "No such file named " + String("dir/") + s + ".txt";
     Serial.write(errorMessage.c_str());
    }
  }
}
