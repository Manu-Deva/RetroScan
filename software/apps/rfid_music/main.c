#include <stdio.h>
#include "nrf_drv_twi.h"
#include "nrf_twi_mngr.h"
#include "rfid_driver.h"
#include "ili9341.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "microbit_v2.h"
#include "nrf_drv_saadc.h"

#define POLLING_INTERVAL APP_TIMER_TICKS(500)
#define ABBEY_ROAD "3A006C84D200"
#define PULP_FICTION "3A006C762F0F"
#define IN_RAINBOWS "00000015C9DC"

#define MAX_TAGS 10
#define TFT_WIDTH 240
#define TFT_HEIGHT 320

typedef struct
{
    const char *tag_id; // RFID tag ID
    const char *type;   // Object type (e.g., "Vinyl", "VHS")
    const char *person; // Director or Artist
    const char *title;  // Title
    const char *field1; // Field 1 (e.g., song/actor)
    const char *field2; // Field 2
    const char *field3; // Field 3
    const char *genre;  // Genre
    const char *year;   // Year
    const char *weight; // Weight
} tag_info_t;

#define MAX_TAGS 10

static tag_info_t tag_info_table[MAX_TAGS] = {
    {"3A006C84D200", "Vinyl", "Travis Scott", "Utopia", "Meltdown", "Thank God", "Fein", "Rap", "2023", "5"},
    {"3A006C762F0F", "VHS", "Sonnenfeld", "Men in Black", "Will Smith", "Tommy Lee Jones", "Rip Torn", "Sci-Fi", "1997", "10"},
    {"00000015C9DC", "Vinyl", "Radiohead", "In Rainbows", "All I Need", "Weird Fishes", "Videotape Garden", "Rock", "2007", "5"},
    // Add more entries as needed
};
static const size_t tag_info_count = sizeof(tag_info_table) / sizeof(tag_info_table[0]);

// TWI Manager instance
NRF_TWI_MNGR_DEF(m_twi_mngr, 1, 0);
APP_TIMER_DEF(rfid_timer);

static char last_displayed_tag[13] = "";
static bool is_displaying_tag = false;

// Function prototypes
void rfid_timer_callback(void *context);
void process_rfid_tag(const char *tag_id);

// SAADC callback (empty)
void saadc_callback(nrf_drv_saadc_evt_t const *p_event) {}

void saadc_init(void)
{
    nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
    saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
    nrf_drv_saadc_init(&saadc_config, saadc_callback);

    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
    nrf_drv_saadc_channel_init(0, &channel_config);
}

float read_fsr_weight(void)
{
    nrf_saadc_value_t saadc_value;
    ret_code_t err_code = nrf_drv_saadc_sample_convert(0, &saadc_value);
    APP_ERROR_CHECK(err_code);

    // Convert SAADC value to weight in ounces
    float weight = (float)saadc_value * 0.01 + 2;

    if (weight < 2.9)
    {
        return 0;
    }
    return weight;
}

// Function to calculate the center-aligned X coordinate
uint16_t calculate_center_aligned_x(const char *text, uint8_t scale)
{
    size_t length = 0;

    while (text[length] != '\0')
    {
        length++;
    }

    uint16_t text_width = length * 8 * scale;

    return (TFT_WIDTH - text_width) / 2;
}

// Function to calculate the right-aligned X coordinate
uint16_t calculate_right_aligned_x(const char *text, uint8_t scale)
{
    size_t length = 0;

    // Calculate the string length
    while (text[length] != '\0')
    {
        length++;
    }

    uint16_t text_width = length * 8 * scale;

    return TFT_WIDTH - text_width - 35;
}

// Function to display a header at the top of the screen in red
void display_header(const char *header)
{
    uint16_t x = calculate_center_aligned_x(header, 1);
    ili9341_draw_string(x, 20, header, 1, 0xFF, 0x00, 0x00);
}

void display_weight(float weight)
{
    if (weight < 10)
    {
        char buffer1[15];
        snprintf(buffer1, sizeof(buffer1), "Weight oz: %d ", (int)weight);
        uint16_t x = calculate_right_aligned_x(buffer1, 1);

        // Define the area for weight display
        set_address_window(10, 280, 240, 20);

        // Draw the updated weight
        ili9341_draw_string(x, 270, buffer1, 1, 0x00, 0x00, 0x00);
    }
    else
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Weight oz: %d", (int)weight);
        uint16_t x = calculate_right_aligned_x(buffer, 1);

        // Define the area for weight display
        set_address_window(10, 280, 240, 20);

        // Draw the updated weight
        ili9341_draw_string(x, 270, buffer, 1, 0x00, 0x00, 0x00);
    }
}

void display_vinyl_record(const char *artist, const char *title, const char *song1, const char *song2, const char *song3, const char *genre, const char *year, const char *vinyl_weight, uint8_t r, uint8_t g, uint8_t b)
{
    ili9341_fill_screen(r, g, b);
    ili9341_fill_screen(0xFF, 0xFF, 0xFF);

    // Display the header in blue
    display_header("Record Info");

    char buffer[64];

    // Display Artist
    snprintf(buffer, sizeof(buffer), "Artist: %s", artist);
    uint16_t x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 60, buffer, 1, 0x00, 0x00, 0x00); // Black color

    // Display Title
    snprintf(buffer, sizeof(buffer), "Title: %s", title);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 90, buffer, 1, 0x00, 0x00, 0x00);

    // Display Songs
    snprintf(buffer, sizeof(buffer), "Song: %s", song1);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 120, buffer, 1, 0x00, 0x00, 0x00);

    snprintf(buffer, sizeof(buffer), "Song: %s", song2);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 150, buffer, 1, 0x00, 0x00, 0x00);

    snprintf(buffer, sizeof(buffer), "Song: %s", song3);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 180, buffer, 1, 0x00, 0x00, 0x00);

    // Display Genre
    snprintf(buffer, sizeof(buffer), "Genre: %s", genre);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 210, buffer, 1, 0x00, 0x00, 0x00);

    // Display Year
    snprintf(buffer, sizeof(buffer), "Year: %s", year);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 240, buffer, 1, 0x00, 0x00, 0x00);

    // Draw vinyl icon in the bottom right
    draw_vinyl_icon(TFT_WIDTH - 30, TFT_HEIGHT - 30);
}

