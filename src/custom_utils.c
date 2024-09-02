#include "custom_utils.h"
#include <stdio.h>
#include <zephyr/sys/util_macro.h>

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
    printf("\t  %s Address: %p\n", name, (void *)reg);
    printf("\t  %s Value (hex): 0x%08x\n", name, *reg);
    printf("\t  %s Value (binary): ", name);
    print_binary(*reg);

    printf("\n\n");
}

void set_bit(volatile uint32_t *reg, int bit, int state) {
    if (state) {
        *reg |= BIT(bit);
    } else {
        *reg &= ~BIT(bit);
    }
}