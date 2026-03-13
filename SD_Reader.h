#include "KickSort.h"

#define MAX_FILES 8
#define MAX_ROWS 8

void redrawFiles(String files[MAX_FILES], int filesCount) {
    u8g2.clearBuffer();
    for (int i = 0; i < filesCount; ++i) {
        OLED_printLine(files[i], i);
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
        if (buf[0] != '.' && buf[0] != '\0') { // Ignore hidden files
            /* Serial.println(buf); */
            files[filesCount] = String(buf);
            ++filesCount;
        }
    }
    dir.close();
    file.close();

    // Sort files
    KickSort<String>::insertionSort(files, filesCount);

    // Strip .srt extensions
    for (int i = 0; i < filesCount; ++i) {
        int len = files[i].length();
        if (files[i].substring(len - 4, len) == ".srt") {
            files[i] = files[i].substring(0, len - 4);
        }
    }

    return filesCount;
}
