#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H
#include <stdint.h>
#include <zephyr/drivers/gpio.h>

void lcd_init(const struct device *_gpio_dev, int rs, int rw, int e, int d0,
              int d1, int d2, int d3, int d4, int d5, int d6, int d7);
void toggle_enable();
void set_lcd_state_bitmask(uint32_t _control_pins, uint32_t _data_pins);
void write_character_using_code(int code);
void write_character(char character);
void change_line();
void write_string(char *string);
void clear_lcd();
void lcd_function_set(int dl_bit_8_mode, int n_two_line, int f_five_by_ten);
void lcd_display_on(int cursor_blink);
void shift_display(int shift_direction);
void lcd_entry_mode_set(int increment, int display_shift);
void set_cursor_position(int row, int col);
#endif