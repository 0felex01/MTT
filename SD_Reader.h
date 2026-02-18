#define MAX_FILES 30
#define MAX_ROWS 9

void redrawFiles(String files[MAX_FILES], int filesCount) {
    u8g2.clearBuffer();
    for (int i = 0; i < filesCount; ++i) {
        OLED_printLine(files[i], i);
    }
}

void bubbleSort(String files[MAX_FILES], int filesCount) {
    bool good = false;
    unsigned int char_pos = 0;
    bool clean = false;
    String temp;

    while (!clean) {
        clean = true; // For exiting sort after one full clean run
        for (int i = 0; i < (filesCount - 1); ++i) {
            good = false; // For checking letter by letter if need to
            char_pos = 0;

            while (!good) {
                // Same letters
                if (files[i][char_pos] == files[i + 1][char_pos]) {
                    if (char_pos < (files[i].length() - 1) && char_pos < (files[i + 1].length() - 1)) { // Keep cursor pos within strings' lengths
                        ++char_pos;
                        continue;
                    } else {
                        break;
                    }
                }

                // Swap
                if (files[i][char_pos] > files[i + 1][char_pos]) {
                    temp = files[i + 1];
                    files[i + 1] = files[i];
                    files[i] = temp;

                    good = true;
                    clean = false;
                }

                // Good as is
                if (files[i][char_pos] < files[i + 1][char_pos]) {
                    good = true;
                }
            }
        }
    }
}

int getFiles(String files[MAX_ROWS]) {
    File dir;
    File file;
    dir.open("/");
    int filesCount = 0;
    char buf[MAX_CHAR_PER_LINE];
    while (file.openNext(&dir, O_READ) && filesCount < 30) {
        file.getName(buf, MAX_CHAR_PER_LINE);
        if (buf[0] != '.' and buf[0] != NULL) { // Ignore hidden files
            Serial.println(buf);
            files[filesCount] = String(buf);
            ++filesCount;
        }
    }
    dir.close();
    file.close();

    // Sort alphabetically
    bubbleSort(files, filesCount);

    // Strip .srt extensions
    for (int i = 0; i < filesCount; ++i) {
        int len = files[i].length();
        if (files[i].substring(len - 4, len) == ".srt") {
            files[i] = files[i].substring(0, len - 4);
        }
    }

    return filesCount;
}
