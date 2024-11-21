#include "rfid_driver.h"
#include <stdio.h>
#include <string.h>

// Pointer to the initialized TWI manager
static const nrf_twi_mngr_t *i2c_manager = NULL;

// Initialize the RFID reader
void rfid_init(const nrf_twi_mngr_t *manager)
{
    i2c_manager = manager;

    // Perform a WHO_AM_I check or similar (replace 0x00 and expected value)
    uint8_t who_am_i_reg = 0x00; // Adjust to actual WHO_AM_I register
    uint8_t who_am_i_val;

    nrf_twi_mngr_transfer_t const transfers[] = {
        NRF_TWI_MNGR_WRITE(RFID_I2C_ADDRESS, &who_am_i_reg, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(RFID_I2C_ADDRESS, &who_am_i_val, 1)};

    ret_code_t err_code = nrf_twi_mngr_perform(i2c_manager, NULL, transfers, 2, NULL);
    if (err_code != NRF_SUCCESS)
    {
        printf("RFID reader initialization failed: 0x%X\n", err_code);
    }
    else
    {
        printf("RFID reader WHO_AM_I: 0x%X\n", who_am_i_val);
    }
}

// Write data to a specific block on the RFID card
uint8_t rfid_write_block(uint8_t block, const uint8_t *data, size_t length)
{
    uint8_t tx_buf[17] = {block}; // First byte is the block number
    memcpy(&tx_buf[1], data, length);

    nrf_twi_mngr_transfer_t const transfer = NRF_TWI_MNGR_WRITE(RFID_I2C_ADDRESS, tx_buf, sizeof(tx_buf), 0);
    ret_code_t err_code = nrf_twi_mngr_perform(i2c_manager, NULL, &transfer, 1, NULL);

    if (err_code != NRF_SUCCESS)
    {
        printf("RFID Write Error: 0x%X\n", err_code);
        return 0;
    }
    return 1;
}

// Read data from a specific block on the RFID card
uint8_t rfid_read_block(uint8_t block, uint8_t *buffer, size_t length)
{
    nrf_twi_mngr_transfer_t const transfers[] = {
        NRF_TWI_MNGR_WRITE(RFID_I2C_ADDRESS, &block, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(RFID_I2C_ADDRESS, buffer, length)};

    ret_code_t err_code = nrf_twi_mngr_perform(i2c_manager, NULL, transfers, 2, NULL);

    if (err_code != NRF_SUCCESS)
    {
        printf("RFID Read Error: 0x%X\n", err_code);
        return 0;
    }
    return 1;
}
