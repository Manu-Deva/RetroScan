#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include "nrf_twi_mngr.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Constants
#define SPARKFUN_RFID_ADDR 0x3E  // 7-bit I2C address
#define RFID_VERSION_REG   0x00  // Example address for version register
#define RFID_STATUS_REG    0x01  // Example address for status register

// Function declarations
void rfid_init(nrf_twi_mngr_t const *twi_mngr);
void rfid_scan_bus(nrf_twi_mngr_t const *twi_mngr);
bool rfid_read_register(nrf_twi_mngr_t const *twi_mngr, uint8_t reg, uint8_t *data);
bool rfid_write_register(nrf_twi_mngr_t const *twi_mngr, uint8_t reg, uint8_t data);
bool rfid_check_tag_present(nrf_twi_mngr_t const *twi_mngr);
#endif // RFID_DRIVER_H
