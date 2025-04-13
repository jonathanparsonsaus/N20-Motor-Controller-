
#include <Arduino.h>
#include <math.h>

// === Pins ===
const uint8_t MOTOR1_PIN_A = 9;
const uint8_t MOTOR1_PIN_B = 10;
const uint8_t ENC1_A = 2;
const uint8_t ENC1_B = 6;

const uint8_t MOTOR2_PIN_A = 8;
const uint8_t MOTOR2_PIN_B = 11;
const uint8_t ENC2_A = 3;
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

// === Control Settings ===
const int MAX_SPEED = 200;
const int MIN_SPEED = 80;
const int DEAD_ZONE = 10;
const int RAMP_RANGE = 1500;

// === Serial Input ===
char inputBuffer[32];
uint8_t inputIndex = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Motor Toggle + Adjustable High Targets via Serial ===");

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

  attachInterrupt(digitalPinToInterrupt(ENC1_A), isrEnc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), isrEnc2, CHANGE);
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Enc1: "); Serial.print(encoderCount1);
    Serial.print(" | Tgt1: "); Serial.print(target1);
    Serial.print(" | Enc2: "); Serial.print(encoderCount2);
    Serial.print(" | Tgt2: "); Serial.print(target2);
    Serial.print(" | BTN1: "); Serial.print(digitalRead(BTN_M1) == LOW ? "PRESSED" : "released");
    Serial.print(" | BTN2: "); Serial.println(digitalRead(BTN_M2) == LOW ? "PRESSED" : "released");
  }

  // === Button Toggles ===
  bool btn1 = digitalRead(BTN_M1);
  if (btn1 == LOW && lastBtnM1State == HIGH) {
    m1AtHigh = !m1AtHigh;
    target1 = m1AtHigh ? highTarget1 : 0;
    Serial.print("Motor 1 toggled to: "); Serial.println(target1);
  }
  lastBtnM1State = btn1;

  bool btn2 = digitalRead(BTN_M2);
  if (btn2 == LOW && lastBtnM2State == HIGH) {
    m2AtHigh = !m2AtHigh;
    target2 = m2AtHigh ? highTarget2 : 0;
    Serial.print("Motor 2 toggled to: "); Serial.println(target2);
  }
  lastBtnM2State = btn2;

  // === Serial Command Parser ===
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

  // === Motor 1 Control ===
  long error1 = target1 - encoderCount1;
  float ramp1 = constrain(abs(error1) / (float)RAMP_RANGE, 0.0, 1.0);
  int speed1 = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * pow(ramp1, 0.7);
  speed1 = constrain(speed1, MIN_SPEED, MAX_SPEED);

  if (abs(error1) < DEAD_ZONE) {
    analogWrite(MOTOR1_PIN_A, 0);
    analogWrite(MOTOR1_PIN_B, 0);
  } else if (error1 > 0) {
    analogWrite(MOTOR1_PIN_A, speed1);
    analogWrite(MOTOR1_PIN_B, 0);
  } else {
    analogWrite(MOTOR1_PIN_A, 0);
    analogWrite(MOTOR1_PIN_B, speed1);
  }

  // === Motor 2 Control ===
  long error2 = target2 - encoderCount2;
  float ramp2 = constrain(abs(error2) / (float)RAMP_RANGE, 0.0, 1.0);
  int speed2 = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * pow(ramp2, 0.7);
  speed2 = constrain(speed2, MIN_SPEED, MAX_SPEED);

  if (abs(error2) < DEAD_ZONE) {
    analogWrite(MOTOR2_PIN_A, 0);
    analogWrite(MOTOR2_PIN_B, 0);
  } else if (error2 > 0) {
    analogWrite(MOTOR2_PIN_A, speed2);
    analogWrite(MOTOR2_PIN_B, 0);
  } else {
    analogWrite(MOTOR2_PIN_A, 0);
    analogWrite(MOTOR2_PIN_B, speed2);
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
  if (cmd[0] == '\0') return; // ignore blank lines

  if (strncmp(cmd, "SET M1:", 7) == 0) {
    highTarget1 = atol(cmd + 7);
    Serial.print("High Target 1 updated to: "); Serial.println(highTarget1);
    if (m1AtHigh) target1 = highTarget1;
  } else if (strncmp(cmd, "SET M2:", 7) == 0) {
    highTarget2 = atol(cmd + 7);
    Serial.print("High Target 2 updated to: "); Serial.println(highTarget2);
    if (m2AtHigh) target2 = highTarget2;
  } else {
    Serial.print("Unrecognised command: "); Serial.println(cmd);
  }
}

