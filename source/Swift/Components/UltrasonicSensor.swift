struct UltrasonicSensor {
  var trig: DigitalOut
  let echo: DigitalIn

  init(trigPin: GPIOPin, echoPin: GPIOPin) {
    trig = DigitalOut(pin: trigPin)
    echo = DigitalIn(pin: echoPin)
  }

  mutating func readDistanceCM() -> Int {
    trig.set(true)
    sleep_us(10)
    trig.set(false)

    let duration = measurePulseWidthUs(
      pin: echo.pin, value: true, timeoutUs: 6000)

    if duration < 0 {
      return -1
    }

    return Int(duration / 58)
  }

  private func measurePulseWidthUs(pin: GPIOPin, value: Bool, timeoutUs: UInt32)
    -> Int32
  {
    let desired: Bool = value
    let startWait = time_us_32()

    // Wait for the pulse to start.
    while gpio_get(pin.pin) != desired {
      if time_us_32() - startWait >= timeoutUs {
        return -1
      }
    }

    let pulseStart = time_us_32()

    // Wait for the pulse to end.
    while gpio_get(pin.pin) == desired {
      if time_us_32() - pulseStart >= timeoutUs {
        return -1
      }
    }

    return Int32(time_us_32() - pulseStart)
  }
}
