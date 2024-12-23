#ifndef ILI9341_H
#define ILI9341_H

#include <stdint.h>

// Function prototypes
void ili9341_init(void);
void ili9341_fill_screen(uint8_t red, uint8_t green, uint8_t blue);
void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint8_t scale, uint8_t r, uint8_t g, uint8_t b);
void ili9341_draw_string(uint16_t x, uint16_t y, const char *str, uint8_t scale, uint8_t r, uint8_t g, uint8_t b);
void draw_vhs_icon(uint16_t x, uint16_t y);
void draw_vinyl_icon(uint16_t x, uint16_t y);
static const uint8_t font8x8_basic[128][8] = {
    // Space (ASCII 32)
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    // Colon (ASCII 58)
    [58] = {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00},

    // .
    [46] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},

    // Uppercase Letters (ASCII 65-90, 'A'-'Z')
    [65] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66}, // A
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x66, 0x7C}, // B
    [67] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x60, 0x66, 0x3C}, // C
    [68] = {0x7C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7C}, // D
    [69] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x7E}, // E
    [70] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60}, // F
    [71] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x66, 0x3C}, // G
    [72] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66}, // H
    [73] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C}, // I
    [74] = {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38}, // J
    [75] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x66}, // K
    [76] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E}, // L
    [77] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x63}, // M
    [78] = {0x66, 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66}, // N
    [79] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C}, // O
    [80] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x60}, // P
    [81] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06}, // Q
    [82] = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x66}, // R
    [83] = {0x3E, 0x60, 0x60, 0x3C, 0x06, 0x06, 0x66, 0x3C}, // S
    [84] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // T
    [85] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C}, // U
    [86] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18}, // V
    [87] = {0x63, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63}, // W
    [88] = {0x66, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x66, 0x66}, // X
    [89] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18}, // Y
    [90] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x7E}, // Z

    // Lowercase Letters (ASCII 97-122, 'a'-'z')
    [97] = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x66, 0x3E},  // a
    [98] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x7C},  // b
    [99] = {0x00, 0x00, 0x3C, 0x66, 0x60, 0x60, 0x66, 0x3C},  // c
    [100] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x3E}, // d
    [101] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x66, 0x3C}, // e
    [102] = {0x0E, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x18}, // f
    [103] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x06}, // g
    [104] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x66}, // h
    [105] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C}, // i
    [106] = {0x06, 0x00, 0x0E, 0x06, 0x06, 0x06, 0x06, 0x3C}, // j
    [107] = {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x66}, // k
    [108] = {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C}, // l
    [109] = {0x00, 0x00, 0x6C, 0x7E, 0x7E, 0x6B, 0x63, 0x63}, // m
    [110] = {0x00, 0x00, 0x5C, 0x66, 0x66, 0x66, 0x66, 0x66}, // n
    [111] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C}, // o
    [112] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60}, // p
    [113] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x06}, // q
    [114] = {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x60}, // r
    [115] = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x06, 0x7C}, // s
    [116] = {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x0E}, // t
    [117] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E}, // u
    [118] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18}, // v
    [119] = {0x00, 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7E, 0x36}, // w
    [120] = {0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66}, // x
    [121] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x06}, // y
    [122] = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x60, 0x7E}, // z

    // Numbers (ASCII 48-57)
    [48] = {0x3C, 0x66, 0x6E, 0x76, 0x7E, 0x66, 0x66, 0x3C}, // 0
    [49] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C}, // 1
    [50] = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E}, // 2
    [51] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x06, 0x66, 0x3C}, // 3
    [52] = {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x0C}, // 4
    [53] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x06, 0x66, 0x3C}, // 5
    [54] = {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C}, // 6
    [55] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30}, // 7
    [56] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x66, 0x3C}, // 8
    [57] = {0x3C, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C}, // 9
};

#endif

