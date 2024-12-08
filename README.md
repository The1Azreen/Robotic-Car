# INF2004-Project-P3G
**Robotic Car Project - Team P3G**

### Members
**Team Members** <br>
Shao <br>
Sebastian <br>
Aloysius <br>
Ashsyahid <br>
Azreen <be>

### Project Description
---
Our project centers on developing an autonomous two-wheeled robot car powered by the Raspberry Pi Pico W as its primary microcontroller. The robot is designed to efficiently navigate predefined maps using advanced navigation algorithms. It features obstacle detection and avoidance capabilities, along with the ability to decode Barcode-39 standard barcodes. To ensure smooth and precise movement, a PID controller is integrated into the system. Additionally, sensor fusion enhances the car's performance and robustness by improving decision-making accuracy and overall reliability.

### Program Design
---
In this section, we will go over the modular design of the car. Our project comprises of 8 components, each catered for a particular sensor/aspect of the car:
- Main (program entrypoint for car to start executions)
- Wifi (send and receive data)
- Motor (moves the car)
- Ultrasonic (for obstacle detection)
- IRLine (for barcode decoding and line following)
- Encoder (pid controller)
- Magnetometer (for car direction tracking)

### Map Design


### Component List

**Raspberry Pi**
- 1 x [Raspberry Pi Pico W](https://www.raspberrypi.com/products/raspberry-pi-pico/)

**Sensors**
- 1 x [Ultrasonic Sensor: HCR04](https://components101.com/sensors/ultrasonic-sensor-working-pinout-datasheet)
- 3 x [IR Sensor](https://sg.cytron.io/p-ir-line-tracking-module)
- 2 x [Wheel Encoder](https://hobbycomponents.com/sensors/1147-compact-ir-infrared-rotary-speed-sensing-module)
- 2 x [Motor Controller: L298N](https://components101.com/modules/l293n-motor-driver-module)
- 2 x Motor
- 1 x [Magnetometer: GY-511](https://quartzcomponents.com/products/gy-511-lsm303dlhc-3-axis-e-compass-sensor)

**Hardware**
- 2 x Car Wheels
- 1 x Car Chassis
- 1 x Wire Splitter
- 1 x Powerbank (Alternatively you can use AA batteries with a battery holder so long the pico is powered via USB)
- Mounting Plates
- Nuts and Bolts
- Jumper Cables (M-M, F-F, M-F)


### Project Structure
```

docs/ (documentation images)

source/encoder.c (driver code for wheel encoders)

source/line.c

source/barcode.c

source/magnetometer

source/main/ (main program code)

source/motor/ 

source/pins

source/ultrasonic

source/

README.md (this file)

Build files:
CMakeLists.txt
FreeRTOS_Kernel_import.cmake
pico_extras_import_optional.cmake
pico_sdk_import.cmake
```

### Getting Started



