long prompt_for_time(int input, String current_timestamp, String cursor, int cursor_pos) {
    while (true) {
        input = checkButtons();
        switch (input) {
            case PB_DOWN:
                if (current_timestamp[cursor_pos] != '0' && current_timestamp[cursor_pos] != ':') {
                    current_timestamp[cursor_pos] -= 1;
                    u8g2.clearDisplay();
                    OLED_print(TIME_GREETING_MESSAGE);
                    OLED_print(current_timestamp, 3);
                }
                break;

            case PB_UP:
                if (current_timestamp[cursor_pos] != '9' && current_timestamp[cursor_pos] != ':') {
                    current_timestamp[cursor_pos] += 1;
                    u8g2.clearDisplay();
                    OLED_print(TIME_GREETING_MESSAGE);
                    OLED_print(current_timestamp, 3);
                }
                break;

            case PB_LEFT:
                if (cursor.length() > 1) {
                    cursor = cursor.substring(1, cursor.length());
                    u8g2.clearDisplay();
                    OLED_print(TIME_GREETING_MESSAGE);
                    OLED_print(current_timestamp, 3);
                    OLED_print(cursor, 4);
                    --cursor_pos;
                }
                break;

            case PB_RIGHT:
                if (cursor.length() != current_timestamp.length()) {
                    cursor = " " + cursor;
                    u8g2.clearDisplay();
                    OLED_print(TIME_GREETING_MESSAGE);
                    OLED_print(current_timestamp, 3);
                    OLED_print(cursor, 4);
                    ++cursor_pos;
                }
                break;

            case PB_A:
                long from_times[4];
                long from_time = calculate_from_time(current_timestamp, from_times);
                // Serial.println(from_time);
                return from_time;
                break;

        }
    }
}
