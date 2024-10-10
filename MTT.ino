#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <SdFat.h>
#include "OLED.h"
#include "SRT.h"
#include "SD_Reader.h"

#define SD_CS 7

#define PB_LEFT 0
#define PB_DOWN 1
#define PB_UP 2
#define PB_RIGHT 3
#define PB_B 4
#define PB_A 5

void setup() {
    //TODO: debugging purposes - delete for production
    Serial.begin(9600);
    while (!Serial);

    // Init PBs
    pinMode(PB_LEFT, INPUT_PULLUP);
    pinMode(PB_DOWN, INPUT_PULLUP);
    pinMode(PB_UP, INPUT_PULLUP);
    pinMode(PB_RIGHT, INPUT_PULLUP);
    pinMode(PB_B, INPUT_PULLUP);
    pinMode(PB_A, INPUT_PULLUP);

    // Init OLED
    u8g2_begin();
    u8g2.clearDisplay();

    // Init SD
    SdFat sd;
    OLED_print("Waiting for SD");
    while (!sd.begin(SD_CS, SPI_QUARTER_SPEED));
    u8g2.clearDisplay();

    // Read Files
    String files[MAX_FILES];
    int filesCount = getFiles(files);
    files[0] = ">" + files[0]; // Cursor on first file
    redrawFiles(files, filesCount);

    // File Select
    int input = 0;
    int cursor_pos = 0;
    bool needRedraw = false;
    while (true) {
        input = checkButtons();
        switch (input) {
            case PB_DOWN:
                if (cursor_pos < MAX_ROWS && cursor_pos < (filesCount - 1)) {
                    files[cursor_pos] = files[cursor_pos].substring(1);
                    ++cursor_pos;
                    files[cursor_pos] = ">" + files[cursor_pos];
                    needRedraw = true;
                }
                break;

            case PB_UP:
                if (cursor_pos > 0) {
                    files[cursor_pos] = files[cursor_pos].substring(1);
                    --cursor_pos;
                    files[cursor_pos] = ">" + files[cursor_pos];
                    needRedraw = true;
                }
                break;

            case PB_A:
                SdFile subs;
                String filename = files[cursor_pos].substring(1) + ".srt";
                subs.open(filename.c_str(), O_READ);
                while (true) {
                    displaySubs(subs);
                }
                subs.close();
                OLED_print("End of subtitles file");
                break;

        }

        if (needRedraw) {
            redrawFiles(files, filesCount);
            needRedraw = false;
        }
    }
}

void loop() {
}

int checkButtons() {
    // delay is to move cursor more controllable
    // TODO: find a better way to do this

    if (digitalRead(PB_LEFT) == LOW) {
        delay(200);
        return PB_LEFT;
    }
    if (digitalRead(PB_DOWN) == LOW) {
        delay(200);
        return PB_DOWN;
    }
    if (digitalRead(PB_UP) == LOW) {
        delay(200);
        return PB_UP;
    }
    if (digitalRead(PB_RIGHT) == LOW) {
        delay(200);
        return PB_RIGHT;
    }
    if (digitalRead(PB_B) == LOW) {
        delay(200);
        return PB_B;
    }
    if (digitalRead(PB_A) == LOW) {
        delay(200);
        return PB_A;
    }

    return -1; // Nothing pressed
}
