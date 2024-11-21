#include "nrf_drv_twi.h"
#include "rfid_driver.h"
#include <stdio.h>

// TWI (I2C) instance ID
#define TWI_INSTANCE_ID 0
#ifndef NRF_GPIO_PIN_MAP
#define NRF_GPIO_PIN_MAP(port, pin) ((port << 5) | (pin & 0x1F))
#endif

// TWI instance
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

// TWI initialization
void twi_init(void)
{
    ret_code_t err_code;

    // Configure TWI
    nrf_drv_twi_config_t config = {
        .scl = NRF_GPIO_PIN_MAP(0, 27), // Replace with 27 if NRF_GPIO_PIN_MAP is undefined
        .sda = NRF_GPIO_PIN_MAP(0, 26), // Replace with 26 if NRF_GPIO_PIN_MAP is undefined
        .frequency = NRF_DRV_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init = false};

    // Initialize TWI
    err_code = nrf_drv_twi_init(&m_twi, &config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Enable TWI
    nrf_drv_twi_enable(&m_twi);
}

int main(void)
{
    printf("Starting RFID Project...\n");

    // Initialize TWI
    twi_init();

    // Initialize RFID reader
    rfid_init(&m_twi);

    // Write data to block 1
    uint8_t write_data[16] = "Hello, RFID!";
    if (rfid_write_block(1, write_data, sizeof(write_data)))
    {
        printf("Data written successfully.\n");
    }

    // Read data from block 1
    uint8_t read_data[16] = {0};
    if (rfid_read_block(1, read_data, sizeof(read_data)))
    {
        printf("Data read from block 1: %s\n", read_data);
    }

    return 0;
}
