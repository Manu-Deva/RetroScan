#include "rfid_driver.h"
#include <stdio.h>
#include "nrf_delay.h"

// Initialize RFID by reading its version and status registers
void rfid_init(nrf_twi_mngr_t const *twi_mngr) {
    uint8_t version = 0;
    uint8_t status = 0;

    nrf_twi_mngr_transfer_t const transfers[] = {
        NRF_TWI_MNGR_WRITE(SPARKFUN_RFID_ADDR, (uint8_t[]){RFID_VERSION_REG}, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(SPARKFUN_RFID_ADDR, &version, 1, 0),
        NRF_TWI_MNGR_WRITE(SPARKFUN_RFID_ADDR, (uint8_t[]){RFID_STATUS_REG}, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(SPARKFUN_RFID_ADDR, &status, 1, 0),
    };

    ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, transfers, 4, NULL);
    if (err_code == NRF_SUCCESS) {
        printf("RFID Version: 0x%02X\n", version);
        printf("RFID Status: 0x%02X\n", status);
    } else {
        printf("RFID initialization failed, error: 0x%lX\n", err_code);
    }
}

// Scan I2C bus for devices
void rfid_scan_bus(nrf_twi_mngr_t const *twi_mngr) {
    printf("\nScanning I2C bus for devices...\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (uint8_t address = 1; address < 127; address++) {
        nrf_twi_mngr_transfer_t const transfer = NRF_TWI_MNGR_WRITE(address, NULL, 0, 0);
        ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, &transfer, 1, NULL);

        if (address % 16 == 0) {
            printf("\n%02X:", address & 0xF0);
        }

        if (err_code == NRF_SUCCESS) {
            printf(" %02X", address);
        } else {
            printf(" --");
        }
    }
    printf("\n\nScan complete!\n");
}

// Read a single byte from a register
bool rfid_read_register(nrf_twi_mngr_t const *twi_mngr, uint8_t reg, uint8_t *data) {
    nrf_twi_mngr_transfer_t const transfers[] = {
        NRF_TWI_MNGR_WRITE(SPARKFUN_RFID_ADDR, &reg, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(SPARKFUN_RFID_ADDR, data, 1, 0),
    };

    ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, transfers, 2, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("Read failed, error: 0x%lX\n", err_code);
        return false;
    }
    return true;
}

// Write a single byte to a register
bool rfid_write_register(nrf_twi_mngr_t const *twi_mngr, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    nrf_twi_mngr_transfer_t const transfer = NRF_TWI_MNGR_WRITE(SPARKFUN_RFID_ADDR, buffer, sizeof(buffer), 0);

    ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, &transfer, 1, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("Write failed, error: 0x%lX\n", err_code);
        return false;
    }
    return true;
}

// Function to check if a tag is present
bool rfid_check_tag_present(nrf_twi_mngr_t const *twi_mngr) {
    uint8_t status = 0;

    // Read the status register
    nrf_twi_mngr_transfer_t const transfers[] = {
        NRF_TWI_MNGR_WRITE(SPARKFUN_RFID_ADDR, (uint8_t[]){RFID_STATUS_REG}, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(SPARKFUN_RFID_ADDR, &status, 1, 0),
    };

    ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, transfers, 2, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("Failed to read status register, error: 0x%lX\n", err_code);
        return false;
    }

    // Check if the LSB (bit 0) indicates a tag is present
    return (status & 0x01) != 0;
}
