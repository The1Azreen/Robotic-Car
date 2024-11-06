# Define target
set(target barcode_lib)

# Add library target for source file
add_library(${target}
    barcode_scanner.c
    barcode_scanner.h
)

# Include directories for the library
target_include_directories(${target} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}/../main
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)

# Link libraries (common dependencies and additional PWM hardware support)
    target_link_libraries(${target} pico_stdlib 
    hardware_pwm 
    hardware_adc 
    pico_multicore 
    FreeRTOS-Kernel-Heap4
)   
