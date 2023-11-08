#include <Servo.h>
#include "IRremote.h" 

// Define pins for ultrasonic sensor
#define TRIG_PIN 3
#define ECHO_PIN 2
// Define pin for IR receiver
IRrecv irrecv(A5); 
decode_results results;

// Define pins for L298N motor driver
#define ENA 5
#define IN1 6
#define IN2 7
#define IN3 8
#define IN4 9
#define ENB 10

// Define pins for Servo motor
#define SERVO_PIN 11

// Define pins for KY-033 sensors
#define SENSOR_A0 A0
#define SENSOR_A1 A1
#define SENSOR_A2 A2

Servo myservo;  // create servo object to control a servo
// Instantiate the IR receiver and servo objects


void setup() {
  irrecv.enableIRIn(); 
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 11 to the servo object
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  pinMode(SENSOR_A0, INPUT);
  pinMode(SENSOR_A1, INPUT);
  pinMode(SENSOR_A2, INPUT);

  Serial.begin(9600);
}
bool AutoPilot = false;

void loop() {
  if(irrecv.decode(&results)) {
    handleIRCommand(results.value); 
    irrecv.resume(); 
  }
  
  if(AutoPilot){
    if(!isOnGround()) {
      scanForObstacles();
    } else {
      Serial.println("Not on ground, stopping.");
      stopMoving();
    }
  }
}

void scanForObstacles() {
  int angles[] = {75, 90, 115};
  int numAngles = sizeof(angles)/sizeof(angles[0]);
  for(int i = 0; i < numAngles; i++) {
    myservo.write(angles[i]);
    delay(100); 
    long distance = measureDistance();
  
    if(distance <= 20) {
      Serial.println("Obstacle detected!");
      stopMoving();
      findPath();
      return; 
    }
  }
  moveForward();
}



bool isOnGround() {
  int sensorValA0 = analogRead(SENSOR_A0);
  int sensorValA1 = analogRead(SENSOR_A1);
  int sensorValA2 = analogRead(SENSOR_A2);
  // Assuming 100 is the threshold value below which sensor detects no ground
  return sensorValA0 > 100 && sensorValA1 > 100 && sensorValA2 > 100;
}

void stopMoving() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  
  digitalWrite(IN1, HIGH);  
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);   
  digitalWrite(IN4, HIGH);
}

long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration / 2) * 0.0343;
  return distance;
}

void findPath() {
  bool pathFound = false;
  for(int angle = 0; angle <= 180 && !pathFound; angle += 30) {
    myservo.write(angle);
    delay(500); // Wait for servo to reach position and for readings to stabilize
    long distance = measureDistance();
    if(distance > 20) {
      pathFound = true;
      Serial.print("Path found at angle: ");
      Serial.println(angle);
      // Depending on your robot's orientation logic, rotate to pathFound angle
      rotateRobotTo(angle); // Implement this function as needed.
    }
  }
  
  if(!pathFound) {
    Serial.println("No path found, rotating to original position.");
    myservo.write(90); // Or rotate to an angle suitable for your robot's orientation
    rotateRobotTo(90); // Implement this function as needed.
  }
}

void rotateRobotTo(int angle) {

  
  // For demonstration purposes, let's assume the robot turns by spinning IN1 and IN4
  if(angle < 90) {
    // Rotate left
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  } else if(angle > 90) {
    // Rotate right
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  
  delay(340);
  
  stopMoving();
  
  myservo.write(90);
}

void handleIRCommand(unsigned long command) {
  switch (command) {
    case 0xFF18E7: 
      moveForward();
      break;
    case 0xFF4AB5: 
      moveBackward();
      break;
    case  0xFF5AA5: 
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 0xFF10EF: 
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
    case 0xFF38C7: 
      stopMoving();
      break;
    case 0xFF9867: 
      AutoPilot = !AutoPilot;
      Serial.print("AutoPilot mode is now: ");
      Serial.println(AutoPilot ? "ON" : "OFF");
      delay(500);
      break;
  }
}

// void AutoPilot()
// {
//  if(!isOnGround()) {
//     long distance = measureDistance();
//     Serial.print("Distance: ");
//     Serial.println(distance);

//     if(distance > 15) {
//       Serial.println("No obstacle, moving forward.");
//       moveForward();
//     } else {
//       Serial.println("Obstacle detected, searching for path.");
//       stopMoving();
//       findPath();
//     }
//   } else {
//     Serial.println("Not on ground, stopping.");
//     stopMoving();
//   }

// }

void moveBackward() {
  // Reverse the logic of moveForward() to move backward
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}



