private let motorRunDurationMs: UInt32 = 2_000
private let cooldownDurationMs: UInt32 = 3_000
private let buzzerPulseDurationMs: UInt32 = 120
private let buzzerDutyOn: UInt16 = 128

private let buttonPin = GPIOPin(pin: 26)
private let motorPinA = GPIOPin(pin: 1)
private let motorPinB = GPIOPin(pin: 2)
private let buzzerPositivePin = GPIOPin(pin: 6)

private func flashLED(_ led: inout DigitalOut, times: Int, onDurationMs: UInt32 = 80, offDurationMs: UInt32 = 80) {
    guard times > 0 else {
        return
    }

    for _ in 0..<times {
        led.set(true)
        sleep_ms(onDurationMs)
        led.set(false)
        sleep_ms(offDurationMs)
    }
}

private func playBuzzerPulse(_ buzzer: inout PWMOut) {
    buzzer.setDuty(buzzerDutyOn)
    sleep_ms(buzzerPulseDurationMs)
    buzzer.stop()
}

private func runMotorForward(_ motorA: inout DigitalOut, _ motorB: inout DigitalOut) {
    motorA.set(true)
    motorB.set(false)
    sleep_ms(motorRunDurationMs)
    motorA.set(false)
    motorB.set(false)
}

private func runActivationSequence(
    statusLed: inout DigitalOut,
    motorA: inout DigitalOut,
    motorB: inout DigitalOut,
    buzzer: inout PWMOut
) {
    flashLED(&statusLed, times: 2)
    ssd1309_show_going()
    playBuzzerPulse(&buzzer)
    runMotorForward(&motorA, &motorB)
    ssd1309_show_done()
    sleep_ms(cooldownDurationMs)
    ssd1309_show_not_going()
}

private func waitForButtonPress(_ button: DigitalIn) {
    while true {
        while button.read() {
            sleep_ms(10)
        }

        sleep_ms(20)
        if !button.read() {
            return
        }
    }
}

private func waitForButtonRelease(_ button: DigitalIn) {
    while true {
        while !button.read() {
            sleep_ms(10)
        }

        sleep_ms(20)
        if button.read() {
            return
        }
    }
}

@main
struct Main {
    static func main() {
        initialize()

        var statusLed = DigitalOut(pin: GPIOPin(pin: defaultLEDPin))
        let button = DigitalIn(pin: buttonPin, pullUp: true)
        var motorA = DigitalOut(pin: motorPinA)
        var motorB = DigitalOut(pin: motorPinB)
        var buzzer = PWMOut(buzzerPositivePin, frequencyHz: 2_000)

        statusLed.set(false)
        motorA.set(false)
        motorB.set(false)
        flashLED(&statusLed, times: 2)
        ssd1309_show_not_going()

        while true {
            waitForButtonPress(button)
            runActivationSequence(
                statusLed: &statusLed,
                motorA: &motorA,
                motorB: &motorB,
                buzzer: &buzzer
            )
            waitForButtonRelease(button)
        }
    }
}
