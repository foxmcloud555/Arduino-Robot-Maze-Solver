#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>


#define LED_PIN 13
#define trigPin 12
#define echoPin 5

//sensor sensitivity
#define QTR_THRESHOLD  300 // microseconds

#define REVERSE_SPEED     100 // 0 is stopped, 400 is full speed
#define TURN_SPEED        100
#define FORWARD_SPEED     100
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     300 // ms

ZumoBuzzer buzzer;
ZumoMotors motors;
int incomingByte;  
bool autoPilot = false;
bool inRoom = false;
int rooms[100];
int pingCountDown = 0;

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];
 
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);


void setup()
{
  pinMode(LED_PIN, OUTPUT);
  // set up ultrasound module
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // initialize serial communication:
  Serial.begin(9600);
  
  // uncomment one or both of the following lines if your motors' directions need to be flipped
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
}

void loop()
{
  ////////////////////////////////////////////////////////
  //ultrasound functions
  ////////////////////////////////////////////////////////
  if (pingCountDown > 32000)
  {
  pingUltraSound();
  }
  pingCountDown++;
//    long duration, distance;
//  digitalWrite(trigPin, LOW);  // Added this line
//  delayMicroseconds(2); // Added this line
//  digitalWrite(trigPin, HIGH);
////  delayMicroseconds(1000); - Removed this line
//  delayMicroseconds(10); // Added this line
//  digitalWrite(trigPin, LOW);
//  duration = pulseIn(echoPin, HIGH);
//  distance = (duration/2) / 29.1;
//  if (distance >= 100 || distance <= 0){
//   // Serial.println("");
//  }
//  else {
//    Serial.print(distance);
//    Serial.println(" cm");
//  }
  ////////////////////////////////////////////////////////
  //ultrasound functions
  ////////////////////////////////////////////////////////
  
  
  ////////////////////////////////////////////////////////
  //Manual Pilot
  ////////////////////////////////////////////////////////
  if (autoPilot == false){
  
  int speed = 100;
  // run left motor forward
   // see if there's incoming serial data:
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    // if it's a capital H (ASCII 72), turn on the LED:
    
    switch (incomingByte)
{
  case 'H':
    digitalWrite(LED_PIN, HIGH);
  break;
  case 'L':
    digitalWrite(LED_PIN, LOW);
   break;
   case 'A':
      motors.setRightSpeed(speed * 2);
      motors.setLeftSpeed(speed * -2);
   break;
   case 'D':
      motors.setRightSpeed(speed * 2);
      motors.setLeftSpeed(speed * -2);
   break;
   case 'W':
      motors.setRightSpeed(speed);
      motors.setLeftSpeed(speed);
   break;
   case 'S':
      motors.setRightSpeed(speed * -1);
      motors.setLeftSpeed(speed * -1);
   break;
   case 'Q':
      motors.setRightSpeed(0);
      motors.setLeftSpeed(0);
   break;
   case 'R':
      Serial.print("entering room");
   break;
   case 'C':
      autoPilot = true;
      inRoom = false;
   break;
}
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
    
  }
  }
  ////////////////////////////////////////////////////////
  //Manual Pilot
  ////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////
  //Auto pilot
  ////////////////////////////////////////////////////////
  if (autoPilot == true)
  {
  sensors.read(sensor_values);
  
   if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    
    if (incomingByte == 'R') {
     
      Serial.print("entering room");
      inRoom = true;
      autoPilot = false;
    }
   }
  
  if ((sensor_values[0] > QTR_THRESHOLD) && (sensor_values[5] < QTR_THRESHOLD))
  { // if leftmost sensor detects line, reverse and turn to the right
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    Serial.print("left hand wall");
  }
  else if ((sensor_values[5] > QTR_THRESHOLD) && (sensor_values[0] < QTR_THRESHOLD))
  {
    // if rightmost sensor detects line, reverse and turn to the left
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    Serial.print("right hand wall");
  }
  else if ((sensor_values[0] > QTR_THRESHOLD) &&(sensor_values[5] > QTR_THRESHOLD))
  {
   autoPilot = false;
   Serial.print("hit a corner");
   motors.setSpeeds(0,0);
  }
  
  else
  {
    // otherwise, go straight
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    Serial.print("going straight");
  }
  }
  ////////////////////////////////////////////////////////
  //Auto pilot
  ////////////////////////////////////////////////////////
  
}

void pingUltraSound()
{
  long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  if (distance >= 100 || distance <= 0){
   // Serial.println("");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  pingCountDown = 0;
}
  
