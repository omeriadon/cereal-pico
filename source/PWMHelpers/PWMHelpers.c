#include "hardware/pwm.h"

uint pwm_slice_for_gpio(uint gpio) {
    return pwm_gpio_to_slice_num(gpio);
}

uint pwm_channel_for_gpio(uint gpio) {
    return pwm_gpio_to_channel(gpio);
}