void display_vhs_movie(const char *director, const char *title, const char *actor1, const char *actor2, const char *actor3, const char *genre, const char *year, const char *vhs_weight)
{
    ili9341_fill_screen(0x00, 0x00, 0xFF);
    ili9341_fill_screen(0xFF, 0xFF, 0xFF);

    // Display the header in blue
    display_header("VHS Info");

    char buffer[64];

    // Display Director
    snprintf(buffer, sizeof(buffer), "Director: %s", director);
    uint16_t x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 60, buffer, 1, 0x00, 0x00, 0x00); // Black color

    // Display Title
    snprintf(buffer, sizeof(buffer), "Title: %s", title);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 90, buffer, 1, 0x00, 0x00, 0x00);

    // Display Actors
    snprintf(buffer, sizeof(buffer), "Actor: %s", actor1);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 120, buffer, 1, 0x00, 0x00, 0x00);

    snprintf(buffer, sizeof(buffer), "Actor: %s", actor2);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 150, buffer, 1, 0x00, 0x00, 0x00);

    snprintf(buffer, sizeof(buffer), "Actor: %s", actor3);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 180, buffer, 1, 0x00, 0x00, 0x00);

    // Display Genre
    snprintf(buffer, sizeof(buffer), "Genre: %s", genre);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 210, buffer, 1, 0x00, 0x00, 0x00);

    // Display Year
    snprintf(buffer, sizeof(buffer), "Year: %s", year);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 240, buffer, 1, 0x00, 0x00, 0x00);

    // Draw VHS icon in the bottom right
    draw_vhs_icon(TFT_WIDTH - 40, TFT_HEIGHT - 20);
}

void rfid_timer_callback(void *context)
{
    rfid_data_t tag_data;

    tag_data = rfid_read_tag(&m_twi_mngr);

    if (strcmp(last_displayed_tag, tag_data.tag) != 0)
    {
        printf("Tag Detected: %s, Timestamp: %lu ms\n", tag_data.tag, tag_data.time);
        strcpy(last_displayed_tag, tag_data.tag);
        process_rfid_tag(tag_data.tag);
    }
    else if (!is_displaying_tag)
    {
        printf("No tag detected or read failed.\n");
    }

    // Read and display the weight
    float weight = read_fsr_weight();
    if (weight > 2.9)
    {
        display_weight(weight);
    }

    rfid_clear_tags(&m_twi_mngr);
}

const tag_info_t *get_tag_info(const char *tag_id)
{
    for (size_t i = 0; i < tag_info_count; i++)
    {
        if (strcmp(tag_info_table[i].tag_id, tag_id) == 0)
        {
            return &tag_info_table[i];
        }
    }
    return NULL;
}

void process_rfid_tag(const char *tag_id)
{
    const tag_info_t *tag_info = get_tag_info(tag_id);

    if (tag_info)
    {

        if (strncmp(tag_id, ABBEY_ROAD, strlen(ABBEY_ROAD)) == 0)
        {
            display_vinyl_record(tag_info->person, tag_info->title, tag_info->field1, tag_info->field2, tag_info->field3, tag_info->genre, tag_info->year, tag_info->weight, 0xC5, 0xFB, 0xFF);
        }
        else if (strncmp(tag_id, PULP_FICTION, strlen(PULP_FICTION)) == 0)
        {
            display_vhs_movie(tag_info->person, tag_info->title, tag_info->field1, tag_info->field2, tag_info->field3, tag_info->genre, tag_info->year, tag_info->weight);
        }
        else if (strncmp(tag_id, IN_RAINBOWS, strlen(IN_RAINBOWS)) == 0)
        {
            display_vinyl_record(tag_info->person, tag_info->title, tag_info->field1, tag_info->field2, tag_info->field3, tag_info->genre, tag_info->year, tag_info->weight, 0xF7, 0xEE, 0x49);
        }
        is_displaying_tag = true;
        strcpy(last_displayed_tag, tag_id);
    }
    else
    {
        is_displaying_tag = false;
    }
}

int main(void)
{
    printf("Starting RFID and Display.\n");

    // Initialize TWI manager
    nrf_drv_twi_config_t twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    twi_config.scl = I2C_QWIIC_SCL;
    twi_config.sda = I2C_QWIIC_SDA;
    twi_config.frequency = NRF_TWIM_FREQ_100K;
    twi_config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;

    ret_code_t err_code = nrf_twi_mngr_init(&m_twi_mngr, &twi_config);
    APP_ERROR_CHECK(err_code);
    printf("TWI Manager initialized successfully.\n");

    saadc_init();

    // Initialize RFID
    rfid_init(&m_twi_mngr);
    rfid_scan_bus(&m_twi_mngr);

    // Initialize ILI9341 display
    ili9341_init();
    ili9341_fill_screen(0xFF, 0xFF, 0xFF);
    display_header("Welcome to RetroScan");

    // Start RFID polling timer
    app_timer_init();
    app_timer_create(&rfid_timer, APP_TIMER_MODE_REPEATED, rfid_timer_callback);
    app_timer_start(rfid_timer, POLLING_INTERVAL, NULL);

    // Main loop
    while (1)
    {
        nrf_delay_ms(1000);
    }

    return 0;
}
