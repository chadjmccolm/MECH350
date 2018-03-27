/* MECH350_Tray_Positioner_Actuation_Code
 * Version: 1.1.1
 * by Michael Giles
 * Date: March 22, 2018
 * 
 * Pin Assignments:
 * D4   ----->  END STOP
 * D6   ----->  SERVO PWM
 * D8   ----->  DIR
 * D9   ----->  STEP
 * D10  ----->  M0
 * D11  ----->  M1
 * D12  ----->  M2
 * D13  ----->  ENABLE
 *
 * Release Notes:
 * This version fixes a bug where reversing the servo would break 
 * the auto home function. Supported commands can be seen in the 
 * table below.
 * 
 * Command    Parameter    Example    Description
 * A          Integer      A300       Moves the arm to position 300mm relative to the home location
 * T          Integer      T15        Moves the tray to an angle of 15 degrees
 * E          None         E          Enables the stepper motor
 * D          None         D          Disables the stepper motor
 * H          None         H          Moves both the arm and tray to their home positions
 *
 * There is a known bug in this release that restricts motor 
 * RPM. Real world motor speed was measured to be ~ 130RPM. 
 * Motor velocities of up to 900RPM can be achieved by disabling 
 * motor acceleration profiles however this is not recommended 
 * as it increases the chances of motor stall at actuation start.
 *
 * This code was designed for metric actuation! While actiation
 * can be done using the imperial system, it should be noted that
 * a small amount of error will accumulate over time. While an
 * imperial lead screw can be used to actuate the arm, it is
 * recommended that a lead screw with metric pitch is used to
 * minimize error accumulation.
 *
 * WARNING:
 * The servo must be powered from an external source providing 
 * 5v DC. Powering the servo from the power rail on the Arduino 
 * may cause a brown out or other failure.
 */
 
#include <Arduino.h>        // Include Arduino.h library
#include <Servo.h>          // Include the servo library
#include "DRV8825.h"        // Include the library for our specific stepper driver

// Mechanical Settings:
#define MOTOR_STEPS 200     // Motor steps per revolution. Our NEMA17 is 1.8 degrees/step = 200 steps/revolution
#define MOTOR_ACCEL 1000    // Motor acceleration parameter (step/s^2)
#define MOTOR_DECEL 1000    // Motor deceleration parameter (step/s^2)
#define MICROSTEP 16        // Microstepping mode. We will use 1/16 microstepping
#define RPM 500             // Constant RPM to run the stepper
#define TILT_SPEED 85       // Tray Tilt speed (0 - 100%)
#define HOME_SPEED 5        // Arm Homing speed (mm/s)
#define PITCH 8             // Lead screw pitch in mm. This is the amount of linear travel per revolution of the lead screw
#define MAX_TRAVEL 440      // Maximum travel of the arm (mm)
#define MAX_ANGLE 30        // Maximum angle of the tray (degrees)
#define MSA_ANGLE 55        // Maximum servo arm angle (degrees)
#define SERVO_TRIM -8       // Servo Trim on servo end (-180 -> 180 degrees)
#define SERVO_REV 1         // Set to 1 to reverse servo direction (Not Implemented)

// Electrical Settings:
#define STOPPER_PIN 4       // Pin 4 corresponds to the end stop 
#define SERVO_PIN 6         // Pin 6 corresponds to the servo PWM control pin
#define DIR 8               // Pin 8 corresponds to the direction pin on the stepper driver
#define STEP 9              // Pin 9 corresponds to the step pin on the stepper driver
#define MODE0 10            // Pin 10 corresponds to the microstepping mode 0 pin on the stepper driver
#define MODE1 11            // Pin 11 corresponds to the microstepping mode 1 pin on the stepper driver
#define MODE2 12            // Pin 12 corresponds to the microstepping mode 2 pin on the stepper driver
#define ENABLE 13           // Pin 13 corresponds to the enable pin on the stepper driver

// Actuator Initializations:
Servo servo_1;                                                               // Configures the servo object to control servo_1
DRV8825 stepper(MOTOR_STEPS, DIR, STEP, ENABLE, MODE0, MODE1, MODE2);        // Configures the settings for our stepper driver

// Operational Data:
int Tray_Position = 0;                                                       // Variable to keep track of tray position
int Arm_Position = 0;                                                        // Variable to keep track of arm position
String User_Input;                                                           // Declare location for user input
bool Is_Homing;                                                              // Boolean to enable homing mode
const int Tilt_Speed_Delay = 100 - TILT_SPEED;                               // Parameter that controls the servo speed

// ------------------------------- FUNCTIONS -------------------------------

// Arm Actuation Function:
void Move_Arm_Position(int New_Position) {
  long Delta_Position = New_Position - Arm_Position;                         // Delta_Position is the difference between the target position and the current position
  long Delta_Rotation = (Delta_Position * MOTOR_STEPS * MICROSTEP);          // Delta_Rotation is the change of position over the pitch of the lead screw times 200steps/revolution
  Delta_Rotation = (Delta_Rotation/PITCH);
  Arm_Position = New_Position;                                               // Set the new position to the target position.
  stepper.move(Delta_Rotation);                                              // Rotate the stepper to move the arm to the target position
}

