#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define LINE_HEIGHT 8
#define MAX_LINE_LENGTH 32
#define ENGLISH_FONT u8g2_font_5x7_tf
#define JAPANESE_FONT u8g2_font_b12_t_japanese3 // 1, 2, 3 goes from lowest to highest flash usage 
#define MAX_CHAR_PER_LINE 24

U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, OLED_RST);

void japanese_print_example() {
  u8g2.setFont(u8g2_font_b12_t_japanese3); // 1, 2, 3 goes from lowest to highest flash usage 
  u8g2.setFontDirection(0);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 40);
    u8g2.print("絞死刑");		// Japanese "Hello World" 
  } while ( u8g2.nextPage() );
}

void OLED_begin() {
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setContrast(0);
  /* u8g2.setFont(NORMAL_FONT); */
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

static void OLED_drawLine(const char* text, uint8_t line, String locale) {
  // JP and EN fonts have different heights
  uint8_t height = 0;
  if (locale == "JP") {
    u8g2.setFont(JAPANESE_FONT);
    height = 12;
  } else {
    u8g2.setFont(ENGLISH_FONT);
    height = 8;
  }
    
  uint8_t y = (line + 1) * height - 1;
  /* u8g2.drawStr(0, y, text); */
  u8g2.drawUTF8(0, y, text);
}

void OLED_print(const String& message, String locale) {
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

      OLED_drawLine(buffer, line, locale);

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

    OLED_drawLine(buffer, line, locale);
  }

  u8g2.sendBuffer();
}

void OLED_printLine(const String& message, uint8_t line, String locale) {
  // JP and EN fonts have different heights
  uint8_t height = 0;
  if (locale == "JP") {
    u8g2.setFont(JAPANESE_FONT);
    height = 12;
  } else {
    u8g2.setFont(ENGLISH_FONT);
    height = 8;
  }

  uint8_t y = (line + 1) * height - 1;
    
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, y, message.c_str());
  u8g2.updateDisplayArea(0, line, DISPLAY_TILE_WIDTH, 1);
  /* u8g2.sendBuffer(); */
}

void OLED_clear() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void drawLocale(String text, String locale) {
  String locale_line = "";
  String spaces = "";
  for (unsigned int i = 0; i < MAX_CHAR_PER_LINE - locale.length() + 1; ++i) {
    spaces += " ";
  }
  locale_line += spaces;
  locale_line += text;
  OLED_printLine(locale_line, MAX_ROWS - 1, locale);
}
