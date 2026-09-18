//Nikitha Prabhakar
//25/03/26
//SPEED TEST


/*
HELLO! this is a speed test that is supposed to make the motors go faster than before.

I have edited motorcw and motorccw
i have made the time between pul shorter
let me know if it hits the end of not

LOOK FOR
//I EDITED THIS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


*/






// ROUGH LIMITS
//j1     28.3cm = 9250 rotations
//j2     18cm = 9250 rotations
//j3     9.3cm = 1200 rotations

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

#define GRIP_SERVO_PIN 15
#define FLIP_SERVO_PIN 7

Servo gripServo;
Servo flipServo;

int rotate = 0;
int FlipHome = 0;
int FlipAngle = 180;
int GripOpen = 30;
int GripClose = 70;
int x = 0;
int y = 0;

struct Joint {
  int pul;
  int dir;
  int home;
  long pos;
};

Joint J1 = {PUL1, DIR1, HOME1, 0};
Joint J2 = {PUL2, DIR2, HOME2, 0};
Joint J3 = {PUL3, DIR3, HOME3, 0};

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
}

//checks if serial is received and expects (x,y) no spaces
bool CameraXY(int &x, int &y){
  if(Serial.available()){
    String msg = Serial.readStringUntil('\n');

    int comma_pos = msg.indexOf(',');

    if (comma_pos != -1) {
    String X_str = msg.substring(0, comma_pos);
    String Y_str = msg.substring(comma_pos + 1);  

    x = X_str.toInt();
    y = Y_str.toInt();

    return true;
    }
  }
  return false;
}

//motorcw(Joint, rotations) is a function that moves the joins towards the home position decreasing count
//motorcw(J1, 100)
void motorcw(Joint &J,int rotate) {
  //direction LOW is Clockwise
  //direction HIGH is Anti-clockwise
  digitalWrite(J.dir, LOW);

  //pulse is the control signal
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(250); //I EDITED THIS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FROM 500
    digitalWrite(J.pul, LOW);
    delayMicroseconds(250);//I EDITED THIS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FROM 500
    J.pos--;
  }
}

//motorcw(Joint, rotations) is a function that moves the joins away from the home position increasing count
//motorccw(J1, 100)
void motorccw(Joint &J,int rotate) {
  //direction LOW is Clockwise
  //direction HIGH is Anti-clockwise
  digitalWrite(J.dir, HIGH);

  //pulse is the control signal
  for (int i = 0; i < rotate; i++) {
    digitalWrite(J.pul, HIGH);
    delayMicroseconds(250);//I EDITED THIS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FROM 500
    digitalWrite(J.pul, LOW);
    delayMicroseconds(250);//I EDITED THIS HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FROM 500
    J.pos++;
    Serial.println(J.pos);
  }
}

////homing(Joint, rotations) is a function that homes the robot and resets the count
//homing(J1, 100)
void homing(Joint &J,int rotate) {
  while (digitalRead(J.home) == LOW){
    Serial.println(digitalRead(J.home));
    motorcw(J, rotate);
    
  }
  J.pos = 0;

}


/*
I believe this function is no longer needed?
//enc_cal(J1, 100, CountA1);
void pos_cal(Joint &J,int rotate, volatile long &count) {
  count = 0;
  motorcw(J, rotate);
  Serial.print("rotations: ");
  Serial.println(rotate);
}
*/

// FlipGripper()
void FlipGripper(){
  flipServo.write(FlipAngle);
  }

void FlipGripperHome(){
  flipServo.write(FlipHome);
  }

//GripperOpen()
void GripperOpen(){
  gripServo.write(GripOpen);
  }

//GripperClose()
void GripperClose(){
  gripServo.write(GripClose);
  }

//set the coordinates for camera IN PROGRESS (you guys tell it wher you want it)
void cameraPose(){
  coords(J1,100, 9250);
  coords(J2,100, 9250);

}

//This function moves the robot to the robot coordinates in one axis taking into account the current position
//coords(JOint, rotations, coord for joint)
void coords(Joint &J,int rotate, int coord){
  while (J.pos < coord) {
    motorccw(J, rotate);
  }
  while (J.pos> coord){
    motorcw(J, rotate);
  }
  delay(2000);

}
//Pick up sequence that uses the translated x,y coordinates
//Please edit coordinate values as necessary
void pick_up(int x,int y){
  coords(J1,100,x);
  delay(100);
  coords(J2,100,y);
  delay(100);
  GripperOpen();
  delay(100);
  coords(J3,100,1200);
  delay(100);
  GripperClose();
  delay(100);
  coords(J3,100,100);
  delay(100);
}

//drop off sequence is the same every time
//adjust values as necessary
void drop_off(){
  coords(J1,100,0);
  coords(J2, 100, 9250);
  FlipGripper();
  coords(J3,100, 700);
  GripperOpen();
  GripperClose();
  
}

//Homes the robot and gets the camera x,y and executes the pick up and drop off
void loop() {
  homing(J1,100);
  homing(J2,100);
  homing(J3,100);
  delay(200);
  motorccw(J1, 9250);
  motorccw(J2, 9250);
  motorccw(J3, 1200);
  delay(200);
  
  
}

