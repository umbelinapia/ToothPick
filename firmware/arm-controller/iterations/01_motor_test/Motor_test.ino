#define PUL 1
#define DIR 2   
void setup() {
  // put your setup code here, to run once:
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);            

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DIR, HIGH);

  //for(int i = 0; i<20000, i++;){
    digitalWrite(PUL, HIGH);
    delayMicroseconds(1000);
    digitalWrite(PUL, LOW);
    delayMicroseconds(1000);
  //}
  //delay(1000);
}
