#include "lcd_driver.h"
#include "custom_utils.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define ENABLE_DELAY 10
#define MODE_8_BIT 1
#define TWO_LINE 1
#define FIVE_BY_TEN 1
#define CURSOR_BLINK 1
#define INCREMENT 1
#define DISPLAY_SHIFT 0

static int LCD_RS;
static int LCD_RW;
static int LCD_E;

static int LCD_D0;
static int LCD_D1;
static int LCD_D2;
static int LCD_D3;
static int LCD_D4;
static int LCD_D5;
static int LCD_D6;
static int LCD_D7;
static int data_pins[8];
static int control_pins[3];

static struct device *gpio_dev;

void set_pins_as_output(int pins[], int length) {
    for (int i = 0; i < length; i++) {
        gpio_pin_configure(gpio_dev, pins[i], GPIO_OUTPUT);
    }
}

void set_pins_as_input(int pins[], int length) {
    for (int i = 0; i < length; i++) {
        gpio_pin_configure(gpio_dev, pins[i], GPIO_INPUT);
    }
}
void toggle_enable() {
    gpio_pin_set_raw(gpio_dev, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
    gpio_pin_set_raw(gpio_dev, LCD_E, 1);
    k_msleep(ENABLE_DELAY);
    gpio_pin_set_raw(gpio_dev, LCD_E, 0);
    k_msleep(ENABLE_DELAY);
}

void smart_toggle_enable() {
    // temporarily store the values of data_pins
    // uint32_t orig_data_pins_values = 0;
    // for (int i = 0; i < sizeof(data_pins) / sizeof(data_pins[0]); i++) {
    //     orig_data_pins_values |= gpio_pin_get(gpio_dev, data_pins[i]);
    // }
    // print_binary(orig_data_pins_values);
    int busy_flag = 1;
    printf("Waiting for busy flag to be 0\n");
    gpio_pin_set_raw(gpio_dev, LCD_E, 0);
    set_pins_as_input(data_pins, sizeof(data_pins) / sizeof(data_pins[0]));
    while (busy_flag != 0) {

        gpio_pin_set_raw(gpio_dev, LCD_RS, 0);
        gpio_pin_set_raw(gpio_dev, LCD_E, 1);
        gpio_pin_set_raw(gpio_dev, LCD_RW, 1);

        int data_pins_value = 0;
        for (int i = 0; i < sizeof(data_pins) / sizeof(data_pins[0]); i++) {
            data_pins_value |= gpio_pin_get(gpio_dev, data_pins[i]) << i;
        }
        busy_flag = (data_pins_value & (1 << 7)) >> 7;
        printf("\tbusy_flag: %d\n", busy_flag);
    }

    printf("Not busy\n");

    set_pins_as_output(data_pins, sizeof(data_pins) / sizeof(data_pins[0]));
    gpio_pin_set_raw(gpio_dev, LCD_RS, 0);
    gpio_pin_set_raw(gpio_dev, LCD_RW, 0);
    gpio_pin_set_raw(gpio_dev, LCD_E, 1);
    gpio_pin_set_raw(gpio_dev, LCD_E, 0);
    // restore the values of data_pins
    // for (int i = 0; i < sizeof(data_pins) / sizeof(data_pins[0]); i++) {
    //     gpio_pin_set_raw(gpio_dev, data_pins[i], (orig_data_pins_values >> i) & 1);
    // }
}

void set_lcd_state_bitmask(uint32_t _control_pin_values, uint32_t _data_pin_values) {
    // create a binary number with the values of the pins
    uint32_t binary = 0;

    int control_pins_length = sizeof(control_pins) / sizeof(control_pins[0]);
    for (int i = 0; i < control_pins_length; i++) {
        int pin_number = control_pins[i];
        int pin_value = (_control_pin_values >> i) & 1;
        set_bit(&binary, pin_number, pin_value);
        gpio_pin_set_raw(gpio_dev, pin_number, pin_value);
        // set_bit(&binary, control_pins[i], (_control_pins >> i) & 1);
    }

    int data_pins_length = sizeof(data_pins) / sizeof(data_pins[0]);
    for (int i = 0; i < data_pins_length; i++) {
        int pin_number = data_pins[i];
        int pin_value = (_data_pin_values >> i) & 1;
        set_bit(&binary, pin_number, pin_value);
        gpio_pin_set_raw(gpio_dev, pin_number, pin_value);
        // set_bit(&binary, data_pins[i], (_data_pins >> i) & 1);
    }
    // *out_reg = binary;
    // gpio_port_set_bits_raw(gpio_dev, binary);
    toggle_enable();

    // k_msleep(400);
}

void set_cursor_position(int row, int col) {
    int address = row == 0 ? 0x80 + col : 0xC0 + col;
    set_lcd_state_bitmask(0x0, address);
}
void shift_display(int shift_direction) {
    uint32_t shift_command = 0x18;
    set_bit(&shift_command, 2, shift_direction);
    set_lcd_state_bitmask(0x0, shift_command);
}

void lcd_function_set(int dl_bit_8_mode, int n_two_line, int f_five_by_ten) {
    /*
     * DL: Sets interface data length, 1: 8-bit, 0: 4-bit
     * N: Sets number of display lines, 1: 2 lines, 0: 1 line
     * F: Sets font type, 1: 5x10 dots, 0: 5x8 dots
     * _: Don't care
     */

    uint32_t function_set_command = 0x20;
    set_bit(&function_set_command, 4, dl_bit_8_mode);
    set_bit(&function_set_command, 3, n_two_line);
    set_bit(&function_set_command, 2, f_five_by_ten);

    set_lcd_state_bitmask(0x0, function_set_command);
}

void lcd_display_on(int cursor_blink) {
    uint32_t display_on_command = 0x0E;
    set_bit(&display_on_command, 0, cursor_blink);

    set_lcd_state_bitmask(0x0, display_on_command);
}

void lcd_entry_mode_set(
    volatile int increment, volatile int display_shift) {
    uint32_t entry_mode_command = 0x04;
    set_bit(&entry_mode_command, 1, increment);
    set_bit(&entry_mode_command, 0, display_shift);

    set_lcd_state_bitmask(0x0, entry_mode_command);
}

// write character

void change_line() {
    set_lcd_state_bitmask(0x0, 0xC0);
}

void write_character(char character) {
    int ascii_value = (int)character;
    write_character_using_code(ascii_value);
}

void write_character_using_code(int code) {
    uint32_t binary = convert_to_binary(code);

    for (int i = 0; i < 8; i++) {
        int bit = binary & (1 << i);
        int pin_number = data_pins[i];
        int pin_value = bit != 0 ? 1 : 0;
        gpio_pin_set_raw(gpio_dev, pin_number, pin_value);
    }
    gpio_pin_set_raw(gpio_dev, LCD_RS, 1);
    gpio_pin_set_raw(gpio_dev, LCD_RW, 0);
    toggle_enable();
}

void write_string(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        char character = string[i];
        if (character == '\n') {
            change_line();
        } else {
            write_character(character);
        }
    }
}

