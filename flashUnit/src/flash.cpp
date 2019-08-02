#ifdef FLASHUNIT

#include "../lib/flash.h"
#include "Arduino.h"

uint16_t flashLightThreshold = DEFAULT_FLASH_LIGHT_THRESHOLD;
uint8_t chargeTriggerDutyCycle = DEFAULT_CHARGE_TRIGGER_DUTY_CYCLE;

bool flashScheduled = false; 
bool relayCloseScheduled = false;
unsigned long flashTriggerTime = 0;
unsigned long relayCloseTime = 100;

void setFlashLightThreshold(uint16_t threshold) {
    flashLightThreshold = threshold;
}

void flashSetup()
{
    pinMode(RELAY_TRIGGER_PIN, OUTPUT);
    pinMode(CHRG_TRIGGER_PIN, OUTPUT);

    digitalWrite(RELAY_TRIGGER_PIN, LOW);
    analogWrite(CHRG_TRIGGER_PIN, 0);

    pinMode(LIGHT_SENS_FLASH_PIN, INPUT);
}

void chargeAndScheduleFlash()
{
    flashTriggerTime = millis() + MAX_CHARGE_TIME;
    flashScheduled = true;

    pinMode(CHRG_TRIGGER_PIN, OUTPUT);
    analogWrite(CHRG_TRIGGER_PIN, chargeTriggerDutyCycle);
}

void flashLoop() {
  if(relayCloseScheduled && millis() > relayCloseTime) {
    pinMode(RELAY_TRIGGER_PIN, OUTPUT);
    digitalWrite(RELAY_TRIGGER_PIN, LOW);
    pinMode(LIGHT_SENS_FLASH_PIN, INPUT);
    digitalWrite(CHRG_TRIGGER_PIN, LOW);
  }

  if(flashScheduled && (millis() > flashTriggerTime)){ //|| cur_avg > flashLightThreshold)) {
    pinMode(RELAY_TRIGGER_PIN, OUTPUT);
    digitalWrite(RELAY_TRIGGER_PIN, HIGH);

    relayCloseScheduled = true;
    relayCloseTime = millis() + RELAY_SWITCH_TIME;

    flashScheduled = false;
  }
}

#endif



/*
  if(it == 100) {  
    cur_avg = avg/100.0;
    it = 0;
    avg = 0;
  }
  else {
    avg += analogRead(LIGHT_SENS_FLASH_PIN);
    it += 1;
  }
*/