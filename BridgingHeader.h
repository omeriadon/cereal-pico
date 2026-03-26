#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "PWMHelpers.h"

static inline void usb_wait_core()
{
    stdio_init_all();
    while (!tud_cdc_connected())
    {
        sleep_ms(10);
    }
}

static inline void print_init()
{
    multicore_launch_core1(usb_wait_core);
}

static inline void print(const char *s)
{
    printf("%s", s);
}
