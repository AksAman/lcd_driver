#include "lcd_driver.h"
#include "custom_utils.h"
#include <zephyr/kernel.h>

#define ENABLE_DELAY 10

void toggle_enable(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
    // *out_reg &= ~((1 << LCD_RW) | (1 << LCD_RS) | (1 << LCD_E));
    set_bit(out_reg, LCD_E, 1);
    k_msleep(ENABLE_DELAY);
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
}

void set_lcd_state(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int rs, int rw, int d7,
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

void set_cursor_position(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int row, int col) {
    int address = row == 0 ? 0x80 + col : 0xC0 + col;
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  (address >> 7) & 1, (address >> 6) & 1, (address >> 5) & 1, (address >> 4) & 1,
                  (address >> 3) & 1, (address >> 2) & 1, (address >> 1) & 1, address & 1);
}
void shift_display(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int shift_direction) {
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  0, 0, 0, 1,
                  1, shift_direction, 0, 0);
    print_register(out_reg, "Shift display out_reg");
}

void lcd_function_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_lcd_state(out_reg, dir_reg,
                  // RS, RW
                  0, 0,
                  // D0-D7
                  0, 0, 1, 1, // 0x3
                  //
                  1, 0, 0, 0); // 0x8
    print_register(out_reg, "function set out_reg");
}

void lcd_display_on(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int cursor_blink) {
    set_lcd_state(out_reg, dir_reg,
                  // RS, RW
                  0, 0,
                  // D0-D7
                  0, 0, 0, 0,
                  //
                  1, 1, 1, cursor_blink);
    print_register(out_reg, "Display on/off control out_reg");
}

void lcd_entry_mode_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg,
                        volatile int increment, volatile int display_shift) {
    set_lcd_state(out_reg, dir_reg,
                  // RS, RW
                  0, 0,
                  // D0-D7
                  0, 0, 0, 0,
                  //
                  0, 1, increment, display_shift);

    print_register(out_reg, "Entry mode set out_reg");
}

void lcd_init(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    // set all pins to output
    *dir_reg |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | (1 << LCD_D0) | (1 << LCD_D1) |
                (1 << LCD_D2) | (1 << LCD_D3) | (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) |
                (1 << LCD_D7);

    lcd_function_set(out_reg, dir_reg);
    lcd_display_on(out_reg, dir_reg, 1);
    lcd_entry_mode_set(out_reg, dir_reg, 1, 0);
}
// write character

void change_line(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  1, 1, 0, 0,
                  0, 0, 0, 0);
    print_register(out_reg, "change_line out_reg");
}

void write_character(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char character) {
    int ascii_value = (int)character;
    write_character_using_code(out_reg, dir_reg, ascii_value);
}

void write_character_using_code(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int code) {
    uint32_t binary = convert_to_binary(code);
    uint32_t out_reg_value = *out_reg;

    int pins[8] = {LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7};
    for (int i = 0; i < 8; i++) {
        int bit = binary & (1 << i);
        set_bit(&out_reg_value, pins[i], bit != 0 ? 1 : 0);
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
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  0, 0, 0, 0,
                  0, 0, 0, 1);
    print_register(out_reg, "clear_lcd out_reg");
}