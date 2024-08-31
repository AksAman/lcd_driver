/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
References:
https://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf, 24, 43
*/

#include "custom_utils.h"
#include "lcd_driver.h"
#include <hal/nrf_gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

#define P0_BASE_ADDRESS 0x50000000
#define P1_BASE_ADDRESS 0x50000300

#define GPIO_OUT_OFFSET 0x504
#define GPIO_IN_OFFSET 0x510
#define GPIO_DIR_OFFSET 0x514
#define LED_BIT_POSITION 14

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

void set_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int state) {
    *dir_reg |= (1 << LED_BIT_POSITION);
    if (state) {
        *out_reg |= (1 << LED_BIT_POSITION);
    } else {
        *out_reg &= ~(1 << LED_BIT_POSITION);
    }
}

void blink_led_once(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, LED_BIT_POSITION, 1);
    k_msleep(SLEEP_TIME_MS);
    set_bit(out_reg, LED_BIT_POSITION, 0);
    k_msleep(SLEEP_TIME_MS);
}

void run_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {

    // char time_string[20];
    clear_lcd(out_reg, dir_reg);
    write_string(out_reg, dir_reg, "Mave Health\nPrivate Limited!");
    write_character_using_code(out_reg, dir_reg, 0xEF);
    int display_length = 14;
    int counter = 0;
    char counter_string[4];
    blink_led_once(out_reg, dir_reg);
    clear_lcd(out_reg, dir_reg);

    char *base_string = "Counter: ";
    write_string(out_reg, dir_reg, base_string);
    // get length of "Counter: "
    int base_string_length = strlen(base_string);
    int max_counter = 16;
    while (1) {
        k_msleep(200); // Update every 500ms for better visibility

        // Set cursor to position where the counter should be displayed
        set_cursor_position(out_reg, dir_reg, 0, base_string_length);

        sprintf(counter_string, "%2d", counter); // Use %3d for consistent width
        write_string(out_reg, dir_reg, counter_string);

        counter = (counter + 1) % (max_counter + 1);
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