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
int GripTightOpen = 65;
int GripClose = 0;

// Flip servo absolute positions
const int FLIP_DOWN_ANGLE = 0;
const int FLIP_UP_ANGLE   = 180;

int currentFlipAngle = FLIP_DOWN_ANGLE;

int J1_MAX = 10000;
int J2_MAX = 10150;
int J3_MAX = 700;

// Camera pose
const int CAM_J1 = 4000;
const int CAM_J2 = 8000;
const int CAM_J3 = 0;

// -----------------------------
// HOME SWITCH POLARITY
// true  = pressed reads HIGH
// false = pressed reads LOW
// -----------------------------
const bool J1_HOME_ACTIVE_HIGH = true;
const bool J2_HOME_ACTIVE_HIGH = true;
const bool J3_HOME_ACTIVE_HIGH = true;

// -----------------------------
// HOMING DIRECTION
// true  = home using motorcw()
// false = home using motorccw()
// -----------------------------
const bool J1_HOME_USES_CW = true;
const bool J2_HOME_USES_CW = true;
const bool J3_HOME_USES_CW = true;

// -----------------------------
// HOMING STEP SIZE
// -----------------------------
const int J1_HOME_STEP = 1;
const int J2_HOME_STEP = 5;
const int J3_HOME_STEP = 2;

// -----------------------------
// PER-JOINT STEP PULSE TIMING
// Bigger number = slower / gentler
// Smaller number = faster
// -----------------------------
const int J1_PULSE_US = 900;
const int J2_PULSE_US = 250;
const int J3_PULSE_US = 700;

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

int getPulseDelayUs(Joint &J) {
  if (&J == &J1) return J1_PULSE_US;
  if (&J == &J2) return J2_PULSE_US;
  return J3_PULSE_US;
}

bool switchPressed(Joint &J) {
  bool activeHigh = true;

  if (&J == &J1) activeHigh = J1_HOME_ACTIVE_HIGH;
  else if (&J == &J2) activeHigh = J2_HOME_ACTIVE_HIGH;
  else if (&J == &J3) activeHigh = J3_HOME_ACTIVE_HIGH;

  int state = digitalRead(J.home);
  return activeHigh ? (state == HIGH) : (state == LOW);
}

bool jointIsHome(Joint &J) {
  return switchPressed(J);
}

void motorcw(Joint &J, int rotate) {
  int pulseUs = getPulseDelayUs(J);

  digitalWrite(J.dir, LOW);
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(pulseUs);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(pulseUs);
    J.pos--;
  }
}

void motorccw(Joint &J, int rotate) {
  int pulseUs = getPulseDelayUs(J);

  digitalWrite(J.dir, HIGH);
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(pulseUs);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(pulseUs);
    J.pos++;
  }
}

void moveTowardHome(Joint &J, int stepSize) {
  bool useCW = true;

  if (&J == &J1) useCW = J1_HOME_USES_CW;
  else if (&J == &J2) useCW = J2_HOME_USES_CW;
  else if (&J == &J3) useCW = J3_HOME_USES_CW;

  if (useCW) {
    motorcw(J, stepSize);
  } else {
    motorccw(J, stepSize);
  }
}

int getHomeStepSize(Joint &J) {
  if (&J == &J1) return J1_HOME_STEP;
  if (&J == &J2) return J2_HOME_STEP;
  return J3_HOME_STEP;
}

// Returns true only if the switch was actually reached
bool homing(Joint &J) {
  unsigned long startTime = millis();
  const unsigned long HOME_TIMEOUT_MS = 20000;
  int stepSize = getHomeStepSize(J);

  Serial.println("Starting homing...");
  Serial.print("Initial switch state: ");
  Serial.println(digitalRead(J.home));

  if (switchPressed(J)) {
    J.pos = 0;
    Serial.println("Already on home switch.");
    Serial.println("Homing complete.");
    return true;
  }

  while (!switchPressed(J)) {
    moveTowardHome(J, stepSize);

    if (millis() - startTime > HOME_TIMEOUT_MS) {
      Serial.println("ERROR: homing timeout");
      return false;
    }
  }

  J.pos = 0;

  Serial.print("Final switch state: ");
  Serial.println(digitalRead(J.home));
  Serial.println("Homing complete.");
  return true;
}

void FlipGripper(int angle) {
  flipServo.write(angle);
  currentFlipAngle = angle;
}

void GripperOpen() {
  gripServo.write(GripOpen);
}

void GripperTightOpen() {
  gripServo.write(GripTightOpen);
}

void GripperClose() {
  gripServo.write(GripClose);
}

bool coords(Joint &J, int rotate, long coord) {
  if (coord < J.minPos || coord > J.maxPos) {
    Serial.print("ERROR: target out of range. Allowed: ");
    Serial.print(J.minPos);
    Serial.print(" to ");
    Serial.println(J.maxPos);
    return false;
  }

  if (rotate <= 0) {
    Serial.println("ERROR: rotate must be > 0");
    return false;
  }

  if (J.pos == coord) return true;

  while (J.pos < coord) {
    long remaining = coord - J.pos;
    int step = (remaining > rotate) ? rotate : (int)remaining;
    motorccw(J, step);
  }

  while (J.pos > coord) {
    long remaining = J.pos - coord;
    int step = (remaining > rotate) ? rotate : (int)remaining;
    motorcw(J, step);
  }

  delay(50);
  return true;
}

