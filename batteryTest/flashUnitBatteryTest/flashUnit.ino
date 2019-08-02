void setup() {
  pinMode(1, OUTPUT);
}

void loop() {
  delay(15000);
  digitalWrite(1, HIGH);
  delay(1000);
  digitalWrite(1, LOW);
}
