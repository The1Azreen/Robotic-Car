// barcode_scanner.c
#include "barcode_scanner.h"
#include "pico/multicore.h"
#include "FreeRTOS.h"
#include "task.h"

// Private structures
typedef struct {
    float width;
    int state;  
} BarInfo;

typedef struct {
    const char* pattern;
    char character;
} Code39Mapping;

// Static variables
static QueueHandle_t xResultQueue = NULL;
static char decoded_message[MAX_MESSAGE_LENGTH];
static int message_length = 0;
static bool using_reverse_dict = false;
static bool in_barcode = false;

// Original CODE39_DICT and CODE39_DICT_REVERSE arrays
static const Code39Mapping CODE39_DICT[] = {
    {"000110100", '0'},
    {"100100001", '1'},
    {"001100001", '2'},
    {"101100000", '3'},
    {"000110001", '4'},
    {"100110000", '5'},
    {"001110000", '6'},
    {"000100101", '7'},
    {"100100100", '8'},
    {"001100100", '9'},
    {"100001001", 'A'},
    {"001001001", 'B'},
    {"101001000", 'C'},
    {"000011001", 'D'},
    {"100011000", 'E'},
    {"001011000", 'F'},
    {"000001101", 'G'},
    {"100001100", 'H'},
    {"001001100", 'I'},
    {"000011100", 'J'},
    {"100000011", 'K'},
    {"001000011", 'L'},
    {"101000010", 'M'},
    {"000010011", 'N'},
    {"100010010", 'O'},
    {"001010010", 'P'},
    {"000000111", 'Q'},
    {"100000110", 'R'},
    {"001000110", 'S'},
    {"000010110", 'T'},
    {"110000001", 'U'},
    {"011000001", 'V'},
    {"111000000", 'W'},
    {"010010001", 'X'},
    {"110010000", 'Y'},
    {"011010000", 'Z'},
    {"010000101", '-'},
    {"110000100", '.'},
    {"011000100", ' '},
    {"010010100", '*'}  // Start/Stop character
};

static const Code39Mapping CODE39_DICT_REVERSE[] = {
    {"001011000", '0'},  // Original: 000110100
    {"100001001", '1'},  // Original: 100100001
    {"100001100", '2'},  // Original: 001100001
    {"000001101", '3'},  // Original: 101100000
    {"100011000", '4'},  // Original: 000110001
    {"000011001", '5'},  // Original: 100110000
    {"000011100", '6'},  // Original: 001110000
    {"101001000", '7'},  // Original: 000100101
    {"001001001", '8'},  // Original: 100100100
    {"001001100", '9'},  // Original: 001100100
    {"100100001", 'A'},  // Original: 100001001
    {"100100100", 'B'},  // Original: 001001001
    {"000100101", 'C'},  // Original: 101001000
    {"100110000", 'D'},  // Original: 000011001
    {"000110001", 'E'},  // Original: 100011000
    {"000110100", 'F'},  // Original: 001011000
    {"101110000", 'G'},  // Original: 000001101
    {"001110000", 'H'},  // Original: 100001100
    {"001110100", 'I'},  // Original: 001001100
    {"001110000", 'J'},  // Original: 000011100
    {"110000001", 'K'},  // Original: 100000011
    {"110000100", 'L'},  // Original: 001000011
    {"010000101", 'M'},  // Original: 101000010
    {"110010000", 'N'},  // Original: 000010011
    {"010010001", 'O'},  // Original: 100010010
    {"010010100", 'P'},  // Original: 001010010
    {"111000000", 'Q'},  // Original: 000000111
    {"011000001", 'R'},  // Original: 100000110
    {"011000100", 'S'},  // Original: 001000110
    {"011010000", 'T'},  // Original: 000010110
    {"100000011", 'U'},  // Original: 110000001
    {"100000110", 'V'},  // Original: 011000001
    {"000000111", 'W'},  // Original: 111000000
    {"100010010", 'X'},  // Original: 010010001
    {"000010011", 'Y'},  // Original: 110010000
    {"000010110", 'Z'},  // Original: 011010000
    {"101000010", '-'},  // Original: 010000101
    {"001000011", '.'},  // Original: 110000100
    {"001000110", ' '},  // Original: 011000100
    {"001010010", '*'},  // Original: 010010100
};


