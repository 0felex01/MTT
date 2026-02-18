#define SD_CS 7
#define TIME_GREETING_MESSAGE "Select your desired time"
#define RESET_MESSAGE "You may reset now"
#define CURSOR_SHAPE "-"

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <SdFat.h>
#include "OLED.h"
#include "PBs.h"
#include "SD_Reader.h"
#include "SRT.h"
#include "UI.h"

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

  // // Init OLED
  OLED_begin();
  u8g2.clearDisplay();

  // Init SD
  SdFat sd;
  OLED_print("Waiting for SD");
  Serial.println("Waiting for SD");
  while (!sd.begin(SD_CS, SPI_EIGHTH_SPEED));
  // while (!sd.begin(SD_CS, SPI_DIV3_SPEED));
  // u8g2.clearDisplay();
  Serial.println("Found");

  // Read Files
  String files[MAX_FILES];
  int filesCount = getFiles(files);
  Serial.println(filesCount);
  files[0] = ">" + files[0];  // Cursor on first file
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
  long periodic_pos[PERIODIC_SIZE];
  for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
    periodic_pos[i] = -1;
  }

  // Main Loop
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
        String filename = files[cursor_pos].substring(1) + ".srt";

        SdFile subs;
        subs.open(filename.c_str(), O_READ);

        // subs = sd.open(filename.c_str(), O_READ);

        unsigned int amount_of_lines = count_lines(subs);
        unsigned int amount_of_subs = gatherTimestamps(subs, periodic_times, periodic_pos, amount_of_lines);

        // TODO: Remove this debugging
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
        OLED_printLine(current_timestamp, 3);
        OLED_printLine(cursor, 4);
        delay(100);  // To avoid double presses

        long chosen_time = 0;
        chosen_time = prompt_for_time(input, current_timestamp, cursor, cursor_pos);

        // Skip to time
        bool skip_check = false;  // If the time is 0 or longer than the last subtitle, don't bother checking
        if (chosen_time == 0) {
          skip_check = true;
        }
        if (chosen_time > periodic_times[amount_of_subs - 1]) {
          subs.seekSet(periodic_pos[amount_of_subs - 1]);
          skip_check = true;
        }

        if (!skip_check) {
          for (unsigned int i = 0; i < amount_of_subs; ++i) {
            if (chosen_time < periodic_times[i]) {
              subs.seekSet(periodic_pos[i - 1]);
              break;
            }
          }
        }

        // Display subs
        subtitles_state current_times;
        int subs_status = 0;
        while (subs_status == 0) {
          subs_status = display_subs(subs, periodic_times, periodic_pos, amount_of_subs, current_times);
        }

        subs.close();
        // OLED_print("End of subtitles file");
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
