struct PWMOut {
  let pin: GPIOPin
  let slice: UInt32
  let channel: UInt32
  let wrap: UInt16

  init(_ pin: GPIOPin, frequencyHz: UInt32 = 2000, duty: UInt16 = 0) {
    self.pin = pin

    gpio_set_function(pin.pin, GPIO_FUNC_PWM)

    slice = pwm_gpio_to_slice_num(pin.pin)
    channel = pwm_gpio_to_channel(pin.pin)
    wrap = UInt16(255)

    pwm_set_wrap(slice, wrap)
    let targetFrequency = Double(frequencyHz)
    let clockDivider = 125_000_000.0 / (targetFrequency * Double(wrap + 1))
    pwm_set_clkdiv(slice, Float(clockDivider))
    pwm_set_chan_level(slice, channel, duty)
    pwm_set_enabled(slice, true)
  }

  func setDuty(_ value: UInt16) {
    pwm_set_chan_level(slice, channel, value)
  }

  func stop() {
    pwm_set_chan_level(slice, channel, 0)
  }
}
