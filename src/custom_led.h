#ifndef CUSTOM_LED_H
#define CUSTOM_LED_H
#include <stdint.h>

void init_led(int bit_position, int sleep_time_ms, volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void set_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg, int state);
void blink_led_once(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);
void blink_led(volatile uint32_t *out_reg, volatile uint32_t *dir_reg);

#endif
