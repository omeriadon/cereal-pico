struct UltrasonicSensor {
  let trig: DigitalOut
  let echo: DigitalIn

  init(trigPin: UInt32, echoPin: UInt32) {
    trig = DigitalOut(pin: trigPin)
    echo = DigitalIn(pin: echoPin)
  }

  func readDistanceCM() -> Int {
    trig.set(true)
    sleep_us(10)
    trig.set(false)

    let duration = time_pulse_us(echo.pin, true, 6000)

    if duration < 0 {
      return -1
    }

    return Int(duration / 58)
  }
}
