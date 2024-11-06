# Define target
set(target motor_lib)

# Add library target for source file
add_library(${target}
    motor.c
    motor.h
)

# Include directories for the library
target_include_directories(${target} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}/../main)

# Link libraries (common dependencies and additional PWM hardware support)
target_link_libraries(${target} pico_stdlib hardware_pwm encoder_lib FreeRTOS-Kernel-Heap4)
