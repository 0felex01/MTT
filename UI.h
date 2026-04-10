#define TIME_PROMPT_CURSOR_SHAPE "-"

void redraw_timestamp(String current_timestamp, char *cursor_line) {
  OLED_printLine(current_timestamp, 3, "EN");
  OLED_printLine(cursor_line, 4, "EN");
}

void render_prompt(String greeting_message, String current_timestamp, char *cursor_line) {
  u8g2.clearDisplay();
  OLED_printLine(greeting_message, 1, "EN");
  redraw_timestamp(current_timestamp, cursor_line);
}

long prompt_for_time(int input, String current_timestamp) {
  String cursor = String(TIME_PROMPT_CURSOR_SHAPE);
  unsigned cursor_pos = 0;
  bool redraw_prompt = false;
  char cursor_line[MAX_CHAR_PER_LINE + 1];
  memset(cursor_line, ' ', MAX_CHAR_PER_LINE);
  memset(cursor_line + MAX_CHAR_PER_LINE, '\0', 1);
  cursor_line[0] = cursor.c_str()[0];

  render_prompt(TIME_GREETING_MESSAGE, current_timestamp, cursor_line);

  while (true) {
    input = checkButtons();

    switch (input) {
    case PB_DOWN:
      if (current_timestamp[cursor_pos] == '0') {
        current_timestamp[cursor_pos] = '9';
      } else if (current_timestamp[cursor_pos] != ':') {
        current_timestamp[cursor_pos] -= 1;
      }
      redraw_prompt = true;
      break;

    case PB_UP:
      if (current_timestamp[cursor_pos] == '9') {
        current_timestamp[cursor_pos] = '0';
      } else if (current_timestamp[cursor_pos] != ':') {
        current_timestamp[cursor_pos] += 1;
      }
      redraw_prompt = true;
      break;

    case PB_LEFT:
      if (cursor_pos > 0) {
        cursor_line[cursor_pos] = ' ';
        --cursor_pos;
        cursor_line[cursor_pos] = cursor.c_str()[0];
        redraw_prompt = true;
      }
      break;

    case PB_RIGHT:
      if (cursor_pos < current_timestamp.length() - 1) {
        cursor_line[cursor_pos] = ' ';
        ++cursor_pos;
        cursor_line[cursor_pos] = cursor.c_str()[0];
        redraw_prompt = true;
      }
      break;

    case PB_A:
      long from_time = calculate_from_time(current_timestamp);
      return from_time;
      break;
    }

    if (redraw_prompt) {
      redraw_timestamp(current_timestamp, cursor_line);
      redraw_prompt = false;
    }
  }
}
