@main
struct Main {
  static func main() {
    initialize()

	// random display something
	// its lib code requires some pins, we need to override that or account for it when placing pins.
    ssd1309_show_resize_centered()

    var led = PWMOut(pin: defaultLEDPin)

    let cycleSeconds: Double = 5.0
    let steps: UInt16 = 255

    let stepTime = cycleSeconds / Double(steps)

    while true {
      // fade up
      for i in 0...steps {
        led.setDuty(i)
        sleep(stepTime)
      }

      // fade down
      for i in (0...steps).reversed() {
        led.setDuty(i)
        sleep(stepTime)
      }
    }
  }
}
