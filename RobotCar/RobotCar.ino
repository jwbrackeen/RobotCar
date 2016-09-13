#include <Wire.h>
#include <Adafruit_Sensor.h>
//#include <Adafruit_HMC5883_U.h>

/* Assign a unique ID to this sensor at the same time */
//#define MAG /* ENABLE MAG */
//Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

#define R_SPEED     6    // A-IA 
#define R_DIRECTION 9    // A-IB
#define L_SPEED     3    // B-IA
#define L_DIRECTION 5    // B-IB
#define REVERSE     1
#define FORWARD     0

#define SONAR_VCC  10
#define SONAR_TRIG 11
#define SONAR_ECHO 12 

#define RIGHT 1
#define LEFT 0

#define HARD 1
#define SOFT 0

const bool DEBUG = 0;
const bool DEBUG_MOTOR = 0;
int timeOutEnable = 0;
unsigned int reverseTimeOut = 0;
unsigned int forwardTimeOut = 0;
int fwTOFlag = 0;
    
/*
***************************
*       Prototypes        *
***************************
*/

void motorRight(int motorSpeed, int motorDirection);
void motorLeft(int motorSpeed, int motorDirection);
int spin(int spinSpeed, int spinDirection, int spinDuration); // spinDuration == ms
unsigned long sonar(void);

/*
***************************
*         SETUP           *
***************************
*/

void setup() {

  pinMode(R_SPEED,     OUTPUT);  // right motor forward
  pinMode(R_DIRECTION, OUTPUT);  // right motor reverse
  pinMode(L_SPEED,     OUTPUT);  // left motor forward
  pinMode(L_DIRECTION, OUTPUT);  // left motor revers
  
  pinMode(SONAR_VCC,   OUTPUT);
  pinMode(SONAR_TRIG,  OUTPUT);
  pinMode(SONAR_ECHO,  INPUT);
  digitalWrite(SONAR_VCC, HIGH);

  if(DEBUG || DEBUG_MOTOR)Serial.begin(115200);

//  #ifdef MAG
//  /* Initialise the Compas sensor */
//  if (!mag.begin())
//  {
//    /* There was a problem detecting the HMC5883 ... check your connections */
//    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
//    while (1);
//  }
//  #endif /* MAG */
  
}

/*
***************************
*         LOOP            *
***************************
*/

void loop() {
  
//  /* Get a new sensor event */
//  #ifdef MAG
//  sensors_event_t event;
//  mag.getEvent(&event);
//  float heading = atan2(event.magnetic.y, event.magnetic.x);
//  // Correct for when signs are reversed.
//  if (heading < 0)
//    heading += 2 * PI;
//  // Check for wrap due to addition of declination.
//  if (heading > 2 * PI)
//    heading -= 2 * PI;
//  // Convert radians to degrees for readability.
//  float headingDegrees = heading * 180 / M_PI;
//  #endif /* MAG */

  /* SONAR */
  unsigned long distance = sonar();
  if(DEBUG)Serial.print("Distance: ");
  if(DEBUG)Serial.print(distance);
  if(DEBUG)Serial.print(" cm");
  if(DEBUG)Serial.print("\n");
  //if(DEBUG)Serial.print("timeOut: ");
  //if(DEBUG)Serial.print(timeOut);
  //if(DEBUG)Serial.print("\n");

  /* MAG */
//  #ifdef MAG
//  if(DEBUG)Serial.print("X-direction: ");
//  if(DEBUG)Serial.print(event.magnetic.x);
//  if(DEBUG)Serial.print("  ");
//  if(DEBUG)Serial.print("Y-direction: ");
//  if(DEBUG)Serial.print(event.magnetic.y);
//  if(DEBUG)Serial.print("  ");
//  if(DEBUG)Serial.print("Z-direction: ");
//  if(DEBUG)Serial.print(event.magnetic.z);
//  if(DEBUG)Serial.print("\n");
//  if(DEBUG)Serial.print("Heading (degrees): ");
//  if(DEBUG)Serial.print(headingDegrees);
//  if(DEBUG)Serial.print("\n");
//  if(DEBUG)Serial.print("forwardTimeOut: ");
//  if(DEBUG)Serial.print(forwardTimeOut);
//  if(DEBUG)Serial.print("\n");
//  #endif MAG
  
  if(distance < 40) {
    forwardTimeOut = 0;    // reset forward timer
    timeOutEnable = 1;     // enable reverse timeout
    motorBreak(SOFT);      // softly break
    int time = millis();   // initialize time
    int cTime = time;
    while(cTime < time + 250) {
      if(DEBUG_MOTOR)Serial.print("REVERSE\n");
      motorRight(150, REVERSE);
      motorLeft(70  , REVERSE);
      cTime = millis();
    }
    reverseTimeOut++;
  }
  else {
    if(DEBUG_MOTOR)Serial.print("FORWARD\n");
    timeOutEnable = 0;
    reverseTimeOut = 0;
    motorRight(150, FORWARD);
    motorLeft(120 , FORWARD);
    
    /* forward timeout */
    forwardTimeOut++;
  }
  
  if(timeOutEnable == 1 && reverseTimeOut > 4) {
    if(DEBUG_MOTOR)Serial.print("SPIN FOR REVERSE TIMEOUT\n");
    reverseTimeOut = spin(255, LEFT, 500);
  }
  
  if(forwardTimeOut > 600) {
    if(DEBUG_MOTOR)Serial.print("SPIN FOR FORWARD TIMEOUT\n");
    forwardTimeOut = spin(240, RIGHT, 600);
  }
}

