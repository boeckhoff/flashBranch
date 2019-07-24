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

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <util/delay.h>

// PINS
#define TX_PIN PB2
#define RX_PIN PB1
#define CHRG_TRIGGER_PIN PB0
#define RELAY_TRIGGER_PIN PB4
#define LIGHT_SENS_AMBIENT_PIN A2 
#define LED_PIN PB3
#define LIGHT_SENS_FLASH_PIN A3 // same pin as PB4, overloaded

// TIMING
#define CLOCK_DIVIDER 4
#define MAX_CHARGE_TIME 100000/CLOCK_DIVIDER
#define BAUD_RATE 9600*CLOCK_DIVIDER
#define RELAY_SWITCH_TIME 100//1000

// SENSING
#define DEFAULT_FLASH_LIGHT_THRESHOLD 1000000//130

// ACTUATION
#define DEFAULT_CHARGE_TRIGGER_DUTY_CYCLE 255

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
bool newData = false;
char id;

uint16_t flashLightThreshold = DEFAULT_FLASH_LIGHT_THRESHOLD;
uint8_t chargeTriggerDutyCycle = DEFAULT_CHARGE_TRIGGER_DUTY_CYCLE;

// SCHEDULING
unsigned long flashTriggerTime;
bool flashScheduled = false; 
unsigned long relayCloseTime;
bool relayCloseScheduled = false;

uint8_t it = 0;
uint16_t avg = 0;
float cur_avg = 0;

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

	return( vccx10 );
}

uint8_t readFlashVoltage() {
  analogRead(A2);
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
  /*
  message[0] = 'E';
  message[1] = error;
  message[2] = TERMINATION_CHARACTER;
  writeMessage();
  */
}

void blink(uint16_t dur)
{
  digitalWrite(LED_PIN, HIGH);
  delay(dur / CLOCK_DIVIDER);
  digitalWrite(LED_PIN, LOW);
  delay(dur / CLOCK_DIVIDER);
}

void setup() {
  mySerial.begin(76800);

  pinMode(RELAY_TRIGGER_PIN, OUTPUT);
  pinMode(CHRG_TRIGGER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(RELAY_TRIGGER_PIN, LOW);
  analogWrite(CHRG_TRIGGER_PIN, 0);

  pinMode(LIGHT_SENS_FLASH_PIN, INPUT);
  //pinMode(LIGHT_SENS_AMBIENT_PIN, INPUT);
  mySerial.println("B");
}

void chargeAndScheduleFlash() {
  flashTriggerTime = millis() + MAX_CHARGE_TIME;
  flashScheduled = true;

  pinMode(CHRG_TRIGGER_PIN, OUTPUT);
  analogWrite(CHRG_TRIGGER_PIN, chargeTriggerDutyCycle);
}

void loop() {

  if(it == 100) {  
    cur_avg = avg/100.0;
    it = 0;
    avg = 0;
  }
  else {
    avg += analogRead(LIGHT_SENS_FLASH_PIN);
    it += 1;
  }

  if(relayCloseScheduled && millis() > relayCloseTime) {
    pinMode(RELAY_TRIGGER_PIN, OUTPUT);
    digitalWrite(RELAY_TRIGGER_PIN, LOW);
    pinMode(LIGHT_SENS_FLASH_PIN, INPUT);
    digitalWrite(CHRG_TRIGGER_PIN, LOW);
  }

  if(flashScheduled && (millis() > flashTriggerTime) || cur_avg > flashLightThreshold)) {
    //mySerial.println("Triggered with avg:");
    //mySerial.println(cur_avg);
    pinMode(RELAY_TRIGGER_PIN, OUTPUT);
    digitalWrite(RELAY_TRIGGER_PIN, HIGH);

    relayCloseScheduled = true;
    relayCloseTime = millis() + RELAY_SWITCH_TIME;

    flashScheduled = false;
  }

  if(error != NONE) {
    writeErrorMessage();
  }

  if (mySerial.available())
  {
    receiveUART();
  }

  if (newData) {
    switch (message[0]) {
      case 'R':
        id = message[1];
        message[1] = id + 1;
        writeMessage();
        break;
      case 'T':
        if (id == message[1]) {
          //blink(1000);
          chargeAndScheduleFlash();
          break;
        }
        writeMessage();
        break;
      case 'E':
        flashLightThreshold = message[1]+50;
        writeMessage();
        break;
      case 'A':
        mySerial.println(analogRead(LIGHT_SENS_FLASH_PIN));
        mySerial.println(readVccVoltage());
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
      case 'P':
        mySerial.write('I');
        mySerial.println(id);
      default:
        break;
    }
    newData = false;
  }
}


/*
void recvWithEndMarker()
{
  static byte ndx = 0;
  
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false)
  {
    rc = Serial.read();

    if (rc != endMarker)
    {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars)
      {
        ndx = numChars - 1;
      }
    }
    else
    {
      receivedChars[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}
*/

/*
void chargeAndFlash() {
  digitalWrite(CHRG_TRIGGER_PIN, HIGH);

  while(analogRead(LIGHT_SENS_FLASH_PIN) < flashLightThreshold && millis() - startTime < MAX_CHARGE_TIME) {}

  digitalWrite(CHRG_TRIGGER_PIN, LOW);
  pinMode(RELAY_TRIGGER_PIN, OUTPUT);

  digitalWrite(RELAY_TRIGGER_PIN, HIGH);
  delay(RELAY_SWITCH_TIME/CLOCK_DIVIDER);
  digitalWrite(RELAY_TRIGGER_PIN, LOW);
}
*/
#endif