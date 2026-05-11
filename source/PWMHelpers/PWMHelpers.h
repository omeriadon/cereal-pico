#pragma once
#include "hardware/pwm.h"
#include "hardware/uart.h"

uint pwm_slice_for_gpio(uint gpio);
uint pwm_channel_for_gpio(uint gpio);
