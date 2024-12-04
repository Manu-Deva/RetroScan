#include <stdio.h>
#include "nrf_drv_twi.h"
#include "nrf_twi_mngr.h"
#include "rfid_driver.h"
#include "ili9341.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "microbit_v2.h"

#define POLLING_INTERVAL APP_TIMER_TICKS(250) // Poll every 500ms

// TWI Manager instance
NRF_TWI_MNGR_DEF(m_twi_mngr, 1, 0);
APP_TIMER_DEF(rfid_timer);

#define VINYL_TAG "3A006C84D200" // Example RFID tag for vinyl records
#define VHS_TAG "3A006C762F0F"    // Example RFID tag for VHS tapes

// Function prototypes
void rfid_timer_callback(void *context);
void process_rfid_tag(const char *tag_id);

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

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

void display_vhs_movie(const char *director, const char *title, const char *actor1, const char *actor2, const char *actor3, const char *genre, const char *year, const char *vhs_weight)
{
    ili9341_fill_screen(0xFF, 0xFF, 0xFF);

    // Display the header in red
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

    // Display Weight
    snprintf(buffer, sizeof(buffer), "Weight: %s lbs", vhs_weight);
    x = calculate_right_aligned_x(buffer, 1);
    ili9341_draw_string(x, 270, buffer, 1, 0x00, 0x00, 0x00);

    // Draw VHS icon in the bottom right
    draw_vhs_icon(TFT_WIDTH - 40, TFT_HEIGHT - 20);
}

void rfid_timer_callback(void *context)
{
    rfid_data_t tag_data = rfid_read_tag(&m_twi_mngr);

    if (tag_data.tag[0] != '\0')
    {
        printf("Tag Detected: %s, Timestamp: %lu ms\n", tag_data.tag, tag_data.time);

        // Process the tag to update the display
        process_rfid_tag(tag_data.tag);
    }
    else
    {
        printf("No tag detected or read failed.\n");
    }

    rfid_clear_tags(&m_twi_mngr);
}

void process_rfid_tag(const char *tag_id)
{
    // Example data for vinyl record
    const char *artist = "The Beatles";
    const char *title = "Abbey Road";
    const char *song1 = "Come Together";
    const char *song2 = "Something";
    const char *song3 = "Octopus Garden";
    const char *genre = "Rock";
    const char *year = "1969";
    const char *vinyl_weight = "0.3";

    // Example data for VHS movie
    const char *director = "Tarantino";
    const char *movie_title = "Pulp Fiction";
    const char *actor1 = "John Travolta";
    const char *actor2 = "Uma Thurman";
    const char *actor3 = "Samuel Jackson";
    const char *movie_genre = "Crime";
    const char *movie_year = "1994";
    const char *vhs_weight = "1";

    // Check the tag ID and update the display accordingly
    if (strncmp(tag_id, VINYL_TAG, strlen(VINYL_TAG)) == 0)
    {
        // Display vinyl record info
        display_vinyl_record(artist, title, song1, song2, song3, genre, year, vinyl_weight);
    }
    else if (strncmp(tag_id, VHS_TAG, strlen(VHS_TAG)) == 0)
    {
        // Display VHS movie info
        display_vhs_movie(director, movie_title, actor1, actor2, actor3, movie_genre, movie_year, vhs_weight);
    }
    else
    {
        // Unknown tag: clear the display
        ili9341_fill_screen(0xFF, 0xFF, 0xFF); // Fill screen with white
        display_header("Unknown Tag");
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

    // Initialize RFID
    rfid_init(&m_twi_mngr);
    rfid_scan_bus(&m_twi_mngr);

    // Initialize ILI9341 display
    ili9341_init();

    // Start RFID polling timer
    app_timer_init();
    app_timer_create(&rfid_timer, APP_TIMER_MODE_REPEATED, rfid_timer_callback);
    app_timer_start(rfid_timer, POLLING_INTERVAL, NULL);

    // Main loop
    while (1)
    {
        nrf_delay_ms(1000); // Add delay for low-power consumption
    }

    return 0;
}
