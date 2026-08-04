#include "DisplaySSD1309.h"

#include <stdbool.h>
#include <string.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Wired from the user's setup; use bit-banged SPI so any GPIO pins work.
static const uint8_t OLED_PIN_RST = 13;
static const uint8_t OLED_PIN_DC = 22;
static const uint8_t OLED_PIN_CS = 19;
static const uint8_t OLED_PIN_SCK = 28;
static const uint8_t OLED_PIN_MOSI = 20;

static uint8_t oledBackBuffer[SSD1309_FRAMEBUFFER_SIZE];

const uint32_t ssd1309_width = SSD1309_WIDTH;
const uint32_t ssd1309_height = SSD1309_HEIGHT;
const uint32_t ssd1309_page_count = SSD1309_PAGE_COUNT;
const uint32_t ssd1309_framebuffer_size = SSD1309_FRAMEBUFFER_SIZE;

const uint8_t resize_bitmap_width = 34;
const uint8_t resize_bitmap_page_count = 4;
const uint32_t resize_bitmap_length = 136;

const uint8_t resize_bitmap[] = {
    // Page 3
    0x00, 0x00, 0xbf, 0x06, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x1f, 0x00, 0x00, 0xff, 0x0f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 
    // Page 2
    0x07, 0x3e, 0x00, 0x00, 0x07, 0x3f, 0x00, 0x00, 0x87, 0x1f, 0x00, 0x00, 0xc7, 0x0f, 0x00, 0x00,
    0xe7, 0x07, 0x00, 0x00, 0xf7, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x7f, 0x00, 
    // Page 1
    0xf8, 0x3d, 0x00, 0x00, 0x7e, 0x3c, 0x00, 0x00, 0x3f, 0x3c, 0x00, 0x80, 0x1f, 0x3c, 0x00, 0x80,
    0x1f, 0x1c, 0x00, 0x80, 0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c,
    0x00, 0x00, 
    // Page 0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x1f, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00, 0xff, 0x3f,
    0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0xf8, 0x3f,
    0x00, 0x01
};

const uint8_t *ssd1309_resize_bitmap_pointer(void)
{
    return resize_bitmap;
}

static inline void ssd1309_write_command(uint8_t command)
{
    gpio_put(OLED_PIN_DC, 0);
    gpio_put(OLED_PIN_CS, 0);
    for (int bit = 7; bit >= 0; bit--) {
        gpio_put(OLED_PIN_SCK, 0);
        gpio_put(OLED_PIN_MOSI, (command >> bit) & 1u);
        sleep_us(1);
        gpio_put(OLED_PIN_SCK, 1);
        sleep_us(1);
    }
    gpio_put(OLED_PIN_SCK, 0);
    gpio_put(OLED_PIN_CS, 1);
}

