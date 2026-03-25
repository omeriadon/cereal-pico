//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift open source project
//
// Copyright (c) 2023 Apple Inc. and the Swift project authors.
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
//
//===----------------------------------------------------------------------===//

@main
struct Main {
    static func main() {
        let pin = UInt32(PICO_DEFAULT_LED_PIN)

        gpio_set_function(pin, GPIO_FUNC_PWM)
        let slice = pwm_slice_for_gpio(pin)
        let channel = pwm_channel_for_gpio(pin)

        var config = pwm_get_default_config()
        pwm_init(slice, &config, true)
        pwm_set_wrap(slice, 255)

        var level: Int32 = 0
        var direction: Int32 = 10

        while true {
            pwm_set_chan_level(slice, channel, UInt16(level))
            level += direction
            if level >= 255 { direction = -1 }
            if level <= 0   { direction = 1 }
            sleep_ms(6)
        }
    }
}
