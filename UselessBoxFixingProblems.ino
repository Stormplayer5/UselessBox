/* Code for mech 307 project
  Adapted from https://blog.hirnschall.net/diy-useless-box/
  Added LCD screen, wake up with an ultrasonic sensor, speaker
  Must have StepperDriver.h library installed. Search A4988 in Arduino Libraries
  Code is used on an Arduino Uno board
*/

#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <DRV8880.h>
#include <MultiDriver.h>
#include <SyncDriver.h>
#include <LiquidCrystal.h>
#include <Servo.h>

//misc
#define TIME_TO_SHUTDOWN_IN_MS 30000 //30s
#define DELAY_BETWEEN_LOOP_RUNS_IN_MS 100


#define TOP_POS 30
#define STANDBY_POS 90
#define OFF_POS 140

#define DISASSEMBLING_POS 170
#define OPEN_POS 75
#define CLOSED_POS 20


//stepper settings
#define RPM 200
#define STEPS_PER_REV 200
#define MICRO_STEPPING 16
#define HOMEING_MOVEMENTS 1
#define DIR_PIN 3
#define STEP_PIN 2
#define MS1_PIN 7
#define MS2_PIN 6
#define MS3_PIN 9
#define ENABLE_PIN 8
#define MIN_POS -1200
#define MAX_POS 0

//switches
#define SW1_PIN A0
#define SW2_PIN A1
#define SW3_PIN A2
#define SW4_PIN A3
#define SW5_PIN A4
//#define SW6_PIN A1
#define NUM_SWITCHES 5
//#define QUEUE_SIZE 6  //NUM_SWITCHES+1

//servos
//PWM pins
// #define SERVO1_PIN 5
#define SERVO2_PIN 13
#define NUM_SERVOS 1
#define SERVO_DELAY 250

//endstop
#define ENDSTOP_PIN A5


A4988 stepper(STEPS_PER_REV, DIR_PIN, STEP_PIN, MS1_PIN, MS2_PIN, MS3_PIN);

Servo* servos[NUM_SERVOS];
short servoPins[] = {SERVO2_PIN};

short switchPins[] = {SW1_PIN, SW2_PIN, SW3_PIN, SW4_PIN, SW5_PIN};

short switchesState[] = {0, 0, 0, 0, 0};
unsigned int switchesPos[] = {-960, -720, -480, -240, 0};

short isHomed = 0;
int currentPos = 0;
short isClosed = 1;
short isStandby = 0;
short isExtended = 0;
short isShutdown = 0;
int Trig = 4; //Need to add trig pin
int Echo = 5; //Need to add Echo Pin
float duration;
float distance;
int speaker = 6; //Need to add speaker pin
int sleep=0;
int pinstate;
int speaker1 = 200;
int speaker2 = 300;
int speaker3 = 400;
int speaker4 = 500;
int speaker5 = 600;
int pinnum;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //Need to make sure these pins are correct


short queue[NUM_SWITCHES] = { -1, -1, -1, -1, -1};
short queuec = 0;

unsigned int idleLoopCounter = 0;

short disassemblingMode = 1;


int goToHome() {
  //Serial.println("homing");
  isHomed = 0;
  while (!isHomed) {
    if (!digitalRead(ENDSTOP_PIN)) {
      isHomed = 1;
      currentPos = 0;
      return 1;
    }
    stepper.rotate(0.9);
  }

  //return 0;
}

//goto position (pos) given in absolute coordinates
int goTo(int pos) {
  //Serial.println("goTo");
  if (pos < MIN_POS || pos > MAX_POS)
    return 0;
  if (!isHomed)
    goToHome();

  int dir = (pos - currentPos) > 0 ? 1 : -1;
  //check if switch is flipped while moving...
  while (currentPos != pos) {
    addSwitchesToQueue();
    stepper.rotate(dir);
    currentPos += dir;
  }
  return 1;
}
//check if item is in queue
int isInQueue(short item) {
  for (int i = 0; i < queuec; ++i) {
    if (queue[i] == item)
      return 1;
  }
  return 0;
}
//add item to end of queue
int qpush(short item) {
  queue[queuec++] = item;
}
//remove item from front of queue
int qpop() {
  queuec -= 1;
  for (int i = 0; i < queuec; ++i) {
    queue[i] = queue[i + 1];
  }
}

