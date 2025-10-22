#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

static uint16_t *vga_buffer = (uint16_t*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

// VGA color codes
enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
    DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, PINK = 13,
    YELLOW = 14, WHITE = 15
};

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | ((uint16_t)BLACK << 12) | ((uint16_t)LIGHT_GRAY << 8);
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

    int offset = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[offset] = (uint16_t)c | ((uint16_t)bg << 12) | ((uint16_t)fg << 8);

    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) cursor_y = 0;
    }
}

void print(const char *str, uint8_t fg, uint8_t bg) {
    while (*str) {
        putchar(*str++, fg, bg);
    }
}

// Simple string functions
int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void itoa(int num, char *str) {
    int i = 0, is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative) str[i++] = '-';
    str[i] = '\0';

    // Reverse string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// Keyboard input (polling PS/2 controller)
char getchar() {
    static const char scancode_to_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
        0, ' '
    };

    uint8_t scancode;
    // Wait for key press
    do {
        __asm__ volatile("inb $0x64, %%al" : "=a"(scancode));
    } while (!(scancode & 1));

    __asm__ volatile("inb $0x60, %%al" : "=a"(scancode));

    if (scancode < sizeof(scancode_to_ascii)) {
        return scancode_to_ascii[scancode];
    }
    return 0;
}

// Simple shell
void shell() {
    char buffer[256];
    int index = 0;

    while (1) {
        print("\nOS> ", LIGHT_GREEN, BLACK);
        index = 0;

        while (1) {
            char c = getchar();
            if (c == '\n') {
                buffer[index] = '\0';
                putchar('\n', WHITE, BLACK);
                break;
            } else if (c == '\b' && index > 0) {
                index--;
                cursor_x--;
                putchar(' ', WHITE, BLACK);
                cursor_x--;
            } else if (c != '\b' && c != 0 && index < 255) {
                buffer[index++] = c;
                putchar(c, WHITE, BLACK);
            }
        }

        // Process commands
        if (strlen(buffer) == 0) continue;

        if (buffer[0] == 'h' && buffer[1] == 'e' && buffer[2] == 'l' &&
            buffer[3] == 'p' && buffer[4] == '\0') {
            print("Available commands:\n", YELLOW, BLACK);
            print("  help  - Show this message\n", WHITE, BLACK);
            print("  clear - Clear screen\n", WHITE, BLACK);
            print("  echo  - Echo message\n", WHITE, BLACK);
            print("  info  - System info\n", WHITE, BLACK);
        } else if (buffer[0] == 'c' && buffer[1] == 'l' && buffer[2] == 'e' &&
                   buffer[3] == 'a' && buffer[4] == 'r' && buffer[5] == '\0') {
            clear_screen();
        } else if (buffer[0] == 'i' && buffer[1] == 'n' && buffer[2] == 'f' &&
                   buffer[3] == 'o' && buffer[4] == '\0') {
            print("Simple OS Kernel v1.0\n", LIGHT_CYAN, BLACK);
            print("Architecture: x86\n", WHITE, BLACK);
            print("VGA Mode: 80x25 Text\n", WHITE, BLACK);
        } else if (buffer[0] == 'e' && buffer[1] == 'c' && buffer[2] == 'h' &&
                   buffer[3] == 'o' && buffer[4] == ' ') {
            print(&buffer[5], WHITE, BLACK);
            putchar('\n', WHITE, BLACK);
        } else {
            print("Unknown command: ", RED, BLACK);
            print(buffer, WHITE, BLACK);
            print("\nType 'help' for available commands", YELLOW, BLACK);
        }
    }
}

void kernel_main() {
    clear_screen();

    print("========================================\n", LIGHT_CYAN, BLACK);
    print("   Welcome to SimpleOS Kernel v1.0     \n", YELLOW, BLACK);
    print("========================================\n", LIGHT_CYAN, BLACK);
    print("\n", WHITE, BLACK);
    print("Bootloader: OK\n", LIGHT_GREEN, BLACK);
    print("VGA Display: Initialized\n", LIGHT_GREEN, BLACK);
    print("Keyboard: Ready\n", LIGHT_GREEN, BLACK);
    print("\nType 'help' for available commands\n", WHITE, BLACK);

    shell();

    // Should never reach here
    while (1) {
        __asm__ volatile("hlt");
    }
}