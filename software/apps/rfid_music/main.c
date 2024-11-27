#include <stdio.h>
#include "nrf_drv_twi.h"
#include "nrf_twi_mngr.h"
#include "rfid_driver.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "microbit_v2.h"

#define POLLING_INTERVAL APP_TIMER_TICKS(500)  // Poll every 500ms

// TWI Manager instance
NRF_TWI_MNGR_DEF(m_twi_mngr, 1, 0);  
APP_TIMER_DEF(rfid_timer);

#define NRF_TWI_MNGR_ENABLED 1


// void twi_init(void) {
//     nrf_drv_twi_config_t config = NRF_DRV_TWI_DEFAULT_CONFIG;
	 
//     config.scl                = I2C_QWIIC_SCL;
//     config.sda                = I2C_QWIIC_SDA;
//     config.frequency          = NRF_TWIM_FREQ_100K;
//     config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;
//     config.clear_bus_init     = false;

//     ret_code_t err_code = nrf_twi_mngr_init(&m_twi_mngr, &config);
//     APP_ERROR_CHECK(err_code);

//     printf("TWI Manager initialized successfully\n");
// }

void rfid_timer_callback(void *context) {
    rfid_data_t tag_data; // Structure to hold the RFID tag ID and timestamp

    // Attempt to read an RFID tag
    tag_data = rfid_read_tag(&m_twi_mngr);
	rfid_check_tag_present(&m_twi_mngr);

    // Check if the tag data is valid
    if (tag_data.tag[0] != '\0') {
        // Print the tag information
        printf("Tag Detected: %s, Timestamp: %lu ms\n", tag_data.tag, tag_data.time);
    } else {
        // No tag detected
        printf("No tag detected or read failed.\n");
    }

	rfid_clear_tags(&m_twi_mngr);
}

int main(void) {

    printf("Starting RFID.\n");

    nrf_drv_twi_config_t config = NRF_DRV_TWI_DEFAULT_CONFIG;
	 
    config.scl                = I2C_QWIIC_SCL;
    config.sda                = I2C_QWIIC_SDA;
    config.frequency          = NRF_TWIM_FREQ_100K;
    config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;
    // config.clear_bus_init     = false;

    ret_code_t err_code = nrf_twi_mngr_init(&m_twi_mngr, &config);
    APP_ERROR_CHECK(err_code);
    printf("TWI Manager initialized successfully\n");

	rfid_init(&m_twi_mngr);
    rfid_scan_bus(&m_twi_mngr);
	
    app_timer_init();
    app_timer_create(&rfid_timer, APP_TIMER_MODE_REPEATED, rfid_timer_callback);
    app_timer_start(rfid_timer, POLLING_INTERVAL, NULL);

    while (1) {
        nrf_delay_ms(1000); // Poll every 500ms

    }

    return 0;

 
}
