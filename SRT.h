#define LINE_FEED 10
#define CARRIAGE_RETURN 13
#define PERIODIC_SIZE 2000

#define LAST_SUBTITLE_RETURN -1
#define CLOSED_FILE_RETURN -2

#define GAP_MODE 1
#define SUBTITLES_MODE 2
#define GAP_DIFF_OFFSET 2000

unsigned long before_calc_time = 0;

// void(* resetFunc) (void) = 0;//declare reset function at address 0

// void reset() {
//     RSTSR0.LVD0RF = 0;
// }

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
    // Keep going back until two line feeds
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
    // subs.read(); // Advance to next line
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
    // Example: 00:01:11,571 --> 00:01:14,359
    // Returns the time difference in milliseconds
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
    // Strips of all HTML formatting in SRT files

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
    // Sliding window of the message inserting newline characters at or before MAX_CHAR_PER_LINE
    unsigned int cur_pos = 0;
    if (message.length() > MAX_CHAR_PER_LINE) {
        do {
            if (message[cur_pos] != ' ' && message[cur_pos] != '?' && message[cur_pos] != '.' && message[cur_pos] != ',' && message[cur_pos] != '!') {
                // Serial.println(message.substring(cur_pos, cur_pos + MAX_CHAR_PER_LINE));
                cur_pos = message.substring(0, cur_pos).lastIndexOf(' ');
            }
            message[cur_pos] = '\n';
            cur_pos += MAX_CHAR_PER_LINE;
        } while (cur_pos <= message.length());
    }

    // Simple wrap of consecutive dash lines
    message.replace(String(" -"), String("\n-"));

    return message;
}

int subtitle_view_pushbuttons(unsigned int mode, SdFile& subs, subtitles_state &current_state, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], long diff, long start_time, String message, String first_time) {
    // Allow user to use pushbuttons during subtitle view
    if (diff > 0) {
        int input = 0;

        while ((micros() - (unsigned long)start_time) < (unsigned long)diff) {
            input = checkButtons();

            switch (input) {
                case PB_A:
                    // TODO: Add PB logic during paused mode, think about the paused bool in the state struct
                    OLED_print(first_time, MAX_ROWS - 1);

                    input = PB_NOT_PRESSED;
                    while (input != PB_A) {
                        input = checkButtons();
                    }

                    // GAP_MODE: Advance to next real segment
                    // SUBTITLES_MODE: Reset wait time
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
                    // TODO: see if a software reset is possible on uno r4 (since it's arm, not avr)
                    // reset();
                    break;

                case PB_LEFT:
                    // TODO: Pressing back on second subtitle doesn't go to first

                    // Go to prev subtitles
                    for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
                        if (current_state.from_time == periodic_times[i]) {
                            subs.seek(periodic_pos[i - 2]);
                            break;
                        }
                    }

                    start_time = diff; // Break out of wait
                    delay(100); // To avoid double presses
                    break;

                case PB_RIGHT:
                    // Jump to next SRT block
                    // We're already at the next block because the reads are done before this
                    start_time = diff;
                    current_state.to_time = 0; // To avoid gap delay when advancing segment
                    delay(100); // To avoid double presses
                    break;
            }
        }
    }

    return 0;
}

int display_subs(SdFile& subs, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], unsigned int amount_of_subs, subtitles_state &current_state) {
    // SRT specs
    // https://docs.fileformat.com/video/srt/

    // First segment will record the time to use to account for calculations here
    // Otherwise, it's done at the end of each segment
    if (before_calc_time == 0) {
        before_calc_time = micros();
    }

    // Index
    read_next_line(subs);

    // Timestamp
    String timestamps = read_next_line(subs);
    long previous_to_time = 0;
    // Store previous to_time for gap calculation
    if (current_state.to_time != 0) {
        previous_to_time = current_state.to_time;
    }
    current_state = calculate_diff(timestamps); // ms

    // Timestamp for Pause
    unsigned int first_space = timestamps.indexOf(" ");
    String first_time = timestamps.substring(0, first_space);
    int gap = MAX_CHAR_PER_LINE - first_time.length() - 2;
    for (int i = 0; i < gap; ++i) {
        first_time += " ";
    }
    first_time += "||";

    // Message
    // Putting all of the lines of the message into one string
    // Also does word wrapping
    String message;
    String currentLine;
    do {
        currentLine = read_next_line(subs);
        message += currentLine + " ";
    } while (currentLine != "\r");
    message = clean_formatting(message);
    message = word_wrap(message);

    // Delay between segments
    long gap_diff = 0;
    long start_time = 0;
    if (previous_to_time != 0) {
        start_time = micros();
        gap_diff = ((current_state.from_time - previous_to_time) * 1000) - (micros() - before_calc_time) + GAP_DIFF_OFFSET; // Accounting for computation time, us

        int return_code = subtitle_view_pushbuttons(GAP_MODE, subs, current_state, periodic_times, periodic_pos, gap_diff, start_time, message, first_time);
        if (return_code != 0) {
            return return_code;
        }

        before_calc_time = micros(); // Reset for subtitles_diff's computation time compensation
    }
    OLED_print(message);

    // Account for computation time for message duration
    long subtitles_diff = 0;
    start_time = micros();
    subtitles_diff = (current_state.subtitles_duration * 1000) - (micros() - before_calc_time); // Accounting for computation time, us

    int return_code = subtitle_view_pushbuttons(SUBTITLES_MODE, subs, current_state, periodic_times, periodic_pos, subtitles_diff, start_time, message, first_time);
    if (return_code != 0) {
        return return_code;
    }

    // Was forgetting to account for the computation time to clear display
    before_calc_time = micros();

    u8g2.clearDisplay(); // Clearing because gap between segments should be blank

    // Check if it's the last subtitle
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
    // Take times and pos of each segment and places them in their respective arrays
    unsigned int periodic_idx = 0;
    unsigned int current_line = 0;
    unsigned int subs_counter = 0;

    while (current_line < amount_of_lines) {
        // Keep a count of how many subs are in the file
        ++subs_counter;

        // Index
        periodic_pos[periodic_idx] = subs.position();
        read_next_line(subs);
        ++current_line;

        // Timestamp
        String timestamps = read_next_line(subs);
        ++current_line;

        // Get start timestamp as an int
        long from_times[4];
        long from_time = calculate_from_time(timestamps, from_times);
        periodic_times[periodic_idx] = from_time;
        ++periodic_idx;

        // Progress the file position to the next segment
        String currentLine;
        do {
            currentLine = read_next_line(subs);
            ++current_line;
        } while (currentLine != "\r");
    }

    subs.seek(0);

    return subs_counter;
}
