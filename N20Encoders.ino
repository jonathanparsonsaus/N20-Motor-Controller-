//This motor control functionality allows for encoders of two N20 motors to be read by executing the P command 
// Motor Speed can also be set through the following command sets MA255 MB255 or MB-255 or MA-255 and any range between
//

#include "n20encoder.h"

// Define motor control pins based on your setup
#define MOTOR_A_DIR 3    // Direction pin for Motor A (AIN1)
#define MOTOR_A_PWM 9    // PWM pin for Motor A (AIN2)
#define MOTOR_B_DIR 5    // Direction pin for Motor B (BIN1)
#define MOTOR_B_PWM 6    // PWM pin for Motor B (BIN2)
#define MOTOR_ENABLE 10  // Enable pin for both motors

void setup() {
    // Initialize Serial Communication at 57600 baud for monitoring
    Serial.begin(57600);

    // Print a welcome message to the Serial Monitor
    Serial.println("****************************************");
    Serial.println("*       Welcome to Motor Controller    *");
    Serial.println("*      N20 Motor Encoder Interface     *");
    Serial.println("*                                      *");
    Serial.println("****************************************");

    // Initialize motor control pins as outputs
    pinMode(MOTOR_A_DIR, OUTPUT);
    pinMode(MOTOR_A_PWM, OUTPUT);
    pinMode(MOTOR_B_DIR, OUTPUT);
    pinMode(MOTOR_B_PWM, OUTPUT);
    pinMode(MOTOR_ENABLE, OUTPUT);

    // Enable the motors
    digitalWrite(MOTOR_ENABLE, HIGH);

    // Initialize the encoders using the n20encoder library
    initEncoders();
}

void loop() {
    // Handle serial commands from the user
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        // Handle motor speed commands
        if (command.startsWith("MA")) {
            int speed = command.substring(2).toInt();
            controlMotor(MOTOR_A_DIR, MOTOR_A_PWM, speed);
            Serial.println("OK");
        } else if (command.startsWith("MB")) {
            int speed = command.substring(2).toInt();
            controlMotor(MOTOR_B_DIR, MOTOR_B_PWM, speed);
            Serial.println("OK");
        } else if (command == "P") {
            printEncoderValues();  // Print encoder counts when 'P' command is received
            Serial.println("OK");
        } else {
            Serial.println("Invalid command");
        }
    }
}

// Function to control the motor based on speed
void controlMotor(int dirPin, int pwmPin, int speed) {
    if (speed > 0) {
        digitalWrite(dirPin, HIGH);   // Set motor direction forward
        analogWrite(pwmPin, speed);   // Set motor speed (0-255)
    } else if (speed < 0) {
        digitalWrite(dirPin, LOW);    // Set motor direction backward
        analogWrite(pwmPin, -speed);  // Set motor speed (-speed to invert PWM)
    } else {
        analogWrite(pwmPin, 0);       // Stop the motor if speed is 0
    }
}

// Function to print the current encoder counts to the Serial Monitor
void printEncoderValues() {
    Serial.println("Motor Encoder Values:");
    Serial.print("Encoder A: ");
    Serial.println(encoderCountA);  // Use the external global variable from n20encoder.cpp to get encoder A count
    Serial.print("Encoder B: ");
    Serial.println(encoderCountB);  // Use the external global variable from n20encoder.cpp to get encoder B count
}
