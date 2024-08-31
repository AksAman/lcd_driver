/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
References:
https://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf, 24, 43
*/

#include <hal/nrf_gpio.h>
#include <time.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

#define P0_BASE_ADDRESS 0x50000000
#define P1_BASE_ADDRESS 0x50000300

#define GPIO_OUT_OFFSET 0x504
#define GPIO_IN_OFFSET 0x510
#define GPIO_DIR_OFFSET 0x514
#define ENABLE_DELAY 10
#define LED_BIT_POSITION 14

// defining LCD PINS
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

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

uint32_t convert_to_binary(int value) {
    uint32_t binary = 0;
    for (int i = 0; i < 32; i++) {
        binary |= (value & 1) << i;
        value >>= 1;
    }
    return binary;
}

void print_binary(uint32_t value) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i % 8 == 0 && i != 0) {
            printf(" ");
        }
        if (i % 4 == 0 && i != 0 && i % 8 != 0) {
            printf("|");
        }
    }
    printf("\n");
}

void print_register(volatile uint32_t *reg, const char *name) {
    printf("Register: %s\n", name);
    printf("%s Address: 0x%p\n", name, (void *)reg);
    printf("%s Value (hex): 0x%08x\n", name, *reg);
    printf("%s Value (binary): ", name);
    print_binary(*reg);

    printf("\n\n");
}

void set_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int state) {
    *dir_reg |= (1 << LED_BIT_POSITION);
    if (state) {
        *out_reg |= (1 << LED_BIT_POSITION);
    } else {
        *out_reg &= ~(1 << LED_BIT_POSITION);
    }
}

void set_bit(volatile uint32_t *reg, int bit, int state) {
    if (state) {
        *reg |= (1 << bit);
    } else {
        *reg &= ~(1 << bit);
    }
}

void toggle_enable(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
    // *out_reg &= ~((1 << LCD_RW) | (1 << LCD_RS) | (1 << LCD_E));
    set_bit(out_reg, LCD_E, 1);
    k_msleep(ENABLE_DELAY);
    set_bit(out_reg, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
}

void get_current_time(char *time_string) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(time_string, 20, "%H:%M:%S", time_info);
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

    k_msleep(400);
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

void blink_led_once(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, LED_BIT_POSITION, 1);
    k_msleep(SLEEP_TIME_MS);
    set_bit(out_reg, LED_BIT_POSITION, 0);
    k_msleep(SLEEP_TIME_MS);
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
// write character
void write_character(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, char character) {
    int ascii_value = (int)character;
    write_character_using_code(out_reg, dir_reg, ascii_value);
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

void change_line(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  1, 1, 0, 0,
                  0, 0, 0, 0);
    print_register(out_reg, "change_line out_reg");
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

void shift_display(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int shift_direction) {
    set_lcd_state(out_reg, dir_reg,
                  0, 0,
                  0, 0, 0, 1,
                  1, shift_direction, 0, 0);
    print_register(out_reg, "Shift display out_reg");
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

void set_cursor_position(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int row, int col) {
    int memory_address = 0x800 + (row * 0x40) + col;
    write_character_using_code(out_reg, dir_reg, memory_address);
}

void run_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {

    char time_string[20];
    clear_lcd(out_reg, dir_reg);
    write_string(out_reg, dir_reg, "Mave Health\nPrivate Limited!");
    write_character_using_code(out_reg, dir_reg, 0xEF);
    int display_length = 14;
    int counter = 0;

    blink_led_once(out_reg, dir_reg);
    while (1) {
        k_msleep(50);
        get_current_time(time_string);
        clear_lcd(out_reg, dir_reg);
        write_string(out_reg, dir_reg, time_string);
        int shift_direction = counter >= display_length ? 1 : 0;
        // shift_display(out_reg, dir_reg, shift_direction);
        printf("Counter: %d\n", counter);
        counter++;
        if (counter >= display_length) {
            counter = 0;
        }
        // blink_led_once(out_reg, dir_reg);
    }
}

void blink_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    printf("Setting direction\n");
    *dir_reg |= (1 << LED_BIT_POSITION);
    print_register(dir_reg, "dir_reg");

    while (1) {
        printf("\n----------------------------------\n");
        *out_reg |= (1 << LED_BIT_POSITION);
        printf("Setting pin high\n");
        print_register(out_reg, "out_reg");
        k_msleep(SLEEP_TIME_MS);

        printf("\n");
        *out_reg &= ~(1 << LED_BIT_POSITION);
        printf("Setting pin low\n");
        print_register(out_reg, "out_reg");
        k_msleep(SLEEP_TIME_MS);
        printf("----------------------------------\n");
    }
}

int main(void) {
    printf("Initializing\n");
    volatile uint32_t *p0_out_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_OUT_OFFSET);
    print_register(p0_out_reg, "p0_out_reg");

    volatile uint32_t *p0_dir_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_DIR_OFFSET);
    print_register(p0_dir_reg, "p0_dir_reg");

    volatile uint32_t *p1_out_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_OUT_OFFSET);
    print_register(p1_out_reg, "p1_out_reg");

    volatile uint32_t *p1_dir_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_DIR_OFFSET);
    print_register(p1_out_reg, "p1_out_reg");

    // blink_led(p1_out_reg, p1_dir_reg);

    lcd_init(p1_out_reg, p1_dir_reg);
    set_led(p1_out_reg, p1_dir_reg, 1);

    run_lcd(p1_out_reg, p1_dir_reg);
}