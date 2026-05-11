struct DigitalOut {
  let pin: UInt32

  var isOn: Bool = false {
    didSet {
      gpio_put(pin, isOn)
    }
  }

  init(pin: UInt32) {
    self.pin = pin
    gpio_init(pin)
    gpio_set_dir(pin, true)
  }

  mutating func set(_ value: Bool) {
    isOn = value
  }

  mutating func toggle() {
    isOn.toggle()
  }
}
