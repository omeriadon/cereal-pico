#pragma once

#include <stdint.h>

#define SSD1309_WIDTH 128
#define SSD1309_HEIGHT 64
#define SSD1309_PAGE_COUNT (SSD1309_HEIGHT / 8)
#define SSD1309_FRAMEBUFFER_SIZE (SSD1309_WIDTH * SSD1309_PAGE_COUNT)

void ssd1309_init_default_spi(void);
void ssd1309_display_buffer(const uint8_t *buffer, uint32_t length);
void ssd1309_clear(void);
void ssd1309_fill(uint8_t value);
void ssd1309_show_resize_centered(void);

void ssd1309_draw_bitmap_pages(
    uint8_t *framebuffer,
    uint32_t framebufferLength,
    uint8_t x,
    uint8_t yPage,
    const uint8_t *bitmap,
    uint8_t bitmapWidth,
    uint8_t bitmapPageCount
);

extern const uint32_t ssd1309_width;
extern const uint32_t ssd1309_height;
extern const uint32_t ssd1309_page_count;
extern const uint32_t ssd1309_framebuffer_size;

extern const uint8_t resize_bitmap[];
extern const uint8_t resize_bitmap_width;
extern const uint8_t resize_bitmap_page_count;
extern const uint32_t resize_bitmap_length;

const uint8_t *ssd1309_resize_bitmap_pointer(void);
