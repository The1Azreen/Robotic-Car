# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(RobotCar C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

if (NOT PICO_NO_HARDWARE)
    add_subdirectory(main)
    add_subdirectory(motor)
    add_subdirectory(irline)
    add_subdirectory(ultrasonic)
    add_subdirectory(wifi)
    add_subdirectory(encoder)
    add_subdirectory(magnetometer)
    add_subdirectory(nav)
endif ()
