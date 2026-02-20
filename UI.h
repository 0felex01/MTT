#define TIME_PROMPT_CURSOR_SHAPE "-"

long prompt_for_time(int input, String current_timestamp, int cursor_pos) {
  String cursor = String(TIME_PROMPT_CURSOR_SHAPE);
  unsigned int cursor_len = cursor.length();

  OLED_print(TIME_GREETING_MESSAGE);
  OLED_printLine(current_timestamp, 3);
  OLED_printLine(cursor, 4);

  while (true) {
    input = checkButtons();

    switch (input) {
    case PB_DOWN:
      if (current_timestamp[cursor_pos] != '0' && current_timestamp[cursor_pos] != ':') {
        current_timestamp[cursor_pos] -= 1;
        u8g2.clearDisplay();
        OLED_print(TIME_GREETING_MESSAGE);
        OLED_printLine(current_timestamp, 3);
      }
      break;

    case PB_UP:
      if (current_timestamp[cursor_pos] != '9' && current_timestamp[cursor_pos] != ':') {
        current_timestamp[cursor_pos] += 1;
        u8g2.clearDisplay();
        OLED_print(TIME_GREETING_MESSAGE);
        OLED_printLine(current_timestamp, 3);
      }
      break;

    case PB_LEFT:
      if (cursor_len > 1) {
        cursor = cursor.substring(1, cursor_len);
        u8g2.clearDisplay();
        OLED_print(TIME_GREETING_MESSAGE);
        OLED_printLine(current_timestamp, 3);
        OLED_printLine(cursor, 4);
        --cursor_pos;
      }
      break;

    case PB_RIGHT:
      if (cursor_len != current_timestamp.length()) {
        cursor = " " + cursor;
        u8g2.clearDisplay();
        OLED_print(TIME_GREETING_MESSAGE);
        OLED_printLine(current_timestamp, 3);
        OLED_printLine(cursor, 4);
        ++cursor_pos;
      }
      break;

    case PB_A:
      long from_times[4];
      long from_time = calculate_from_time(current_timestamp, from_times);
      return from_time;
      break;

    }
  }
}
