#include <stdint.h>

#define LINE_FEED 10
#define CARRIAGE_RETURN 13
#define PERIODIC_SIZE 2000

#define LAST_SUBTITLE_RETURN -1
#define CLOSED_FILE_RETURN -2

#define GAP_MODE 1
#define SUBTITLES_MODE 2

unsigned long before_calc_time = 0; // us

struct subtitles_state {
    long from_time = 0;
    long to_time = 0;
    long subtitles_duration = 0;
    bool paused = false;
};

String read_next_line(SdFile& subs) {
    int data = 0;
    String buf;

    do {
        data = subs.read();
        buf += char(data);
    } while (data >= 0 && (data != LINE_FEED && data != CARRIAGE_RETURN));
    subs.read(); // Advance to next line

    return buf;
}

void go_to_prev_line(SdFile& subs) {
    int data = 0;
    unsigned int linefeed_count = 0;

    while (true) {
        if (subs.position() <= 2) {
            return;
        }

        data = subs.read();
        if (data == LINE_FEED) {
            ++linefeed_count;
            if (linefeed_count == 6) {
                return;
            }
        }

        subs.seek(subs.position() - 2);
    }
}

long calculate_from_time(String timestamps, long from_times[4]) {
    from_times[0] = timestamps.substring(0, 2).toInt();
    from_times[1] = timestamps.substring(3, 5).toInt();
    from_times[2] = timestamps.substring(6, 8).toInt();
    from_times[3] = timestamps.substring(9, 12).toInt();

    long from_time = (from_times[0] * 3600000) +
        (from_times[1] * 60000) +
        (from_times[2] * 1000) +
        (from_times[3]);

    return from_time;
}

long calculate_to_time(String timestamps, long to_times[4]) {
    to_times[0] = timestamps.substring(17, 19).toInt();
    to_times[1] = timestamps.substring(20, 22).toInt();
    to_times[2] = timestamps.substring(23, 25).toInt();
    to_times[3] = timestamps.substring(26, 29).toInt();

    long to_time = (to_times[0] * 3600000) +
        (to_times[1] * 60000) +
        (to_times[2] * 1000) +
        (to_times[3]);

    return to_time;
}

struct subtitles_state calculate_diff(String timestamps) {
    long from_times[4];
    long from_time = calculate_from_time(timestamps, from_times);
    long to_times[4];
    long to_time = calculate_to_time(timestamps, to_times);

    long subtitles_diff = (to_times[0] - from_times[0]) * 3600000 +
        (to_times[1] - from_times[1]) * 60000 +
        (to_times[2] - from_times[2]) * 1000 +
        (to_times[3] - from_times[3]);

    return {from_time, to_time, subtitles_diff};
}

String clean_formatting(String message) {
    message.replace("<b>", "");
    message.replace("</b>", "");
    message.replace("<i>", "");
    message.replace("</i>", "");
    message.replace("<u>", "");
    message.replace("</u>", "");
    message.replace("\r", "");

    return message;
}

String word_wrap(String message) {
    unsigned int cur_pos = 0;
    if (message.length() > MAX_CHAR_PER_LINE) {
        do {
            if (message[cur_pos] != ' ' && message[cur_pos] != '?' && message[cur_pos] != '.' && message[cur_pos] != ',' && message[cur_pos] != '!') {
                cur_pos = message.substring(0, cur_pos).lastIndexOf(' ');
            }
            message[cur_pos] = '\n';
            cur_pos += MAX_CHAR_PER_LINE;
        } while (cur_pos <= message.length());
    }

    message.replace(String(" -"), String("\n-"));

    return message;
}

