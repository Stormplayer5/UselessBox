// Include the AccelStepper Library
#include <AccelStepper.h>
#include <Servo.h>
#include <TimerOne.h>
#include <LiquidCrystal.h>

// Define pin connections
const int dirPin = 3;
const int stepPin = 2;
int ENDSTOP_PIN = 19;//A5
// Define motor interface type
#define motorInterfaceType 1
int isHomed = 0;
int pos = 0;
int sw1 = 14; //A0
int sw2 = 15; //A1
int sw3 = 16; //A2
int sw4 = 17; //A3
int sw5 = 18; //A4

short location[] = { -50, -100, -200, -300, -400};
short switchpins[] = {sw1, sw2, sw3, sw4, sw5};
short numswitch[] = { -1, -1, -1, -1, -1, -1};
short sounds[] = {100, 200, 300, 400, 500};
int NUM_SWITCH=6;

int Trig = 4; //Need to add trig pin
int Echo = 5; //Need to add Echo Pin
float duration;
float distance;
int speaker = 6; //Need to add speaker pin
int sleep = 1;
int pinstate;
int pinnum;
int away;
int awaynew;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
// Creates an instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);
Servo myservo;  // create servo object to control a servo


void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);
  myservo.attach(13);  // attaches the servo on pin 6 to the servo object
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(500);
  myStepper.setSpeed(400);
  myStepper.moveTo(2000);
  Serial.begin(9600);
  Serial.println(NUM_SWITCH);

  goToHome();
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);
  pinMode(sw5, INPUT);
  pinMode(ENDSTOP_PIN, INPUT);
  pinMode(speaker, OUTPUT);
  Serial.println("Pinmodes");
  //Set up LCD Display
  lcd.setCursor(0, 0);
  // define the bounds for the LCD String
  lcd.begin(16, 2);
  lcd.print("Starting Up");
  delay(4000);
  lcd.clear();
  lcd.print("Ready");
  delay(2500);
  lcd.clear();
}//End of Setup Loop


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
int distancemeasure() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH);
  distance = (duration / 2.) / (29.1);//Distance in centimeters
  return distance;
}

void loop() {
  // Change direction once the motor reaches target position
  // Move the motor one step
  //myStepper.run();
  //Serial.println(myStepper.currentPosition());
  distance=distancemeasure();
      away = 0;
  Serial.println(distance);
  if (distance <= 16) {
    awaynew = away + 1;
    for (int i = 0; i < 6; ++i) {
      //int n = digitalRead(switchpins[i]);
      if (digitalRead(switchpins[i])==1) {
        checkEStop();
        myStepper.runToNewPosition(location[i]);
        hitSwitch();
        tone(speaker, sounds[i], 1000);
        checkEStop();
       // lcd.clear()
        //lcd.print(i);
        Serial.println(i);
      }
      //Serial.println("loop");
      //distancemeasure();
      
    }
    Serial.println(distance);
    if (awaynew == 1) {
      Hello();
    }
   // distancemeasure();
  }
  else {
    //away = 0;
    awaynew = away -1;
    if (awaynew == -1) {
      Goodbye();
    }
  }
  /* checkEStop();
    myStepper.runToNewPosition(-50);
    hitSwitch();
    myStepper.runToNewPosition(-100);
    hitSwitch();
    myStepper.runToNewPosition(-200);
    hitSwitch();
    myStepper.runToNewPosition(-400);
    moveServo(180);
    checkEStop();*/
}//End of Void Loop Function

void hitSwitch() {
  moveServo(100);
  myservo.write(180);
  delay(10);
}//End hitSwitch loop

int moveServo(int desiredPos) {
  for (pos = 180; pos >= desiredPos; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}//End moveServo loop

void checkEStop() {
  if (digitalRead(ENDSTOP_PIN)) {
    myStepper.setCurrentPosition(0);
    myStepper.stop();
  }
}//End checkEStop loop

void Hello() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hello");
  delay(2500);
  lcd.clear();
}//End Hello Loop

void Goodbye() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Goodbye");
  delay(2500);
  lcd.clear();
}//End Goodbye Loop
