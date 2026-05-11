#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "PWMHelpers.h"
#include "DisplaySSD1309.h"

void multicore_launch_core1(void (*entry)(void));

static inline void usb_wait_core()
{
    stdio_init_all();
    sleep_ms(2000);
}

static inline void launch_usb_core()
{
    multicore_launch_core1(usb_wait_core);
}

static inline void print(const char *s)
{
    printf("%s\n", s);
}
