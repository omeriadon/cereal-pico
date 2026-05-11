@main
struct Main {
    static func main() {
        launch_usb_core()

        ssd1309_init_default_spi()
        ssd1309_show_resize_centered()

        let ledPin = UInt32(PICO_DEFAULT_LED_PIN)
        gpio_init(ledPin)
        gpio_set_dir(ledPin, true)

        var isLedOn = false

        while true {
            isLedOn.toggle()
            gpio_put(ledPin, isLedOn)
            sleep_ms(500)
        }
    }
}
