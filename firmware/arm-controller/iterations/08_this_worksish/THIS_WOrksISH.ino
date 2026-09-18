#include <ESP32Servo.h>

#define PUL1 1   // J1
#define DIR1 2
#define PUL2 40  // J2
#define DIR2 39
#define PUL3 42  // J3
#define DIR3 41

#define HOME1 4
#define HOME2 5
#define HOME3 6

#define GRIP_SERVO_PIN 7
#define FLIP_SERVO_PIN 15

Servo gripServo;
Servo flipServo;

int GripOpen = 100;
int GripClose = 0;
int currentFlipAngle = 0;

int J1_MAX = 10000;
int J2_MAX = 10150;
int J3_MAX = 700;

const int CAM_J1 = 5000;
const int CAM_J2 = 8000;
const int CAM_J3 = 0;

struct Joint {
  int pul;
  int dir;
  int home;
  long pos;
  long minPos;
  long maxPos;
};

Joint J1 = {PUL1, DIR1, HOME1, 0, 0, J1_MAX};
Joint J2 = {PUL2, DIR2, HOME2, 0, 0, J2_MAX};
Joint J3 = {PUL3, DIR3, HOME3, 0, 0, J3_MAX};

String inputString = "";
bool stringComplete = false;

void motorcw(Joint &J, int rotate) {
  digitalWrite(J.dir, LOW);
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(500);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(500);
    J.pos--;
  }
}

void motorccw(Joint &J, int rotate) {
  digitalWrite(J.dir, HIGH);
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(500);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(500);
    J.pos++;
  }
}

void homing(Joint &J, int rotate) {
  while (digitalRead(J.home) == LOW) {
    motorcw(J, rotate);
  }
  J.pos = 0;
}

void FlipGripper(int angle) {
  flipServo.write(angle);
}

void GripperOpen() {
  gripServo.write(GripOpen);
}

void GripperClose() {
  gripServo.write(GripClose);
}

void coords(Joint &J, int rotate, int coord) {
  if (coord < J.minPos || coord > J.maxPos) {
    Serial.print("ERROR: target out of range. Allowed: ");
    Serial.print(J.minPos);
    Serial.print(" to ");
    Serial.println(J.maxPos);
    return;
  }

  if (J.pos == coord) return;

  while (J.pos < coord) {
    int remaining = coord - J.pos;
    int step = (remaining > rotate) ? rotate : remaining;
    motorccw(J, step);
  }

  while (J.pos > coord) {
    int remaining = J.pos - coord;
    int step = (remaining > rotate) ? rotate : remaining;
    motorcw(J, step);
  }

  delay(50);
}

void cameraPose() {
  coords(J1, 250, CAM_J1);
  coords(J2, 250, CAM_J2);
  coords(J3, 50, CAM_J3);
}

Joint* getJointByName(String name) {
  name.trim();
  if (name == "J1") return &J1;
  if (name == "J2") return &J2;
  if (name == "J3") return &J3;
  return nullptr;
}

void printPositions() {
  Serial.print("J1 pos = ");
  Serial.println(J1.pos);
  Serial.print("J2 pos = ");
  Serial.println(J2.pos);
  Serial.print("J3 pos = ");
  Serial.println(J3.pos);
}

void executeCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  Serial.print("Received: ");
  Serial.println(cmd);

  if (cmd.startsWith("coords(") && cmd.endsWith(")")) {
    int start = cmd.indexOf('(');
    int end = cmd.lastIndexOf(')');
    String inside = cmd.substring(start + 1, end);

    int c1 = inside.indexOf(',');
    int c2 = inside.indexOf(',', c1 + 1);

    if (c1 == -1 || c2 == -1) {
      Serial.println("ERROR: invalid coords format. Use coords(J2,100,8000)");
      return;
    }

    String jointName = inside.substring(0, c1);
    String rotateStr = inside.substring(c1 + 1, c2);
    String coordStr = inside.substring(c2 + 1);

    Joint* joint = getJointByName(jointName);
    if (joint == nullptr) {
      Serial.println("ERROR: invalid joint. Use J1, J2, or J3.");
      return;
    }

    int rotate = rotateStr.toInt();
    long coord = coordStr.toInt();

    coords(*joint, rotate, coord);
    Serial.println("DONE");
    return;
  }

  if (cmd.startsWith("home(") && cmd.endsWith(")")) {
    int start = cmd.indexOf('(');
    int end = cmd.lastIndexOf(')');
    String jointName = cmd.substring(start + 1, end);

    Joint* joint = getJointByName(jointName);
    if (joint == nullptr) {
      Serial.println("ERROR: invalid joint. Use J1, J2, or J3.");
      return;
    }

    homing(*joint, 10);
    Serial.println("DONE");
    return;
  }

  if (cmd == "open()") {
    GripperOpen();
    Serial.println("DONE");
    return;
  }

  if (cmd == "close()") {
    GripperClose();
    Serial.println("DONE");
    return;
  }

  if (cmd == "flip()") {
    if (currentFlipAngle == 180) {
      currentFlipAngle = 0;
    } else {
      currentFlipAngle = 180;
    }
    FlipGripper(currentFlipAngle);
    Serial.println("DONE");
    return;
  }

  if (cmd == "pos()") {
    printPositions();
    Serial.println("DONE");
    return;
  }

  if (cmd == "cameraPose()") {
    cameraPose();
    Serial.println("DONE");
    return;
  }

  Serial.println("ERROR: unknown command");
}

void readSerialInput() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    if (inChar == '\n' || inChar == '\r') {
      if (inputString.length() > 0) {
        stringComplete = true;
      }
    } else {
      inputString += inChar;
    }
  }
}

void setup() {
  pinMode(PUL1, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(PUL2, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(PUL3, OUTPUT);
  pinMode(DIR3, OUTPUT);

  pinMode(HOME1, INPUT);
  pinMode(HOME2, INPUT);
  pinMode(HOME3, INPUT);

  Serial.begin(115200);

  gripServo.attach(GRIP_SERVO_PIN, 500, 2400);
  flipServo.attach(FLIP_SERVO_PIN, 500, 2400);

  GripperOpen();
  FlipGripper(0);
  currentFlipAngle = 0;

  inputString.reserve(100);

  Serial.println("Ready.");
  Serial.println("Starting homing sequence...");

  homing(J1, 10);
  delay(300);
  homing(J2, 10);
  delay(300);
  homing(J3, 10);
  delay(300);

  Serial.println("Startup homing complete.");
}

void loop() {
  readSerialInput();

  if (stringComplete) {
    executeCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}