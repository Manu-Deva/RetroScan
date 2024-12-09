#include "ili9341.h"
#include <nrf_delay.h>
#include <nrfx_spim.h>
#include <nrf_gpio.h>
#include "microbit_v2.h"

static const nrfx_spim_t SPIM_INST = NRFX_SPIM_INSTANCE(2);

#define TFT_SCK EDGE_P13  // SPI clock
#define TFT_MOSI EDGE_P15 // SPI MOSI
#define TFT_CS EDGE_P12   // Chip select
#define TFT_DC EDGE_P8    // Data Command

// ILI9341 Commands
#define ILI9341_SWRESET 0x01 // Software reset
#define ILI9341_SLPOUT 0x11  // Sleep out
#define ILI9341_DISPON 0x29  // Display ON
#define ILI9341_CASET 0x2A   // Column address set
#define ILI9341_PASET 0x2B   // Page address set
#define ILI9341_RAMWR 0x2C   // Memory write
#define ILI9341_MADCTL 0x36  // Memory data access control
#define ILI9341_PIXFMT 0x3A  // Pixel format

#define ILI9341_PWCTR1 0xC0   ///< Power Control 1
#define ILI9341_PWCTR2 0xC1   ///< Power Control 2
#define ILI9341_PWCTR3 0xC2   ///< Power Control 3
#define ILI9341_PWCTR4 0xC3   ///< Power Control 4
#define ILI9341_PWCTR5 0xC4   ///< Power Control 5
#define ILI9341_VMCTR1 0xC5   ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7   ///< VCOM Control 2
#define ILI9341_GMCTRP1 0xE0  ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1  ///< Negative Gamma Correction
#define ILI9341_FRMCTR1 0xB1  ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_DFUNCTR 0xB6  ///< Display Function Control
#define ILI9341_GAMMASET 0x26 ///< Gamma Set

// TFT Dimensions
#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// Initialization commands
static const uint8_t initcmd[] = {
    0xEF, 3, 0x03, 0x80, 0x02,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    ILI9341_PWCTR1, 1, 0x23,
    ILI9341_PWCTR2, 1, 0x10,
    ILI9341_VMCTR1, 2, 0x3e, 0x28,
    ILI9341_VMCTR2, 1, 0x86,
    ILI9341_MADCTL, 1, 0x48,
    ILI9341_PIXFMT, 1, 0x55,
    ILI9341_FRMCTR1, 2, 0x00, 0x18,
    ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27,
    0xF2, 1, 0x00,
    ILI9341_GAMMASET, 1, 0x01,
    ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    ILI9341_SLPOUT, 0x80,
    ILI9341_DISPON, 0x80,
    0x00};

// Send a command to the display
static void send_command(uint8_t cmd)
{
    nrf_gpio_pin_clear(TFT_DC); // DC low for command
    nrf_gpio_pin_clear(TFT_CS); // CS low to select the screen

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(&cmd, 1);
    nrfx_spim_xfer(&SPIM_INST, &xfer_desc, 0);

    nrf_gpio_pin_set(TFT_CS); // CS high to deselect
}

// Send data to the display
static void send_data(uint8_t *data, size_t len)
{
    nrf_gpio_pin_set(TFT_DC);   // DC high for data
    nrf_gpio_pin_clear(TFT_CS); // CS low to select the screen

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(data, len);
    nrfx_spim_xfer(&SPIM_INST, &xfer_desc, 0);

    nrf_gpio_pin_set(TFT_CS); // CS high to deselect
}

// Initialize the ILI9341 display using initcmd array
void ili9341_init(void)
{
    // Configure GPIO
    nrf_gpio_cfg_output(TFT_CS);
    nrf_gpio_cfg_output(TFT_DC);

    // Initialize SPI
    nrfx_spim_config_t spim_config = NRFX_SPIM_DEFAULT_CONFIG;
    spim_config.sck_pin = TFT_SCK;
    spim_config.mosi_pin = TFT_MOSI;
    spim_config.miso_pin = EDGE_P14;
    spim_config.frequency = NRF_SPIM_FREQ_8M;
    spim_config.mode = NRF_SPIM_MODE_0;
    nrfx_spim_init(&SPIM_INST, &spim_config, NULL, NULL);

    // Perform software reset
    send_command(ILI9341_SWRESET);
    nrf_delay_ms(150); // Allow time for reset

    // Send initialization commands from initcmd array
    const uint8_t *addr = initcmd;
    uint8_t cmd, x, numArgs;
    while ((cmd = *addr++) > 0)
    {
        x = *addr++;
        numArgs = x & 0x7F;
        send_command(cmd);
        if (numArgs)
        {
            send_data((uint8_t *)addr, numArgs);
            addr += numArgs;
        }
        if (x & 0x80)
        {
            nrf_delay_ms(150);
        }
    }
}

