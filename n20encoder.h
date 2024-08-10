#ifndef N20ENCODER_H
#define N20ENCODER_H

#include <Arduino.h>
#include <avr/interrupt.h>

// External global variables for encoder counts
extern volatile int16_t encoderCountA;  // Encoder A count
extern volatile int16_t encoderCountB;  // Encoder B count

// Function prototypes
void initEncoders();
void encoderAISR();
void encoderBISR();

#endif // N20ENCODER_H