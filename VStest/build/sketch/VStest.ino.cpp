#line 1 "C:\\Users\\nishi\\Arduino\\VStest\\VStest.ino"
#include <Arduino.h>

// the setup function runs once when you press reset or power the board
#line 4 "C:\\Users\\nishi\\Arduino\\VStest\\VStest.ino"
void setup();
#line 10 "C:\\Users\\nishi\\Arduino\\VStest\\VStest.ino"
void loop();
#line 4 "C:\\Users\\nishi\\Arduino\\VStest\\VStest.ino"
void setup() {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
  }
  
  // the loop function runs over and over again forever
  void loop() {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(200);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(100);                      // wait for a second
  }
