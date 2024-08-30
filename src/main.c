/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
References:
https://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf, 24, 43
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <hal/nrf_gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

#define P0_BASE_ADDRESS 0x50000000
#define P1_BASE_ADDRESS 0x50000300

#define GPIO_OUT_OFFSET 0x504
#define GPIO_IN_OFFSET  0x510
#define GPIO_DIR_OFFSET 0x514

#define LED_BIT_POSITION 14

// defining LCD PINS
#define LCD_RS 12
#define LCD_RW 11
#define LCD_E  10

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
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void print_register(volatile uint32_t *reg, const char *name)
{
	printf("Register: %s\n", name);
	printf("%s Address: 0x%p\n", name, (void *)reg);
	printf("%s Value (hex): 0x%08lx\n", name, *reg);
	printf("%s Value (binary): ", name);
	for (int i = 31; i >= 0; i--) {
		printf("%d", (*reg >> i) & 1);
		if (i % 8 == 0 && i != 0) {
			printf(" ");
		}
	}
	printf("\n\n");
}

int main(void)
{
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

void blink_led_once(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
	// set_bit(out_reg, LED_BIT_POSITION, 1);
	k_msleep(SLEEP_TIME_MS);
	// set_bit(out_reg, LED_BIT_POSITION, 0);
	// k_msleep(SLEEP_TIME_MS);
}

void run_lcd(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
	set_bit(out_reg, LCD_E, 1);
	// lcd_function_set(out_reg, dir_reg);
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      0, 0,
		      // D0-D7
		      0, 0, 1, 1,
		      //
		      1, 0, 0, 0);
	// lcd_display_on(out_reg, dir_reg, 0);
	set_bit(out_reg, LCD_E, 1);
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      0, 0,
		      // D0-D7
		      0, 0, 0, 0,
		      //
		      1, 1, 1, 0);
	// lcd_entry_mode_set(out_reg, dir_reg, 1, 0);
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      0, 0,
		      // D0-D7
		      0, 0, 0, 0,
		      //
		      0, 1, 1, 0);

	// write Aman
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      1, 0,
		      // D0-D7
		      0, 1, 0, 0,
		      //
		      1, 0, 0, 0);

	// set_lcd_state(out_reg, dir_reg,
	// 	      // RS, RW
	// 	      1, 0,
	// 	      // D0-D7
	// 	      0, 1, 0, 0,
	// 	      //
	// 	      0, 0, 0, 1);

	// set_lcd_state(out_reg, dir_reg,
	// 	      // RS, RW
	// 	      1, 0,
	// 	      // D0-D7
	// 	      0, 1, 1, 0,
	// 	      //
	// 	      1, 1, 0, 1);
}

void toggle_enable(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
	set_bit(out_reg, LCD_E, 0);
	*out_reg &= ~((1 << LCD_RW) | (1 << LCD_RS) | (1 << LCD_E));
	set_bit(out_reg, LCD_E, 1);
}

void lcd_init(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
	// set all pins to output
	*dir_reg |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | (1 << LCD_D0) | (1 << LCD_D1) |
		    (1 << LCD_D2) | (1 << LCD_D3) | (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) |
		    (1 << LCD_D7);
}

void lcd_function_set(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      0, 0,
		      // D0-D7
		      0, 0, 1, 1,
		      //
		      1, 0, 0, 0);
	print_register(out_reg, "function set out_reg");
}

void lcd_display_on(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int cursor_blink)
{
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
			int cursor_direction, int cursor_display)
{
	set_lcd_state(out_reg, dir_reg,
		      // RS, RW
		      0, 0,
		      // D0-D7
		      0, 0, 0, 0,
		      //
		      0, 1, 1, cursor_direction);
	print_register(out_reg, "Entry mode set out_reg");
}

void set_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int state)
{
	*dir_reg |= (1 << LED_BIT_POSITION);
	if (state) {
		*out_reg |= (1 << LED_BIT_POSITION);
	} else {
		*out_reg &= ~(1 << LED_BIT_POSITION);
	}
}

void set_bit(uint32_t *reg, int bit, int state)
{
	if (state) {
		*reg |= (1 << bit);
	} else {
		*reg &= ~(1 << bit);
	}
}

void set_lcd_state(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int rs, int rw, int d7,
		   int d6, int d5, int d4, int d3, int d2, int d1, int d0)
{
	// create a binary number with the values of the pins
	uint32_t binary = 0;
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
}

void blink_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg)
{
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