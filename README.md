# INF2004-Project-P3G
**Robotic Car Project - Team P3G**

### Members (Adduing the name later)
**Team Members** <br>
Shao <br>
Seb <br>
Aloy <br>
Ash <br>
Azreen <be>

### Project Description
---
Our project centers on developing an autonomous two-wheeled robot car powered by the Raspberry Pi Pico W as its primary microcontroller. The robot is designed to efficiently navigate predefined maps using advanced navigation algorithms. It features obstacle detection and avoidance capabilities, along with the ability to decode Barcode-39 standard barcodes. To ensure smooth and precise movement, a PID controller is integrated into the system. Additionally, sensor fusion enhances the car's performance and robustness by improving decision-making accuracy and overall reliability.

### Program Design
---
In this section, we will go over the modular design of the car. Our project comprises of 8 components, each catered for a particular sensor/aspect of the car:
- Main (program entrypoint for car to start executions)
- Navigation and Mapping
- Wifi
- Motor
- Ultrasonic (for obstacle detection)
- IRLine (for barcode decoding and line following)
- Encoder (for car movement)
- Magnetometer (for car direction tracking)
