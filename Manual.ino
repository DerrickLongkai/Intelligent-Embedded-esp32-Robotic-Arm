#include <Arduino.h>

// ================= 1. Hardware Pin Allocation =================
// Assigning ESP32 GPIO pins to each servo.
const int SERVO_BASE = 18;      // Base rotation (B)
const int SERVO_SHOULDER = 27;  // Primary vertical lift / Shoulder (S)
const int SERVO_ELBOW = 23;     // Secondary forward extension / Elbow (E)
const int SERVO_CLAW = 13;      // End effector / Claw (C)

// Standard Servo specifications: 50Hz frequency (20ms period)
const int PWM_FREQ = 50;
// Using 16-bit resolution (0-65535) for ultra-precise angle control
const int PWM_RESOLUTION = 16;

// ================= 2. Current State Tracking =================
// We use 'float' instead of 'int' here. This is crucial for the smooth 
// interpolation algorithm, allowing the joints to take tiny fractional steps.
float currentB = 65.0;  // Base centered
float currentS = 90.0;  // Shoulder neutral
float currentE = 80.0;  // Elbow tucked
float currentC = 45.0;  // Claw safely opened

// ================= 3. Low-Level Hardware Drivers =================
/**
 * Converts a target angle (0-180) into an ESP32 16-bit PWM duty cycle.
 * For a standard 50Hz servo:
 * - 0 degrees   ~ 0.5ms pulse ~ 2.5% duty cycle  -> ~1638 (out of 65535)
 * - 180 degrees ~ 2.5ms pulse ~ 12.5% duty cycle -> ~8192 (out of 65535)
 */
uint32_t angleToDuty(float angle) {
    // Hardware protection limit to prevent stripping gears
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    
    float minDuty = 1638.0f;    
    float maxDuty = 8192.0f;    
    return (uint32_t)(minDuty + (angle / 180.0f) * (maxDuty - minDuty));
}

// Wrapper function to write the mapped duty cycle to the GPIO pin
void setServoAngle(int pin, float angle) {
    ledcWrite(pin, angleToDuty(angle));
}

// ================= 4. Core Algorithm: Single-Axis Smooth Motion =================
/**
 * Moves a SINGLE joint smoothly from its current position to the target.
 * Used for manual debugging to prevent the arm from jerking when receiving commands.
 * 
 * @param pin The servo pin to move
 * @param currentAngle Reference to the state variable being updated
 * @param targetAngle The desired end angle
 * @param steps Resolution of the movement (higher = smoother)
 * @param waitTime Delay between steps in ms (higher = slower movement)
 */
void moveJointSmooth(int pin, float &currentAngle, float targetAngle, int steps = 40, int waitTime = 15) {
    // Calculate the size of each micro-step
    float stepSize = (targetAngle - currentAngle) / steps;
    
    // Execute the interpolated movement step-by-step
    for (int i = 1; i <= steps; i++) {
        setServoAngle(pin, currentAngle + stepSize * i);
        delay(waitTime); // Controls the speed of the motion
    }
    
    // Update the global state variable to the exact target angle once finished
    currentAngle = targetAngle; 
}

// ================= 5. System Initialization =================
void setup() {
    // Start serial communication at 115200 baud for fast terminal response
    Serial.begin(115200);
    
    // Attach the ESP32 native LEDC (hardware PWM) to the servo pins
    ledcAttach(SERVO_BASE, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_SHOULDER, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_ELBOW, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_CLAW, PWM_FREQ, PWM_RESOLUTION);

    // Boot Sequence: Instantly lock the arm to the predefined safe posture
    setServoAngle(SERVO_BASE, currentB);
    setServoAngle(SERVO_SHOULDER, currentS);
    setServoAngle(SERVO_ELBOW, currentE);
    setServoAngle(SERVO_CLAW, currentC);
    
    // Print the instruction manual to the Serial Monitor
    Serial.println("\n=============================================");
    Serial.println("Smooth Calibration Console Started");
    Serial.println("Enter commands to fine-tune joints. Format: [Joint Letter][Angle]");
    Serial.println("Examples:");
    Serial.println("  b65  -> Move Base to 65 deg");
    Serial.println("  s100 -> Move Shoulder to 100 deg");
    Serial.println("  e70  -> Move Elbow to 70 deg");
    Serial.println("  c70  -> Move Claw to 70 deg");
    Serial.println("=============================================\n");
}

// ================= 6. Serial Parsing & Command Execution =================
void loop() {
    // Check if the user has typed anything into the Serial Monitor
    if (Serial.available() > 0) {
        // Read the first character (the joint identifier)
        char cmd = Serial.read();
        
        // Ignore stray newlines, carriage returns, or spaces
        if (cmd == '\n' || cmd == '\r' || cmd == ' ') return;
        
        // Read the following numbers as the target angle
        float angle = Serial.parseFloat();
        
        // Software bounds checking
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        bool isValidCmd = true;
        
        // Route the command to the correct servo using the smooth motion engine
        if (cmd == 'b' || cmd == 'B') {
            moveJointSmooth(SERVO_BASE, currentB, angle);
        } else if (cmd == 's' || cmd == 'S') {
            moveJointSmooth(SERVO_SHOULDER, currentS, angle);
        } else if (cmd == 'e' || cmd == 'E') {
            moveJointSmooth(SERVO_ELBOW, currentE, angle);
        } else if (cmd == 'c' || cmd == 'C') {
            moveJointSmooth(SERVO_CLAW, currentC, angle);
        } else {
            isValidCmd = false; // Invalid character entered
        }
        
        // If a valid command was executed, print the PERFECT keyframe coordinates
        // This is exactly what you copy-paste into the Automatic Production Line script!
        if (isValidCmd) {
            Serial.print("Current Smooth Posture -> B:");
            Serial.print((int)currentB);
            Serial.print(", S:");
            Serial.print((int)currentS);
            Serial.print(", E:");
            Serial.print((int)currentE);
            Serial.print(", C:");
            Serial.println((int)currentC);
            Serial.println("---------------------------------------------");
        }
    }
}
