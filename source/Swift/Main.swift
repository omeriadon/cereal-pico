@main
struct Main {
  static func main() {
    initialize()

    // random display something
    // its lib code requires some pins, we need to override that or account for it when placing pins.
    ssd1309_show_resize_centered()

    // light stuff
    var led = PWMOut(pin: defaultLEDPin)
    let cycleSeconds: Double = 5.0
    let steps: UInt16 = 255
    let stepTime = cycleSeconds / Double(steps)

    // timer to make something work idk
    var ledTimer = IntervalTask(intervalSec: 0.01)
    var sensorTimer = IntervalTask(intervalSec: 0.1)

    while true {
      let now = time_us_32()

      if ledTimer.shouldRun(now: now) {
        brightness = UInt16(Int(brightness) + Int(direction))

        if brightness == 0 || brightness == 255 {
          direction *= -1
        }

        led.setDuty(brightness)
      }

      if sensorTimer.shouldRun(now: now) {
        let d = sensor.readDistanceCM()
        print(d)
      }
    }

  }
}
