/*
 * Jack the Maker for Mercedes
 * November 2018
 * Read a switch to start the counter, ring the buzzer, 
 * turn on LEDs, and send values to the receiver Arduino
 * to turn off the green taxi lights.
 * Counter counts down from three minutes and then counts up.
 
 # When debugging, ensure that Serial is on 115200 so that you don't 
 # disrupt the comms with the DF Player that are on 9600

 * NRF24L01 code adapted from https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
 * DFPlayer code adapted from https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Copy_your_mp3_into_you_micro_SD_card
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define startMinute 03  //  Change these to change the timer's start time
#define startSecond 01  // To show 03:00, make it 03, 01

Adafruit_7segment matrix = Adafruit_7segment(); // 7-segment counter
unsigned long previousSecondMillis = 0UL;
long oneSecond = 1000UL; // milliseconds per second
int minutes = startMinute;
int seconds = startSecond;
bool countDownFinished = false; // to switch between count down & count up functions
const byte addresses[][6] = {"00001", "00002"};
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

//Arduino pins
int switch_pin = 18; // Switch pin
RF24 radio(7, 8); // CE, CSN pins
byte switchState = 0; // send value to receiving arduin to turn on top light.
int redLed = 6; // Red LED pin
int greenLed = 5; // Green LED pin
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX pins for DFPlayer comms


void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200); // serial should be on 115200 when debugging
  pinMode(switch_pin, INPUT_PULLUP);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  matrix.begin(0x70);
  matrix.drawColon(true); // draw the colons between min & sec
  matrix.setBrightness(0); // set brightness between 0-15
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  
  // DFPlayer initialization
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  // Use softwareSerial to communicate with mp3
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to be compatible with ESP8266 watch dog. (from DFPlayer example code, not sure what this does)
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value from 0 to 30
}


void loop() {
  switchState = digitalRead(switch_pin);
  radio.write(&switchState, sizeof(switchState)); // send switch state to other Arduino

  if (switchState == 1) { // if switch is on,
  //count down from 3 minutes then count up indefinitely
   if (millis() - previousSecondMillis >= oneSecond) {  // if a second has passed, maybe the problem is that it's still greater than one second
      if (countDownFinished == true){
      startCountup();
      }
      else if (seconds-- == 0 && countDownFinished == false) { // if seconds are about to go negative maybe need to add countdownfinished == false
          if (minutes == 0) { // and if minutes get to zero, start counting up
              seconds = 1;
              countDownFinished = true; // make sure it stays counting up now
              myDFPlayer.play(1);
              startCountup();
          } 
          else { // if seconds go negative and minutes aren't yet zero,
              minutes -= 1; // go down a minute
              seconds = 59; // make seconds = 59
              startCountdown();
           }
        }
      else if (countDownFinished == false){ // if the countdown hasn't finished, keep counting down
        startCountdown();
        Serial.println("counting down");
      }
      previousSecondMillis += oneSecond; // maybe this is the problem? because it's within the if statement?
   }
 }  
  if (switchState == 0) { // if switch is off,
   // turn off lights
   digitalWrite(redLed, LOW);
   digitalWrite(greenLed, LOW);
   // make counter show 00:00
   matrix.writeDigitNum(0, 0);
   matrix.writeDigitNum(1, 0);
   matrix.writeDigitNum(3, 0);
   matrix.writeDigitNum(4, 0);
   matrix.writeDisplay();
   //reset time for when switch is on again:
   minutes = startMinute;
   seconds = startSecond;
   countDownFinished = false;
   if (millis() - previousSecondMillis >= oneSecond) {
   previousSecondMillis += oneSecond;
   }
 }
}


void startCountdown(){    
    digitalWrite(redLed, HIGH);  
    digitalWrite(greenLed, LOW);   
    // count down from 3 minutes
    matrix.writeDigitNum(0, (minutes / 10));
    matrix.writeDigitNum(1, (minutes % 10));
    matrix.writeDigitNum(3, (seconds / 10));
    matrix.writeDigitNum(4, (seconds % 10));
    matrix.writeDisplay();
  }


void startCountup(){
    digitalWrite(greenLed, HIGH);  
    digitalWrite(redLed, LOW);   
    // write negative sign
    if (seconds > 59) {
      seconds = 0;
      minutes += 100;
    }
    int count = minutes + seconds;
    // separate "count" into separate numbers for matrix
    matrix.writeDigitNum(0, ((count / 1000) % 10));
    matrix.writeDigitNum(1, ((count / 100) % 10));
    matrix.writeDigitNum(3, ((count / 10) % 10));
    matrix.writeDigitNum(4, ((count % 10)));
    matrix.writeDisplay();
    seconds = seconds +1;
}
