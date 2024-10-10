float calculateDiff(String timestamps) {
    // Example: 00:01:11,571 --> 00:01:14,359
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

    int diff = (to_times[0] - from_times[0]) * 3600000 +
               (to_times[1] - from_times[1]) * 60000 +
               (to_times[2] - from_times[2]) * 1000 +
               (to_times[3] - from_times[3]);
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
    float delayTime = calculateDiff(timestamps);

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
    // Serial.println((micros() - startTime) / 1000);
    float diff = delayTime - ((micros() - startTime) / 1000);
    if (diff > 0) {
        delay(diff);
    }
}
