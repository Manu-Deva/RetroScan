#include <stdio.h>
#include "nrf_drv_twi.h"
#include "nrf_twi_mngr.h"
#include "rfid_driver.h"
#include "nrf_delay.h"

// Define GPIO pins
#define TWI_SCL_PIN 19
#define TWI_SDA_PIN 20

// TWI Manager instance
NRF_TWI_MNGR_DEF(m_twi_mngr, 5, 0);  // Max 5 queued transfers

void twi_init(void) {
    nrf_drv_twi_config_t const config = {
        .scl                = TWI_SCL_PIN,
        .sda                = TWI_SDA_PIN,
        .frequency          = NRF_TWIM_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = false
    };

    ret_code_t err_code = nrf_twi_mngr_init(&m_twi_mngr, &config);
    APP_ERROR_CHECK(err_code);

    printf("TWI Manager initialized successfully\n");
}

int main(void) {
    printf("Starting RFID Project...\n");

    // Initialize TWI
    twi_init();

    // Scan the I2C bus
    rfid_scan_bus(&m_twi_mngr);

    // Initialize the RFID reader
    rfid_init(&m_twi_mngr);

    // Main loop
    while (1) {
        nrf_delay_ms(1000);
    }

    return 0;
}