// Helper functions from original implementation
static int get_denoised_state(void) {
    int sum = 0;
    for(int i = 0; i < SAMPLE_SIZE; i++) {
        sum += gpio_get(IR_SENSOR_PIN);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    float average = (float)sum / SAMPLE_SIZE;
    return (average > THRESHOLD) ? 1 : 0;
}

static void sort_bars(BarInfo* bars, int count) {
    for(int i = 0; i < count-1; i++) {
        for(int j = 0; j < count-i-1; j++) {
            if(bars[j].width < bars[j+1].width) {
                BarInfo temp = bars[j];
                bars[j] = bars[j+1];
                bars[j+1] = temp;
            }
        }
    }
}

static char decode_pattern(const char* pattern) {
    size_t dict_size = sizeof(CODE39_DICT) / sizeof(CODE39_DICT[0]);
    
    if (!in_barcode) {
        if (strcmp(pattern, CODE39_DICT[dict_size-1].pattern) == 0) {
            using_reverse_dict = false;
            in_barcode = true;
            return '*';
        }
        if (strcmp(pattern, CODE39_DICT_REVERSE[dict_size-1].pattern) == 0) {
            using_reverse_dict = true;
            in_barcode = true;
            return '*';
        }
    }
    
    const Code39Mapping* dict = using_reverse_dict ? CODE39_DICT_REVERSE : CODE39_DICT;
    
    for (size_t i = 0; i < dict_size; i++) {
        if (strcmp(pattern, dict[i].pattern) == 0) {
            if (dict[i].character == '*' && in_barcode) {
                in_barcode = false;
            }
            return dict[i].character;
        }
    }
    
    return '?';
}

static bool has_initial_outlier(BarInfo* bars) {
    BarInfo sorted_bars[BARS_PER_CHAR];
    memcpy(sorted_bars, bars, sizeof(BarInfo) * BARS_PER_CHAR);
    sort_bars(sorted_bars, BARS_PER_CHAR);
    
    if (sorted_bars[0].width > sorted_bars[1].width * OUTLIER_RATIO) {
        for (int i = 0; i < BARS_PER_CHAR; i++) {
            if (bars[i].width == sorted_bars[0].width && i == 0) {
                return true;
            }
        }
    }
    return false;
}

static void process_bars(BarInfo* bars) {
    char pattern[BARS_PER_CHAR + 1];
    BarInfo sorted_bars[BARS_PER_CHAR];
    memcpy(sorted_bars, bars, sizeof(BarInfo) * BARS_PER_CHAR);
    sort_bars(sorted_bars, BARS_PER_CHAR);
    
    float threshold = sorted_bars[2].width - 0.01;
    
    for(int i = 0; i < BARS_PER_CHAR; i++) {
        pattern[i] = (bars[i].width > threshold) ? '1' : '0';
    }
    pattern[BARS_PER_CHAR] = '\0';
    
    BarcodeMessage msg = {
        .is_reset_command = false,
        .decoded_char = decode_pattern(pattern)
    };
    strcpy(msg.pattern, pattern);
    xQueueSendToBack(xResultQueue, &msg, portMAX_DELAY);
    
    if (msg.decoded_char == '*' && !in_barcode) {
        BarcodeMessage complete_msg = {
            .is_reset_command = false,
            .decoded_char = '\0'
        };
        strcpy(complete_msg.pattern, decoded_message);
        xQueueSendToBack(xResultQueue, &complete_msg, portMAX_DELAY);
        message_length = 0;
        decoded_message[0] = '\0';
    } else if (in_barcode && msg.decoded_char != '*') {
        if (message_length < MAX_MESSAGE_LENGTH - 1) {
            decoded_message[message_length++] = msg.decoded_char;
            decoded_message[message_length] = '\0';
        }
    }
}

static void reset_scanner_state(void) {
    message_length = 0;
    decoded_message[0] = '\0';
    in_barcode = false;
    using_reverse_dict = false;
}

// The main barcode scanner task
static void vBarcodeTask(void *pvParameters) {
    // Initialize GPIO
    gpio_init(IR_SENSOR_PIN);
    gpio_set_dir(IR_SENSOR_PIN, GPIO_IN);
    gpio_init(RESET_BUTTON_PIN);
    gpio_set_dir(RESET_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(RESET_BUTTON_PIN);

    BarInfo bars[BARS_PER_CHAR];
    int current_state = get_denoised_state();
    int bar_count = 0;
    TickType_t bar_start_time = xTaskGetTickCount();
    bool collecting_extra = false;
    BarInfo temp_bars[BARS_PER_CHAR + 1];
    bool skip_next_white = false;

    while (true) {
        if (gpio_get(RESET_BUTTON_PIN) == 0) {
            BarcodeMessage reset_msg = {.is_reset_command = true};
            xQueueSendToBack(xResultQueue, &reset_msg, portMAX_DELAY);
            reset_scanner_state();
            bar_count = 0;
            collecting_extra = false;
            skip_next_white = false;
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        int new_state = get_denoised_state();
        
        if (new_state != current_state) {
            TickType_t current_time = xTaskGetTickCount();
            float bar_width_ms = (current_time - bar_start_time) * portTICK_PERIOD_MS;
            
            if (skip_next_white && new_state == 1) {
                skip_next_white = false;
            }
            else if (!collecting_extra) {
                bars[bar_count].width = bar_width_ms;
                bars[bar_count].state = new_state;
                bar_count++;
                
                if (bar_count >= BARS_PER_CHAR) {
                    if (has_initial_outlier(bars)) {
                        memcpy(temp_bars, bars, sizeof(bars));
                        collecting_extra = true;
                        bar_count = BARS_PER_CHAR;
                    } else {
                        process_bars(bars);
                        bar_count = 0;
                        skip_next_white = true;
                    }
                }
            } else {
                temp_bars[BARS_PER_CHAR].width = bar_width_ms;
                temp_bars[BARS_PER_CHAR].state = new_state;
                
                for (int i = 0; i < BARS_PER_CHAR; i++) {
                    bars[i] = temp_bars[i + 1];
                }
                
                process_bars(bars);
                bar_count = 0;
                collecting_extra = false;
                skip_next_white = true;
            }
            
            bar_start_time = current_time;
            current_state = new_state;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

BaseType_t xBarcodeTaskInit(UBaseType_t uxPriority, QueueHandle_t resultQueue) {
    xResultQueue = resultQueue;
    
    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned = xTaskCreate(
        vBarcodeTask,
        "BarcodeScanner",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        uxPriority,
        &xHandle
    );
    
    if (xReturned == pdPASS) {
        // Set core affinity to Core 1
        UBaseType_t uxCoreAffinityMask = (1 << 1); // Core 1
        vTaskCoreAffinitySet(xHandle, uxCoreAffinityMask);
    }
    
    return xReturned;
}