int addSwitchesToQueue() {
  Serial.println("addToQueue");
  for (int i = 0; i < NUM_SWITCHES; ++i) {
    if (digitalRead(switchPins[i])==1) {
      if (!isInQueue(i)) {
        qpush(i);
        Serial.println(i);
      }
    }
  }
}

int openDoor() {
  //Serial.println("open");
  //servo 1 open door
  servos[1]->write(OPEN_POS);
  delay(SERVO_DELAY);
  //servo0 to standby position
  servos[0]->write(STANDBY_POS);
  delay(SERVO_DELAY);
  isClosed = 0;
  return 1;
}

int closeDoor() {
  //Serial.println("close");
  //servo0 to off position
  servos[0]->write(OFF_POS);

  delay(SERVO_DELAY);
  //servo 1 close door
  servos[1]->write(CLOSED_POS);


  delay(SERVO_DELAY);
  isClosed = 1;
  return 1;
}

int extend() {
  //Serial.println("extend");
  servos[0]->write(TOP_POS);
  delay(SERVO_DELAY);
}

int retract() {
  //Serial.println("retract");
  servos[0]->write(STANDBY_POS);
  delay(SERVO_DELAY);
}

int activateMotors() {
  //Serial.println("activateMotors");
  isShutdown = 0;
  //attach servos
  for (int i = 0; i < NUM_SERVOS; ++i) {
    servos[i]->attach(servoPins[i]);
  }
  //close door for safety
  closeDoor();

  //enable stepper
  digitalWrite(ENABLE_PIN, LOW);
}
int deactivateMotors() {
  //Serial.println("deactivateMotors");
  isShutdown = 1;
  if (!isClosed)
    closeDoor();
  //detach servos
  for (int i = 0; i < NUM_SERVOS; ++i) {
    servos[i]->detach();
  }
  isHomed = 0;
  //disable stepper
  digitalWrite(ENABLE_PIN, HIGH);
}

void distancemeasure() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH);
  distance = (duration / 2.) / (29.1); //Distance in centimeters
Serial.println(distance);
  if (distance < 16) { //Set to 16 centimeters.  Roughly 1/2 ft
    //sleep = 0;
    tone(speaker,300, 500);

    tone(speaker, 400, 500);

    tone(speaker, 600, 500);

    activateMotors();
    lcd.clear();
    lcd.print("Welcome to");
    lcd.setCursor(0,1);
    lcd.print("the game...");
    delay(2500);
    lcd.clear();
  }
  else{
    tone(speaker,600, 500);

    tone(speaker, 400, 500);

    tone(speaker, 300, 500);

  deactivateMotors();
  lcd.print("Goodbye");
  delay(2500);
  lcd.clear();
  //sleep=1;
}

  //Serial.println("System on");
}

void sounds() {
  //Use this loop to play tones based on the pin
  for (int i = 0; i < NUM_SWITCHES; ++i) {
    int pinstate = digitalRead(switchPins[i]);
    if (pinstate == HIGH && i == 1) {
      tone(speaker, speaker1, 1000);
      noTone(speaker);
    }
    else if (pinstate == HIGH && i == 2) {
      tone(speaker, speaker2, 1000);
      noTone(speaker);
    }
    else if (pinstate == HIGH && i == 3) {
      tone(speaker, speaker3, 1000);
      noTone(speaker);
    }
    else if (pinstate == HIGH && i == 4) {
      tone(speaker, speaker2, 1000);
      noTone(speaker);
    }
    else if (pinstate == HIGH && i == 5) {
      tone(speaker, speaker5, 1000);
      noTone(speaker);
    }
    //Each switch plays a different tone, specified by the different speaker#s
  }
}
/* void repeat(){
  for (int i=0;i<NUM_SWITCHES;++i){
   int pinstate=digitalRead(switchPins[i]);
    if (pinstate==HIGH && i==1){
       pinnum=1;
    }
    else if (pinstate==HIGH && i==2){
        pinnum=2;
    }
    else if (pinstate==HIGH && i==3){
       pinnum=3;
    }
    else if (pinstate==HIGH && i==4){
        pinnum=4;
    }
    else if (pinstate==HIGH&&i==5){
      pinnum=5;
    }
    //Names the different pins with different variables
  }
  if (newstate==oldstate){
  presscount=presscount+1;
  oldstate=pinnum;
  }
  else{
  count=0;
  }
  if (count=3){
  lcd.print("Press another switch");
  delay(2000);
  lcd.clear();
  }
  else if (count=4){
  lcd.print("I said choose differently")
  delay(2000);
  lcd.clear();
  }
  else if (count>=5){
  lcd.print("Im Leaving");
  delay(2000);
  lcd.clear();
  deactivateMotors();
  lcd.print("Bye >:(");
  delay(2000);
  lcd.clear();
  }
  }
*/

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  // put your setup code here, to run once:
  // setup switches as digital inputs
  pinMode(ENDSTOP_PIN, INPUT);
  for (int i = 0; i < NUM_SWITCHES; ++i) {
    pinMode(switchPins[i], INPUT);
  }
  Serial.println("Switch Pinmodes");

  //create servo objects and attach pwm pins
  for (int i = 0; i < NUM_SERVOS; ++i) {
    servos[i] = new Servo();
    servos[i]->attach(servoPins[i]);
  }

  // Set target motor RPM to 1RPM and microstepping to 1 (full step mode)
  stepper.begin(RPM, MICRO_STEPPING);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);