bool cameraPose() {
  if (!coords(J1, 250, CAM_J1)) return false;
  if (!coords(J2, 250, CAM_J2)) return false;
  if (!coords(J3, 50, CAM_J3)) return false;
  return true;
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

  Serial.print("HOME1 = ");
  Serial.println(digitalRead(HOME1));
  Serial.print("HOME2 = ");
  Serial.println(digitalRead(HOME2));
  Serial.print("HOME3 = ");
  Serial.println(digitalRead(HOME3));

  Serial.print("J1 at home = ");
  Serial.println(jointIsHome(J1) ? "YES" : "NO");
  Serial.print("J2 at home = ");
  Serial.println(jointIsHome(J2) ? "YES" : "NO");
  Serial.print("J3 at home = ");
  Serial.println(jointIsHome(J3) ? "YES" : "NO");
}

bool j1HomeSwitchPressed() {
  return switchPressed(J1);
}

bool dropIsSafe() {
  return j1HomeSwitchPressed();
}

// -----------------------------
// flipping is ONLY allowed when J3 home switch is pressed
// -----------------------------
bool flipIsSafe() {
  return jointIsHome(J3);
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

    if (coords(*joint, rotate, coord)) {
      Serial.println("DONE");
    }
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

    if (homing(*joint)) {
      Serial.println("DONE");
    }
    return;
  }

  if (cmd == "opentight()") {
    GripperTightOpen();
    Serial.println("DONE");
    return;
  }

  if (cmd == "open()") {
    GripperOpen();
    Serial.println("DONE");
    return;
  }

  if (cmd == "dropOpen()") {
    if (!dropIsSafe()) {
      Serial.print("ERROR: drop blocked. J1 switch not pressed. HOME1 = ");
      Serial.println(digitalRead(HOME1));
      return;
    }

    GripperOpen();
    Serial.println("DONE");
    return;
  }

  if (cmd == "close()") {
    GripperClose();
    Serial.println("DONE");
    return;
  }

  if (cmd == "flipUp()") {
    if (!flipIsSafe()) {
      Serial.print("ERROR: flip blocked. J3 not at true home. HOME3 = ");
      Serial.print(digitalRead(HOME3));
      Serial.print(" J3 pos = ");
      Serial.println(J3.pos);
      return;
    }

    FlipGripper(FLIP_UP_ANGLE);
    Serial.println("DONE");
    return;
  }

  if (cmd == "flipDown()") {
    if (!flipIsSafe()) {
      Serial.print("ERROR: flip blocked. J3 not at true home. HOME3 = ");
      Serial.print(digitalRead(HOME3));
      Serial.print(" J3 pos = ");
      Serial.println(J3.pos);
      return;
    }

    FlipGripper(FLIP_DOWN_ANGLE);
    Serial.println("DONE");
    return;
  }

  if (cmd == "flip()") {
    if (!flipIsSafe()) {
      Serial.print("ERROR: flip blocked. J3 not at true home. HOME3 = ");
      Serial.print(digitalRead(HOME3));
      Serial.print(" J3 pos = ");
      Serial.println(J3.pos);
      return;
    }

    if (currentFlipAngle == FLIP_UP_ANGLE) {
      FlipGripper(FLIP_DOWN_ANGLE);
    } else {
      FlipGripper(FLIP_UP_ANGLE);
    }

    Serial.println("DONE");
    return;
  }

  if (cmd == "pos()") {
    printPositions();
    Serial.println("DONE");
    return;
  }

  if (cmd == "cameraPose()") {
    if (cameraPose()) {
      Serial.println("DONE");
    }
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

  pinMode(HOME1, INPUT_PULLUP);
  pinMode(HOME2, INPUT_PULLUP);
  pinMode(HOME3, INPUT_PULLUP);

  Serial.begin(115200);
  delay(3000);
  Serial.println("Ready.");

  gripServo.attach(GRIP_SERVO_PIN, 500, 2400);
  flipServo.attach(FLIP_SERVO_PIN, 500, 2400);

  GripperOpen();
  FlipGripper(FLIP_DOWN_ANGLE);

  inputString.reserve(100);

  Serial.println("Commands:");
  Serial.println("  open()");
  Serial.println("  dropOpen()");
  Serial.println("  close()");
  Serial.println("  flipUp()");
  Serial.println("  flipDown()");
  Serial.println("  flip()");
  Serial.println("  home(J1)");
  Serial.println("  home(J2)");
  Serial.println("  home(J3)");
  Serial.println("  coords(J2,100,8000)");
  Serial.println("  cameraPose()");
  Serial.println("  pos()");
  Serial.println("DONE is only printed on real success.");
}

void loop() {
  readSerialInput();

  if (stringComplete) {
    executeCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}