static inline void ssd1309_write_data(const uint8_t *data, uint32_t length)
{
    gpio_put(OLED_PIN_DC, 1);
    gpio_put(OLED_PIN_CS, 0);
    for (uint32_t i = 0; i < length; i++) {
        uint8_t value = data[i];
        for (int bit = 7; bit >= 0; bit--) {
            gpio_put(OLED_PIN_SCK, 0);
            gpio_put(OLED_PIN_MOSI, (value >> bit) & 1u);
            sleep_us(1);
            gpio_put(OLED_PIN_SCK, 1);
            sleep_us(1);
        }
    }
    gpio_put(OLED_PIN_SCK, 0);
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
    gpio_init(OLED_PIN_RST);
    gpio_set_dir(OLED_PIN_RST, GPIO_OUT);
    gpio_put(OLED_PIN_RST, 1);

    gpio_init(OLED_PIN_DC);
    gpio_set_dir(OLED_PIN_DC, GPIO_OUT);
    gpio_put(OLED_PIN_DC, 1);

    gpio_init(OLED_PIN_CS);
    gpio_set_dir(OLED_PIN_CS, GPIO_OUT);
    gpio_put(OLED_PIN_CS, 1);

    gpio_init(OLED_PIN_SCK);
    gpio_set_dir(OLED_PIN_SCK, GPIO_OUT);
    gpio_put(OLED_PIN_SCK, 0);

    gpio_init(OLED_PIN_MOSI);
    gpio_set_dir(OLED_PIN_MOSI, GPIO_OUT);
    gpio_put(OLED_PIN_MOSI, 0);

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

typedef struct {
    const char *rows[7];
} Ssd1309Glyph;

static const Ssd1309Glyph ssd1309_space_glyph = {
    { "     ", "     ", "     ", "     ", "     ", "     ", "     " }
};

static const Ssd1309Glyph ssd1309_glyph_d = {
    { "#### ", "#   #", "#   #", "#   #", "#   #", "#   #", "#### " }
};

static const Ssd1309Glyph ssd1309_glyph_e = {
    { "#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#####" }
};

static const Ssd1309Glyph ssd1309_glyph_g = {
    { " ### ", "#   #", "#    ", "# ###", "#   #", "#   #", " ### " }
};

static const Ssd1309Glyph ssd1309_glyph_i = {
    { "#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "#####" }
};

static const Ssd1309Glyph ssd1309_glyph_l = {
    { "#    ", "#    ", "#    ", "#    ", "#    ", "#    ", "#####" }
};

static const Ssd1309Glyph ssd1309_glyph_n = {
    { "#   #", "##  #", "# # #", "#  ##", "#   #", "#   #", "#   #" }
};

static const Ssd1309Glyph ssd1309_glyph_o = {
    { " ### ", "#   #", "#   #", "#   #", "#   #", "#   #", " ### " }
};

static const Ssd1309Glyph ssd1309_glyph_p = {
    { "#### ", "#   #", "#   #", "#### ", "#    ", "#    ", "#    " }
};

static const Ssd1309Glyph ssd1309_glyph_s = {
    { " ####", "#    ", "#    ", " ### ", "    #", "    #", "#### " }
};

static const Ssd1309Glyph ssd1309_glyph_t = {
    { "#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  " }
};

static const Ssd1309Glyph *ssd1309_glyph_for_character(char character)
{
    if (character >= 'a' && character <= 'z') {
        character = (char)(character - ('a' - 'A'));
    }

    switch (character) {
    case ' ':
        return &ssd1309_space_glyph;
    case 'D':
        return &ssd1309_glyph_d;
    case 'E':
        return &ssd1309_glyph_e;
    case 'G':
        return &ssd1309_glyph_g;
    case 'I':
        return &ssd1309_glyph_i;
    case 'L':
        return &ssd1309_glyph_l;
    case 'N':
        return &ssd1309_glyph_n;
    case 'O':
        return &ssd1309_glyph_o;
    case 'P':
        return &ssd1309_glyph_p;
    case 'S':
        return &ssd1309_glyph_s;
    case 'T':
        return &ssd1309_glyph_t;
    default:
        return &ssd1309_space_glyph;
    }
}

static void ssd1309_set_buffer_pixel(uint16_t x, uint16_t y)
{
    if (x >= SSD1309_WIDTH || y >= SSD1309_HEIGHT) {
        return;
    }

    const uint32_t index = (uint32_t)(y >> 3u) * SSD1309_WIDTH + x;
    oledBackBuffer[index] |= (uint8_t)(1u << (y & 7u));
}

static void ssd1309_set_buffer_pixel_dithered(int16_t x, int16_t y, bool solid)
{
    if (x < 0 || y < 0 || x >= SSD1309_WIDTH || y >= SSD1309_HEIGHT) {
        return;
    }

    if (solid || (((uint16_t)x + (uint16_t)y) & 1u) == 0u) {
        ssd1309_set_buffer_pixel((uint16_t)x, (uint16_t)y);
    }
}

static uint16_t ssd1309_measure_text_width(const char *text, uint8_t scale)
{
    if (text == NULL || *text == '\0') {
        return 0;
    }

    const uint16_t glyphWidth = (uint16_t)5u * scale;
    const uint16_t spacing = scale;
    uint16_t width = 0;
    bool firstCharacter = true;

    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        if (!firstCharacter) {
            width += spacing;
        }

        width += glyphWidth;
        firstCharacter = false;
    }

    return width;
}

static void ssd1309_draw_glyph(uint16_t x, uint16_t y, const Ssd1309Glyph *glyph, uint8_t scale)
{
    for (uint8_t row = 0; row < 7; row++) {
        const char *pattern = glyph->rows[row];
        for (uint8_t col = 0; col < 5; col++) {
            if (pattern[col] == ' ') {
                continue;
            }

            const uint16_t pixelX = x + (uint16_t)col * scale;
            const uint16_t pixelY = y + (uint16_t)row * scale;

            for (uint8_t dy = 0; dy < scale; dy++) {
                for (uint8_t dx = 0; dx < scale; dx++) {
                    ssd1309_set_buffer_pixel(pixelX + dx, pixelY + dy);
                }
            }
        }
    }
}

static void ssd1309_draw_glyph_clipped(int16_t x, int16_t y, const Ssd1309Glyph *glyph)
{
    for (uint8_t row = 0; row < 7; row++) {
        const char *pattern = glyph->rows[row];
        const int16_t pixelY = y + row;
        if (pixelY < 0 || pixelY > 10) {
            continue;
        }

        for (uint8_t col = 0; col < 5; col++) {
            const int16_t pixelX = x + col;
            if (pattern[col] != ' ' && pixelX >= 0 && pixelX < 100) {
                ssd1309_set_buffer_pixel((uint16_t)pixelX, (uint16_t)pixelY);
            }
        }
    }
}

static void ssd1309_draw_status(const char *text, int16_t y)
{
    const uint16_t textWidth = ssd1309_measure_text_width(text, 1);
    int16_t cursorX = (textWidth < 96) ? (int16_t)(96 - textWidth) / 2 + 2 : 2;

    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        ssd1309_draw_glyph_clipped(cursorX, y, ssd1309_glyph_for_character(*cursor));
        cursorX += 6;
    }
}