Serial.println("Target Motor");
  servos[0]->write(OFF_POS);
  servos[1]->write(CLOSED_POS);
  delay(SERVO_DELAY);
  Serial.println("Servos");

  //Serial.begin(9600);

  //Initialize Ultrasonic Sensor
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
Serial.println("Pinmodes");
  //Set up LCD Display
  lcd.setCursor(0, 0);
  // define the bounds for the LCD String
    tone(speaker,349.23, 500);
    noTone(speaker);
    tone(speaker, 415.3, 500);
    noTone(speaker);
    tone(speaker, 622.44, 500);
    noTone(speaker);
    Serial.println("Sounds");
  lcd.begin(16, 2);
  lcd.print("Starting Up");
  delay(4000);
  lcd.clear();
  lcd.print("Ready");
  delay(2500);
  lcd.clear(); //Does this work??  Trying to reset the display to not display anything

Serial.println("LCD Bootup");
  //if all switches are active on bootup, activate disassembling mode.
  for (int i = 0; i < NUM_SWITCHES; ++i) {
    if (digitalRead(switchPins[i])==1) {
      disassemblingMode = 0;
      break;
    }
  }
  Serial.println("Activate Disassemble");
 /* if (disassemblingMode) {
    servos[1]->write(DISASSEMBLING_POS);
    delay(SERVO_DELAY);
    //detach servos
    for (int i = 0; i < NUM_SERVOS; ++i) {
      servos[i]->detach();
    }
    Serial.println("Dissemble");
    isHomed = 0;
    //disable stepper
    digitalWrite(ENABLE_PIN, HIGH);
    while (1) {
      delay(10000);
    }
  }*/
  Serial.println("Setup end");
}
//end of setup loop

void loop() {
 // Serial.println("sleep");
  //Serial.println("loop");
  //Serial.println("idle Loop: " + String(idleLoopCounter) +" of " + String(TIME_TO_SHUTDOWN_IN_MS/DELAY_BETWEEN_LOOP_RUNS_IN_MS));
  // put your main code here, to run repeatedly:
  delay(DELAY_BETWEEN_LOOP_RUNS_IN_MS);
 distancemeasure();
  //Serial.println("Out of sleep loop");
  //check if switches are pressed. if so, goto correct position and extend arm
  addSwitchesToQueue();
  if (queuec) {
    idleLoopCounter = 0; //reset idle loop counter
    if (isShutdown) { //if motors are turned off, turn them back on
      activateMotors();
      goTo(switchesPos[queue[0]]);
      sounds();  //Play a tone based on which switch is pressed.  Is this the best spot for this?
    }
    if (isClosed) {
      openDoor();
      extend();
      retract();
      qpop();
    }
  }
  addSwitchesToQueue();
  //if queue is empty and door is open, close it
  if (!queuec && !isClosed) {
    closeDoor();
  }
  //if door is closed, increment idle loop counter
  if (isClosed && !(idleLoopCounter >= TIME_TO_SHUTDOWN_IN_MS / DELAY_BETWEEN_LOOP_RUNS_IN_MS)) {
    ++idleLoopCounter;
  }

  if (!isShutdown && idleLoopCounter >= TIME_TO_SHUTDOWN_IN_MS / DELAY_BETWEEN_LOOP_RUNS_IN_MS) {
    deactivateMotors(); //turn motors off after idle for more than TIME_TO_SHUTDOWN_IN_MS ms
    lcd.print("Goodbye");
    lcd.clear();
    distancemeasure();
  }
}// end of void loop
  
