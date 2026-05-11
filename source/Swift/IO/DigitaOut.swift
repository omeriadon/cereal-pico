struct DigitalOut {
  let pin: GPIOPin

  var isOn: Bool = false {
    didSet {
      gpio_put(pin.pin, isOn)
    }
  }

  init(pin: GPIOPin) {
    self.pin = pin
    gpio_init(pin.pin)
    gpio_set_dir(pin.pin, true)
  }

  mutating func set(_ value: Bool) {
    isOn = value
  }

  mutating func toggle() {
    isOn.toggle()
  }
}
