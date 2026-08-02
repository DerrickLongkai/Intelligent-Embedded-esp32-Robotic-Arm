#include <Arduino.h>

// ================= Hardware Pin Allocation =================
// Assigning ESP32 GPIO pins to each servo
const int SERVO_BASE = 18; //Controls the horizontal rotation(Base)  
const int SERVO_SHOULDER = 27;  //Controls the primary vertical lift(Shoulder)
const int SERVO_ELBOW = 23;  //Controls the secondary forward extension(Elbow)
const int SERVO_CLAW = 13; //Controls the Claw     

// Standard Servo specifications: 50Hz frequency (20ms period)
const int PWM_FREQ = 50;
// Using 16-bit resolution (0-65535) for ultra-precise angle control
const int PWM_RESOLUTION = 16;

// ================= Original State =================
//Angles of the robotic arm
float currentB = 65.0;  // Base
float currentS = 90.0;  // Shoulder
float currentE = 80.0;  // Elbow
float currentC = 45.0;  // Claw

// ================= Bottom-Driven =================
/**
 * Converts a target angle (0-180) into an ESP32 16-bit PWM duty cycle.
 * For a 50Hz signal:
 * - 0 degrees ~ 0.5ms pulse ~ 2.5% duty cycle -> 1638 (out of 65535)
 * - 180 degrees ~ 2.5ms pulse ~ 12.5% duty cycle -> 8192 (out of 65535)
 */
uint32_t angleToDuty(float angle) {
    // Hardware protection: Prevent software from requesting out-of-bounds angles
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    // Duty cycle mapping for standard 180-degree micro servos
    float minDuty = 1638.0f;    
    float maxDuty = 8192.0f;   
    // Linear interpolation from angle to 16-bit duty cycle value
    return (uint32_t)(minDuty + (angle / 180.0f) * (maxDuty - minDuty));
}

void setServoAngle(int pin, float angle) {
    ledcWrite(pin, angleToDuty(angle));
}

// ================= Core Algorithm: Four-axis Linkage engine =================
/**
 * Moves all 4 joints simultaneously and smoothly to avoid mechanical shocks.
 * It slices the total movement into tiny steps, acting like an industrial S-curve.
 * 
 * @param targetB Target Base angle
 * @param targetS Target Shoulder angle
 * @param targetE Target Elbow angle
 * @param targetC Target Claw angle
 * @param steps Number of slices (higher = smoother but slower execution)
 * @param waitTime Delay between steps in ms (higher = slower overall movement)
 */
void moveArmSmooth(float targetB, float targetS, float targetE, float targetC, int steps = 50, int waitTime = 20) {
    // Calculate the incremental step size for each joint
    float stepB = (targetB - currentB) / steps;
    float stepS = (targetS - currentS) / steps;
    float stepE = (targetE - currentE) / steps;
    float stepC = (targetC - currentC) / steps;

    // Execute the interpolated movement
    for (int i = 1; i <= steps; i++) {
        setServoAngle(SERVO_BASE, currentB + stepB * i);
        setServoAngle(SERVO_SHOULDER, currentS + stepS * i);
        setServoAngle(SERVO_ELBOW, currentE + stepE * i);
        setServoAngle(SERVO_CLAW, currentC + stepC * i);
        delay(waitTime); 
    }
    // Update the current real status
    currentB = targetB;
    currentS = targetS;
    currentE = targetE;
    currentC = targetC;
}

// ================= System initialization =================
void setup() {
    Serial.begin(115200);
    // Attach the ESP32 native LEDC (hardware PWM) to the servo pins
    ledcAttach(SERVO_BASE, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_SHOULDER, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_ELBOW, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_CLAW, PWM_FREQ, PWM_RESOLUTION);

    // Lock to the specified initial posture at the moment of startup
    setServoAngle(SERVO_BASE, currentB);
    setServoAngle(SERVO_SHOULDER, currentS);
    setServoAngle(SERVO_ELBOW, currentE);
    setServoAngle(SERVO_CLAW, currentC);
    
    Serial.println("[Assembly Line System Ready] Waiting to enter the main cycle......");
}

// ================= Automate the main loop script =================
void loop() {
    Serial.println("=====================================");
    Serial.println("Action team ready, wait 3 seconds");
    delay(3000); //Wait for 5 seconds before the initial action starts and have a 5-second interval between cycles
    
    //Movement 1: B: from 65 to 155 Turn to target
    Serial.println("Step 1: B -> 155");
    moveArmSmooth(155, 90, 80, 45, 50, 20); 
    delay(300);

    //Movement 2: E: from 80 to 130 Elbow extends outward to reach the object
    Serial.println("Step 2: E -> 130");
    moveArmSmooth(155, 90, 130, 45, 40, 20);
    delay(300);

    //Movement 3: C: from 45 to 70 Claw closes to 70 degrees to grip the object
    Serial.println("Step 3: C -> 70");
    moveArmSmooth(155, 90, 130, 70, 30, 20);
    delay(300);

    //Movement 4: S: from 90 to 120 Shoulder raises to lift the payload
    Serial.println("Step 4: S -> 120");
    moveArmSmooth(155, 120, 130, 70, 40, 20);
    delay(300);

    //Movement 5: C: from 70 to 45 Claw close to grab the object
    Serial.println("Step 5: C -> 45");
    moveArmSmooth(155, 120, 130, 45, 30, 20);
    delay(300);


    //Movement 6: S: from 120 to 70 Shoulder back to stand angle
    Serial.println("Step 6: S -> 70");
    moveArmSmooth(155, 70, 130, 45, 50, 20);
    delay(300);
    

    //Movement 7: E: from 130 to 90 Elbow back to standard angle
    Serial.println("Step 7: E -> 90");
    moveArmSmooth(155, 70, 90, 45, 40, 20);
    delay(300);

    //Movement 8: B: from 165 to 65 Base rotates back to original angle
    Serial.println("Step 8: B -> 65");
    moveArmSmooth(65, 70, 90, 45, 60, 20);
    delay(300);

    //Movement 9: E: from 90 to 140 Elbow extended
    Serial.println("Step 9: E -> 140");
    moveArmSmooth(65, 70, 140, 45, 50, 20);
    delay(300);

    //Movement 10: S: from 70 to 120 Shoulder extened
    Serial.println("Step 10: S -> 120");
    moveArmSmooth(65, 120, 140, 45, 50, 20);
    delay(300);

    //Movement 11: C: from 45 to 70 claw open
    Serial.println("Step 11: C -> 70");
    moveArmSmooth(65, 120, 140, 70, 30, 20);
    delay(300);

    //Movement 12: C: from 79 to 45 claw close
    Serial.println("Step 12: C -> 45");
    moveArmSmooth(65, 120, 140, 45, 30, 20);
    delay(300);

    //Movement 13: S: from 120 to 90 Shoulder back to original position
    Serial.println("Step 13: S -> 90");
    moveArmSmooth(65, 90, 140, 45, 40, 20);
    delay(300);

    //Last Movement 14: E: from 140 to 80 Elbow back to original positon 
    Serial.println("Step 14: E -> 80");
    moveArmSmooth(65, 90, 80, 45, 50, 20);

    Serial.println("The process is completed!");
}