static void ssd1309_draw_vertical_line(uint8_t x, uint8_t yStart, uint8_t yEnd)
{
    for (uint8_t y = yStart; y <= yEnd; y++) {
        ssd1309_set_buffer_pixel(x, y);
    }
}

static void ssd1309_draw_filled_circle(int16_t centerX, int16_t centerY, uint8_t radius)
{
    const int16_t radiusSquared = radius * radius;
    for (int16_t y = -radius; y <= radius; y++) {
        for (int16_t x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radiusSquared) {
                ssd1309_set_buffer_pixel(centerX + x, centerY + y);
            }
        }
    }
}

static void ssd1309_draw_circle_outline(int16_t centerX, int16_t centerY, uint8_t radius)
{
    const int16_t outerRadiusSquared = radius * radius;
    const int16_t innerRadius = (radius > 1) ? (radius - 2) : 0;
    const int16_t innerRadiusSquared = innerRadius * innerRadius;

    for (int16_t y = -radius; y <= radius; y++) {
        for (int16_t x = -radius; x <= radius; x++) {
            const int16_t distanceSquared = x * x + y * y;
            if (distanceSquared <= outerRadiusSquared && distanceSquared >= innerRadiusSquared) {
                ssd1309_set_buffer_pixel(centerX + x, centerY + y);
            }
        }
    }
}

// Unpressed buttons render as solid filled circles (raised look). A pressed
// button renders as a hollow ring instead, so it visually reads as pushed in.
static void ssd1309_draw_button_circle(int16_t centerX, int16_t centerY, uint8_t radius, bool pressed)
{
    if (pressed) {
        ssd1309_draw_circle_outline(centerX, centerY, radius);
    } else {
        ssd1309_draw_filled_circle(centerX, centerY, radius);
    }
}

// Index 0 = top circle (more), 1 = middle circle (normal), 2 = bottom circle (less).
static bool buttonPressedState[3] = { false, false, false };

static void ssd1309_draw_controls(void)
{
    ssd1309_draw_vertical_line(101, 3, 60);
    ssd1309_draw_button_circle(115, 11, 6, buttonPressedState[0]);
    ssd1309_draw_button_circle(115, 32, 6, buttonPressedState[1]);
    ssd1309_draw_button_circle(115, 53, 6, buttonPressedState[2]);
}

static const uint8_t moonZzzBitmap[] = {
    0x00, 0xc0, 0xe0, 0x30, 0x18, 0x0c, 0x0c, 0x8c, 0x8c, 0xcc, 0xcc, 0xcc, 0x58, 0x78, 0x70, 0xe0,
    0x40, 0x00, 0x00, 0x0f, 0x7f, 0xe0, 0x80, 0xf0, 0xfc, 0x0f, 0x03, 0x01, 0x00, 0x00, 0x00, 0xb0,
    0xf0, 0x90, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x88, 0xb8, 0xe8, 0xc8, 0x88, 0x00, 0x0f, 0x0f, 0x0d,
};

