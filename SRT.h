#define LINE_FEED 10
#define CARRIAGE_RETURN 13
#define FILE_POSITIONS_SIZE 2000
#define INITIAL_SAFETY_BARRIER 200 // Crashes when you seek before the beginning of the file

String readLine(SdFile& subs) {
    int data = 0;
    String buf;

    do {
        data = subs.read();
        buf += char(data);
    } while (data >= 0 && (data != LINE_FEED && data != CARRIAGE_RETURN));
    subs.read(); // Advance to next line

    return buf;
}

float calculateDiff(String timestamps) {
    // Example: 00:01:11,571 --> 00:01:14,359
    // Returns the time difference in milliseconds
    int from_times[4];
    int to_times[4];

    from_times[0] = timestamps.substring(0, 2).toInt();
    from_times[1] = timestamps.substring(3, 5).toInt();
    from_times[2] = timestamps.substring(6, 8).toInt();
    from_times[3] = timestamps.substring(9, 12).toInt();

    to_times[0] = timestamps.substring(17, 19).toInt();
    to_times[1] = timestamps.substring(20, 22).toInt();
    to_times[2] = timestamps.substring(23, 25).toInt();
    to_times[3] = timestamps.substring(26, 29).toInt();

    int diff = (to_times[0] - from_times[0]) * 3600000 + (to_times[1] - from_times[1]) * 60000 + (to_times[2] - from_times[2]) * 1000 + (to_times[3] - from_times[3]);
    return diff;
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

void displaySubs(SdFile& subs) {
    // SRT specs
    // https://docs.fileformat.com/video/srt/

    float startTime = micros();

    // Index
    int subsIndex = readLine(subs).toInt();

    // Timestamp
    String timestamps = readLine(subs);
    float delayTime = calculateDiff(timestamps);  // ms

    // Timestamp for Pause
    int first_space = timestamps.indexOf(" ");
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
        currentLine = readLine(subs);
        message += currentLine + " ";
    } while (currentLine != "\r");
    message = cleanFormatting(message);
    OLED_print(message);

    // Account for computation time for message duration
    float diff = (delayTime * 1000) - ((micros() - startTime)); // us

    // Allow user to go back and forth one message and pause
    if (diff > 0) {
        int input = 0;
        int newlines = 0;
        char ch;

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
                    // Jumps back to the beginning of the previous SRT block
                    // We're at the end of the current block so we have to go back twice
                    // because the reads are done before this
                    twice = false;
                    if ((subs.position() - 4) >= 0) {
                        subs.seek(subs.position() - 4);
                        while (newlines < 2 and subs.position() != 0) {
                            ch = subs.read();
                            if (ch == LINE_FEED) {
                                ++newlines;
                            } else if (ch != CARRIAGE_RETURN) {
                                newlines = 0;
                            }
                            if ((subs.position() - 2) >= 0) {
                                subs.seek(subs.position() - 2);
                            } else {
                                break;
                            }

                            if (newlines >= 2 && !twice) {
                                twice = true;
                                newlines = 0;
                            }
                        }
                        if (subs.position() != 0) {
                            subs.seek(subs.position() + 4);
                        }

                        startTime = diff; // Break out of wait
                        delay(100);
                    }
                    break;

                case PB_RIGHT:
                    // Jump to next SRT block
                    // We're already at the next block because the reads are done before this
                    startTime = diff;
                    delay(100);
            }
        }
    }
}
