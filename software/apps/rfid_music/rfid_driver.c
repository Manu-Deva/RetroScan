#include "rfid_driver.h"
#include <stdio.h>
#include "nrf_delay.h"

// Initialize the I2C address
uint8_t rfid_address = DEFAULT_ADDR;
static const nrf_twi_mngr_t* i2c_manager = NULL;

// Initialize RFID by reading its version and status registers
void rfid_init(nrf_twi_mngr_t const *twi_mngr) {
	i2c_manager = twi_mngr;
	rfid_write_register(SPARKFUN_RFID_ADDR, TAG_STATUS_REG, 1);
	rfid_write_register(SPARKFUN_RFID_ADDR, STATUS_REG, 1);

}

// Scan I2C bus for devices
void rfid_scan_bus(nrf_twi_mngr_t const *twi_mngr) {
    printf("\nScanning I2C bus for devices...\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

	uint8_t dummy_data = 0x1;

	for (uint8_t address = 0; address < 127; address++) {
        nrf_twi_mngr_transfer_t transfer = NRF_TWI_MNGR_WRITE(address, &dummy_data, 1, 0);
        ret_code_t err_code = nrf_twi_mngr_perform(twi_mngr, NULL, &transfer, 1, NULL);

        if (err_code == NRF_SUCCESS) {
            printf(" RFID sensor located at address of %02X\n", address);
		}
    }
	
    printf("\n\nScan complete!\n");
}

// // Helper function to convert a nibble (4 bits) to its ASCII character
// char nibble_to_ascii(uint8_t nibble) {
//     static const char hex_chars[] = "0123456789ABCDEF";
//     return hex_chars[nibble & 0x0F];
// }

rfid_data_t rfid_read_tag(nrf_twi_mngr_t const *twi_mngr) {
    rfid_data_t rfid_data = { .tag = {0}, .time = 0 };
    uint8_t request_bytes = TAG_AND_TIME_REQUEST;
    uint8_t buffer[TAG_AND_TIME_REQUEST] = {0};

    // Request data from the RFID reader
    nrf_twi_mngr_transfer_t transfer[] = {
        NRF_TWI_MNGR_READ(rfid_address, &buffer, TAG_AND_TIME_REQUEST, 0),
    };

    ret_code_t err_code = nrf_twi_mngr_perform(i2c_manager, NULL, transfer, 1	, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("Failed to read tag data. Error: 0x%lX\n", err_code);
        return rfid_data;
    }
	
	// Log raw data for debugging
    printf("Raw RFID Reader Data: ");
    for (int i = 0; i < TAG_AND_TIME_REQUEST; i++) {
        printf("0x%02X ", buffer[i]);
    }
    printf("\n");

	// Decode tag data as hexadecimal
    char decoded_tag[TAG_AND_TIME_REQUEST * 2 + 1] = {0}; // Buffer for decoded hex string (+1 for null terminator)
    for (int i = 0; i < 6; i++) { // Adjust based on the tag data length
        char hex_byte[3];
        snprintf(hex_byte, sizeof(hex_byte), "%02X", buffer[i]);
        strcat(decoded_tag, hex_byte);
    }

    // Store the decoded tag in rfid_data structure
    strncpy(rfid_data.tag, decoded_tag, sizeof(rfid_data.tag) - 1);

	// // Convert each individual hex digit back to ASCII
    // // Assuming the tag data is in the first 5 bytes of the buffer
    // int tag_bytes = 6; // Adjust based on your actual tag data length
    // int decoded_tag_length = tag_bytes * 2;
    // char decoded_tag[decoded_tag_length + 1]; // +1 for null terminator
    // for (int i = 0; i < tag_bytes; i++) {
    //     uint8_t byte = buffer[i];
    //     // High nibble
    //     decoded_tag[2 * i] = nibble_to_ascii(byte >> 4);
    //     // Low nibble
    //     decoded_tag[2 * i + 1] = nibble_to_ascii(byte);
    // }
    // decoded_tag[decoded_tag_length] = '\0'; // Null-terminate the string

    // // Store the decoded tag in rfid_data structure
    // strncpy(rfid_data.tag, decoded_tag, sizeof(rfid_data.tag) - 1);
    // rfid_data.tag[sizeof(rfid_data.tag) - 1] = '\0'; // Ensure null term
	
    // Parse the timestamp
    rfid_data.time = (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | buffer[9];

	// printf("RFID Data: \n");
    // printf("  Tag: %s\n", rfid_data.tag);
    // printf("  Time: %lu\n", rfid_data.time);

    return rfid_data;
}

void rfid_clear_tags(nrf_twi_mngr_t const *twi_mngr) {
    uint8_t request_bytes = TAG_AND_TIME_REQUEST;
    uint8_t buffer[MAX_TAG_STORAGE * TAG_AND_TIME_REQUEST] = {0};

    nrf_twi_mngr_transfer_t transfer[] = {
        NRF_TWI_MNGR_WRITE(rfid_address, &request_bytes, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(rfid_address, buffer, sizeof(buffer), 0),
    };

    ret_code_t err_code = nrf_twi_mngr_perform(i2c_manager, NULL, transfer, 2, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("Failed to clear tags. Error: 0x%lX\n", err_code);
    } else {
        printf("Tag buffer cleared.\n");
    }
}

// Read a single byte from a register
static uint8_t rfid_read_register(uint8_t i2c_addr, uint8_t reg_addr) {
	uint8_t rx_buf = 0;
    nrf_twi_mngr_transfer_t const read_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
        NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0),
    };

    ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
    if (result != NRF_SUCCESS) {
        printf("I2C transaction failed! Error: %lX\n", result);
    }
	return rx_buf;
}

// Write a single byte to a register
static void rfid_write_register(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
    uint8_t buffer[2] = {reg_addr, data};
    nrf_twi_mngr_transfer_t const write_transfer[] = {NRF_TWI_MNGR_WRITE(i2c_addr, buffer, sizeof(buffer), 0)};

    ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
    if (result != NRF_SUCCESS) {
        printf("I2C write failed! Error: %lX\n", result);
    }
}


// Function to check if a tag is present
void rfid_check_tag_present(nrf_twi_mngr_t const *twi_mngr) {
	uint8_t tag_present = rfid_read_register(SPARKFUN_RFID_ADDR, TAG_DATA_REG);
	printf("tag presence value of %x\n", tag_present);
}







