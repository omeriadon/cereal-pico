private let motorRunDurationMs: UInt32 = 2000
private let cooldownDurationMs: UInt32 = 3000
private let buzzerPulseDurationMs: UInt32 = 120
private let buzzerDutyOn: UInt16 = 128

private let ultrasonicTriggerDistanceCM: Int = 5
private let ultrasonicPollIntervalMs: UInt32 = 200

private let buttonPin = GPIOPin(pin: 26)
private let motorPinA = GPIOPin(pin: 1)
private let motorPinB = GPIOPin(pin: 2)
private let buzzerPositivePin = GPIOPin(pin: 6)

// Set these to your actual ultrasonic pins.
private let ultrasonicTrigPin = GPIOPin(pin: 14)
private let ultrasonicEchoPin = GPIOPin(pin: 15)

private func flashLED(_ led: inout DigitalOut, times: Int, onDurationMs: UInt32 = 80, offDurationMs: UInt32 = 80) {
    guard times > 0 else { return }

    for _ in 0 ..< times {
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
    buzzer: inout PWMOut,
) {
    flashLED(&statusLed, times: 2)
    ssd1309_show_going()
    playBuzzerPulse(&buzzer)
    runMotorForward(&motorA, &motorB)
    ssd1309_show_done()
    sleep_ms(cooldownDurationMs)
    ssd1309_show_not_going()
}

private func ultrasonicTriggered(_ ultrasonic: inout UltrasonicSensor) -> Bool {
    let distance = ultrasonic.readDistanceCM()
    return distance > 0 && distance < ultrasonicTriggerDistanceCM
}

private func waitForTrigger(button: DigitalIn, ultrasonic: inout UltrasonicSensor) {
    while true {
        if !button.read() {
            return
        }

        if ultrasonicTriggered(&ultrasonic) {
            return
        }

        sleep_ms(ultrasonicPollIntervalMs)
    }
}

private func waitForRearm(button: DigitalIn, ultrasonic: inout UltrasonicSensor) {
    while true {
        let buttonPressed = !button.read()
        let objectNear = ultrasonicTriggered(&ultrasonic)

        if !buttonPressed, !objectNear {
            return
        }

        sleep_ms(ultrasonicPollIntervalMs)
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
        var buzzer = PWMOut(buzzerPositivePin, frequencyHz: 2000)
        var ultrasonic = UltrasonicSensor(trigPin: ultrasonicTrigPin, echoPin: ultrasonicEchoPin)

        statusLed.set(false)
        motorA.set(false)
        motorB.set(false)
        flashLED(&statusLed, times: 2)
        ssd1309_show_not_going()

        while true {
            waitForTrigger(button: button, ultrasonic: &ultrasonic)

            runActivationSequence(
                statusLed: &statusLed,
                motorA: &motorA,
                motorB: &motorB,
                buzzer: &buzzer,
            )

            waitForRearm(button: button, ultrasonic: &ultrasonic)
        }
    }
}
