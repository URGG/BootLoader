
#define VGA_MEMORY 0xB8000;
#define VGA_WIDTH 80;
#define VGA_HEIGHT 25;


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
    DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, PINK = 13,
    YELLOW = 14, WHITE = 15
};

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)'' | ((uint16_t) BLACK << 12) | ((uint16_t) LIGHT_GRAY << 8);
    }
    cursor_x = 0;
    cursor_y = 0;
}

void putchar(char c, uint8_t fg, uint8_t bg) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            // Scroll screen
            for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
                vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
            }
            for (int i = 0; i < VGA_WIDTH; i++) {
                vga_buffer[VGA_WIDTH * (VGA_HEIGHT - 1) + i] = ' ' | ((uint16_t)bg << 12) | ((uint16_t)fg << 8);
            }
        }
        return;
    }