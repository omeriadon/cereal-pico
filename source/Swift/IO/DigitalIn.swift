struct DigitalIn {
  let pin: GPIOPin

  init(pin: GPIOPin) {
    self.pin = pin
    gpio_init(pin.pin)
    gpio_set_dir(pin.pin, false)
  }

  func read() -> Bool {
    gpio_get(pin.pin)
  }
}
