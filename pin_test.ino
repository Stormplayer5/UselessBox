#define pin1 A0
#define pin2 A1
#define pin3 A2
#define pin4 A3
#define pin5 A4
#define numSwitches 6
short switchPins[] = {pin1, pin2, pin3, pin4, pin5};
void setup() {
  // put your setup code here, to run once:
for (int i=0; i<=numSwitches; ++i){
pinMode(switchPins[i],INPUT);
}
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
for (int i=0; i<numSwitches; ++i){
if (digitalRead(switchPins[i])==1){
  Serial.println("HIGH");
  Serial.println(switchPins[i]);
}
else {
Serial.println("LOW");
}
}
}
