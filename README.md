# Intelligent-Embedded-esp32-Robotic-Arm

A 4-degree-of-freedom robotic arm built from scratch using an ESP32. This project implements a low-level multi-axis linear interpolation algorithm to achieve industrial-grade S-curve motion. This repository documents the entire journey—from physical assembly and PWM register debugging, and built with passion and a few burned servos.

![Demo GIF](./Demo1.gif)

## Repository Structure

This project provides two sets of code, covering everything from low-level debugging to full automation:

*   **[`Manual.ino`](./Manual.ino)** (Static Tuning Console)
    *   **Purpose**: Used for initial mechanical alignment ("bone-setting") and capturing keyframe poses. Accepts serial commands (e.g., `b65`, `s120` at `115200` baud) with single-axis smooth filtering and auto-prints the perfect coordinates. The ultimate debugger.
*   **[`Automatic.ino`](./Automatic.ino)** (Automatic Production Line)
    *   **Purpose**: The showcase and production task script. Features the built-in 4-axis interpolation engine and a complete industrial-grade state machine script. Auto-homes on boot and executes a precise, interference-free grab-and-transport loop.

## Full Demonstration

Check out the raw, unedited 30-second operational cycles showcasing the smooth kinematics and interference-free logic:

*   **[Watch Demo One(click view raw to download the raw video)](./demo_one.mp4)** 
*   **[Watch Demo Two(click view raw to download the raw video)](./demo_two.mp4)**

## Hardware Components
*   **Main Controller**: ESP32 Development Board (utilizing its hardware PWM resources)
*   **Actuators**: 4 x 180° Micro Servos (Shoulder, Elbow, Base, Claw)
*   **Frame**: Acrylic robotic arm kit + custom metal screws
*   **Accessories**: Breadboard, Breadboard power supply, jumper wires, Micro USB cable.

## Software & Development Environment
*   **IDE**: Arduino IDE
*   **Core Libraries**: Pure low-level drivers. Uses the ESP32's native `ledc` hardware PWM interface (`50Hz` frequency, `16-bit` resolution). No bloated third-party servo libraries were used.

### Arduino IDE Setup & Deployment
To get this code running on your ESP32, follow these standard deployment steps:
1. **Board Configuration**
   * Ensure you have the **ESP32 Board Package** installed in your Arduino IDE via the Boards Manager.
   * Navigate to `Tools` > `Board` and select **ESP32 Dev Module** (or your specific ESP32 variant).
2. **Port Connection**
   * Connect the ESP32 to your PC using a data-capable USB cable.
   * Navigate to `Tools` > `Port` and select the appropriate COM port.
3. **Compilation & Upload**
   * Click the **Upload** button. 
4. **Serial Monitor Debugging**
   * Open the **Serial Monitor** and set the baud rate to **`115200`** to match the code.
  
## Core Logic & Physics Tuning
The biggest highlight of this project is solving the three major pain points of micro robotic arms: jittering, stalling, and burning out. It deeply integrates code logic with physical mechanics:
### 1. Multi-axis S-Curve Smooth Interpolation
Micro servos lack built-in acceleration/deceleration curves. Sending a target angle directly causes violent mechanical shocks.
*   **Solution**: Rewrote a multi-axis coordinated `moveArmSmooth()` engine. It slices large-span angle jumps into dozens of tiny steps with a controlled delay rhythm, allowing the 4 joints to reach their target positions slowly and synchronously.

### 2. Center of Gravity (CoG) & Lever Arm Management
When the shoulder and elbow are fully extended, they create a massive lateral leverage that can jam the base servo's gears (stalling).
*   **Solution (Software + Hardware)**: 
    1. **Software Limits**: Established strict "fold-then-turn" state machine rules. Before making large horizontal base movements, the arm is forced to retract to a high position (reducing the lever arm), and only extends after the turn is complete.
    2. **Hardware Counterweight**: Added physical counterweights to the back of the shoulder plate to perfectly offset the downward pull of the extended claw.

### 3. Physical Deadzones & Soft Limits
Grabbing hard objects or hitting physical limits easily causes the micro servos to stall, burning out the internal boards within seconds.
*   **Solution**: Accurately mapped the absolute safe comfort zone of each joint using a custom serial console. The claw's closing angle is strictly soft-limited, and a sponge at the gripping end absorbs excess travel, eliminating the deadly stalling "buzz".

## Future Plans (To-Do)
*   [ ] Optimize the physical tail counterweight structure.
*   [ ] Integrate an ultrasonic sensor for distance detection and dynamic grabbing.
*   [ ] Add a Bluetooth module for wireless gamepad control.



