#include "n20encoder.h"

// Global variables to store encoder counts
volatile int16_t encoderCountA = 0;  // Encoder A count
volatile int16_t encoderCountB = 0;  // Encoder B count

// Variables to track the previous state of each encoder channel
volatile int lastStateA0 = LOW;  // Previous state of Encoder A Channel A (D2)
volatile int lastStateA1 = LOW;  // Previous state of Encoder A Channel B (D4)
volatile int lastStateB0 = LOW;  // Previous state of Encoder B Channel A (A0)
volatile int lastStateB1 = LOW;  // Previous state of Encoder B Channel B (A1)

void initEncoders() {
    // Set Encoder A pins as input
    pinMode(2, INPUT); // Encoder A Channel A (D2)
    pinMode(4, INPUT); // Encoder A Channel B (D4)
    
    // Set Encoder B pins as input
    pinMode(A0, INPUT); // Encoder B Channel A (A0)
    pinMode(A1, INPUT); // Encoder B Channel B (A1)

    // Enable pin change interrupts for PORTD (D2 and D4) to handle Encoder A
    PCICR |= (1 << PCIE2);    // Enable Pin Change Interrupts for PCINT[23:16] (PORTD group)
    PCMSK2 |= (1 << PCINT18); // Enable pin change interrupt for D2 (PCINT18)
    PCMSK2 |= (1 << PCINT20); // Enable pin change interrupt for D4 (PCINT20)

    // Enable pin change interrupts for PORTA (A0 and A1) to handle Encoder B
    PCICR |= (1 << PCIE1);    // Enable Pin Change Interrupts for PCINT[14:8] (PORTA group)
    PCMSK1 |= (1 << PCINT8);  // Enable pin change interrupt for A0 (PCINT8)
    PCMSK1 |= (1 << PCINT9);  // Enable pin change interrupt for A1 (PCINT9)

    // Initialize the last known states of the encoder channels
    lastStateA0 = digitalRead(2);
    lastStateA1 = digitalRead(4);
    lastStateB0 = digitalRead(A0);
    lastStateB1 = digitalRead(A1);
}

// Interrupt Service Routine (ISR) for Encoder A
ISR(PCINT2_vect) {
    encoderAISR();  // Call the function to handle Encoder A state changes
}

// Interrupt Service Routine (ISR) for Encoder B
ISR(PCINT1_vect) {
    encoderBISR();  // Call the function to handle Encoder B state changes
}

// Function to handle state changes for Encoder A and update the count
void encoderAISR() {
    int currentStateA0 = digitalRead(2);  // Read current state of Encoder A Channel A (D2)
    int currentStateA1 = digitalRead(4);  // Read current state of Encoder A Channel B (D4)

    // Determine direction based on the relative states of Channel A and Channel B
    if (lastStateA0 == currentStateA1) {
        encoderCountA++; // Moving forward
    } else {
        encoderCountA--; // Moving backward
    }

    // Update the last known states for Encoder A
    lastStateA0 = currentStateA0;
    lastStateA1 = currentStateA1;
}

// Function to handle state changes for Encoder B and update the count
void encoderBISR() {
    int currentStateB0 = digitalRead(A0);  // Read current state of Encoder B Channel A (A0)
    int currentStateB1 = digitalRead(A1);  // Read current state of Encoder B Channel B (A1)

    // Determine direction based on the relative states of Channel A and Channel B
    if (lastStateB0 == currentStateB1) {
        encoderCountB--; // Moving forward
    } else {
        encoderCountB++; // Moving backward
    }

    // Update the last known states for Encoder B
    lastStateB0 = currentStateB0;
    lastStateB1 = currentStateB1;
}
