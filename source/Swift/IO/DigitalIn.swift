struct DigitalIn {
  let pin: GPIOPin

  init(pin: GPIOPin, pullUp: Bool = false) {
    self.pin = pin
    gpio_init(pin.pin)
    gpio_set_dir(pin.pin, false)
    if pullUp {
      gpio_pull_up(pin.pin)
    } else {
      gpio_pull_down(pin.pin)
    }
  }

  func read() -> Bool {
    gpio_get(pin.pin)
  }
}
