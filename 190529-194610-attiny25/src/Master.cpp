#ifdef __INTELLISENSE__
#define MASTER
#endif

#ifdef MASTER

#include "Arduino.h"
#include <SoftwareSerial.h>

#define BUTTON_PIN 2

SoftwareSerial mySerial(PD3, PD4); //RX, TX

uint8_t buttonCount = 0;

bool prevButtonRead = HIGH;

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    mySerial.println("R0");
}

void loop() {
    if(Serial.available()) {
        mySerial.write(Serial.read());
    }
    if(mySerial.available()) {
        Serial.write(mySerial.read());
    }

    if(digitalRead(BUTTON_PIN) == LOW) {
        if(prevButtonRead == HIGH) {

            switch(buttonCount) {
                case 0:
                    mySerial.println("R0");
                    break;
                case 1:
                    mySerial.println("T0");
                    break;
                case 2:
                    mySerial.println("T1");
                    break;
                default:
                    break;
            }
            
            buttonCount += 1;
            prevButtonRead = LOW;
        }
    }
    else {
        prevButtonRead = HIGH;
    }
}

#endif