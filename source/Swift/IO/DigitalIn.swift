struct DigitalIn {
  let pin: UInt32

  init(pin: UInt32) {
    self.pin = pin
    gpio_init(pin)
    gpio_set_dir(pin, false)
  }

  func read() -> Bool {
    gpio_get(pin)
  }
}
