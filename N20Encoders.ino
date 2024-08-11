//This motor control functionality allows for encoders of two N20 motors to be read by executing the P command 
// Motor Speed can also be set through the following command sets MA255 MB255 or MB-255 or MA-255 and any range between


#include "n20encoder.h"

// Define motor control pins based on your setup
#define MOTOR_A_DIR 3    // Direction pin for Motor A (AIN1)
#define MOTOR_A_PWM 9    // PWM pin for Motor A (AIN2)
#define MOTOR_B_DIR 5    // Direction pin for Motor B (BIN1)
#define MOTOR_B_PWM 6    // PWM pin for Motor B (BIN2)
#define MOTOR_ENABLE 10  // Enable pin for both motors

// Global variables for PID control
int desiredSpeedA = 0;  // Desired speed for Motor A in encoder ticks per second
int desiredSpeedB = 0;  // Desired speed for Motor B in encoder ticks per second
int currentSpeedA = 0;  // Current speed of Motor A in encoder ticks per second
int currentSpeedB = 0;  // Current speed of Motor B in encoder ticks per second
int lastEncoderCountA = 0;  // Last encoder count for Motor A
int lastEncoderCountB = 0;  // Last encoder count for Motor B
unsigned long lastUpdateTime = 0;  // Last time the speed was updated

// PID control variables
float kp = 0.05;  // Proportional gain
float ki = 0.005;  // Integral gain
float kd = 0.00001; // Derivative gain

int integralA = 0;  // Integral term for Motor A
int integralB = 0;  // Integral term for Motor B
int lastErrorA = 0;  // Last error for Motor A
int lastErrorB = 0;  // Last error for Motor B

void setup() {
    // Initialize Serial Communication at 57600 baud for monitoring
    Serial.begin(57600);

    // Print a welcome message to the Serial Monitor
    Serial.println("*************************************************");
    Serial.println("*       Welcome to Motor Controller             *");
    Serial.println("*      N20 Motor Encoder Interface              *");
    Serial.println("*                                               *");
    Serial.println("*************************************************");
    Serial.println("This rogram allows you to:                      *");
    Serial.println("*1. Set open loop motor speed                   *");
    Serial.println("*2. Set the closed loop motor speed (ticks/s)   *");
    Serial.println("*3. Set the desired position in ticks           *");
    Serial.println("*3. Set the desired position in ticks           *");
    Serial.println("*************************************************");

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

    // Initialize the last update time
    lastUpdateTime = millis();
}

void loop() {
    // Handle serial commands from the user
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();  // Convert command to uppercase for case insensitivity

        if (command.startsWith("SET SPEED A ")) {
            int speed = command.substring(12).toInt();
            controlMotor(MOTOR_A_DIR, MOTOR_A_PWM, speed);
            Serial.println("Motor A speed set to " + String(speed));
        } else if (command.startsWith("SET SPEED B ")) {
            int speed = command.substring(12).toInt();
            controlMotor(MOTOR_B_DIR, MOTOR_B_PWM, speed);
            Serial.println("Motor B speed set to " + String(speed));
        } else if (command == "READ ENCODERS") {
            printEncoderValues();  // Print encoder counts
        } else if (command.startsWith("SET TARGET SPEED A ")) {
            desiredSpeedA = command.substring(18).toInt();
            Serial.println("Target speed for Motor A set to " + String(desiredSpeedA) + " ticks/sec");
        } else if (command.startsWith("SET TARGET SPEED B ")) {
            desiredSpeedB = command.substring(18).toInt();
            Serial.println("Target speed for Motor B set to " + String(desiredSpeedB) + " ticks/sec");
        } else if (command == "HELP") {
            printHelp();
        } else {
            Serial.println("Invalid command. Type 'HELP' for a list of available commands.");
        }
    }


    // Update motor speeds based on encoder feedback
    updateMotorSpeeds();


}

// Function to control the motor based on speed
void controlMotor(int dirPin, int pwmPin, int speed) {
    if (speed > 0) {
        digitalWrite(dirPin, LOW);   // Set motor direction forward
        analogWrite(pwmPin, speed);   // Set motor speed (0-255)
    } else if (speed < 0) {
        digitalWrite(dirPin, HIGH);    // Set motor direction backward
        analogWrite(pwmPin,255+speed);  // Set motor speed (-speed to invert PWM)
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

// Function to update motor speeds based on desired speed and encoder feedback


void updateMotorSpeeds() {
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;

    if (deltaTime >= 50) {  // Update every 10ms (10Hz)
        // Calculate current speeds in ticks per second
        currentSpeedA = (encoderCountA - lastEncoderCountA) * (1000.0 / deltaTime);
        currentSpeedB = (encoderCountB - lastEncoderCountB) * (1000.0 / deltaTime);

        // Calculate the speed errors
        int errorA = desiredSpeedA - currentSpeedA;
        int errorB = desiredSpeedB - currentSpeedB;

        // Update integral term
        integralA += errorA * deltaTime;
        integralB += errorB * deltaTime;

        // Calculate derivative term
        int derivativeA = (errorA - lastErrorA) / deltaTime;
        int derivativeB = (errorB - lastErrorB) / deltaTime;

        // PID control output
        int motorSpeedA = constrain((kp * desiredSpeedA) + (0 * integralA) + (0 * derivativeA), -255, 255);
        int motorSpeedB = constrain((kp * desiredSpeedB) + (0 * integralB) + (0 * derivativeB), -255, 255);

        // Apply the new PWM values to the motors
        controlMotor(MOTOR_A_DIR, MOTOR_A_PWM, motorSpeedA);
        controlMotor(MOTOR_B_DIR, MOTOR_B_PWM, motorSpeedB);

        //Print Motor speeds as current
        Serial.println("Motor A speed is " + String(currentSpeedA) + " Motor B speed is " + String(currentSpeedB) + " Motor Control Signal A is" + String(motorSpeedA) + " Motor Control Signal B is" + String(motorSpeedB));
      

        // Update last error and last encoder counts
        lastErrorA = errorA;
        lastErrorB = errorB;
        lastEncoderCountA = encoderCountA;
        lastEncoderCountB = encoderCountB;
        lastUpdateTime = currentTime;
    }
}


// Function to print a list of available commands
void printHelp() {
    Serial.println("Available Commands:");
    Serial.println("SET SPEED A <value>      - Set speed for Motor A (value between -255 to 255)");
    Serial.println("SET SPEED B <value>      - Set speed for Motor B (value between -255 to 255)");
    Serial.println("SET TARGET SPEED A <ticks/sec>  - Set target speed for Motor A in encoder ticks per second");
    Serial.println("SET TARGET SPEED B <ticks/sec>  - Set target speed for Motor B in encoder ticks per second");
    Serial.println("READ ENCODERS            - Read and display the encoder values for both motors");
    Serial.println("HELP                     - Display this help message");
}
