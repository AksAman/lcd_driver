#include "lcd_driver.h"
#include "custom_utils.h"
#include <zephyr/kernel.h>

#define ENABLE_DELAY 10
#define MODE_8_BIT 1
#define TWO_LINE 1
#define FIVE_BY_TEN 1
#define CURSOR_BLINK 0
#define INCREMENT 1
#define DISPLAY_SHIFT 0

static int LCD_RS;
static int LCD_RW;
static int LCD_E;

static int LCD_D0;
static int LCD_D1;
static int LCD_D2;
static int LCD_D3;
static int LCD_D4;
static int LCD_D5;
static int LCD_D6;
static int LCD_D7;
static int data_pins[8];
static int control_pins[3];

void toggle_enable(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
    // *out_reg &= ~((1 << LCD_RW) | (1 << LCD_RS) | (1 << LCD_E));
    set_bit(out_reg, LCD_E, 1);
    k_msleep(ENABLE_DELAY);
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
}

void set_lcd_state_2(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int rs, int rw, int d7,
                     int d6, int d5, int d4, int d3, int d2, int d1, int d0) {
    // create a binary number with the values of the pins
    uint32_t binary = 0;
    *out_reg = binary;

    set_bit(&binary, LCD_RS, rs);
    set_bit(&binary, LCD_RW, rw);
    set_bit(&binary, LCD_D0, d0);
    set_bit(&binary, LCD_D1, d1);
    set_bit(&binary, LCD_D2, d2);
    set_bit(&binary, LCD_D3, d3);
    set_bit(&binary, LCD_D4, d4);
    set_bit(&binary, LCD_D5, d5);
    set_bit(&binary, LCD_D6, d6);
    set_bit(&binary, LCD_D7, d7);

    *out_reg = binary;
    toggle_enable(out_reg, dir_reg);

    // k_msleep(400);
}

void set_lcd_state_bitmask(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, uint32_t _control_pins, uint32_t _data_pins) {
    // create a binary number with the values of the pins
    uint32_t binary = 0;
    *out_reg = binary;

    int control_pins_length = sizeof(control_pins) / sizeof(control_pins[0]);
    int data_pins_length = sizeof(data_pins) / sizeof(data_pins[0]);

    printf("control_pins_length: %d\n", control_pins_length);
    printf("data_pins_length: %d\n", data_pins_length);

    for (int i = 0; i < control_pins_length; i++) {
        set_bit(&binary, control_pins[i], _control_pins & (1 << i));
        // set_bit(&binary, control_pins[i], (_control_pins >> i) & 1);
    }

    for (int i = 0; i < data_pins_length; i++) {
        set_bit(&binary, data_pins[i], _data_pins & (1 << i));
        // set_bit(&binary, data_pins[i], (_data_pins >> i) & 1);
    }

    *out_reg = binary;
    toggle_enable(out_reg, dir_reg);

    // k_msleep(400);
}

void set_lcd_state_array(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int _control_pins[3],
                         int _data_pins[8]) {
    // create a binary number with the values of the pins
    uint32_t binary = 0;
    *out_reg = binary;

    int control_pins_length = sizeof(control_pins) / sizeof(control_pins[0]);
    int data_pins_length = sizeof(data_pins) / sizeof(data_pins[0]);

    for (int i = 0; i < control_pins_length; i++) {
        set_bit(&binary, control_pins[i], _control_pins[i]);
    }

    for (int i = 0; i < data_pins_length; i++) {
        set_bit(&binary, data_pins[i], _data_pins[i]);
    }

    *out_reg = binary;
    toggle_enable(out_reg, dir_reg);

    // k_msleep(400);
}

void set_cursor_position(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int row, int col) {
    int address = row == 0 ? 0x80 + col : 0xC0 + col;
    set_lcd_state_2(out_reg, dir_reg,
                    0, 0,
                    (address >> 7) & 1, (address >> 6) & 1, (address >> 5) & 1, (address >> 4) & 1,
                    (address >> 3) & 1, (address >> 2) & 1, (address >> 1) & 1, address & 1);
}
void shift_display(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int shift_direction) {
    uint32_t shift_command = 0x18;
    set_bit(&shift_command, 2, shift_direction);
    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, shift_command);
    print_register(out_reg, "Shift display out_reg");
}

