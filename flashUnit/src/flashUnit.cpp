/* FlashUnit
 *
 * TODO:
 * - adjust clock frequency for different attiny version
 * - check if clock frequency differs with variable input voltage and effect on UART
 * - implement ambient brightness sensing for 
 *  - daylight detection
 *  - mode switching
 * - implement protocol for mode switching using flashlight
 * - create pin assignment
 * - test double use pin relay_trigger
 * - create option to modify eeprom with uart for saving configurations for battery usage
 * - address overloaded line feed char
 * - add reset command
 *
*/

#ifdef __INTELLISENSE__
#define FLASHUNIT
#endif

#ifdef FLASHUNIT

#include "../lib/led.h"
#include "../lib/flash.h"
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <util/delay.h>

// PINS
#define TX_PIN PB2
#define RX_PIN PB1
#define LIGHT_SENS_AMBIENT_PIN A2 

// TIMING
#define CLOCK_DIVIDER 8

// BAUD
#define BAUD_RATE 38400
#define USE_ECC false
#define TERMINATION_CHARACTER '\0'

enum ERROR_CODE
{
  NONE,
  MESSAGE_TOO_LONG,
  RECEIVED_NO_EOM,
  NOT_CONFORM,
} error = NONE;

SoftwareSerial mySerial(RX_PIN, TX_PIN);

// UART
const uint8_t MESSAGE_LENGTH = 6;
char message[MESSAGE_LENGTH];

char buffer;
char checksum;
uint8_t index = 0;

char errorMessage[8];
bool newData = false;
char id;

void blink() {
  digitalWrite(LED_PIN, HIGH);
  delay(50/CLOCK_DIVIDER);
  digitalWrite(LED_PIN, LOW);
  delay(50/CLOCK_DIVIDER);
}

char receivePayloadByte() {
  char b[3];

  for(int i = 0; i < 3; ++i) {
    while(!Serial.available());
    b[i] = Serial.read();
  }

  if(b[0] == b[1]) {
    return b[0];
  }

  if(b[1] == b[2]) {
    return b[1];
  }

  if(b[2] == b[3]) {
    return b[2];
  }

  blink();
  return '\n';
}

void receiveByte() {
  blink();

  if(USE_ECC) {
    buffer = receivePayloadByte();
  }
  else {
    buffer = mySerial.read();
  }

  if(index == MESSAGE_LENGTH-1) {
    if(buffer == checksum) {
      newData = true;
    }
    else {
      blink();
      blink();
    }
    checksum = 0;
    index = 0;
    return;
  }

  checksum ^= buffer;
  message[index] = buffer;
  index++;
}

/*
void receiveUART()
{
  uint8_t index = 0;

  char buffer;
  char checksum = 0;

  while (mySerial.available() > 0 && newData == false)
  {
    buffer = mySerial.read();
    message[index] = buffer;

    if(index != MESSAGE_LENGTH-1) {
      checksum ^= buffer;
      index++;
      continue;
    }

    if(checksum == buffer) {
      newData = true;        
      return;
    }
  }
}
*/

void writeMessage()
{
  checksum = 0;
  for (int i = 0; i < MESSAGE_LENGTH-1; ++i)
  {
    checksum ^= message[i];
    mySerial.write(message[i]);
  }
  mySerial.write(checksum);
  checksum = 0;
}

uint16_t extractMillis(char input) {
  if(input <= 10) {
    return input*10;
  }
  else {
    return (input-10)*100;
  }
}

void setup() {
  delay(500/CLOCK_DIVIDER);
  mySerial.begin(BAUD_RATE);

  pinMode(LIGHT_SENS_AMBIENT_PIN, INPUT);

  ledSetup();
}

void loop() {

  ledLoop();

  if (mySerial.available())
  {
    receiveByte();
  }

  if (newData) {
    switch (message[0]) {
      case 'C':
        ledOn = false;
      case 'R':
        id = message[1];
        message[1] = id + 1;
        writeMessage();
        break;

      case 'L':
        if (id == message[1]) {
          ledStartTime = millis();
          ledOn = true;

          ledBrightness = message[2];
          ledOnDuration = extractMillis(message[3])/CLOCK_DIVIDER;
          ledFadeDuration = extractMillis(message[4])/CLOCK_DIVIDER;
          break;
        }
        writeMessage();
        break;
      default:
        break;
    }
    newData = false;
  }
}


#endif