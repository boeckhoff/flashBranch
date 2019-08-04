#ifdef FLASHUNIT

#include "../lib/led.h"
#include "Arduino.h"

bool ledOn = false;
unsigned long ledStartTime;
uint16_t ledOnDuration;
uint16_t ledFadeDuration;
uint8_t ledBrightness;

void ledSetup() {
    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, 0);
}

void ledLoop()
{

    if (ledOn)
    {
        if ((millis() - ledStartTime) <= ledOnDuration) {
            analogWrite(LED_PIN, ledBrightness);
            return;
        }
        if ((millis() - ledStartTime) < (ledOnDuration + ledFadeDuration)) {
            analogWrite(LED_PIN, ledBrightness - map(millis() - ledStartTime - ledOnDuration, 0, ledFadeDuration, 0, ledBrightness));
            return;
        }
        analogWrite(LED_PIN, 0);
        ledOn = false;
    }
}

#endif