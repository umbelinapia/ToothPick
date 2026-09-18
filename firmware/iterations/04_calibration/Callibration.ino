//Nikitha Prabhakar
//20/03/26
//Caqlliobration

//30-1.7 j1
//30-12= ? 17.5? j2
//9.8- 0.5 j3

/*motorccw(J1, 9250);
  motorccw(J2, 9250);
  motorccw(J3, 1200);*/
#include <ESP32Servo.h>

#define PUL1 1 //J1
#define DIR1 2
#define PUL2 40 //J2
#define DIR2 39
#define PUL3 42 //J2
#define DIR3 41

#define HOME1 4
#define HOME2 5
#define HOME3 6

#define GRIP_SERVO_PIN 7
#define FLIP_SERVO_PIN 15

Servo gripServo;
Servo flipServo;

bool flipState = false;

int rotate = 0;
int FlipHome = 0;
int FlipAngle = 180;
int GripOpen = 0;
int GripClose = 180;

int J1_MAX = 10000;
int J2_MAX = 10150;
int J3_MAX = 625;

const int CAM_J1 = 3750;
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

//delete
String inputString = "";
bool stringComplete = false;

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

  inputString.reserve(100);

  Serial.println("Ready.");
  Serial.println("Example commands:");
  Serial.println("coords(J2,100,8000)");
  Serial.println("home(J1)");
  Serial.println("open()");
  Serial.println("close()");
  Serial.println("flip()");
  Serial.println("pos()");
}

//motorcw(J1, 100)
void motorcw(Joint &J,int rotate) {
  //direction LOW is Clockwise
  //direction HIGH is Anti-clockwise
  digitalWrite(J.dir, LOW);

  //pulse is the control signal
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(500);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(500);
    J.pos--;
  }
}

//motorccw(J1, 100)
void motorccw(Joint &J,int rotate) {
  //direction LOW is Clockwise
  //direction HIGH is Anti-clockwise
  digitalWrite(J.dir, HIGH);

  //pulse is the control signal
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(500);
    digitalWrite(J.pul, LOW);
    delayMicroseconds(500);
    J.pos++;
    Serial.println(J.pos);
  }
}

//homing(J1, 100)
void homing(Joint &J,int rotate) {
  while (digitalRead(J.home) == LOW){
    Serial.println(digitalRead(J.home));
    motorcw(J, rotate);
    
  }
  J.pos = 0;

}



//enc_cal(J1, 100, CountA1);
void pos_cal(Joint &J,int rotate, volatile long &count) {
  count = 0;
  motorcw(J, rotate);
  Serial.print("rotations: ");
  Serial.println(rotate);
}

// FlipGripper()
void FlipGripper() {
  if (flipState == false) {
    flipServo.write(FlipAngle);
    flipState = true;
  } else {
    flipServo.write(FlipHome);
    flipState = false;
  }
}

//GripperOpen()
void GripperOpen(){
  gripServo.write(GripOpen);
  }

//GripperClose()
void GripperClose(){
  gripServo.write(GripClose);
  }
//set the coordinates for camera IN PROGRESS
void cameraPose(){
  coords(J1, 50, CAM_J1);
  coords(J2, 50, CAM_J2);
  coords(J3, 50, CAM_J3);
}

//coords(J1, rotations, coord for joint)
void coords(Joint &J,int rotate, int coord){
  
  if (coord < J.minPos || coord > J.maxPos) {
    Serial.print("ERROR: ");
    Serial.print(" target out of range. Allowed: ");
    Serial.print(J.minPos);
    Serial.print(" to ");
    Serial.println(J.maxPos);
    return;
  }

  while (J.pos < coord) {
    // motorccw(J, rotate);
    
    //delete
    int remaining = coord - J.pos;
    int step = (remaining > rotate) ? rotate : remaining;
    motorccw(J, step);
  }

  while (J.pos> coord){
    // motorcw(J, rotate);

    //delete
    int remaining = coord - J.pos;
    int step = (remaining > rotate) ? rotate : remaining;
    motorccw(J, step);
  }
  delay(2000);

}

//delete

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

//delete
void executeCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) return;

  Serial.print("Received: ");
  Serial.println(cmd);

  // coords(J2,100,8000)
  if (cmd.startsWith("coords(") && cmd.endsWith(")")) {
    int start = cmd.indexOf('(');
    int end = cmd.lastIndexOf(')');
    String inside = cmd.substring(start + 1, end);

    int c1 = inside.indexOf(',');
    int c2 = inside.indexOf(',', c1 + 1);

    if (c1 == -1 || c2 == -1) {
      Serial.println("Invalid coords format. Use: coords(J2,100,8000)");
      return;
    }

    String jointName = inside.substring(0, c1);
    String rotateStr = inside.substring(c1 + 1, c2);
    String coordStr = inside.substring(c2 + 1);

    Joint* joint = getJointByName(jointName);
    if (joint == nullptr) {
      Serial.println("Invalid joint. Use J1, J2, or J3.");
      return;
    }

    int rotate = rotateStr.toInt();
    long coord = coordStr.toInt();

    coords(*joint, rotate, coord);
    return;
  }

  // home(J1)
  if (cmd.startsWith("home(") && cmd.endsWith(")")) {
    int start = cmd.indexOf('(');
    int end = cmd.lastIndexOf(')');
    String jointName = cmd.substring(start + 1, end);

    Joint* joint = getJointByName(jointName);
    if (joint == nullptr) {
      Serial.println("Invalid joint. Use J1, J2, or J3.");
      return;
    }

    homing(*joint, 10);   // smaller step size is safer for homing
    return;
  }

  // open()
  if (cmd == "open()") {
    GripperOpen();
    return;
  }

  // close()
  if (cmd == "close()") {
    GripperClose();
    return;
  }

  // flip()
  if (cmd == "flip()") {
    FlipGripper();
    return;
  }

  // pos()
  if (cmd == "pos()") {
    printPositions();
    return;
  }

  Serial.println("Unknown command.");
}

//delete
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


void loop() {
  readSerialInput();

  if (stringComplete) {
    executeCommand(inputString);
    inputString = "";
    stringComplete = false;
  }

  // homing(J1,100);
  // homing(J2,100);
  // homing(J3,100);
  // delay(200);
  // coords(J2,100,8000);
  // delay(100);
  // coords(J2,100,200);
  // delay(100);
  // coords(J2,100,8000);
  // delay(200);
  // GripperClose();
  // delay(200);
  // GripperOpen();
  // delay(200);
  // coords(J1,100,8000);
  // delay(100);
  // coords(J2,100,8000);
  // delay(100);


}