// Tray Actuation Function:
void Move_Tray_Position(int New_Position) {
  
  int New_Rotation = (New_Position * MSA_ANGLE);                             // Convert desired table movement to servo movement
  New_Rotation = (New_Rotation/MAX_ANGLE) + SERVO_TRIM;

  if(SERVO_REV == 1) {                                                       // Reverse servo direction based on initial mechanical settings
    New_Rotation = MSA_ANGLE - New_Rotation;
  }

  if(New_Rotation >= Tray_Position) {
    for (int pos = Tray_Position; pos <= New_Rotation; pos++) {              // Servo speed control
    servo_1.write(pos);
    delay(Tilt_Speed_Delay);
    }
  }else{
    for (int pos = Tray_Position; pos >= New_Rotation; pos--) {              // Servo speed control in opposite direction
    servo_1.write(pos);
    delay(Tilt_Speed_Delay);
    }
  }
  Tray_Position = New_Rotation;                                              // Set the tilt position to the new position.
}

// Arm Function:
void Arm_Movement(String Value) {
  int Target_Position = Value.toInt();                                       // Convert user input to a target position
  
  if(0 <= Target_Position && Target_Position <= MAX_TRAVEL) {
    Serial.print("Moving Arm to Position: ");                                // Command feedback to operator
    Serial.print(Target_Position);
    Serial.println("mm");
    Move_Arm_Position(Target_Position);                                      // Call Move_Arm_Position function 
  }
  else
  {   
    Serial.print("Invalid Arm Position: ");                                  // If target position is outside the range of motion complain to the operator
    Serial.println(Target_Position);
  }
}

// Tray Function:
void Tray_Movement(String Value) {
  int Target_Position = Value.toInt();                                       // Convert user input to a target position
  
  if(0 <= Target_Position && Target_Position <= MAX_ANGLE) {
    Serial.print("Moving Tray to Position: ");                               // Command feedback to operator
    Serial.print(Target_Position);
    Serial.println("°");
    Move_Tray_Position(Target_Position);                                     // Call Move_Tray_Position function 
  }
  else
  {   
    Serial.print("Invalid Tray Position: ");                                 // If target position is outside the range of motion complain to the operator
    Serial.println(Target_Position);
  }
}

// Homing Function
void Home() {
    Serial.println("Moving Arm to home position...");
    Is_Homing = true;                                                        // Enable the homing function in the loop
}

// Manual Step Function
void Manual_Step(bool Direction) {                                           // Manually move the motor one step in the specified direction
    if(Direction) {
      digitalWrite(DIR,HIGH);
    }
    else
    {
      digitalWrite(DIR,LOW);
    }
    digitalWrite(STEP,HIGH); 
    delayMicroseconds((500000*PITCH)/(HOME_SPEED*MOTOR_STEPS*MICROSTEP)); 
    digitalWrite(STEP,LOW); 
    delayMicroseconds((500000*PITCH)/(HOME_SPEED*MOTOR_STEPS*MICROSTEP));
}

// ------------------------------- Program Core -------------------------------

void setup() {
  
    pinMode(STOPPER_PIN, INPUT_PULLUP);                                      // Set the end stop to the end stop pin
    servo_1.attach(SERVO_PIN);                                               // Set the servo to the servo pin
    servo_1.write(0);                                                        // Set home tilt angle to position 0
    stepper.begin(RPM);                                                      // Set the desired stepper RPM
    stepper.setMicrostep(MICROSTEP);                                         // Set microstep mode
    stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL); // Set motor acceleration profile
    stepper.enable();                                                        // Enable the stepper motor
    
    Serial.begin(9600);                                                      // Start serial communication with computer at 9600 baud
    while (! Serial);                                                        // Wait until Serial is ready (We shouldn't have to do this but it's a good failsafe)
    Serial.println("MECH350 System Controller V1.1.1 by Michael Giles");     // Program start message
    Serial.println("--> This version is fully tested and stable");           // Version message
    
    delay(3000);                                                             // Small delay at startup to ensure the stepper is ready to go
    
    Home();                                                                  // Home axes at system start
}

void loop() {
  
  if(Is_Homing == true) {                                                    // Homing function in loop
    if (digitalRead(STOPPER_PIN) == HIGH) {                                  // Verify arm is not at home location
      Manual_Step(false);                                                    // Manually step motor back one step
    }
    if (digitalRead(STOPPER_PIN) == LOW) {                                   // If end stop is triggered, finish homing process
      Serial.println("Moving Tray to home position...");
//      Move_Tray_Position(MAX_ANGLE);                                         // This is really just for asthetics cause you don't need to zero a servo lol
//      Move_Tray_Position(0);                                                 // Send servo to home location
      Arm_Position = 0;                                                      // Ensure arm position is set to zero
      Serial.println("Both axes homed successfully");
      Is_Homing = false;                                                     // Exit homing mode
    }
  }

  while (Serial.available()) {
    char Input = Serial.read();                                              // Read one byte from serial buffer
    User_Input += Input;                                                     // Add byte to current request
    delay(2);                                                                // Delay looping to allow buffer to fill with next character
  }

  if (User_Input.length() > 0) {
    Serial.println(User_Input);                                              // Repeat what the operator requested (confirmation)
    
    if (User_Input.charAt(0) == 'A') {                                       // Identify whether it is an arm command
      User_Input.remove(0, 1);
      Arm_Movement(User_Input);
    }
    
    if (User_Input.charAt(0) == 'T') {                                       // Identify whether it is a tray command
      User_Input.remove(0, 1);
      Tray_Movement(User_Input);
    }
    
    if (User_Input.charAt(0) == 'E') {                                       // Enable the stepper driver
      stepper.enable();
      Serial.println("Stepper Enabled");
    }
    
    if (User_Input.charAt(0) == 'D') {                                       // Disable the stepper driver
      stepper.disable();
      Serial.println("Stepper Disabled");
    }
    
    if (User_Input.charAt(0) == 'H') {                                       // Home axes
      Home();
    }
    
    User_Input = "";                                                         // Reset for next input
  } 
 
}