void lcd_function_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int dl_bit_8_mode, int n_two_line, int f_five_by_ten) {
    /*
     * DL: Sets interface data length, 1: 8-bit, 0: 4-bit
     * N: Sets number of display lines, 1: 2 lines, 0: 1 line
     * F: Sets font type, 1: 5x10 dots, 0: 5x8 dots
     * _: Don't care
     */

    uint32_t function_set_command = 0x20;
    set_bit(&function_set_command, 4, dl_bit_8_mode);
    set_bit(&function_set_command, 3, n_two_line);
    set_bit(&function_set_command, 2, f_five_by_ten);

    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, function_set_command);
    print_register(out_reg, "function set out_reg");
}

void lcd_display_on(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int cursor_blink) {
    uint32_t display_on_command = 0x0E;
    set_bit(&display_on_command, 0, cursor_blink);
    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, display_on_command);
    print_register(out_reg, "Display on/off control out_reg");
}

void lcd_entry_mode_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg,
                        volatile int increment, volatile int display_shift) {
    uint32_t entry_mode_command = 0x04;
    set_bit(&entry_mode_command, 1, increment);
    set_bit(&entry_mode_command, 0, display_shift);
    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, entry_mode_command);

    print_register(out_reg, "Entry mode set out_reg");
}

void lcd_init(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int rs, int rw, int e, int d0,
              int d1, int d2, int d3, int d4, int d5, int d6, int d7) {
    control_pins[0] = LCD_RS = rs;
    control_pins[1] = LCD_RW = rw;
    control_pins[2] = LCD_E = e;
    data_pins[0] = LCD_D0 = d0;
    data_pins[1] = LCD_D1 = d1;
    data_pins[2] = LCD_D2 = d2;
    data_pins[3] = LCD_D3 = d3;
    data_pins[4] = LCD_D4 = d4;
    data_pins[5] = LCD_D5 = d5;
    data_pins[6] = LCD_D6 = d6;
    data_pins[7] = LCD_D7 = d7;

    // set all pins to output
    *dir_reg |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | (1 << LCD_D0) | (1 << LCD_D1) |
                (1 << LCD_D2) | (1 << LCD_D3) | (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) |
                (1 << LCD_D7);

    lcd_function_set(out_reg, dir_reg, MODE_8_BIT, TWO_LINE, FIVE_BY_TEN);
    lcd_display_on(out_reg, dir_reg, CURSOR_BLINK);
    lcd_entry_mode_set(out_reg, dir_reg, INCREMENT, DISPLAY_SHIFT);
}
// write character

void change_line(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, 0xC0);
    // print_register(out_reg, "change_line out_reg");
}

void write_character(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char character) {
    int ascii_value = (int)character;
    write_character_using_code(out_reg, dir_reg, ascii_value);
}

void write_character_using_code(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int code) {
    uint32_t binary = convert_to_binary(code);
    uint32_t out_reg_value = *out_reg;

    for (int i = 0; i < 8; i++) {
        int bit = binary & (1 << i);
        set_bit(&out_reg_value, data_pins[i], bit != 0 ? 1 : 0);
    }
    // enable write mode
    set_bit(&out_reg_value, LCD_RS, 1);
    set_bit(&out_reg_value, LCD_RW, 0);
    *out_reg = out_reg_value;
    toggle_enable(out_reg, dir_reg);
}

void write_string(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char *string) {
    for (int i = 0; i < strlen(string); i++) {
        char character = string[i];
        if (character == '\n') {
            change_line(out_reg, dir_reg);
        } else {
            write_character(out_reg, dir_reg, character);
        }
    }
}

void clear_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_lcd_state_bitmask(out_reg, dir_reg, 0x0, 0x01);
    print_register(out_reg, "clear_lcd out_reg");
}