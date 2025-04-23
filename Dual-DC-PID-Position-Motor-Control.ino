#include <Arduino.h>

// === Pins ===
const uint8_t MOTOR1_PIN_A = 9;  // Digital Pin
const uint8_t MOTOR1_PIN_B = 10; // PWM (Timer1)
const uint8_t ENC1_A = 2;        // Interrupt pin
const uint8_t ENC1_B = 6;

const uint8_t MOTOR2_PIN_A = 8;  // Digital Pin 
const uint8_t MOTOR2_PIN_B = 11; // PWM (Timer2)
const uint8_t ENC2_A = 3;        // Interrupt pin
const uint8_t ENC2_B = 7;

const uint8_t BTN_M1 = 12;
const uint8_t BTN_M2 = 13;

// === Motor States ===
volatile long encoderCount1 = 0;
volatile long encoderCount2 = 0;

long target1 = 0;
long target2 = 0;

long highTarget1 = 5000;
long highTarget2 = 5000;

bool m1AtHigh = false;
bool m2AtHigh = false;

bool lastBtnM1State = HIGH;
bool lastBtnM2State = HIGH;

// === PID Parameters ===
struct PID {
  float Kp = 2.0;    // Proportional gain
  float Ki = 0.05;   // Integral gain
  float Kd = 0.1;    // Derivative gain
  long lastError = 0;
  float integral = 0;
  unsigned long lastTime = 0;
};

PID pid1, pid2;

const int MAX_PWM = 255;    // 8-bit PWM limit
const int DEAD_ZONE = 10;   // Error dead zone (encoder counts)
const float MAX_INTEGRAL = 1000.0; // Anti-windup limit
const unsigned long PID_INTERVAL = 10; // PID update interval (ms)

// === Serial Input ===
char inputBuffer[32];
uint8_t inputIndex = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("=== PID Motor Control with Adjustable Targets ===");

  // Configure pins
  pinMode(MOTOR1_PIN_A, OUTPUT);
  pinMode(MOTOR1_PIN_B, OUTPUT);
  pinMode(MOTOR2_PIN_A, OUTPUT);
  pinMode(MOTOR2_PIN_B, OUTPUT);

  pinMode(ENC1_A, INPUT);
  pinMode(ENC1_B, INPUT);
  pinMode(ENC2_A, INPUT);
  pinMode(ENC2_B, INPUT);
  pinMode(BTN_M1, INPUT_PULLUP);
  pinMode(BTN_M2, INPUT_PULLUP);

  // Attach interrupts for encoders
  attachInterrupt(digitalPinToInterrupt(ENC1_A), isrEnc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), isrEnc2, CHANGE);

  // Initialize PWM to 0
  digitalWrite(MOTOR1_PIN_A, 0);
  analogWrite(MOTOR1_PIN_B, 0);
  digitalWrite(MOTOR2_PIN_A, 0);
  analogWrite(MOTOR2_PIN_B, 0);
}