int subtitle_view_pushbuttons(unsigned int mode, SdFile& subs, subtitles_state &current_state, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], long diff, long start_time, String message, String first_time) {
    if (diff > 0) {
        int input = 0;

        while ((micros() - (unsigned long)start_time) < (unsigned long)diff) {
            input = checkButtons();

            switch (input) {
                case PB_A:
                    OLED_print(first_time, MAX_ROWS - 1);

                    input = PB_NOT_PRESSED;
                    while (input != PB_A) {
                        input = checkButtons();
                    }

                    OLED_print(message);
                    if (mode == GAP_MODE) {
                        start_time = diff;
                    } else {
                        start_time = micros();
                    }
                    diff = current_state.subtitles_duration * 1000; // us
                    break;

                case PB_B:
                    OLED_print(RESET_MESSAGE);
                    return CLOSED_FILE_RETURN;
                    break;

                case PB_LEFT:
                    // FIX: exit loop immediately and reset timing for previous subtitle
                    for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
                        if (current_state.from_time == periodic_times[i]) {
                            if (i > 0) {
                                subs.seek(periodic_pos[i - 1]);
                            } else {
                                subs.seek(0);
                            }
                            break;
                        }
                    }
                    diff = 0;                    // <-- added
                    before_calc_time = micros();  // <-- added
                    break;

                case PB_RIGHT:
                    // FIX: exit loop immediately and reset timing for next subtitle
                    diff = 0;                    // <-- added
                    current_state.to_time = 0;   // avoid gap delay
                    before_calc_time = micros();  // <-- added
                    break;
            }
        }
    }

    return 0;
}

int display_subs(SdFile& subs, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], unsigned int amount_of_subs, subtitles_state &current_state) {
    if (before_calc_time == 0) {
        before_calc_time = micros();
    }

    read_next_line(subs);

    String timestamps = read_next_line(subs);
    long previous_to_time = 0;
    if (current_state.to_time != 0) {
        previous_to_time = current_state.to_time;
    }
    current_state = calculate_diff(timestamps); // ms

    unsigned int first_space = timestamps.indexOf(" ");
    String first_time = timestamps.substring(0, first_space);
    int gap = MAX_CHAR_PER_LINE - first_time.length() - 2;
    for (int i = 0; i < gap; ++i) {
        first_time += " ";
    }
    first_time += "||";

    String message;
    String currentLine;
    do {
        currentLine = read_next_line(subs);
        message += currentLine + " ";
    } while (currentLine.length() > 1);
    message = clean_formatting(message);
    message = word_wrap(message);

    long gap_diff = 0;
    long start_time = 0;
    if (previous_to_time != 0) {
        start_time = micros();
        gap_diff = ((current_state.from_time - previous_to_time) * 1000) - (micros() - before_calc_time);

        int return_code = subtitle_view_pushbuttons(GAP_MODE, subs, current_state, periodic_times, periodic_pos, gap_diff, start_time, message, first_time);
        if (return_code != 0) {
            return return_code;
        }

        before_calc_time = micros();
    }
    OLED_print(message);

    long subtitles_diff = 0;
    start_time = micros();
    subtitles_diff = (current_state.subtitles_duration * 1000) - (micros() - before_calc_time);

    int return_code = subtitle_view_pushbuttons(SUBTITLES_MODE, subs, current_state, periodic_times, periodic_pos, subtitles_diff, start_time, message, first_time);
    if (return_code != 0) {
        return return_code;
    }

    before_calc_time = micros();
    u8g2.clearDisplay();

    if ((long)subs.position() == periodic_pos[amount_of_subs - 1]) {
        return LAST_SUBTITLE_RETURN;
    }

    return 0;
}

unsigned int count_lines(SdFile& subs) {
    unsigned int amount_of_lines = 0;
    while (subs.available()) {
        read_next_line(subs);
        ++amount_of_lines;
    }

    subs.seek(0);
    return amount_of_lines;
}

unsigned int gatherTimestamps(SdFile& subs, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], unsigned int amount_of_lines) {
    unsigned int periodic_idx = 0;
    unsigned int current_line = 0;
    unsigned int subs_counter = 0;

    while (current_line < amount_of_lines) {
        ++subs_counter;

        periodic_pos[periodic_idx] = subs.position();
        read_next_line(subs);
        ++current_line;

        String timestamps = read_next_line(subs);
        ++current_line;

        long from_times[4];
        long from_time = calculate_from_time(timestamps, from_times);
        periodic_times[periodic_idx] = from_time;
        ++periodic_idx;

        String currentLine;
        do {
            currentLine = read_next_line(subs);
            ++current_line;
        } while (currentLine.length() > 1);
    }

    subs.seek(0);
    return subs_counter;
}

