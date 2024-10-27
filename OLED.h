#define OLED_RST 8
#define OLED_DC 9
#define OLED_CS 10
#define OLED_DATA 11
#define OLED_CLK 13

#define MAX_CHAR_PER_LINE 25
#define HEIGHT 7 // 8 pixels tall, starts from 0
#define NORMAL_FONT u8g2_font_5x7_tf

U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST); // SW conflicts with SD SPI

void u8g2_begin() {
    u8g2.begin();
    u8g2.setContrast(0); // Not much variation between 0 and 255 but 0 is fine.
}

void OLED_print(String message) {
    unsigned int col = 0;
    unsigned int row = 0;
    float len = message.length();

    u8g2.clearDisplay();

    // If row isn't specified, automatically split message
    unsigned int rows = ceil(len / MAX_CHAR_PER_LINE);
    for (unsigned int i = 0; i < rows; ++i) {
        unsigned int from = MAX_CHAR_PER_LINE * i;
        unsigned int to = MAX_CHAR_PER_LINE * (i + 1);
        String truncated_message = message.substring(from, to);

        // If line starts with a space
        if (truncated_message[0] == ' ') {
            truncated_message = truncated_message.substring(1, truncated_message.length());
        }

        // Print row by row according to MAX_CHAR_PER_LINE
        row = HEIGHT + i * HEIGHT;
        do {
            u8g2.setFont(NORMAL_FONT);
            // u8g2.drawStr(col, (i + 1) * (HEIGHT - 1) + i, truncated_message.c_str());
            u8g2.drawUTF8(col, (i + 1) * (HEIGHT - 1) + i, truncated_message.c_str());
            // TODO: Test amount of rows with this newly added gap between rows
        } while (u8g2.nextPage());
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
