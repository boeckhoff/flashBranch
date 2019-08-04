#ifdef __INTELLISENSE__
#define MASTERNANO
#endif

#ifdef MASTERNANO

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <stdlib.h>

#define NUM_UNITS 10

SoftwareSerial mySerial(4, 3); //RX, TX

const uint8_t MESSAGE_LENGTH = 6;
char message[MESSAGE_LENGTH];

char checksum;
uint8_t index;

uint16_t chaseDelay;

char chaseParams[5];

void setup()
{
    Serial.begin(9600);
    mySerial.begin(4800);
    chaseParams[0] = 255;
}

void writeMessage()
{
    Serial.print("sending message: ");
    checksum = 0;

    for (index = 0; index < MESSAGE_LENGTH - 1; ++index)
    {
        checksum ^= message[index];
        mySerial.write(message[index]);
        Serial.print((uint8_t)message[index]);
        Serial.print(',');
    }
    mySerial.write(checksum);
    Serial.println((uint8_t)checksum);
}

void chaseLoop() {
    switch(chaseParams[0]) {
        case 'A':
            for(int i = 0; i<NUM_UNITS; ++i) {
                message[0] = 'L';
                message[1] = i;
                message[2] = chaseParams[2];
                message[3] = chaseParams[3];
                message[4] = chaseParams[4];
                writeMessage();
                delay(chaseParams[1]*100);
            }
        break;
        case 'B':
            int i = rand() % NUM_UNITS;
            message[0] = 'L';
            message[1] = i;
            message[2] = chaseParams[2];
            message[3] = chaseParams[3];
            message[4] = chaseParams[4];
            writeMessage();
            delay(chaseParams[1]*100);

        break;
        default:
        break;
    }
}


char readParams() {
    char s[4];

    for(int i = 0; i<3; ++i) {
        while(!Serial.available());
        char c = Serial.read();
        s[i] = c;
    }
    s[3] = '\0';
    Serial.print(s);
    Serial.print("->");
    return (char)atoi(s);
}

void loop()
{
    if (Serial.available())
    {
        index = 0;
        checksum = 0;
        Serial.print("collecting message: ");

        message[0] = Serial.read();
        index++;
        Serial.print(message[0]);
        Serial.print(",");

        while (index < MESSAGE_LENGTH - 1)
        {
            if(Serial.available()) {
                char c = readParams();
                Serial.print(c);
                Serial.print(",");
                message[index] = c;
                index++;
            }
        }

        Serial.println();
        if(message[0] == 'A' || message[0] == 'B') {
            memcpy(chaseParams, message, sizeof(chaseParams));
        }
        else {
            writeMessage();
        }
    }

    if (mySerial.available())
    {
        Serial.print((char)mySerial.read());
    }
    chaseLoop();
}

#endif