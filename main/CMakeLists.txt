# Add your source files here
add_executable(${PROJECT_NAME}
    main.c
    blink.c
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
    
    motor  # Include the motor directory
    encoder  # Include the encoder directory
    ultrasonic
    barcode
)

# Add required libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    FreeRTOS-Kernel-Heap4
    pico_cyw43_arch_none
    motor_lib  # Link the motor library
    encoder_lib  # Link the encoder library
    ultrasonic_lib
    barcode_lib
)

# Enable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Set the runtime output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/main"
)


pico_add_extra_outputs(${PROJECT_NAME})