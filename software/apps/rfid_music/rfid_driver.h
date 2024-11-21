#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include "nrf_twi_mngr.h"
#include <stdint.h>
#include <stddef.h>

// RFID Reader I2C Address
#define RFID_I2C_ADDRESS 0x28

// Function Prototypes
void rfid_init(const nrf_twi_mngr_t *i2c_manager);
uint8_t rfid_write_block(uint8_t block, const uint8_t *data, size_t length);
uint8_t rfid_read_block(uint8_t block, uint8_t *buffer, size_t length);

#endif // RFID_DRIVER_H