// Fill the screen with a solid color
void ili9341_fill_screen(uint8_t r, uint8_t g, uint8_t b)
{
    // Set full screen address window
    send_command(ILI9341_CASET);
    uint8_t caset_data[] = {
        0x00, 0x00,
        (TFT_WIDTH - 1) >> 8,
        (TFT_WIDTH - 1) & 0xFF};
    send_data(caset_data, 4);

    send_command(ILI9341_PASET);
    uint8_t paset_data[] = {
        0x00, 0x00,
        (TFT_HEIGHT - 1) >> 8,
        (TFT_HEIGHT - 1) & 0xFF};
    send_data(paset_data, 4);

    // Start memory write
    send_command(ILI9341_RAMWR);

    // Create a buffer for multiple pixels
    uint8_t pixel_buffer[3];

    pixel_buffer[0] = r;
    pixel_buffer[1] = g;
    pixel_buffer[2] = b;

    // Send pixels in chunks
    for (uint32_t i = 0; i < (TFT_WIDTH * TFT_HEIGHT); i += 1)
    {
        send_data(pixel_buffer, 3);
    }
}

// Set the address window for the drawing area
void set_address_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x2 = x + w - 1;
    uint16_t y2 = y + h - 1;

    // Column address set
    send_command(ILI9341_CASET);
    uint8_t caset_data[] = {
        (x >> 8) & 0xFF, x & 0xFF,    // Start column
        (x2 >> 8) & 0xFF, x2 & 0xFF}; // End column
    send_data(caset_data, 4);

    // Row address set
    send_command(ILI9341_PASET);
    uint8_t paset_data[] = {
        (y >> 8) & 0xFF, y & 0xFF,    // Start row
        (y2 >> 8) & 0xFF, y2 & 0xFF}; // End row
    send_data(paset_data, 4);

    // Write to RAM
    send_command(ILI9341_RAMWR);
}

// Function to reverse the bit order in a byte
static uint8_t reverse_bits(uint8_t byte)
{
    byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
    byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
    byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);
    return byte;
}

// Draw a single character at a specific position with color
void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint8_t scale, uint8_t r, uint8_t g, uint8_t b)
{
    if ((unsigned char)c > 127)
        return; // Ensure character is valid

    const uint8_t *bitmap = font8x8_basic[(unsigned char)c];

    uint16_t char_width = 8 * scale;
    uint16_t char_height = 8 * scale;

    // Clear the character background (white)
    set_address_window(x, y, char_width, char_height);
    uint8_t background_pixel[3] = {0xFF, 0xFF, 0xFF}; // White pixel
    for (uint16_t i = 0; i < char_width * char_height; i++)
    {
        send_data(background_pixel, 3);
    }

    // Draw the character with the specified color
    for (uint8_t row = 0; row < 8; row++)
    {
        uint8_t reversed_row = reverse_bits(bitmap[row]); // Reverse the bits for correct rendering

        for (uint8_t col = 0; col < 8; col++)
        {
            if ((reversed_row >> (7 - col)) & 0x1)
            {
                set_address_window(x + col * scale, y + row * scale, scale, scale);
                uint8_t foreground_pixel[3] = {r, g, b}; // Custom color pixel
                for (uint16_t i = 0; i < scale * scale; i++)
                {
                    send_data(foreground_pixel, 3);
                }
            }
        }
    }
}

void ili9341_draw_string(uint16_t x, uint16_t y, const char *str, uint8_t scale, uint8_t r, uint8_t g, uint8_t b)
{

    size_t len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    for (size_t i = 0; i < len; i++)
    {
        ili9341_draw_char(x, y, str[len - 1 - i], scale, r, g, b);
        x += 8 * scale;
    }
}

// Function to draw a circle
void draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t r, uint8_t g, uint8_t b)
{
    for (int16_t dy = -radius; dy <= radius; dy++)
    {
        for (int16_t dx = -radius; dx <= radius; dx++)
        {
            if (dx * dx + dy * dy <= radius * radius)
            {
                set_address_window(x + dx, y + dy, 1, 1);
                uint8_t pixel_color[3] = {r, g, b}; // Custom color
                send_data(pixel_color, 3);
            }
        }
    }
}

// Function to draw a filled rectangle
void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b)
{
    set_address_window(x, y, w, h);
    uint8_t pixel_color[3] = {r, g, b}; // Custom color
    for (uint32_t i = 0; i < w * h; i++)
    {
        send_data(pixel_color, 3);
    }
}

void draw_vinyl_icon(uint16_t x, uint16_t y)
{
    // Calculate position to be in the bottom-right corner
    uint16_t center_x = 35;
    uint16_t center_y = TFT_HEIGHT - 30;

    // Outer circle (black)
    draw_circle(center_x, center_y, 20, 0x00, 0x00, 0x00);
    // Inner circle (white)
    draw_circle(center_x, center_y, 10, 0x80, 0x80, 0x80);
    draw_circle(center_x, center_y, 5, 0xFF, 0xFF, 0xFF);
}

void draw_vhs_icon(uint16_t x, uint16_t y)
{
    // Calculate position to be in the bottom-right corner
    uint16_t rect_x = 20;
    uint16_t rect_y = TFT_HEIGHT - 40;

    // Outer rectangle (black)
    draw_rectangle(rect_x, rect_y, 60, 25, 0x00, 0x00, 0x00);

    // Left reel (white circle)
    draw_circle(rect_x + 15, rect_y + 12.5, 6, 0xFF, 0xFF, 0xFF);

    // Right reel (white circle)
    draw_circle(rect_x + 45, rect_y + 12.5, 6, 0xFF, 0xFF, 0xFF);
}