struct UltrasonicSensor {
  var trig: DigitalOut
  let echo: DigitalIn

  init(trigPin: GPIOPin, echoPin: GPIOPin) {
    trig = DigitalOut(pin: trigPin)
    echo = DigitalIn(pin: echoPin)
  }

  mutating func readDistanceCM() -> Int {
    // Ensure clean trigger pulse
    trig.set(false)
    sleep_us(2)

    trig.set(true)
    sleep_us(10)
    trig.set(false)

    // Measure echo pulse width
    let duration = measurePulseWidthUs(
      pin: echo.pin,
      value: true,
      timeoutUs: 20000,
    )

    if duration < 0 {
      return -1
    }

    // Convert microseconds to cm (HC-SR04 scaling)
    return Int(duration) / 58
  }

  private func measurePulseWidthUs(
    pin: GPIOPin,
    value: Bool,
    timeoutUs: UInt32,
  ) -> Int32 {
    let startTimeout = time_us_32()

    // Wait for echo to go HIGH
    while gpio_get(pin.pin) != value {
      if time_us_32() - startTimeout >= timeoutUs {
        return -1
      }
    }

    // Capture start of pulse
    let pulseStart = time_us_32()

    // Wait for echo to go LOW again
    while gpio_get(pin.pin) == value {
      if time_us_32() - pulseStart >= timeoutUs {
        return -1
      }
    }

    return Int32(time_us_32() - pulseStart)
  }
}