void clear_lcd() {
    set_lcd_state_bitmask(0x0, 0x01);
}

void lcd_init(struct device *_gpio_dev, int rs, int rw, int e, int d0,
              int d1, int d2, int d3, int d4, int d5, int d6, int d7) {
    control_pins[0] = LCD_RS = rs;
    control_pins[1] = LCD_RW = rw;
    control_pins[2] = LCD_E = e;
    data_pins[0] = LCD_D0 = d0;
    data_pins[1] = LCD_D1 = d1;
    data_pins[2] = LCD_D2 = d2;
    data_pins[3] = LCD_D3 = d3;
    data_pins[4] = LCD_D4 = d4;
    data_pins[5] = LCD_D5 = d5;
    data_pins[6] = LCD_D6 = d6;
    data_pins[7] = LCD_D7 = d7;

    gpio_dev = _gpio_dev;
    set_pins_as_output(control_pins, sizeof(control_pins) / sizeof(control_pins[0]));
    set_pins_as_output(data_pins, sizeof(data_pins) / sizeof(data_pins[0]));

    lcd_function_set(MODE_8_BIT, TWO_LINE, FIVE_BY_TEN);
    lcd_display_on(CURSOR_BLINK);
    lcd_entry_mode_set(INCREMENT, DISPLAY_SHIFT);
}
