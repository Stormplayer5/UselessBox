/* Code for mech 307 project
* Adapted from https://blog.hirnschall.net/diy-useless-box/
* Added LCD screen, wake up with an ultrasonic sensor, speaker
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
#define DIR_PIN 10
#define STEP_PIN 11
#define MS1_PIN 7
#define MS2_PIN 6
#define MS3_PIN 9
#define ENABLE_PIN 8
#define MIN_POS -1200
#define MAX_POS 0

//switches
#define SW1_PIN 2
#define SW2_PIN 4
#define SW3_PIN 12
#define SW4_PIN 13
#define SW5_PIN A0
#define SW6_PIN A1
#define NUM_SWITCHES 6
//#define QUEUE_SIZE 7  //NUM_SWITCHES+1

//servos
//PWM pins
#define SERVO1_PIN 5
#define SERVO2_PIN 3
#define NUM_SERVOS 2
#define SERVO_DELAY 250

//endstop
#define ENDSTOP_PIN A2


A4988 stepper(STEPS_PER_REV, DIR_PIN, STEP_PIN, MS1_PIN, MS2_PIN, MS3_PIN);

Servo* servos[NUM_SERVOS];
short servoPins[] = {SERVO1_PIN,SERVO2_PIN};

short switchPins[] = {SW1_PIN,SW2_PIN,SW3_PIN,SW4_PIN,SW5_PIN,SW6_PIN};

short switchesState[]={0,0,0,0,0,0};
unsigned int switchesPos[] = {-1200,-960,-720,-480,-240,0};

short isHomed = 0;
int currentPos = 0;
short isClosed = 1;
short isStandby = 0;
short isExtended = 0;
short isShutdown = 0;
int Trig  ; //Need to add trig pin
int Echo  ; //Need to add Echo Pin
float duration;
float distance;
int Speaker; //Need to add speaker pin

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //Need to make sure these pins are correct


short queue[NUM_SWITCHES] = {-1,-1,-1,-1,-1,-1};
short queuec = 0;

unsigned int idleLoopCounter = 0;

short disassemblingMode=1;


int goToHome(){
  //Serial.println("homing");
  isHomed = 0;
  while(!isHomed){
    if(!digitalRead(ENDSTOP_PIN)){
      isHomed = 1;
      currentPos=0;
      return 1;
    }
    stepper.rotate(0.9);
  }
}

//goto position (pos) given in absolute coordinates
int goTo(int pos){
  //Serial.println("goTo");
  if(pos< MIN_POS || pos > MAX_POS)
    return 0;
  if(!isHomed)
    goToHome();

  int dir = (pos-currentPos)>0?1:-1;
  //check if switch is flipped while moving...
  while(currentPos != pos){
    addSwitchesToQueue();
    stepper.rotate(dir);
    currentPos+=dir;
  }
  return 1;
}
//check if item is in queue
int isInQueue(short item){
  for(int i=0; i< queuec;++i){
    if(queue[i] == item)
      return 1;
  }
  return 0;
}
//add item to end of queue
int qpush(short item){
  queue[queuec++]=item;
}
//remove item from front of queue
int qpop(){
  queuec-=1;
  for(int i=0;i<queuec;++i){
    queue[i] = queue[i+1];
  }
}

int addSwitchesToQueue(){
  //Serial.println("addToQueue");
  for(int i=0;i<NUM_SWITCHES;++i){
    if(!digitalRead(switchPins[i])){
      if(!isInQueue(i)){
        qpush(i);
      }
    }
  }
}

int openDoor(){
  //Serial.println("open");
  //servo 1 open door
  servos[1]->write(OPEN_POS);
  delay(SERVO_DELAY);
  //servo0 to standby position
  servos[0]->write(STANDBY_POS);
  delay(SERVO_DELAY);
  isClosed=0;
  return 1;
}

int closeDoor(){
  //Serial.println("close");
  //servo0 to off position
  servos[0]->write(OFF_POS);

  delay(SERVO_DELAY);
  //servo 1 close door
  servos[1]->write(CLOSED_POS);

  
  delay(SERVO_DELAY);
  isClosed=1;
  return 1;
}

int extend(){
  //Serial.println("extend");
  servos[0]->write(TOP_POS);
  delay(SERVO_DELAY);
}

int retract(){
  //Serial.println("retract");
  servos[0]->write(STANDBY_POS);
  delay(SERVO_DELAY);
}

int activateMotors(){
  //Serial.println("activateMotors");
  isShutdown = 0;
  //attach servos
  for(int i=0;i<NUM_SERVOS;++i){
    servos[i]->attach(servoPins[i]);
  }
  //close door for safety
  closeDoor();

  //enable stepper
  digitalWrite(ENABLE_PIN,LOW);
}
int deactivateMotors(){
  //Serial.println("deactivateMotors");
  isShutdown=1;
  if(!isClosed)
    closeDoor();
  //detach servos
  for(int i=0;i<NUM_SERVOS;++i){
    servos[i]->detach();
  }
  isHomed=0;
  //disable stepper
  digitalWrite(ENABLE_PIN,HIGH);
}

void distancemeaure(){
  digitalWrite(Trig,LOW);
  delayMicroseconds(2);
  digitalWrite(Trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig,LOW);
  duration =pulseIn(Echo,HIGH);
  distance = (duration/2.)/(29.1); //Distance in centimeters
  
  if(distance < 16){ //Set to 16 centimeters.  Roughly 1/2 ft
lcd.print("Welcome to the game...");
    }

void setup() {
  // put your setup code here, to run once:
  // setup switches as digital inputs
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);
  for(int i=0;i<NUM_SWITCHES;++i)
    pinMode(switchPins[i],INPUT_PULLUP);

  //create servo objects and attach pwm pins
  for(int i=0;i<NUM_SERVOS;++i){
    servos[i] = new Servo();
    servos[i]->attach(servoPins[i]);
  }
  
  // Set target motor RPM to 1RPM and microstepping to 1 (full step mode)
  stepper.begin(RPM, MICRO_STEPPING);
  pinMode(ENABLE_PIN,OUTPUT);
  digitalWrite(ENABLE_PIN,LOW);

  servos[0]->write(OFF_POS);
  servos[1]->write(CLOSED_POS);
  delay(SERVO_DELAY);

  //Serial.begin(9600);
  
  //Initialize Ultrasonic Sensor
  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);
  
  //Set up LCD Display
    lcd.setCursor(0, 0);
  // define the bounds for the LCD String
  lcd.begin(16, 2);
  lcd.print("Starting Up");
  delay(4000);
  lcd.print("Ready");
  delay(2500);
  lcd.print(); //Does this work??  Trying to reset the display to not display anything


  //if all switches are active on bootup, activate disassembling mode.
  for(int i=0;i<NUM_SWITCHES;++i){
    if(digitalRead(switchPins[i])){
      disassemblingMode=0;
      break;
    }
  }
  if(disassemblingMode){
    servos[1]->write(DISASSEMBLING_POS);
    delay(SERVO_DELAY);
    //detach servos
    for(int i=0;i<NUM_SERVOS;++i){
      servos[i]->detach();
    }
    isHomed=0;
    //disable stepper
    digitalWrite(ENABLE_PIN,HIGH);
    while(1){
      delay(10000);
    }
  }
}

void loop() {
  //Serial.println("idle Loop: " + String(idleLoopCounter) +" of " + String(TIME_TO_SHUTDOWN_IN_MS/DELAY_BETWEEN_LOOP_RUNS_IN_MS));
  // put your main code here, to run repeatedly:
  delay(DELAY_BETWEEN_LOOP_RUNS_IN_MS);

  //check if switches are pressed. if so, goto correct position and extend arm
  addSwitchesToQueue();
  if(queuec){
    idleLoopCounter = 0; //reset idle loop counter
    if(isShutdown){  //if motors are turned off, turn them back on
      activateMotors();
    goTo(switchesPos[queue[0]]);}
    if(isClosed){
      openDoor();
    extend();
    retract();
    qpop();
    }
  }
  addSwitchesToQueue();
  //if queue is empty and door is open, close it
  if(!queuec && !isClosed){
    closeDoor();
  }
  //if door is closed, increment idle loop counter
  if(isClosed && !(idleLoopCounter >= TIME_TO_SHUTDOWN_IN_MS/DELAY_BETWEEN_LOOP_RUNS_IN_MS)){
    ++idleLoopCounter;}

  if(!isShutdown && idleLoopCounter >= TIME_TO_SHUTDOWN_IN_MS/DELAY_BETWEEN_LOOP_RUNS_IN_MS){
    deactivateMotors(); //turn motors off after idle for more than TIME_TO_SHUTDOWN_IN_MS ms
  }
}// end of void loop
