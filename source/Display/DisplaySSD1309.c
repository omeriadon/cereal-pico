#include "DisplaySSD1309.h"

#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

static const uint8_t OLED_PIN_SCK = 10;
static const uint8_t OLED_PIN_MOSI = 11;
static const uint8_t OLED_PIN_RST = 12;
static const uint8_t OLED_PIN_CS = 13;
static const uint8_t OLED_PIN_DC = 14;

static spi_inst_t *const OLED_SPI = spi1;
static uint8_t oledBackBuffer[SSD1309_FRAMEBUFFER_SIZE];

const uint32_t ssd1309_width = SSD1309_WIDTH;
const uint32_t ssd1309_height = SSD1309_HEIGHT;
const uint32_t ssd1309_page_count = SSD1309_PAGE_COUNT;
const uint32_t ssd1309_framebuffer_size = SSD1309_FRAMEBUFFER_SIZE;

const uint8_t resize_bitmap_width = 34;
const uint8_t resize_bitmap_page_count = 4;
const uint32_t resize_bitmap_length = 136;

const uint8_t resize_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfc,
    0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x1f, 0xfc,
    0x00, 0x00, 0x1f, 0xbc, 0x00, 0x00, 0x7e, 0x3c, 0x00, 0x00, 0xfc, 0x3c, 0x00, 0x01, 0xf8, 0x3c,
    0x00, 0x01, 0xf8, 0x38, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3c, 0x00, 0x00, 0xe0, 0x7c, 0x00, 0x00, 0xe0, 0xfc, 0x00, 0x00, 0xe1, 0xf8, 0x00, 0x00,
    0xe3, 0xf0, 0x00, 0x00, 0xe7, 0xe0, 0x00, 0x00, 0xef, 0xc0, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xfd, 0x60, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00,
    0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00,
};

const uint8_t *ssd1309_resize_bitmap_pointer(void)
{
    return resize_bitmap;
}

static inline void ssd1309_write_command(uint8_t command)
{
    gpio_put(OLED_PIN_DC, 0);
    gpio_put(OLED_PIN_CS, 0);
    spi_write_blocking(OLED_SPI, &command, 1);
    gpio_put(OLED_PIN_CS, 1);
}

static inline void ssd1309_write_data(const uint8_t *data, uint32_t length)
{
    gpio_put(OLED_PIN_DC, 1);
    gpio_put(OLED_PIN_CS, 0);
    spi_write_blocking(OLED_SPI, data, length);
    gpio_put(OLED_PIN_CS, 1);
}

static void ssd1309_set_full_window(void)
{
    ssd1309_write_command(0x21); // Set column address
    ssd1309_write_command(0x00);
    ssd1309_write_command(SSD1309_WIDTH - 1);

    ssd1309_write_command(0x22); // Set page address
    ssd1309_write_command(0x00);
    ssd1309_write_command(SSD1309_PAGE_COUNT - 1);
}

