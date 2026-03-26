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

    enum MotorSelection {
        case motor1
        case motor2
    }

    struct Pin {
        let number: UInt32
        var state: pinState = .high

         init(number: UInt32) {
            self.number = number
            gpio_init(number)
            gpio_set_dir(number, false) // starting state
        }

        mutating func on() {
            state.on()
            gpio_put(number, true)
        }

        mutating func off() {
            state.off()
            gpio_put(number, false)
        }

        mutating func toggle() {
            state.toggle()
            gpio_put(number, state == .high)
        }


    }

    struct PWMPin {
        let slice: UInt32
        let channel: UInt32
        var level: Int32 = 0

        init(pin: UInt32) {
            gpio_set_function(pin, GPIO_FUNC_PWM)
            slice = pwm_slice_for_gpio(pin)
            channel = pwm_channel_for_gpio(pin)
            var config = pwm_get_default_config()
            pwm_init(slice, &config, true)
            pwm_set_wrap(slice, 255)
        }

        mutating func setLevel(_ newLevel: Int32) {
            level = max(0, min(255, newLevel))
            pwm_set_chan_level(slice, channel, UInt16(level))
        }
    }

    struct Motor {
        var pin1: Pin
        var pin2: Pin
    }

    struct MotorDriver {
        var pwmPin: PWMPin
        var motor1: Motor
        var motor2: Motor

        init(pwmPinNumber: UInt32, motor1: Motor, motor2: Motor) {
            pwmPin = PWMPin(pin: pwmPinNumber)
            self.motor1 = motor1
            self.motor2 = motor2
        }

        mutating func setMotor1Direction(forward: Bool) {
            if forward {
                motor1.pin1.on()
                motor1.pin2.off()
            } else {
                motor1.pin1.off()
                motor1.pin2.on()
            }
        }

        mutating func setMotor2Direction(forward: Bool) {
            if forward {
                motor2.pin1.on()
                motor2.pin2.off()
            } else {
                motor2.pin1.off()
                motor2.pin2.on()
            }
        }

        mutating func setMotorSpeed(_ speed: Float) {
            let scaledSpeed = Int32(max(0.0, min(1.0, speed)) * 255.0)
            pwmPin.setLevel(scaledSpeed)
        }

        mutating func turnMotor1(on: Bool) {
            if on {
                motor1.pin1.on()
                motor1.pin2.off()
            } else {
                motor1.pin1.off()
                motor1.pin2.off()
            }
        }

        mutating func turnMotor2(on: Bool) {
            if on {
                motor2.pin1.on()
                motor2.pin2.off()
            } else {
                motor2.pin1.off()
                motor2.pin2.off()
            }
        }

        mutating func enableMotor(_ motor: MotorSelection, for duration: Float) {
            let durationMs = Int32(max(0.0, duration) * 1000.0)

            switch motor {
            case .motor1:
                turnMotor1(on: true)
            case .motor2:
                turnMotor2(on: true)
            }

            let callback: @convention(c) (Int32, UnsafeMutableRawPointer?) -> Int64 = { id, userData in
                let driver = userData!.assumingMemoryBound(to: MotorDriver.self)
                switch motor {
                case .motor1:
                    driver.pointee.turnMotor1(on: false)
                case .motor2:
                    driver.pointee.turnMotor2(on: false)
                }
                return 0
            }

            add_alarm_in_ms(durationMs, callback, &self, false)
        }
    }



    static func main() {
    
        // debug print init
        launch_usb_core()

        // status LED
        var statusPWM = PWMPin(pin: UInt32(PICO_DEFAULT_LED_PIN))
        var statusDirection: Int32 = 10

        // motor driver
        

        while true {
            statusPWM.setLevel(statusPWM.level + direction)
            if statusPWM.level >= 255 { direction = -1 }
            if statusPWM.level <= 0   { direction = 1 }
            sleep_ms(6)
        }
    }
}
