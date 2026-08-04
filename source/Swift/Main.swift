private let motorRunDurationShortMs: UInt32 = 400
private let motorRunDurationMs: UInt32 = 1000
private let motorRunDurationLongMs: UInt32 = 2000
private let cooldownDurationMs: UInt32 = 1000
private let buzzerPulseDurationMs: UInt32 = 120
private let buzzerDutyOn: UInt16 = 128
private let progressSegmentCount: UInt32 = 12

private let ultrasonicTriggerDistanceCM: Int = 5
private let ultrasonicPollIntervalMs: UInt32 = 200

private let buttonPin = GPIOPin(pin: 26)
private let buttonLessPin = GPIOPin(pin: 27)
private let buttonMorePin = GPIOPin(pin: 18)
private let motorPinA = GPIOPin(pin: 1)
private let motorPinB = GPIOPin(pin: 2)
private let buzzerPositivePin = GPIOPin(pin: 6)

// Set these to your actual ultrasonic pins.
private let ultrasonicTrigPin = GPIOPin(pin: 14)
private let ultrasonicEchoPin = GPIOPin(pin: 15)

private enum TriggerSource {
	case less
	case normal
	case more
}

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

private func runMotorForward(_ motorA: inout DigitalOut, _ motorB: inout DigitalOut, durationMs: UInt32) {
	motorA.set(true)
	motorB.set(false)

	let durationUs = durationMs * 1000
	let startedAtUs = time_us_32()
	let firstFrameStartedAtUs = time_us_32()
	ssd1309_show_dispensing_progress(0)
	var measuredFrameDurationUs = max(time_us_32() &- firstFrameStartedAtUs, 1)
	var renderedSegmentCount: UInt32 = 0

	while renderedSegmentCount < progressSegmentCount {
		let elapsedUs = min(time_us_32() &- startedAtUs, durationUs)
		let predictedFrameEndUs = min(elapsedUs + measuredFrameDurationUs, durationUs)
		let segmentAtPredictedEnd = (
			predictedFrameEndUs * progressSegmentCount + durationUs - 1,
		) / durationUs
		let nextSegmentCount = min(
			max(renderedSegmentCount + 1, segmentAtPredictedEnd),
			progressSegmentCount,
		)
		let nextSegmentDeadlineUs = (durationUs * nextSegmentCount) / progressSegmentCount
		let nextFrameStartUs = nextSegmentDeadlineUs > measuredFrameDurationUs
			? nextSegmentDeadlineUs - measuredFrameDurationUs
			: elapsedUs

		let elapsedBeforeFrameUs = min(time_us_32() &- startedAtUs, durationUs)
		if elapsedBeforeFrameUs < nextFrameStartUs {
			sleep_us(UInt64(nextFrameStartUs - elapsedBeforeFrameUs))
		}

		let frameStartedAtUs = time_us_32()
		ssd1309_show_dispensing_progress(UInt8(nextSegmentCount))
		measuredFrameDurationUs = max(time_us_32() &- frameStartedAtUs, 1)
		renderedSegmentCount = nextSegmentCount
	}

	let elapsedAfterFinalFrameUs = min(time_us_32() &- startedAtUs, durationUs)
	if elapsedAfterFinalFrameUs < durationUs {
		sleep_us(UInt64(durationUs - elapsedAfterFinalFrameUs))
	}

	motorA.set(false)
	motorB.set(false)
}

private func runActivationSequence(
	statusLed: inout DigitalOut,
	motorA: inout DigitalOut,
	motorB: inout DigitalOut,
	buzzer: inout PWMOut,
	durationMs: UInt32,
) {
	flashLED(&statusLed, times: 2)
	ssd1309_show_going()
	playBuzzerPulse(&buzzer)
	runMotorForward(&motorA, &motorB, durationMs: durationMs)
	ssd1309_show_done()
	sleep_ms(cooldownDurationMs)
	ssd1309_show_not_going()
}

private func ultrasonicTriggered(_ ultrasonic: inout UltrasonicSensor) -> Bool {
	let distance = ultrasonic.readDistanceCM()
	return distance > 0 && distance < ultrasonicTriggerDistanceCM
}

private func updateButtonDisplay(buttonNormal: DigitalIn, buttonLess: DigitalIn, buttonMore: DigitalIn) {
	ssd1309_update_button_states(!buttonLess.read(), !buttonNormal.read(), !buttonMore.read())
}

private func waitForTrigger(
	buttonNormal: DigitalIn,
	buttonLess: DigitalIn,
	buttonMore: DigitalIn,
	ultrasonic: inout UltrasonicSensor,
) -> TriggerSource {
	while true {
		updateButtonDisplay(buttonNormal: buttonNormal, buttonLess: buttonLess, buttonMore: buttonMore)

		if !buttonLess.read() {
			return .less
		}

		if !buttonMore.read() {
			return .more
		}

		if !buttonNormal.read() {
			return .normal
		}

		if ultrasonicTriggered(&ultrasonic) {
			return .more
		}

		sleep_ms(ultrasonicPollIntervalMs)
	}
}

private func waitForRearm(
	buttonNormal: DigitalIn,
	buttonLess: DigitalIn,
	buttonMore: DigitalIn,
	ultrasonic: inout UltrasonicSensor,
) {
	while true {
		updateButtonDisplay(buttonNormal: buttonNormal, buttonLess: buttonLess, buttonMore: buttonMore)

		let anyButtonPressed = !buttonNormal.read() || !buttonLess.read() || !buttonMore.read()
		let objectNear = ultrasonicTriggered(&ultrasonic)

		if !anyButtonPressed, !objectNear {
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
		let buttonLess = DigitalIn(pin: buttonLessPin, pullUp: true)
		let buttonMore = DigitalIn(pin: buttonMorePin, pullUp: true)
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
			let trigger = waitForTrigger(
				buttonNormal: button,
				buttonLess: buttonLess,
				buttonMore: buttonMore,
				ultrasonic: &ultrasonic,
			)

			let durationMs: UInt32 = switch trigger {
				case .less:
					motorRunDurationShortMs
				case .normal:
					motorRunDurationMs
				case .more:
					motorRunDurationLongMs
			}

			runActivationSequence(
				statusLed: &statusLed,
				motorA: &motorA,
				motorB: &motorB,
				buzzer: &buzzer,
				durationMs: durationMs,
			)

			waitForRearm(
				buttonNormal: button,
				buttonLess: buttonLess,
				buttonMore: buttonMore,
				ultrasonic: &ultrasonic,
			)
		}
	}
}
