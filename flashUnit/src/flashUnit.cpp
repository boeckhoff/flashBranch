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
#define BAUD_RATE 38400

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

void receiveByte() {
  buffer = mySerial.read();

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
          ledOnDuration = (message[3]*100)/CLOCK_DIVIDER;
          ledFadeDuration = (message[4]*100)/CLOCK_DIVIDER;
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