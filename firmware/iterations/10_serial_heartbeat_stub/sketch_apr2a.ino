void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Ready.");
}

void loop() {
  Serial.println("Looping...");
  delay(1000);
}