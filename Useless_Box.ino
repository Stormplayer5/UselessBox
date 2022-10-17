/* Code for useless box
 *  First iteration on 10/17/2022
 */
int sw0 = 0; //Starting position of arm
int sw1 = ; //Switch 1
int sw2 = ; //Switch 2
int sw3 = ; //Switch 3
int sw4 = ; //Switch 4
void setup() {
  // put your setup code here, to run once:
pinMode(sw1,INPUT);
  pinMode(sw2,INPUT);
  pinMode(sw3,INPUT);
  pinMode(sw4,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
/* Thinking about the best way to move the arm back and forth. I was thinking about
* timing the time it took the motor to move the arm from its start position to each switch position.
* we could then do some sort of loop that uses the timing of the motor to control its position.  For example
* sw3 takes 2 seconds and sw1 takes 1 second.  To move it from sw1 to sw3, we would run the motor for 1 second and 
* then activate the servo motor on the arm to turn off the switch.  The loop could look something like this:
* int sw0 = 0;
* location = 0;
* for (sw3 = HIGH) {
* time=time3-time(location);
* digitalWrite(motor,time);
* activate servo motor;
* location=3
* }
*/
}
