#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>

#define LCD_RS 12
#define LCD_RW 11
#define LCD_E 10

#define LCD_D0 8
#define LCD_D1 7
#define LCD_D2 6
#define LCD_D3 5
#define LCD_D4 4
#define LCD_D5 3
#define LCD_D6 2
#define LCD_D7 1

void lcd_init(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void toggle_enable(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void set_lcd_state(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int rs, int rw, int d7,
                   int d6, int d5, int d4, int d3, int d2, int d1, int d0);
void write_character_using_code(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int code);
void write_character(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char character);
void change_line(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void write_string(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char *string);
void clear_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void lcd_function_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void lcd_display_on(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int cursor_blink);
void shift_display(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int shift_direction);
void lcd_entry_mode_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int increment, int display_shift);
void set_cursor_position(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int row, int col);
#endif