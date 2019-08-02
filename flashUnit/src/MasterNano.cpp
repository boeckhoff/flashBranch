#ifdef __INTELLISENSE__
#define MASTERNANO
#endif

#ifdef MASTERNANO

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <stdlib.h>

SoftwareSerial mySerial(4, 3); //RX, TX


void setup()
{
    Serial.begin(9600);
    mySerial.begin(4800);
}

void loop()
{
    if (Serial.available())
    {
        char c = Serial.read();
        if(c=='c'){
            while(true) {
                char r = rand() % 20;
                mySerial.write('L');
                mySerial.println((char)('A'+r));
                Serial.println((char)('A'+r));
                delay(300);
            }
        }
        else {
            mySerial.write(c);
        }
    }
    if (mySerial.available())
    {
        Serial.write(mySerial.read());
    }
}

#endif