void loop() {
  static unsigned long lastPrint = 0;
  static unsigned long lastPidUpdate = 0;

  // Periodic status print
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();
    Serial.print("Enc1: "); Serial.print(encoderCount1);
    Serial.print(" | Tgt1: "); Serial.print(target1);
    Serial.print(" | Enc2: "); Serial.print(encoderCount2);
    Serial.print(" | Tgt2: "); Serial.print(target2);
    Serial.print(" | BTN1: "); Serial.print(digitalRead(BTN_M1) == LOW ? "PRESSED" : "released");
    Serial.print(" | BTN2: "); Serial.println(digitalRead(BTN_M2) == LOW ? "PRESSED" : "released");
  }

  // Button toggles
  bool btn1 = digitalRead(BTN_M1);
  if (btn1 == LOW && lastBtnM1State == HIGH) {
    m1AtHigh = !m1AtHigh;
    target1 = m1AtHigh ? highTarget1 : 0;
    pid1.integral = 0; // Reset integral on target change
    Serial.print("Motor 1 toggled to: "); Serial.println(target1);
  }
  lastBtnM1State = btn1;

  bool btn2 = digitalRead(BTN_M2);
  if (btn2 == LOW && lastBtnM2State == HIGH) {
    m2AtHigh = !m2AtHigh;
    target2 = m2AtHigh ? highTarget2 : 0;
    pid2.integral = 0; // Reset integral on target change
    Serial.print("Motor 2 toggled to: "); Serial.println(target2);
  }
  lastBtnM2State = btn2;

  // Serial command parser
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer[inputIndex] = '\0';
      parseSerialCommand(inputBuffer);
      inputIndex = 0;
    } else if (inputIndex < sizeof(inputBuffer) - 1) {
      inputBuffer[inputIndex++] = c;
    }
  }

  // PID control update
  if (millis() - lastPidUpdate >= PID_INTERVAL) {
    lastPidUpdate = millis();

    // Motor 1 PID
    long error1 = target1 - encoderCount1;
    if (abs(error1) < DEAD_ZONE) {
      analogWrite(MOTOR1_PIN_A, 0);
      analogWrite(MOTOR1_PIN_B, 0);
      pid1.integral = 0; // Reset integral in dead zone
    } else {
      unsigned long now = millis();
      float dt = (now - pid1.lastTime) / 1000.0; // Time delta in seconds
      if (dt > 0) { // Avoid division by zero
        float derivative = (error1 - pid1.lastError) / dt;
        pid1.integral += error1 * dt;
        pid1.integral = constrain(pid1.integral, -MAX_INTEGRAL, MAX_INTEGRAL); // Anti-windup

        float output = pid1.Kp * error1 + pid1.Ki * pid1.integral + pid1.Kd * derivative;
        int pwm = constrain(abs(output), 0, MAX_PWM);

        if (output > 0) {
          digitalWrite(MOTOR1_PIN_A, 1);
          analogWrite(MOTOR1_PIN_B, 255 - pwm);
        } else {
          digitalWrite(MOTOR1_PIN_A, 0);
          analogWrite(MOTOR1_PIN_B, pwm);
        }
      }
      pid1.lastError = error1;
      pid1.lastTime = now;
    }

    // Motor 2 PID
    long error2 = target2 - encoderCount2;
    if (abs(error2) < DEAD_ZONE) {
      analogWrite(MOTOR2_PIN_A, 0);
      analogWrite(MOTOR2_PIN_B, 0);
      pid2.integral = 0; // Reset integral in dead zone
    } else {
      unsigned long now = millis();
      float dt = (now - pid2.lastTime) / 1000.0; // Time delta in seconds
      if (dt > 0) { // Avoid division by zero
        float derivative = (error2 - pid2.lastError) / dt;
        pid2.integral += error2 * dt;
        pid2.integral = constrain(pid2.integral, -MAX_INTEGRAL, MAX_INTEGRAL); // Anti-windup

        float output = pid2.Kp * error2 + pid2.Ki * pid2.integral + pid2.Kd * derivative;
        int pwm = constrain(abs(output), 0, MAX_PWM);

        if (output > 0) {
          digitalWrite(MOTOR2_PIN_A, 1);
          analogWrite(MOTOR2_PIN_B, 255 - pwm);
        } else {
          digitalWrite(MOTOR2_PIN_A, 0);
          analogWrite(MOTOR2_PIN_B, pwm);
        }
      }
      pid2.lastError = error2;
      pid2.lastTime = now;
    }
  }
}

// === ISR: Encoder Handling ===
void isrEnc1() {
  bool A = digitalRead(ENC1_A);
  bool B = digitalRead(ENC1_B);
  encoderCount1 += (A == B) ? -1 : +1;
}

void isrEnc2() {
  bool A = digitalRead(ENC2_A);
  bool B = digitalRead(ENC2_B);
  encoderCount2 += (A == B) ? +1 : -1;
}

// === Parse Serial Command ===
void parseSerialCommand(const char* cmd) {
  if (cmd[0] == '\0') return; // Ignore blank lines

  if (strncmp(cmd, "SET M1:", 7) == 0) {
    highTarget1 = atol(cmd + 7);
    Serial.print("High Target 1 updated to: "); Serial.println(highTarget1);
    if (m1AtHigh) {
      target1 = highTarget1;
      pid1.integral = 0; // Reset integral
    }
  } else if (strncmp(cmd, "SET M2:", 7) == 0) {
    highTarget2 = atol(cmd + 7);
    Serial.print("High Target 2 updated to: "); Serial.println(highTarget2);
    if (m2AtHigh) {
      target2 = highTarget2;
      pid2.integral = 0; // Reset integral
    }
  } else {
    Serial.print("Unrecognised command: "); Serial.println(cmd);
  }
}
