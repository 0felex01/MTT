#define LINE_FEED 10
#define CARRIAGE_RETURN 13
#define PERIODIC_SIZE 2000

struct times {
    unsigned int from_time;
    float delay_time;
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

unsigned int calculate_from_time(String timestamps, unsigned int from_times[4]) {
    // I seperated this into another function 
    // because I'm gathering all start times upon file load
    // and this is used when calculating the diff when displaying subs
    from_times[0] = timestamps.substring(0, 2).toInt();
    from_times[1] = timestamps.substring(3, 5).toInt();
    from_times[2] = timestamps.substring(6, 8).toInt();
    from_times[3] = timestamps.substring(9, 12).toInt();

    unsigned int from_time = (from_times[0] * 3600000) +
        (from_times[1] * 60000) +
        (from_times[2] * 1000) +
        (from_times[3]);

    return from_time;
}

struct times calculateDiff(String timestamps) {
    // Example: 00:01:11,571 --> 00:01:14,359
    // Returns the time difference in milliseconds
    unsigned int from_times[4];
    unsigned int from_time = calculate_from_time(timestamps, from_times);
    unsigned int to_times[4];

    to_times[0] = timestamps.substring(17, 19).toInt();
    to_times[1] = timestamps.substring(20, 22).toInt();
    to_times[2] = timestamps.substring(23, 25).toInt();
    to_times[3] = timestamps.substring(26, 29).toInt();

    float diff = (to_times[0] - from_times[0]) * 3600000 +
        (to_times[1] - from_times[1]) * 60000 +
        (to_times[2] - from_times[2]) * 1000 +
        (to_times[3] - from_times[3]);

    return {from_time, diff};
}

String cleanFormatting(String message) {
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

void displaySubs(SdFile& subs, long periodic_times[PERIODIC_SIZE], int periodic_pos[PERIODIC_SIZE]) {
    // SRT specs
    // https://docs.fileformat.com/video/srt/

    float startTime = micros();

    // Index
    read_next_line(subs);

    // Timestamp
    String timestamps = read_next_line(subs);
    times current_times = calculateDiff(timestamps); // ms

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
    String message;
    String currentLine;
    do {
        currentLine = read_next_line(subs);
        message += currentLine + " ";
    } while (currentLine != "\r");
    message = cleanFormatting(message);
    OLED_print(message);

    // Account for computation time for message duration
    float diff = (current_times.delay_time * 1000) - ((micros() - startTime)); // us

    // Allow user to go back and forth one message and pause
    if (diff > 0) {
        int input = 0;
        int newlines = 0;

        while ((micros() - startTime) < diff) {
            bool twice = false;
            input = checkButtons();

            switch (input) {
                case PB_A:
                    OLED_print(first_time, MAX_ROWS - 1);

                    input = PB_NOT_PRESSED;
                    while (input != PB_A) {
                        input = checkButtons();
                    }

                    // Reset wait time
                    OLED_print(message);
                    startTime = micros();
                    break;

                case PB_LEFT:
                    // TODO: Pressing back on first subtitle advances it instead

                    // Go to prev subtitles
                    for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
                        if (current_times.from_time == periodic_times[i]) {
                            Serial.println(periodic_pos[i]);
                            subs.seek(periodic_pos[i - 1]);
                            break;
                        }
                    }

                    startTime = diff; // Break out of wait
                    delay(100); // To avoid double presses
                    break;

                case PB_RIGHT:
                    // Jump to next SRT block
                    // We're already at the next block because the reads are done before this
                    startTime = diff;
                    delay(100); // To avoid double presses
                    break;
            }
        }
    }
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

void gatherTimestamps(SdFile& subs, long periodic_times[PERIODIC_SIZE], int periodic_pos[PERIODIC_SIZE], unsigned int amount_of_lines) {
    // Take times and pos of each segment and places them in their respective arrays
    unsigned int periodic_idx = 0;
    unsigned int current_line = 0;

    while (current_line < amount_of_lines) {
        // Index
        periodic_pos[periodic_idx] = subs.position();
        read_next_line(subs);
        ++current_line;

        // Timestamp
        String timestamps = read_next_line(subs);
        ++current_line;

        // Get start timestamp as an int
        unsigned int from_times[4];
        unsigned int from_time = calculate_from_time(timestamps, from_times);
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
}
