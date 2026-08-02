#include <Arduino.h>

// ================= 1. 硬件引脚分配 =================
const int SERVO_BASE = 18;      // 底座 (B)
const int SERVO_SHOULDER = 27;  // 大臂 (S)
const int SERVO_ELBOW = 23;     // 小臂 (E)
const int SERVO_CLAW = 13;      // 机械爪 (C)

const int PWM_FREQ = 50;
const int PWM_RESOLUTION = 16;

// 记录当前姿态 (改为浮点数以支持细腻平滑)
float currentB = 65.0;  
float currentS = 90.0;  
float currentE = 80.0;  
float currentC = 45.0;  

// ================= 2. 底层驱动 =================
uint32_t angleToDuty(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    float minDuty = 1638.0f;    
    float maxDuty = 8192.0f;    
    return (uint32_t)(minDuty + (angle / 180.0f) * (maxDuty - minDuty));
}

void setServoAngle(int pin, float angle) {
    ledcWrite(pin, angleToDuty(angle));
}

// ================= 3. 核心算法：单轴丝滑移动 =================
// steps = 步数 (越大越细腻); waitTime = 延迟(越大越慢)
void moveJointSmooth(int pin, float &currentAngle, float targetAngle, int steps = 40, int waitTime = 15) {
    float stepSize = (targetAngle - currentAngle) / steps;
    
    for (int i = 1; i <= steps; i++) {
        setServoAngle(pin, currentAngle + stepSize * i);
        delay(waitTime); // 这里的延迟控制了移动的缓慢程度
    }
    
    currentAngle = targetAngle; // 动作结束，更新当前真实状态
}

// ================= 4. 初始化 =================
void setup() {
    Serial.begin(115200);
    
    ledcAttach(SERVO_BASE, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_SHOULDER, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_ELBOW, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(SERVO_CLAW, PWM_FREQ, PWM_RESOLUTION);

    // 锁定初始姿态
    setServoAngle(SERVO_BASE, currentB);
    setServoAngle(SERVO_SHOULDER, currentS);
    setServoAngle(SERVO_ELBOW, currentE);
    setServoAngle(SERVO_CLAW, currentC);
    
    Serial.println("\n=============================================");
    Serial.println("✨ 丝滑调优控制台已启动！");
    Serial.println("请输入指令微调关节，例如：");
    Serial.println("  b65  -> 底座");
    Serial.println("  s100 -> 大臂");
    Serial.println("  e70  -> 小臂");
    Serial.println("  c70  -> 爪子");
    Serial.println("=============================================\n");
}

// ================= 5. 串口解析与记录 =================
void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\n' || cmd == '\r' || cmd == ' ') return;
        
        float angle = Serial.parseFloat();
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        bool isValidCmd = true;
        
        // 收到指令后，调用丝滑移动函数
        if (cmd == 'b' || cmd == 'B') {
            moveJointSmooth(SERVO_BASE, currentB, angle);
        } else if (cmd == 's' || cmd == 'S') {
            moveJointSmooth(SERVO_SHOULDER, currentS, angle);
        } else if (cmd == 'e' || cmd == 'E') {
            moveJointSmooth(SERVO_ELBOW, currentE, angle);
        } else if (cmd == 'c' || cmd == 'C') {
            moveJointSmooth(SERVO_CLAW, currentC, angle);
        } else {
            isValidCmd = false;
        }
        
        if (isValidCmd) {
            Serial.print("✅ 当前丝滑姿态 -> B:");
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
