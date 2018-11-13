/*
 * Jack the Maker for Mercedes
 * November 2018
 
 * Read incoming values from transmitter Arduino, 
 * turn off green taxi lights when switch on counter box is switched on 
 * to indicate that the cab is occupied.
 
 # When debugging, ensure that Serial is on 115200 so that you don't 
 # disrupt the comms with the DF Player that are on 9600 on the other Arduino.
 
 * NRF24L01 code adapted from https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
 */
 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
RF24 radio(7, 8); // CE, CSN pins
const byte addresses[][6] = {"00001", "00002"};
int relay = 2; // Relay pin

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00002
  radio.openReadingPipe(1, addresses[1]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}
void loop() {
  if (radio.available()) {
      int switchState = 0; // value to read switch from other Arduino inside the cab
      radio.read(&switchState, sizeof(switchState));
      Serial.println(switchState);
      if (switchState == 1){
        digitalWrite(relay, LOW);      
      }
      else{
         digitalWrite(relay, HIGH);
      }
    }
        else{
    }
    delay(5); // don't need to read the incoming data so often, so delay 5 milliseconds
    }
