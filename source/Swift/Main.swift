@main
struct Main {
  static func main() {
    initialize()

    // random display something
    // its lib code requires some pins, we need to override that or account for it when placing pins.
    ssd1309_show_resize_centered()

    // light stuff
    let led = PWMOut(GPIOPin(pin: defaultLEDPin))
    var brightness: UInt16 = 0
    var direction: Int = 1

    // timers (IntervalTask uses whole seconds)
    var ledTimer = IntervalTask(intervalSec: 0)
    var sensorTimer = IntervalTask(intervalSec: 1)

    var sensor = UltrasonicSensor(
      trigPin: GPIOPin(pin: 2),
      echoPin: GPIOPin(pin: 3))

    while true {
      let now = time_us_32()

      if ledTimer.shouldRun(now: now) {
        let next = Int(brightness) + direction
        if next <= 0 {
          brightness = 0
          direction = 1
        } else if next >= 255 {
          brightness = 255
          direction = -1
        } else {
          brightness = UInt16(next)
        }

        led.setDuty(brightness)
      }

      if sensorTimer.shouldRun(now: now) {
        let d = sensor.readDistanceCM()
        _ = d
      }

      // Prevent this loop from spinning at full speed.
      sleep_ms(1)
    }

  }
}
