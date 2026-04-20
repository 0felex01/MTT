#include <Arduino.h>

#define SD_CS 7
#define OLED_RST 8

#define SD_WAITING_MESSAGE "Waiting for SD"
#define TIME_GREETING_MESSAGE "Select your desired time"
#define RESET_MESSAGE "You may reset now"
#define CURSOR_SHAPE ">"
#define SRT_FILE_EXTENSION ".srt"
#define INITAL_TIMESTAMP "00:00:00:00"
#define MAX_ROWS 8

uint8_t DISPLAY_HEIGHT = 64;
uint8_t DISPLAY_WIDTH = 128;
uint8_t TILE_HEIGHT = 8;
uint8_t TILE_WIDTH = 8;
uint8_t FONT_HEIGHT = 5;
uint8_t DISPLAY_BOTTOM_ROW_TILE_X = 0;
uint8_t DISPLAY_BOTTOM_ROW_TILE_Y = (DISPLAY_HEIGHT / TILE_HEIGHT - 1);
uint8_t DISPLAY_BOTTOM_ROW_PIXEL_Y = (DISPLAY_BOTTOM_ROW_TILE_Y + 1) * TILE_HEIGHT;
uint8_t DISPLAY_TILE_WIDTH = DISPLAY_WIDTH / TILE_WIDTH;
uint8_t DISPLAY_BOTTOM_ROW_AREA_HEIGHT = TILE_HEIGHT / TILE_HEIGHT;

#include <U8g2lib.h>
#include <SPI.h>
#include <SdFat.h>
#include "OLED.h"
#include "PBs.h"
#include "SD_Reader.h"
#include "SRT.h"
#include "UI.h"

void setup() {
  Serial.begin(921600);
  while (!Serial);

	// UI
	String cursor = String(CURSOR_SHAPE);

  // Locale
  String locale = "EN";

  // Init PBs
  pinMode(PB_LEFT, INPUT_PULLUP);
  pinMode(PB_DOWN, INPUT_PULLUP);
  pinMode(PB_UP, INPUT_PULLUP);
  pinMode(PB_RIGHT, INPUT_PULLUP);
  pinMode(PB_B, INPUT_PULLUP);
  pinMode(PB_A, INPUT_PULLUP);

  // // Init OLED
  OLED_begin();

  // Init SD
  SdFat sd;
  OLED_print(SD_WAITING_MESSAGE, locale);
  /* while (!sd.begin(SD_CS, SPI_EIGHTH_SPEED)); */
  while (!sd.begin(SD_CS, SPI_HALF_SPEED));
  /* while (!sd.begin(SD_CS, SPI_DIV3_SPEED)); */

  // Read Files
  String files[MAX_FILES];
  unsigned int files_count = get_files(files);
  unsigned int cursor_pos = 0;
  files[0] = cursor + files[0]; // Cursor on first file
  draw_files(files, files_count, cursor_pos, "EN");
  draw_locale(locale, "EN");

  // File Select
  int input = 0;
  bool redraw = false;

  // Timestamps
  long periodic_times[PERIODIC_SIZE]; // Used to go to the closest subtitle after user provides time
  for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
    periodic_times[i] = -1;
  }
  long periodic_pos[PERIODIC_SIZE]; // Used to go back and forth in subtitles
  for (unsigned int i = 0; i < PERIODIC_SIZE; ++i) {
    periodic_pos[i] = -1;
  }

  // Main Loop
  while (true) {
    input = check_buttons();
    switch (input) {
		case PB_DOWN: // Down one file
			if (cursor_pos < (files_count - 1)) {
				files[cursor_pos] = files[cursor_pos].substring(1);
				++cursor_pos;
				files[cursor_pos] = cursor + files[cursor_pos];
				redraw = true;
			}
			break;

		case PB_UP: // Up one file
			if (cursor_pos > 0) {
				files[cursor_pos] = files[cursor_pos].substring(1);
				--cursor_pos;
				files[cursor_pos] = cursor + files[cursor_pos];
				redraw = true;
			}
			break;

    case PB_B: // Switch locale
      if (locale == "EN") {
        locale = "JP";
      } else {
        locale = "EN";
      }
      redraw = true;
      break;

		case PB_A: // Select file
			String filename = files[cursor_pos].substring(1) + SRT_FILE_EXTENSION;

			SdFile subs;
			subs.open(filename.c_str(), O_READ);
			long amount_of_subs = gather_timestamps(subs, periodic_times, periodic_pos);

			// Prompt user to select time
			String current_timestamp = INITAL_TIMESTAMP;
			long chosen_time = 0;
			cursor_pos = 0;
			chosen_time = prompt_for_time(input, current_timestamp);

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
				for (long i = 0; i < amount_of_subs; ++i) {
					if (chosen_time < periodic_times[i]) {
						subs.seekSet(periodic_pos[i - 1]);
						break;
					}
				}
			}

			// Display subs
			subtitle first_subtitle;
			subtitle second_subtitle;

      // Populate first two subtitles then start rendering
      if (first_subtitle.index == 0) {
        read_subtitle(first_subtitle, subs);
      }
      if (second_subtitle.index == 0) {
        read_subtitle(second_subtitle, subs);
      }
      display_subs(subs, locale, periodic_times, periodic_pos, amount_of_subs, first_subtitle, second_subtitle);

			subs.close();
      Serial.println("End of subtitles");
			break;
    }

    if (redraw) {
      draw_files(files, files_count, cursor_pos, "EN");
      draw_locale(locale, "EN");
      redraw = false;
    }
  }
}

void loop() {
}
