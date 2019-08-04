#include <stdint.h>
#include "Arduino.h"

#define LED_PIN PB0

extern bool ledOn;
extern unsigned long ledStartTime;
extern uint16_t ledOnDuration;
extern uint16_t ledFadeDuration;
extern uint8_t ledBrightness;

void ledSetup();
void ledLoop();