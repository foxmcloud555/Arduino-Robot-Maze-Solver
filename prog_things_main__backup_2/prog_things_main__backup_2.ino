#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>


#define LED_PIN 13
#define trigPin 12
#define echoPin 2

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
int rooms[100][2];
int roomNumber;
int pingCountDown = 0;
int corridorObjects[100]; //number held indicates what room was last visited.
int objectsOutsideRooms = 0;

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];
 
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);


void setup()
{
   sensors.init();

   //delay(500);
  
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  
  Serial.println("calibration start");
  buzzer.playNote(NOTE_G(3), 200, 15);
   unsigned long startTime = millis();
  while(millis() - startTime < 10000)   // make the calibration take 10 seconds
  {
    sensors.calibrate();
  }
  Serial.println("calibration end");
  buzzer.playNote(NOTE_G(4), 500, 15); 
  
  // set up ultrasound module
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // initialize serial communication:
  
  roomNumber = 0;
   for (byte i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(sensors.calibratedMinimumOn[i]);
    Serial.print(' ');
  }
  Serial.println();

   for (byte i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(sensors.calibratedMaximumOn[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(1000);
  
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
  int objectDistance = pingUltraSound();
  if (objectDistance < 10)
    {
   if (inRoom == true)
     {
       rooms[roomNumber][0] = 1 ;//rooms with a 1 mean an object was found
       Serial.print("I found an object in room: ");
       Serial.print(roomNumber);
     } 
     else
         {
     corridorObjects[objectsOutsideRooms] = roomNumber;
         }
    }
  }
  pingCountDown++;

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
    
    incomingByte = Serial.read();
    
    switch (incomingByte)
{
  case 'H':
    digitalWrite(LED_PIN, HIGH);
     
  break;
  case 'L':
    digitalWrite(LED_PIN, LOW);
   break;
   case 'A':
      motors.setRightSpeed(speed * 1);
      motors.setLeftSpeed(speed * -1);
   break;
   case 'D':
      motors.setLeftSpeed(speed * 1);
      motors.setRightSpeed(speed * -1);
   break;
   case 'W':
      motors.setRightSpeed(speed);
      motors.setLeftSpeed(speed);
   break;
   case 'S':
      motors.setRightSpeed(speed * -1);
      motors.setLeftSpeed(speed * -1);
   break;
   case 32:
   case 'Q':
   case 'q':
      motors.setRightSpeed(0);
      motors.setLeftSpeed(0);
   break;
   case 'R':
      Serial.print("entering room");
      roomNumber++;
   break;
   case 'C':
      autoPilot = true;
      inRoom = false;
   break;
   case 91:
     rooms[roomNumber][1] = 0;
    break;
    case 93:
     rooms[roomNumber][1] = 1;
    break;
    case 'P':
      Serial.print("Objects found in room(s): ");
      for (int i = 0; i <= roomNumber; i++){
        if (rooms[i][0] == 1)
        {
          Serial.print(i);
         Serial.print(',');
        }
      }
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
  

  
  if ((sensor_values[0] > QTR_THRESHOLD) && (sensor_values[5] < QTR_THRESHOLD))
  { // if leftmost sensor detects line, reverse and turn to the right
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    //Serial.print("left hand wall");
  }
  else if ((sensor_values[5] > QTR_THRESHOLD) && (sensor_values[0] < QTR_THRESHOLD))
  {
    // if rightmost sensor detects line, reverse and turn to the left
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
   // Serial.print("right hand wall");
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
    //Serial.println("going straight");
  }
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    
    if (incomingByte == 'R') {
     autoPilot = false;
      Serial.println("entering room");
      roomNumber++;
      inRoom = true;
      motors.setSpeeds(0,0);
    }
    if (incomingByte == 32) {
     autoPilot = false;
     motors.setSpeeds(0,0);
    }
   }
  }
  ////////////////////////////////////////////////////////
  //Auto pilot
  ////////////////////////////////////////////////////////
  
}

long pingUltraSound()
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
  if (distance >= 10 || distance <= 0){
   // Serial.println("");
   
  }
  else {
    
    Serial.print(distance);
    Serial.println(" cm");
  }
  pingCountDown = 0;
  return distance;
}
  
