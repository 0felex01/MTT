#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <SdFat.h>
#include "OLED.h"
#include "PBs.h"
#include "SD_Reader.h"
#include "SRT.h"

#define SD_CS 7
#define TIME_GREETING_MESSAGE "Select your desired time"
#define CURSOR_SHAPE "-"

void setup() {
    // TODO: debugging purposes - delete for production
    Serial.begin(921600);
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
    while (!sd.begin(SD_CS, SPI_DIV3_SPEED));
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

    // Timestamps
    long periodic_times[PERIODIC_SIZE];
    for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
        periodic_times[i] = -1;
    }
    int periodic_pos[PERIODIC_SIZE];
    for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
        periodic_pos[i] = -1;
    }

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

                unsigned int amount_of_lines = count_lines(subs);
                gatherTimestamps(subs, periodic_times, periodic_pos, amount_of_lines);

                // unsigned int idx = 0;
                // while (periodic_times[idx] != -1) {
                //     Serial.print(idx);
                //     Serial.print(": time, ");
                //     Serial.print(periodic_times[idx]);
                //     Serial.print(": pos, ");
                //     Serial.println(periodic_pos[idx]);
                //     ++idx;
                // }

                // Prompt user to select time
                String current_timestamp = "00:00:00:00";
                String cursor = CURSOR_SHAPE;
                cursor_pos = 0;

                OLED_print(TIME_GREETING_MESSAGE);
                OLED_print(current_timestamp, 3);
                OLED_print(cursor, 4);
                delay(100); // To avoid double presses

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

                    }
                }

                // Display subs
                while (true) {
                    displaySubs(subs, periodic_times, periodic_pos);
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
