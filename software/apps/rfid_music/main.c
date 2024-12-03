#include <stdio.h>
#include "nrf_drv_twi.h"
#include "nrf_twi_mngr.h"
#include "rfid_driver.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "microbit_v2.h"
#include "ili9341.h"

#define POLLING_INTERVAL APP_TIMER_TICKS(500)  // Poll every 500ms

// TWI Manager instance
NRF_TWI_MNGR_DEF(m_twi_mngr, 1, 0);
APP_TIMER_DEF(rfid_timer);

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

typedef struct {
    const char *tag_id;         // RFID tag ID
    const char *type;           // Type of item (e.g., "Vinyl", "VHS")
	const char *artist;
    const char *title;          // Title or description
	const char* song1;
	const char* song2;
	const char* song3;
	const char *genre;
	const char *year;
	const char *sensed_weight
} tag_info_t;

// Lookup table for tag information
static const tag_info_t tag_info_table[] = {
    { "3A006C84D200", "Vinyl", "The Beatles", "Abbey Road", "Come Together", "Something", "Octopus Garden", "Rock", "1969", "0.3"},
    { "3A006C762F0F", "VHS", "Pulp Fiction", "Director: Tarantino" },
    { "3A006C6AE3DF", "Vinyl", "Dark Side of the Moon", "Artist: Pink Floyd" },
    { "3A006C7C8EA4", "VHS", "The Matrix", "Director: Wachowski" },
    // Add more entries as needed
};
static const size_t tag_info_count = sizeof(tag_info_table) / sizeof(tag_info_table[0]);

// Function to calculate the center-aligned X coordinate
uint16_t calculate_center_aligned_x(const char *text, uint8_t scale)
{
    size_t length = 0;

    // Calculate the string length
    while (text[length] != '\0')
    {
        length++;
    }

    // Each character is 8 pixels wide (font size), scaled by `scale`
    uint16_t text_width = length * 8 * scale;

    // Return the X position to center-align the text
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

    // Each character is 8 pixels wide (font size), scaled by `scale`
    uint16_t text_width = length * 8 * scale;

    // Return the X position to right-align the text
    return TFT_WIDTH - text_width - 35; // Subtract 35 for padding
}

// Function to display a header at the top of the screen in red
void display_header(const char *header)
{
    uint16_t x = calculate_center_aligned_x(header, 1);      // Center-align the header
    ili9341_draw_string(x, 20, header, 1, 0xFF, 0x00, 0x00); // Red color
}

void display_vinyl_record(const char *artist, const char *title, const char *song1, const char *song2, const char *song3, const char *genre, const char *year, const char *vinyl_weight)
{
    ili9341_fill_screen(0xFF, 0xFF, 0xFF);

    // Display the header in red
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

    // Display Weight
    snprintf(buffer, sizeof(buffer), "Weight: %s lbs", vinyl_weight);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 270, buffer, 1, 0x00, 0x00, 0x00);

    // Draw vinyl icon in the bottom right
    draw_vinyl_icon(TFT_WIDTH - 30, TFT_HEIGHT - 30);
}

const tag_info_t* get_tag_info(const char *tag_id) {
    for (size_t i = 0; i < tag_info_count; i++) {
        if (strcmp(tag_info_table[i].tag_id, tag_id) == 0) {
            return &tag_info_table[i]; // Return pointer to matching entry
        }
    }
    return NULL; // Return NULL if no match is found
}

// Display RFID tag information on the screen
void display_rfid_tag_info(const char *tag_id) {
    const tag_info_t *tag_info = get_tag_info(tag_id);

    // ili9341_fill_screen(0xFF, 0xFF, 0xFF); // Clear screen to white

    if (tag_info) {
		display_vinyl_record(tag_info->artist, tag_info->title, tag_info->song1,tag_info->song2,tag_info->song3,tag_info->genre,tag_info->year, tag_info->sensed_weight);
       
    } else {
        // Display "unknown tag" message
        ili9341_draw_string(10, 50, "Unknown Tag", 1, 0x00, 0x00, 0x00);
    }
}

void rfid_timer_callback(void *context) {
    rfid_data_t tag_data; // Structure to hold the RFID tag ID and timestamp

    // Attempt to read an RFID tag
    tag_data = rfid_read_tag(&m_twi_mngr);

    if (tag_data.tag[0] != '\0') {
        // Print the tag information
        printf("Tag Detected: %s, Timestamp: %lu ms\n", tag_data.tag, tag_data.time);

        // Display tag data on the screen
        // ili9341_draw_string(10, 10, "RFID Tag Detected", 1, 0x00, 0x00, 0x00);
        // ili9341_draw_string(10, 30, tag_data.tag, 1, 0x00, 0x00, 0x00);

        // Example: Based on tag ID, show additional info
        if (strcmp(tag_data.tag, "3A006C84D200") == 0) {
			display_rfid_tag_info(tag_data.tag);
        } else if (strcmp(tag_data.tag, "789012") == 0) {
            ili9341_draw_string(10, 50, "Type: VHS", 1, 0x00, 0x00, 0x00);
            // ili9341_draw_string(10, 70, "Title: Pulp Fiction", 1, 0x00, 0x00, 0x00);
        }
    } else {
        // No tag detected
        printf("No tag detected or read failed.\n");
    }

    rfid_clear_tags(&m_twi_mngr); // Clear RFID tag buffer
}


int main(void) {
    printf("Starting application.\n");

    // Initialize TWI Manager
    nrf_drv_twi_config_t twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    twi_config.scl = I2C_QWIIC_SCL;
    twi_config.sda = I2C_QWIIC_SDA;
    twi_config.frequency = NRF_TWIM_FREQ_100K;

    ret_code_t err_code = nrf_twi_mngr_init(&m_twi_mngr, &twi_config);
    APP_ERROR_CHECK(err_code);
    printf("TWI Manager initialized successfully\n");

    // Initialize RFID
    rfid_init(&m_twi_mngr);
	printf("got to rfid init\n");

    // Initialize Display
    ili9341_init();
	printf("got to display init\n");

    // App timer setup
    app_timer_init();
	printf("got to app timer init\n");
    app_timer_create(&rfid_timer, APP_TIMER_MODE_REPEATED, rfid_timer_callback);
    app_timer_start(rfid_timer, POLLING_INTERVAL, NULL);

    while (1) {
        nrf_delay_ms(1000);
    }

    return 0;
}
