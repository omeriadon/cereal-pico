struct PWMOut {
  let pin: GPIOPin
  let slice: UInt32
  let channel: UInt32

  init(_ pin: GPIOPin) {
    self.pin = pin

    gpio_set_function(pin.pin, GPIO_FUNC_PWM)

    self.slice = pwm_gpio_to_slice_num(pin.pin)
    self.channel = pwm_gpio_to_channel(pin.pin)

    pwm_set_wrap(slice, UInt16(255))
    pwm_set_enabled(slice, true)
  }

  func setDuty(_ value: UInt16) {
    pwm_set_chan_level(slice, channel, value)
  }
}
