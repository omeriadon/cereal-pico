@main
struct Main {

    enum pinState {
        case high, low

        mutating func toggle() {
            switch self {
            case .high: self = .low
            case .low: self = .high
            }
        }

        mutating func on() {
            self = .high
        }

        mutating func off() {
            self = .low
        }
    }

    struct Pin {
        var state: pinState = .low
        var name: String
    }







    static func main() {
    
        multicore_launch_core1(usb_wait_core)


        let pin = UInt32(PICO_DEFAULT_LED_PIN)
    
        gpio_set_function(pin, GPIO_FUNC_PWM)
        let slice = pwm_slice_for_gpio(pin)
        let channel = pwm_channel_for_gpio(pin)
    
        var config = pwm_get_default_config()
        pwm_init(slice, &config, true)
        pwm_set_wrap(slice, 255)
    
        var level: Int32 = 0
        var direction: Int32 = 10
    
        sleep_ms(2000)
        print("Starting PWM...\n")
    
        while true {
            pwm_set_chan_level(slice, channel, UInt16(level))
            level += direction
            if level >= 255 { direction = -1 }
            if level <= 0   { direction = 1 }
            sleep_ms(6)
        }
  }
}