static void ssd1309_draw_moon_zzz(void)
{
    const uint8_t sourceWidth = 19;
    const uint8_t sourceHeight = 24;
    const uint8_t startX = 31;
    const uint8_t startY = 14;

    for (uint8_t sourceY = 0; sourceY < sourceHeight; sourceY++) {
        for (uint8_t sourceX = 0; sourceX < sourceWidth; sourceX++) {
            const uint32_t byteIndex = (uint32_t)(sourceY >> 3u) * sourceWidth + sourceX;
            if ((moonZzzBitmap[byteIndex] & (1u << (sourceY & 7u))) == 0) {
                continue;
            }

            // Flipped over the x axis: read rows bottom-to-top so the image
            // is mirrored vertically while columns stay in place.
            const uint8_t flippedSourceY = (sourceHeight - 1) - sourceY;
            const uint8_t x = startX + sourceX * 2;
            const uint8_t y = startY + flippedSourceY * 2;
            ssd1309_set_buffer_pixel(x, y);
            ssd1309_set_buffer_pixel(x + 1, y);
            ssd1309_set_buffer_pixel(x, y + 1);
            ssd1309_set_buffer_pixel(x + 1, y + 1);
        }
    }
}

typedef struct {
    int8_t innerX;
    int8_t innerY;
    int8_t outerX;
    int8_t outerY;
} Ssd1309ProgressSegment;

static const Ssd1309ProgressSegment progressSegments[] = {
    { 50, 25, 50, 19 }, { 57, 27, 60, 22 }, { 61, 32, 67, 29 },
    { 63, 38, 69, 38 }, { 61, 45, 67, 48 }, { 57, 49, 60, 54 },
    { 50, 51, 50, 57 }, { 43, 49, 40, 54 }, { 39, 45, 33, 48 },
    { 37, 38, 31, 38 }, { 39, 32, 33, 29 }, { 43, 27, 40, 22 },
};

static void ssd1309_draw_thick_line(const Ssd1309ProgressSegment *segment, bool solid)
{
    int16_t x = segment->innerX;
    int16_t y = segment->innerY;
    const int16_t deltaX = (segment->outerX > x) ? segment->outerX - x : x - segment->outerX;
    const int16_t stepX = (x < segment->outerX) ? 1 : -1;
    const int16_t deltaY = (segment->outerY > y) ? y - segment->outerY : segment->outerY - y;
    const int16_t stepY = (y < segment->outerY) ? 1 : -1;
    int16_t error = deltaX + deltaY;

    while (true) {
        for (int8_t offsetY = -1; offsetY <= 1; offsetY++) {
            for (int8_t offsetX = -1; offsetX <= 1; offsetX++) {
                ssd1309_set_buffer_pixel_dithered(x + offsetX, y + offsetY, solid);
            }
        }

        if (x == segment->outerX && y == segment->outerY) {
            break;
        }

        const int16_t doubledError = 2 * error;
        if (doubledError >= deltaY) {
            error += deltaY;
            x += stepX;
        }
        if (doubledError <= deltaX) {
            error += deltaX;
            y += stepY;
        }
    }
}

static void ssd1309_draw_progress(uint8_t completedSegmentCount)
{
    if (completedSegmentCount > 12) {
        completedSegmentCount = 12;
    }

    for (uint8_t index = 0; index < 12; index++) {
        ssd1309_draw_thick_line(&progressSegments[index], index < completedSegmentCount);
    }
}

typedef enum {
    SSD1309_STATE_NONE,
    SSD1309_STATE_IDLE,
    SSD1309_STATE_DISPENSING,
    SSD1309_STATE_DONE,
} Ssd1309UiState;

static Ssd1309UiState currentUiState = SSD1309_STATE_NONE;

static const char *ssd1309_status_for_state(Ssd1309UiState state)
{
    switch (state) {
    case SSD1309_STATE_IDLE:
        return "Idle";
    case SSD1309_STATE_DISPENSING:
        return "Dispensing";
    case SSD1309_STATE_DONE:
        return "Done";
    case SSD1309_STATE_NONE:
    default:
        return "";
    }
}

