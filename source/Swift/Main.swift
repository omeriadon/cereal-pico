@main
struct Main {
  static func main() {
    initialize()

	// random display something
	// its lib code requires some pins, we need to override that or account for it when placing pins.
    ssd1309_show_resize_centered()

    let led = DigitalOut(pin: defaultLEDPin)

    while true {
      led.toggle()
      sleep(0.5)
    }
  }
}
