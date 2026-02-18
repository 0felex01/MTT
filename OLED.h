// OLED SPI
// #define OLED_DC 9
// #define OLED_CS 10

// OLED I2C
#define OLED_RST 8
#define OLED_SCK 13
#define OLED_SDA 11

#define MAX_CHAR_PER_LINE 25
#define HEIGHT 7 // 8 pixels tall, starts from 0
#define NORMAL_FONT u8g2_font_5x7_tf

// U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST); // SW conflicts with SD SPI
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R0, OLED_RST);
// U8G2_SSD1309_128X64_NONAME2_F_SW_I2C u8g2(U8G2_R0, OLED_SCK, OLED_SDA, OLED_RST);

void u8g2_begin() {
    u8g2.begin();
    u8g2.setContrast(0); // Not much variation between 0 and 255 but 0 is fine.
}

unsigned int count_new_lines(String message) {
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < message.length() - 1; ++i) {
        if (message[i] == '\n') {
            ++cnt;
        }
    }

    return cnt;
}

void OLED_draw(String message, const uint8_t *font, unsigned int col, unsigned int row) {
    // Serial.println(message);
    // Serial.print(col);
    // Serial.print(", ");
    // Serial.println(row);
    // Display row
    do {
        u8g2.setFont(font);
        u8g2.drawUTF8(col, row, message.c_str());
    } while (u8g2.nextPage());
}

void OLED_print(String message) {
    u8g2.clearDisplay();

    unsigned int total_rows = count_new_lines(message) + 1;
    unsigned int prev_newline_pos = 0;
    unsigned int cur_newline_pos = 0;
    String truncated_message;
    // Iterate through all but last row because last row is usually not MAX_CHAR_PER_LINE
    for (unsigned int i = 0; i < (total_rows - 1); ++i) {
        // Truncate row based on newlines positions
        cur_newline_pos = message.indexOf('\n', cur_newline_pos + 1);
        if (prev_newline_pos == 0) { // First line
            truncated_message = message.substring(0, cur_newline_pos);
        } else {
            truncated_message = message.substring(prev_newline_pos + 1, cur_newline_pos);
        }
        prev_newline_pos = cur_newline_pos;

        // Display current row
        OLED_draw(truncated_message, NORMAL_FONT, 0, (i + 1) * HEIGHT + i);
    }

    // First or last row
    // Watch out for the newline characters
    if (total_rows == 1) {
        truncated_message = message.substring(0, message.length());
        OLED_draw(truncated_message, NORMAL_FONT, 0, HEIGHT);
    } else {
        truncated_message = message.substring(cur_newline_pos + 1, message.length());
        OLED_draw(truncated_message, NORMAL_FONT, 0, total_rows * HEIGHT + (total_rows - 1));
    }
}

void OLED_print(String message, unsigned int row) {
    unsigned int col = 0;

    do {
        u8g2.setFont(NORMAL_FONT);
        // u8g2.drawStr(col, (row + 1) * (HEIGHT - 1) + row, message.c_str());
        u8g2.drawUTF8(col, (row + 1) * (HEIGHT - 1) + row, message.c_str());
        // TODO: Test amount of rows with this newly added gap between rows
    } while (u8g2.nextPage());
}
