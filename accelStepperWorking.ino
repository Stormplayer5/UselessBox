// Include the AccelStepper Library
#include <AccelStepper.h>
#include <Servo.h>


// Define pin connections
const int dirPin = 3;
const int stepPin = 2;
int ENDSTOP_PIN = 5;
// Define motor interface type
#define motorInterfaceType 1
int isHomed = 0;
int pos = 0; 

// Creates an instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);
Servo myservo;  // create servo object to control a servo


void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);
  myservo.attach(6);  // attaches the servo on pin 6 to the servo object
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(500);
  myStepper.setSpeed(400);
  myStepper.moveTo(2000);
  Serial.begin(9600);
  
  goToHome(); 
}

int goToHome() {
  Serial.println("homing");
  isHomed = 0;
  while (!isHomed) {
    //Serial.println("Current Position");
    //Serial.println(myStepper.currentPosition());
    if (digitalRead(ENDSTOP_PIN)) {
      isHomed = 1;
      myStepper.setCurrentPosition(0);
      //Serial.println("Current Position");
      //Serial.println(myStepper.currentPosition());
      //Serial.println("Homed!");
      return 1; 
    }
    myStepper.run();
  }
  //return 0;
}

void loop() {
  // Change direction once the motor reaches target position
  // Move the motor one step
  //myStepper.run();
  //Serial.println(myStepper.currentPosition());
  checkEStop();
  myStepper.runToNewPosition(-50);
  hitSwitch();
  myStepper.runToNewPosition(-100);
  hitSwitch();
  myStepper.runToNewPosition(-200);
  hitSwitch();
  myStepper.runToNewPosition(-400);
  moveServo(180);
  checkEStop();
}

void hitSwitch(){
  moveServo(100);
  myservo.write(180); 
  delay(10);
}
int moveServo(int desiredPos){
    for (pos = 180; pos >= desiredPos; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void checkEStop() {
  if (digitalRead(ENDSTOP_PIN)) {
    myStepper.setCurrentPosition(0);
    myStepper.stop();
  }
}