void ssd1309_init_default_spi(void)
{
    spi_init(OLED_SPI, 10 * 1000 * 1000); // 10 MHz
    gpio_set_function(OLED_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(OLED_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(OLED_PIN_CS);
    gpio_set_dir(OLED_PIN_CS, GPIO_OUT);
    gpio_put(OLED_PIN_CS, 1);

    gpio_init(OLED_PIN_DC);
    gpio_set_dir(OLED_PIN_DC, GPIO_OUT);
    gpio_put(OLED_PIN_DC, 1);

    gpio_init(OLED_PIN_RST);
    gpio_set_dir(OLED_PIN_RST, GPIO_OUT);
    gpio_put(OLED_PIN_RST, 1);

    sleep_ms(10);
    gpio_put(OLED_PIN_RST, 0);
    sleep_ms(20);
    gpio_put(OLED_PIN_RST, 1);
    sleep_ms(20);

    ssd1309_write_command(0xAE); // Display off

    ssd1309_write_command(0xD5); // Display clock divide ratio / oscillator
    ssd1309_write_command(0x70);

    ssd1309_write_command(0xA8); // Multiplex ratio
    ssd1309_write_command(0x3F);

    ssd1309_write_command(0xD3); // Display offset
    ssd1309_write_command(0x00);

    ssd1309_write_command(0x40); // Start line = 0

    ssd1309_write_command(0xAD); // DC-DC control
    ssd1309_write_command(0x8B); // Enable internal DC-DC

    ssd1309_write_command(0xA1); // Segment remap
    ssd1309_write_command(0xC8); // COM scan direction remapped

    ssd1309_write_command(0xDA); // COM pins hardware configuration
    ssd1309_write_command(0x12);

    ssd1309_write_command(0x81); // Contrast
    ssd1309_write_command(0x7F);

    ssd1309_write_command(0xD9); // Pre-charge period
    ssd1309_write_command(0x22);

    ssd1309_write_command(0xDB); // VCOMH deselect level
    ssd1309_write_command(0x20);

    ssd1309_write_command(0x20); // Memory addressing mode
    ssd1309_write_command(0x00); // Horizontal addressing mode

    ssd1309_write_command(0xA4); // Entire display follows RAM
    ssd1309_write_command(0xA6); // Normal (not inverted)

    ssd1309_set_full_window();

    ssd1309_write_command(0xAF); // Display on

    ssd1309_clear();
}

void ssd1309_display_buffer(const uint8_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return;
    }

    uint32_t clippedLength = length;
    if (clippedLength > SSD1309_FRAMEBUFFER_SIZE) {
        clippedLength = SSD1309_FRAMEBUFFER_SIZE;
    }

    ssd1309_set_full_window();
    ssd1309_write_data(buffer, clippedLength);
}

void ssd1309_fill(uint8_t value)
{
    memset(oledBackBuffer, value, sizeof(oledBackBuffer));
    ssd1309_display_buffer(oledBackBuffer, sizeof(oledBackBuffer));
}

void ssd1309_clear(void)
{
    ssd1309_fill(0x00);
}

void ssd1309_show_resize_centered(void)
{
    memset(oledBackBuffer, 0x00, sizeof(oledBackBuffer));

    const uint8_t centeredX = (SSD1309_WIDTH - resize_bitmap_width) / 2;
    const uint8_t centeredPage = (SSD1309_PAGE_COUNT - resize_bitmap_page_count) / 2;

    ssd1309_draw_bitmap_pages(
        oledBackBuffer,
        sizeof(oledBackBuffer),
        centeredX,
        centeredPage,
        resize_bitmap,
        resize_bitmap_width,
        resize_bitmap_page_count);

    ssd1309_display_buffer(oledBackBuffer, sizeof(oledBackBuffer));
}

void ssd1309_draw_bitmap_pages(
    uint8_t *framebuffer,
    uint32_t framebufferLength,
    uint8_t x,
    uint8_t yPage,
    const uint8_t *bitmap,
    uint8_t bitmapWidth,
    uint8_t bitmapPageCount)
{
    if (framebuffer == NULL || bitmap == NULL) {
        return;
    }

    if (framebufferLength < SSD1309_FRAMEBUFFER_SIZE) {
        return;
    }

    if (x >= SSD1309_WIDTH || yPage >= SSD1309_PAGE_COUNT || bitmapWidth == 0 || bitmapPageCount == 0) {
        return;
    }

    uint8_t copyWidth = bitmapWidth;
    if ((uint16_t)x + copyWidth > SSD1309_WIDTH) {
        copyWidth = SSD1309_WIDTH - x;
    }

    for (uint8_t sourcePage = 0; sourcePage < bitmapPageCount; sourcePage++) {
        uint8_t destinationPage = yPage + sourcePage;
        if (destinationPage >= SSD1309_PAGE_COUNT) {
            break;
        }

        uint32_t destinationIndex = (uint32_t)destinationPage * SSD1309_WIDTH + x;
        uint32_t sourceIndex = (uint32_t)sourcePage * bitmapWidth;
        memcpy(&framebuffer[destinationIndex], &bitmap[sourceIndex], copyWidth);
    }
}
