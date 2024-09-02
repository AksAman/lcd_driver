/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
References:
https://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf, 24, 43
*/

// #include "custom_led.h"
#include "custom_utils.h"
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

static const struct gpio_dt_spec external_led = GPIO_DT_SPEC_GET(DT_NODELABEL(ext_led), gpios);

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

#define HIGH 1
#define LOW 0

void run_lcd(const struct device *_gpio_dev) {

    clear_lcd();
    write_string("Mave Health\nPrivate Limited!");
    // write_character_using_code(0xEF);
    k_msleep(1000);
    int counter = 0;
    char counter_string[4];
    clear_lcd();

    char *base_string = "Counter: ";
    write_string(base_string);
    // get length of "Counter: "
    int base_string_length = strlen(base_string);
    int max_counter = 16;
    while (1) {
        k_msleep(COUNTER_SLEEP_TIME_MS); // Update every 500ms for better visibility

        // Set cursor to position where the counter should be displayed
        set_cursor_position(0, base_string_length);

        sprintf(counter_string, "%2d", counter); // Use %3d for consistent width
        write_string(counter_string);

        counter = (counter + 1) % (max_counter + 1);
        int ret = gpio_pin_toggle_dt(&external_led);
        if (ret < 0) {
            printk("Error toggling external led\n");
            return 0;
        }
        // gpio_pin_set(_gpio_dev, external_led.pin, counter % 2);
        // printf("counter: %d\n", counter);
    }
}

int main(void) {
    const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio_dev)) {
        printk("gpio_dev::Device %s not ready!\n", gpio_dev->name);
        return 0;
    }

    if (!gpio_is_ready_dt(&external_led)) {
        printk("external_led::Device %s not ready!\n", external_led.port->name);
        return 0;
    }

    // printf("Initializing\n");
    // volatile uint32_t *p0_dir_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_DIR_OFFSET);
    // print_register(p0_dir_reg, "p0_dir_reg");
    // volatile uint32_t *p0_out_reg = (volatile uint32_t *)(P0_BASE_ADDRESS + GPIO_OUT_OFFSET);
    // print_register(p0_out_reg, "p0_out_reg");
    // volatile uint32_t *p1_out_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_OUT_OFFSET);
    // print_register(p1_out_reg, "p1_out_reg");
    // volatile uint32_t *p1_dir_reg = (volatile uint32_t *)(P1_BASE_ADDRESS + GPIO_DIR_OFFSET);
    // print_register(p1_out_reg, "p1_out_reg");
    int ret;
    ret = gpio_pin_configure_dt(&external_led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    lcd_init(gpio_dev, lcd_rs.pin, lcd_rw.pin, lcd_e.pin, lcd_d0.pin, lcd_d1.pin, lcd_d2.pin, lcd_d3.pin, lcd_d4.pin, lcd_d5.pin, lcd_d6.pin, lcd_d7.pin);
    run_lcd(gpio_dev);
}