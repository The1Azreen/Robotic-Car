#define SAMPLE_SIZE 10
#define THRESHOLD 0.5
#define BARS_PER_CHAR 9
#define WIDE_BARS_PER_CHAR 3
#define OUTLIER_RATIO 5.0 // If largest width is this many times bigger than second largest, it's an outlier
#define MAX_MESSAGE_LENGTH 50


typedef struct
{
    float width;
    int state;
} BarInfo;

typedef struct
{
    const char *pattern;
    char character;
} Code39Mapping;

// Function prototypes
void sort_bars(BarInfo *bars, int count);
int count_wide_bars(const char *pattern);
bool has_initial_outlier(BarInfo *bars);
char decode_pattern(const char *pattern);
void process_bars(BarInfo *bars, int character_count);
void track_bars();
int get_denoised_state();
void add_to_message(char character);

void barcodeTask(void *pvParameters);