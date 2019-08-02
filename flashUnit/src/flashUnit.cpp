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
const char TERMINATION_CHARACTER = '\n';
const uint8_t MAX_CHARS = 10;
char message[MAX_CHARS];
char errorMessage[8];
bool newData = false;
char id;

uint8_t readVccVoltage(void) {
  ADMUX = 0b00001100;

	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
	_delay_ms(1);
	ADCSRA |= _BV(ADSC);
	while( ADCSRA & _BV( ADSC) ) ;

	uint8_t low  = ADCL;
	uint8_t high = ADCH;
	uint16_t adc = (high << 8) | low;
	uint8_t vccx10 = (uint8_t) ( (11 * 1024) / adc); 

	return vccx10;
}

void receiveUART()
{
  static uint8_t index = 0;

  char buffer;

  while (mySerial.available() > 0 && newData == false)
  {
    buffer = mySerial.read();
    message[index] = buffer;

    if (buffer != TERMINATION_CHARACTER)
    {
      index++;
      if (index >= MAX_CHARS)
      {
        error = MESSAGE_TOO_LONG;
      }
    }
    else
    {
      index = 0;
      newData = true;
    }
  }
}

void writeMessage()
{
  for (int i = 0; i < MAX_CHARS; ++i)
  {
    mySerial.write(message[i]);
    if (message[i] == TERMINATION_CHARACTER) return;
  }
}

void writeErrorMessage()
{
  mySerial.write('#E');
  mySerial.write(errorMessage);
  mySerial.write(TERMINATION_CHARACTER);
}

void setup() {
  mySerial.begin(BAUD_RATE);

  pinMode(LIGHT_SENS_AMBIENT_PIN, INPUT);

  mySerial.write('#B');
  mySerial.write(TERMINATION_CHARACTER);

  //flashSetup();
  ledSetup();
}

void loop() {

  //flashLoop();
  ledLoop();

/* 
  if(error != NONE) {
    writeErrorMessage();
  }
  */

  if (mySerial.available())
  {
    receiveUART();
  }

  if (newData) {
    switch (message[0]) {
      // GENERAL
      /* 
      case '#':
        writeMessage();
        break;
      */
      case 'R':
        id = message[1];
        message[1] = id + 1;
        writeMessage();
        break;
        /* 
      case 'P':
        writeMessage();
        mySerial.write('#');
        mySerial.write(id);
        mySerial.write(TERMINATION_CHARACTER);
        break;
      case 'V':
        mySerial.write('#');
        mySerial.write(id);
        mySerial.write(readVccVoltage());
        mySerial.write(TERMINATION_CHARACTER);
        break;
        */

      // LED
      case 'L':
        if (id == message[1]) {
          ledStartTime = millis();
          ledOn = true;
          ledDuration = 700/CLOCK_DIVIDER;
          break;
        }
        writeMessage();
        break;
      // FLASH
      /*
      case 'T':
        if (id == message[1]) {
          chargeAndScheduleFlash();
          break;
        }
        writeMessage();
        break;
      case 'A':
        break;
      case 'C':
        pinMode(RELAY_TRIGGER_PIN, OUTPUT);
        digitalWrite(RELAY_TRIGGER_PIN, HIGH);
        delay(RELAY_SWITCH_TIME);
        digitalWrite(RELAY_TRIGGER_PIN, LOW);
        pinMode(LIGHT_SENS_FLASH_PIN, INPUT);

        pinMode(CHRG_TRIGGER_PIN, OUTPUT);
        analogWrite(CHRG_TRIGGER_PIN, 0);

        flashScheduled = false;
        relayCloseScheduled = false;

        writeMessage();
        break;
      case 'S':
        chargeTriggerDutyCycle = message[1]+50;
        if(flashScheduled) {
          pinMode(CHRG_TRIGGER_PIN, OUTPUT);
          analogWrite(CHRG_TRIGGER_PIN, chargeTriggerDutyCycle);
        }
        writeMessage();
        break;
      case 'K':
        chargeTriggerDutyCycle = 255;
        if(flashScheduled) {
          pinMode(CHRG_TRIGGER_PIN, OUTPUT);
          analogWrite(CHRG_TRIGGER_PIN, chargeTriggerDutyCycle);
        }
        writeMessage();
        break;
        */
      default:
        break;
    }
    newData = false;
  }
}


#endif