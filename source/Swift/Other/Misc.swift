

func sleep(_ seconds: Int) {
  sleep_ms(UInt32(seconds * 1000))
}

var defaultLEDPin: UInt32 {
  UInt32(PICO_DEFAULT_LED_PIN)
}
