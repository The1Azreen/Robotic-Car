#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "motor.h"
#include "encoder.h"
#include "ultrasonic.h"

#define BUTTON_PIN 21      // GPIO pin connected to the button
typedef enum {
    IDLE = 0,
    Ultrasonic10cm,
    Turn90Degrees,
    Travel90cm
} RobotState;

// Function prototypes
void init_interrupt();
void ihandler(uint gpio, uint32_t events);
void turn_right_90();
void move_forward_cm(float distance);
void change_state(); // Ensure this matches the definition

// Global variables
RobotState current_state = IDLE;
kalman_state *ultrasonic_state;

int main()
{
    // Initialize standard I/O  
    stdio_init_all();
    init_motor_setup();
    init_motor_pwm();
    init_encoder_setup();
    ultrasonic_init();
    ultrasonic_state = kalman_init(1, 100, 1, 0);
    
    // Initialize interrupt
    init_interrupt();
    
    // Initialize Button GPIO
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  // Use pull-up resistor
    
    printf("Press the button to start.\n");
    
    // Wait for button press (active low)
    while (gpio_get(BUTTON_PIN)) {
        sleep_ms(100);
    }
    
    printf("Button pressed. Starting program.\n");
    current_state = Ultrasonic10cm;
    
    while (1)
    {
        change_state();
        sleep_ms(50);
    }
}

void change_state() {
    printf("Current state: %d\n", current_state);
    switch (current_state)
    {   
        case IDLE:
        {
            printf("IDLE\n");
            stop_motor();
            break;
        }
        case Ultrasonic10cm:
        {
            double distance = ultrasonic_get_distance(ultrasonic_state);
            printf("Distance: %.2lf cm\n", distance);
            if (distance > 12.0)
            {
                move_motor(pwm_l, pwm_r); // Move forward
            }
            else
            {
                printf("Obstacle detected. Stopping motor.\n");
                stop_motor();
                current_state = Turn90Degrees;
            }
            break;
        }
        case Turn90Degrees:
        {
            printf("Turning 90 degrees\n");
            turn_right_90();
            current_state = Travel90cm;
            break;
        }
        case Travel90cm:
        {
            printf("Traveling Mode\n");
            move_forward_cm(90.0); // Move forward 90 cm
            current_state = IDLE;
            break;
        }
        default:
            break;
    }
}

void init_interrupt() {
    // Configure single-edge triggering for encoder pins (e.g., rising edge only)
    gpio_set_irq_enabled_with_callback(L_ENCODER_OUT, GPIO_IRQ_EDGE_RISE, true, &ihandler);
    gpio_set_irq_enabled_with_callback(R_ENCODER_OUT, GPIO_IRQ_EDGE_RISE, true, &ihandler);
    
    // Set up interrupt for ultrasonic sensor (both edges are okay for this one)
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &ihandler);

    // Set up button interrupt on falling edge
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &ihandler);
}

void ihandler(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN)
    {
        if (current_state == IDLE)
            current_state = Ultrasonic10cm;         
        else if (current_state == Ultrasonic10cm)
            current_state = Turn90Degrees;
        else if (current_state == Turn90Degrees)
            current_state = Travel90cm;
    }
    // Encoder interrupt handler
    if (gpio == L_ENCODER_OUT || gpio == R_ENCODER_OUT) {
        encoder_pulse(gpio, events);
    }
    // Ultrasonic sensor interrupt handler
    else if (gpio == ECHO_PIN) {
        echo_pulse_handler(gpio, events);
    }
}

void turn_right_90() {
    // Example PWM values for turning
    pwm_l = 3125;  // Left motor power for right turn
    pwm_r = 2500;  // Right motor power for right turn

    int delay_ms = 400; // Adjust based on testing for a 90-degree turn

    // Call turn_motor with direction 1 (right turn), PWM values, and delay
    turn_motor(1, pwm_l, pwm_r, delay_ms);
}

// Function to move forward a specific distance in cm
void move_forward_cm(float distance) {
    // Set PWM values for forward movement
    pwm_l = 2800;  // Adjusted left motor speed
    pwm_r = 3150;  // Adjusted right motor speed

    // Set the motors to move forward
    uint slice_left = pwm_gpio_to_slice_num(L_MOTOR_ENA);
    uint slice_right = pwm_gpio_to_slice_num(R_MOTOR_ENB);

    pwm_set_chan_level(slice_left, pwm_gpio_to_channel(L_MOTOR_ENA), pwm_l);
    pwm_set_chan_level(slice_right, pwm_gpio_to_channel(R_MOTOR_ENB), pwm_r);

    gpio_put(L_MOTOR_IN1, 0);  // Left motor forward
    gpio_put(L_MOTOR_IN2, 1);
    gpio_put(R_MOTOR_IN3, 0);  // Right motor forward
    gpio_put(R_MOTOR_IN4, 1);

    gpio_put(L_MOTOR_ENA, 1);  // Enable left motor
    gpio_put(R_MOTOR_ENB, 1);  // Enable right motor

    // Hardcoded delay to approximate distance
    int delay_ms = distance * 27;  // Adjust multiplier based on testing
    printf("Moving forward for %d ms to cover approximately %.2f cm.\n", delay_ms, distance);

    sleep_ms(delay_ms);  // Wait for the calculated delay

    // Stop the motors after the delay
    stop_motor();
    printf("Hard-coded distance reached. Stopping motor.\n");
}
