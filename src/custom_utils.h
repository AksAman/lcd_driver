#ifndef CUSTOM_UTILS_H
#define CUSTOM_UTILS_H
#include <stdint.h>

uint32_t convert_to_binary(int value);
void print_binary(uint32_t value);
void print_register(volatile uint32_t *reg, const char *name);
void set_bit(volatile uint32_t *reg, int bit, int state);
#endif
