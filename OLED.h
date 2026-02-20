#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define LINE_HEIGHT 8
#define MAX_LINE_LENGTH 32
#define NORMAL_FONT u8g2_font_5x7_tf
#define MAX_CHAR_PER_LINE 25

U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, OLED_RST);

void OLED_begin()
{
    u8g2.begin();
    u8g2.setContrast(0);
    u8g2.setFont(NORMAL_FONT);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

static void OLED_drawLine(const char* text, uint8_t line)
{
    uint8_t y = (line + 1) * LINE_HEIGHT - 1;
    u8g2.drawStr(0, y, text);
}

void OLED_print(const String& message)
{
    u8g2.clearBuffer();

    const char* msg = message.c_str(); // No allocation
    uint8_t line = 0;

    const char* start = msg;
    const char* ptr = msg;

    while (*ptr && line < 8) {
        if (*ptr == '\n') {
            char buffer[MAX_LINE_LENGTH];

            size_t len = ptr - start;
            if (len >= MAX_LINE_LENGTH)
                len = MAX_LINE_LENGTH - 1;

            memcpy(buffer, start, len);
            buffer[len] = '\0';

            OLED_drawLine(buffer, line);

            line++;
            start = ptr + 1;
        }
        ptr++;
    }

    // Final line
    if (line < 8 && start != ptr) {
        char buffer[MAX_LINE_LENGTH];

        size_t len = ptr - start;
        if (len >= MAX_LINE_LENGTH)
            len = MAX_LINE_LENGTH - 1;

        memcpy(buffer, start, len);
        buffer[len] = '\0';

        OLED_drawLine(buffer, line);
    }

    u8g2.sendBuffer();
}

void OLED_printLine(const String& message, uint8_t line) {
    OLED_drawLine(message.c_str(), line);
    u8g2.sendBuffer();
}

void OLED_clear() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}
