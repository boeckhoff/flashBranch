#include <stdint.h>
#include "Arduino.h"

#define CHRG_TRIGGER_PIN PB0
#define RELAY_TRIGGER_PIN PB4

#define LIGHT_SENS_FLASH_PIN A3 // same pin as PB4, overloaded

#define MAX_CHARGE_TIME 100000

// SENSING
#define DEFAULT_FLASH_LIGHT_THRESHOLD 1000000//130

// ACTUATION
#define RELAY_SWITCH_TIME 100//1000
#define DEFAULT_CHARGE_TRIGGER_DUTY_CYCLE 255

extern uint16_t flashLightThreshold;
extern uint8_t chargeTriggerDutyCycle;

extern unsigned long flashTriggerTime;
extern bool flashScheduled; 
extern unsigned long relayCloseTime;
extern bool relayCloseScheduled;

void flashSetup();
void flashLoop();
void chargeAndScheduleFlash();