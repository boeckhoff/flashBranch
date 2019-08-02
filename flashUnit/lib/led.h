#include <stdint.h>
#include "Arduino.h"

#define LED_PIN PB0

extern bool ledOn;
extern unsigned long ledStartTime;
extern uint16_t ledDuration;

void ledSetup();
void ledLoop();