/*
***************************
*       FUNCTIONS         *
***************************
*/

void motorRight(int motorSpeed, int motorDirection) {

  if(motorDirection == FORWARD) {
    analogWrite(R_SPEED, motorSpeed);
    digitalWrite(R_DIRECTION, motorDirection);
  }
  else if(motorDirection == REVERSE){
    analogWrite(R_SPEED, ~motorSpeed);
    digitalWrite(R_DIRECTION, motorDirection);
  }
  else
    analogWrite(R_SPEED, 0);
}

void motorLeft(int motorSpeed, int motorDirection) {

  if(motorDirection == FORWARD) {
    analogWrite(L_SPEED, motorSpeed);
    digitalWrite(L_DIRECTION, motorDirection);
  }
  else if(motorDirection == REVERSE){
    analogWrite(L_SPEED, ~motorSpeed);
    digitalWrite(L_DIRECTION, motorDirection);
  }
  else
    analogWrite(L_SPEED, 0);
}

void motorBreak(int breakType) {
  
  digitalWrite(R_SPEED    , breakType);
  digitalWrite(R_DIRECTION, breakType);
  digitalWrite(L_SPEED    , breakType);
  digitalWrite(L_DIRECTION, breakType);
  
}

int spin(int spinSpeed, int spinDirection, int spinDuration) {

    int time = millis();
    int cTime = time;
    while(cTime < time + spinDuration) {  // spin around
      if(spinDirection == RIGHT) {
        motorRight(spinSpeed, REVERSE);
        motorLeft(spinSpeed , FORWARD);
      }
      else {
        motorRight(spinSpeed, FORWARD);
        motorLeft(spinSpeed , REVERSE);
      }
      cTime = millis();
    }
    return 0;
}

unsigned long sonar(void) {

  unsigned long duration, distance;
  digitalWrite(SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONAR_TRIG, LOW);
  duration = pulseIn(SONAR_ECHO, HIGH);
  distance = (duration/2) / 29.1;

//  if(DEBUG)Serial.print("SONAR duration: ");
//  if(DEBUG)Serial.print(duration);
//  if(DEBUG)Serial.print("\n");
//  if(DEBUG)Serial.print("SONAR distance: ");
//  if(DEBUG)Serial.print(distance);
//  if(DEBUG)Serial.print("\n");

  return distance;
}
