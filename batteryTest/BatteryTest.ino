#include "Queue.h"

#define photocellPin 1
#define THRESHOLD 800
#define queueMaxSize 50

unsigned int lightVal;
unsigned int flashNum = 0;
bool firstFlashVal = true;
unsigned long lastFlash = 0;
Queue<unsigned int> flashPowerVals = Queue<unsigned int>(queueMaxSize);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("seconds,powerFlash,powerMC");
}

void loop() {
  unsigned int val = analogRead(6);
 
  if(flashPowerVals.count() >= queueMaxSize) {
    flashPowerVals.pop();
  }
  flashPowerVals.push(val);


  lightVal = analogRead(photocellPin);

  if(lightVal > THRESHOLD) {
    if(firstFlashVal) {
      Serial.print(millis()/1000);
      Serial.print(",");
      Serial.print(calcAvgPower(flashPowerVals));
      Serial.print(",");
      Serial.println(analogRead(5)); 

      firstFlashVal = false;
    }
    
  }
  else {
    firstFlashVal = true;
  }
}

unsigned int calcAvgPower(Queue<unsigned int>& q) {
  q.pop();
  unsigned int total = 0;
  unsigned int c = 0;
  while(q.count() > 4) {
    unsigned int el = q.pop();
    total += el;
    c += 1;
  }
  q.clear();

  return (int)(((float)total/(float)c));
}
