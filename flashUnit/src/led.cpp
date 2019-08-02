#ifdef FLASHUNIT

#include "../lib/led.h"
#include "Arduino.h"

bool ledOn = false;
unsigned long ledStartTime;
uint16_t ledDuration;

void ledSetup() {
    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, 0);
}

void ledLoop() {

  if(ledOn) {
      if((millis() - ledStartTime) < ledDuration) {
          analogWrite(LED_PIN, 255 - map(millis()-ledStartTime, 0, ledDuration, 0, 255));
      }
      else {
          analogWrite(LED_PIN, 0);
          ledOn = false;             
      }
  }

}

#endif