#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include "nrf_twi_mngr.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Constants
#define SPARKFUN_RFID_ADDR 0x7D
#define DEFAULT_ADDR 0x7D

#define TAG_ID_SIZE 16      // Total tag ID size (adjust as necessary)
#define RFID_DATA_LENGTH 10 // 10 ASCII characters for the tag ID
#define CHECKSUM_LENGTH 2   // 2 ASCII characters for the checksum

#define STATUS_REG 0xFF     // Status register address
#define TAG_STATUS_REG 0x00 // Register for checking tag presence
#define TAG_LENGTH_REG 0x01 // Tag length register
#define TAG_DATA_REG 0x02   // Start of tag data registers
#define END_BYTE 0x03       // ETX byte indicating the end of the data

#define TAG_AND_TIME_REQUEST 10
#define MAX_TAG_STORAGE 20
#define BYTES_IN_BUFFER 4

typedef struct
{
    char tag[13];  // 6-character tag + null terminator
    uint32_t time; // Timestamp in milliseconds
} rfid_data_t;

// Function declarations
void rfid_init(nrf_twi_mngr_t const *twi_mngr);
void rfid_scan_bus(nrf_twi_mngr_t const *twi_mngr);
bool rfid_begin(nrf_twi_mngr_t const *twi_mngr);
rfid_data_t rfid_read_tag(nrf_twi_mngr_t const *twi_mngr);
void rfid_clear_tags(nrf_twi_mngr_t const *twi_mngr);
static uint8_t rfid_read_register(uint8_t i2c_addr, uint8_t reg_addr);
static void rfid_write_register(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data);
void rfid_check_tag_present(nrf_twi_mngr_t const *twi_mngr);

#endif