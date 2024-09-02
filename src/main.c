/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
References:
https://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf, 24, 43
*/

#include "custom_led.h"
#include "lcd_driver.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000
#define COUNTER_SLEEP_TIME_MS 200

#define P0_BASE_ADDRESS 0x50000000
#define P1_BASE_ADDRESS 0x50000300

#define GPIO_OUT_OFFSET 0x504
#define GPIO_IN_OFFSET 0x510
#define GPIO_DIR_OFFSET 0x514

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const struct gpio_dt_spec lcd_rs = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_rs), gpios);
static const struct gpio_dt_spec lcd_rw = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_rw), gpios);
static const struct gpio_dt_spec lcd_e = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_enable), gpios);
static const struct gpio_dt_spec lcd_d0 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d0), gpios);
static const struct gpio_dt_spec lcd_d1 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d1), gpios);
static const struct gpio_dt_spec lcd_d2 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d2), gpios);
static const struct gpio_dt_spec lcd_d3 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d3), gpios);
static const struct gpio_dt_spec lcd_d4 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d4), gpios);
static const struct gpio_dt_spec lcd_d5 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d5), gpios);
static const struct gpio_dt_spec lcd_d6 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d6), gpios);
static const struct gpio_dt_spec lcd_d7 = GPIO_DT_SPEC_GET(DT_NODELABEL(lcd_d7), gpios);

#define LED_BIT_POSITION 14
#define HIGH 1
#define LOW 0

void run_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {

    // char time_string[20];
    clear_lcd(out_reg, dir_reg);
    write_string(out_reg, dir_reg, "Mave Health\nPrivate Limited!");
    write_character_using_code(out_reg, dir_reg, 0xEF);
    k_msleep(1000);
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
        k_msleep(COUNTER_SLEEP_TIME_MS); // Update every 500ms for better visibility

        // Set cursor to position where the counter should be displayed
        set_cursor_position(out_reg, dir_reg, 0, base_string_length);

        sprintf(counter_string, "%2d", counter); // Use %3d for consistent width
        write_string(out_reg, dir_reg, counter_string);

        counter = (counter + 1) % (max_counter + 1);
        set_led(out_reg, dir_reg, counter % 2);
        // printf("counter: %d\n", counter);
    }
}

int main(void) {
    const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio_dev)) {
        printk("gpio_dev::Device %s not ready!\n", gpio_dev->name);
        return 0;
    }

    printf("Initializing\n");
    volatile uint32_t *p0_dir_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_DIR_OFFSET);
    print_register(p0_dir_reg, "p0_dir_reg");
    volatile uint32_t *p0_out_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_OUT_OFFSET);
    print_register(p0_out_reg, "p0_out_reg");

    int ret;
    ret = gpio_pin_configure(gpio_dev, LED_BIT_POSITION, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure GPIO pin %d, ret: %d\n", LED_BIT_POSITION, ret);
        return 0;
    }
    ret = gpio_pin_set_raw(gpio_dev, LED_BIT_POSITION, LOW);
    if (ret != 0) {
        printk("Failed to set GPIO pin %d, ret: %d\n", LED_BIT_POSITION, ret);
        return 0;
    }
    k_msleep(1000);
    ret = gpio_pin_set_raw(gpio_dev, LED_BIT_POSITION, HIGH);
    if (ret != 0) {
        printk("Failed to set GPIO pin %d, ret: %d\n", LED_BIT_POSITION, ret);
        return 0;
    }
    // printf("led.pin: %d\n", led.pin);
    // printf("LED_BIT_POSITION: %d\n", LED_BIT_POSITION);

    // set_bit(p0_dir_reg, LED_BIT_POSITION, 1);
    // set_bit(p0_out_reg, LED_BIT_POSITION, 1);
    // set_led(p0_out_reg, p0_dir_reg, 1);

    // uint32_t pin13address = P0_BASE_ADDRESS + GPIO_OUT_OFFSET + 13;
    // print_register(pin13address, "pin13address");
    volatile uint32_t *p1_out_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_OUT_OFFSET);
    print_register(p1_out_reg, "p1_out_reg");
    volatile uint32_t *p1_dir_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_DIR_OFFSET);
    print_register(p1_out_reg, "p1_out_reg");

    lcd_init(p1_out_reg, p1_dir_reg, lcd_rs.pin, lcd_rw.pin, lcd_e.pin, lcd_d0.pin, lcd_d1.pin, lcd_d2.pin, lcd_d3.pin, lcd_d4.pin, lcd_d5.pin, lcd_d6.pin, lcd_d7.pin);
    set_led(p1_out_reg, p1_dir_reg, 1);
    run_lcd(p1_out_reg, p1_dir_reg);
}