static void ssd1309_render_ui(
    Ssd1309UiState state,
    uint8_t completedSegmentCount,
    const char *outgoingStatus,
    int8_t outgoingY,
    const char *incomingStatus,
    int8_t incomingY)
{
    memset(oledBackBuffer, 0x00, sizeof(oledBackBuffer));
    ssd1309_draw_controls();

    if (state == SSD1309_STATE_IDLE) {
        ssd1309_draw_moon_zzz();
    } else {
        ssd1309_draw_progress(state == SSD1309_STATE_DONE ? 12 : completedSegmentCount);
    }

    if (outgoingStatus != NULL) {
        ssd1309_draw_status(outgoingStatus, outgoingY);
    }
    if (incomingStatus != NULL) {
        ssd1309_draw_status(incomingStatus, incomingY);
    }

    if (state == SSD1309_STATE_DISPENSING) {
        for (uint32_t index = 0; index < sizeof(oledBackBuffer); index++) {
            oledBackBuffer[index] = (uint8_t)~oledBackBuffer[index];
        }
    }

    ssd1309_display_buffer(oledBackBuffer, sizeof(oledBackBuffer));
}

static void ssd1309_transition_to_state(Ssd1309UiState state)
{
    const char *incomingStatus = ssd1309_status_for_state(state);
    if (currentUiState == SSD1309_STATE_NONE || currentUiState == state) {
        ssd1309_render_ui(state, 0, NULL, 0, incomingStatus, 3);
        currentUiState = state;
        return;
    }

    const char *outgoingStatus = ssd1309_status_for_state(currentUiState);
    for (uint8_t frame = 0; frame <= 5; frame++) {
        const int8_t offset = (int8_t)((frame * 10u) / 5u);
        ssd1309_render_ui(state, 0, outgoingStatus, 3 - offset, incomingStatus, 13 - offset);
        sleep_ms(18);
    }

    currentUiState = state;
}

void ssd1309_show_text(const char *text)
{
    if (text == NULL || *text == '\0') {
        ssd1309_clear();
        return;
    }

    memset(oledBackBuffer, 0x00, sizeof(oledBackBuffer));

    uint8_t scale = 2;
    uint16_t textWidth = ssd1309_measure_text_width(text, scale);
    if (textWidth > SSD1309_WIDTH) {
        scale = 1;
        textWidth = ssd1309_measure_text_width(text, scale);
    }

    const uint16_t textHeight = 7u * scale;
    const uint16_t startX = (textWidth < SSD1309_WIDTH) ? (SSD1309_WIDTH - textWidth) / 2 : 0;
    const uint16_t startY = (textHeight < SSD1309_HEIGHT) ? (SSD1309_HEIGHT - textHeight) / 2 : 0;

    uint16_t cursorX = startX;
    const uint16_t glyphWidth = (uint16_t)5u * scale;
    const uint16_t spacing = scale;

    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        const Ssd1309Glyph *glyph = ssd1309_glyph_for_character(*cursor);
        ssd1309_draw_glyph(cursorX, startY, glyph, scale);
        cursorX += glyphWidth + spacing;
    }

    ssd1309_display_buffer(oledBackBuffer, sizeof(oledBackBuffer));
}

// lessPressed/normalPressed/morePressed map to the bottom/middle/top circles.
// Only takes visual effect while idle, since dispensing/done screens don't
// show the controls.
void ssd1309_update_button_states(bool lessPressed, bool normalPressed, bool morePressed)
{
    buttonPressedState[0] = morePressed;
    buttonPressedState[1] = normalPressed;
    buttonPressedState[2] = lessPressed;

    if (currentUiState == SSD1309_STATE_IDLE) {
        ssd1309_render_ui(SSD1309_STATE_IDLE, 0, NULL, 0, ssd1309_status_for_state(SSD1309_STATE_IDLE), 3);
    }
}

void ssd1309_show_not_going(void)
{
    ssd1309_transition_to_state(SSD1309_STATE_IDLE);
}

void ssd1309_show_going(void)
{
    ssd1309_transition_to_state(SSD1309_STATE_DISPENSING);
}

void ssd1309_show_dispensing_progress(uint8_t completedSegmentCount)
{
    ssd1309_render_ui(
        SSD1309_STATE_DISPENSING,
        completedSegmentCount,
        NULL,
        0,
        ssd1309_status_for_state(SSD1309_STATE_DISPENSING),
        3);
    currentUiState = SSD1309_STATE_DISPENSING;
}

void ssd1309_show_done(void)
{
    ssd1309_transition_to_state(SSD1309_STATE_DONE);
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
