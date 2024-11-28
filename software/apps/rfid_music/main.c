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

void rfid_timer_callback(void *context) {
    rfid_data_t tag_data; // Structure to hold the RFID tag ID and timestamp

    // Attempt to read an RFID tag
    tag_data = rfid_read_tag(&m_twi_mngr);

    if (tag_data.tag[0] != '\0') {
        // Print the tag information
        printf("Tag Detected: %s, Timestamp: %lu ms\n", tag_data.tag, tag_data.time);

        // Display tag data on the screen
        ili9341_fill_screen(0xFF, 0xFF, 0xFF); // Clear the screen to white
        ili9341_draw_string(10, 10, "RFID Tag Detected", 1, 0x00, 0x00, 0x00);
        ili9341_draw_string(10, 30, tag_data.tag, 1, 0x00, 0x00, 0x00);

        // Example: Based on tag ID, show additional info
        if (strcmp(tag_data.tag, "123456") == 0) {
            ili9341_draw_string(10, 50, "Type: Vinyl", 1, 0x00, 0x00, 0x00);
            ili9341_draw_string(10, 70, "Artist: The Beatles", 1, 0x00, 0x00, 0x00);
        } else if (strcmp(tag_data.tag, "789012") == 0) {
            ili9341_draw_string(10, 50, "Type: VHS", 1, 0x00, 0x00, 0x00);
            ili9341_draw_string(10, 70, "Title: Pulp Fiction", 1, 0x00, 0x00, 0x00);
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

    // Initialize Display
    ili9341_init();

    // App timer setup
    app_timer_init();
    app_timer_create(&rfid_timer, APP_TIMER_MODE_REPEATED, rfid_timer_callback);
    app_timer_start(rfid_timer, POLLING_INTERVAL, NULL);

    while (1) {
        nrf_delay_ms(1000);
    }

    return 0;
}
