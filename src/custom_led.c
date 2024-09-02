#include "custom_led.h"
#include "custom_utils.h"
#include <zephyr/kernel.h>

static int led_bit_postion;
static int sleep_time_ms;

void init_led(int _bit_position, int _sleep_time_ms, volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    led_bit_postion = _bit_position;
    sleep_time_ms = _sleep_time_ms;
    printf("bit position: %d\n", led_bit_postion);
    printf("sleep time: %d\n", sleep_time_ms);
    set_bit(dir_reg, led_bit_postion, 1);
    set_bit(out_reg, led_bit_postion, 0);
}

void set_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int state) {
    set_bit(dir_reg, led_bit_postion, 1);
    if (state) {
        set_bit(out_reg, led_bit_postion, 1);
    } else {
        set_bit(out_reg, led_bit_postion, 0);
    }
}

void blink_led_once(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    set_bit(out_reg, led_bit_postion, 1);
    k_msleep(sleep_time_ms);
    set_bit(out_reg, led_bit_postion, 0);
    k_msleep(sleep_time_ms);
}

void blink_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg) {
    printf("Setting direction\n");
    set_bit(dir_reg, led_bit_postion, 1);
    print_register(dir_reg, "dir_reg");

    while (1) {
        printf("\n----------------------------------\n");
        set_bit(out_reg, led_bit_postion, 1);
        printf("Setting pin high\n");
        print_register(out_reg, "out_reg");
        k_msleep(sleep_time_ms);

        printf("\n");
        set_bit(out_reg, led_bit_postion, 0);
        printf("Setting pin low\n");
        print_register(out_reg, "out_reg");
        k_msleep(sleep_time_ms);
        printf("----------------------------------\n");
    }
}
