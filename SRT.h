#include <stdint.h>

#define LINE_FEED 10
#define CARRIAGE_RETURN 13
#define PERIODIC_SIZE 2000

#define LAST_SUBTITLE_RETURN -1
#define CLOSED_FILE_RETURN -2

#define GAP_MODE 1
#define SUBTITLES_MODE 2

struct subtitle {
  long index = 0;
  long from_time = 0;  // ms
  long to_time = 0;    // ms
  long duration = 0;   // ms
  String dialogue = "";
};

String read_next_line(SdFile& subs) {
  int data = 0;
  String buf;

  do {
    data = subs.read();
    buf += char(data);
  } while (data >= 0 && (data != LINE_FEED && data != CARRIAGE_RETURN));
  subs.read();  // Advance to next line

  return buf;
}

void go_to_prev_line(SdFile& subs) {
  int data = 0;
  unsigned int linefeed_count = 0;

  while (true) {
    if (subs.curPosition() <= 2) {
      return;
    }

    data = subs.read();
    if (data == LINE_FEED) {
      ++linefeed_count;
      if (linefeed_count == 6) {
        return;
      }
    }

    subs.seekSet(subs.curPosition() - 2);
  }
}

long calculate_from_time(const String& timestamps) {

  const char* ts = timestamps.c_str();

  int h = (ts[0] - '0') * 10 + (ts[1] - '0');
  int m = (ts[3] - '0') * 10 + (ts[4] - '0');
  int s = (ts[6] - '0') * 10 + (ts[7] - '0');
  int ms = (ts[9] - '0') * 100 + (ts[10] - '0') * 10 + (ts[11] - '0');

  return (long)h * 3600000L + (long)m * 60000L + (long)s * 1000L + ms;
}

long calculate_to_time(const String& timestamps) {

  const char* ts = timestamps.c_str() + 17;

  int h = (ts[0] - '0') * 10 + (ts[1] - '0');
  int m = (ts[3] - '0') * 10 + (ts[4] - '0');
  int s = (ts[6] - '0') * 10 + (ts[7] - '0');
  int ms = (ts[9] - '0') * 100 + (ts[10] - '0') * 10 + (ts[11] - '0');

  return (long)h * 3600000L + (long)m * 60000L + (long)s * 1000L + ms;
}

String clean_formatting(String message) {
  const char* tags[] = {
    "<b>", "</b>",
    "<i>", "</i>",
    "<u>", "</u>"
  };

  for (const char* tag : tags) {
    message.replace(tag, "");
  }

  message.replace("\r", "");
  return message;
}

String word_wrap(String message) {
  String result = "";

  unsigned int lineStart = 0;
  unsigned int message_len = message.length();
  while (lineStart < message_len) {
    // Find the end of the current line or existing newline
    int lineEnd = message.indexOf('\n', lineStart);
    if (lineEnd == -1) lineEnd = message_len;

    String line = message.substring(lineStart, lineEnd);
    unsigned int line_len = line.length();

    // Wrap the line if it's too long
    int start = 0;
    while ((unsigned int)start < line_len) {
      int end = start + MAX_CHAR_PER_LINE;
      if ((unsigned int)end >= line_len) {
        end = line_len;
      } else {
        // Find last space or punctuation before max width
        int wrapPos = -1;
        for (int i = end; i > start; i--) {
          char c = line[i];
          if (c == ' ' || c == '?' || c == '.' || c == ',' || c == '!') {
            wrapPos = i + 1;  // include punctuation/space
            break;
          }
        }
        if (wrapPos != -1) end = wrapPos;
      }

      result += line.substring(start, end);
      result += '\n';

      start = end;
      // Skip leading spaces
      while ((unsigned int)start < line_len && line[start] == ' ') start++;
    }

    lineStart = lineEnd + 1;  // skip past existing newline
  }

  // Replace " -" with "\n-" as before
  result.replace(" -", "\n-");

  return result;
}

void read_subtitle(subtitle& given_subtitle, SdFile& subs) {
  // Subtitle index, skip
  long index = read_next_line(subs).toInt();
  given_subtitle.index = index;

  // Timestamps
  String timestamps = read_next_line(subs);
  long from_time = calculate_from_time(timestamps);
  long to_time = calculate_to_time(timestamps);
  /* long subtitles_duration = to_time - from_time; */
  given_subtitle.from_time = from_time;
  given_subtitle.to_time = to_time;
  long duration = to_time - from_time;
  given_subtitle.duration = duration;

  // Get all of the dialogue
  String dialogue = "";
  String current_read = "something";
  while (current_read.length() > 1) {
    current_read = read_next_line(subs);
    dialogue += current_read + "\n";
  }
  given_subtitle.dialogue = dialogue;

  return;
}

void transfer_subtitles(subtitle& first_subtitle, subtitle& second_subtitle) {
  subtitle temp = first_subtitle;
  first_subtitle = second_subtitle;
  second_subtitle = temp;
  second_subtitle.index = 0;
}

void debug_subs_print(long render_time, long wait_time, long display_time, long downtime) {
  Serial.print("render_time: ");
  Serial.print(render_time);
  Serial.print(", ");
  Serial.print("wait_time: ");
  Serial.print(wait_time);
  Serial.print(", ");
  Serial.print("display_time: ");
  Serial.print(display_time);
  Serial.print(", ");
  Serial.print("downtime: ");
  Serial.println(downtime);
}

void print_subtitle(subtitle& given_subtitle) {
  Serial.print("index: ");
  Serial.print(given_subtitle.index);
  Serial.print(", ");
  Serial.print("from_time: ");
  Serial.print(given_subtitle.from_time);
  Serial.print(", ");
  Serial.print("to_time: ");
  Serial.print(given_subtitle.to_time);
  Serial.print(", ");
  Serial.print("duration: ");
  Serial.print(given_subtitle.duration);
  Serial.print(", ");
  Serial.print("dialogue: ");
  Serial.println(given_subtitle.dialogue);
}

unsigned int count_lines(SdFile& subs) {
  unsigned int amount_of_lines = 0;
  while (subs.available()) {
    read_next_line(subs);
    ++amount_of_lines;
  }

  subs.seekSet(0);
  return amount_of_lines;
}

long gather_timestamps(SdFile& subs, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], long amount_of_lines) {
  long periodic_idx = 0;
  long current_line = 0;
  long subs_counter = 0;

  while (current_line < amount_of_lines) {
    ++subs_counter;

    periodic_pos[periodic_idx] = subs.curPosition();
    read_next_line(subs);
    ++current_line;

    String timestamps = read_next_line(subs);
    ++current_line;

    long from_time = calculate_from_time(timestamps);
    periodic_times[periodic_idx] = from_time;
    ++periodic_idx;

    String currentLine;
    do {
      currentLine = read_next_line(subs);
      ++current_line;
    } while (currentLine.length() > 1);
  }

  subs.seekSet(0);
  return subs_counter;
}

String make_paused_line(long timestamp) {
  long hours = timestamp / 3600000;
  timestamp %= 3600000;

  long minutes = timestamp / 60000;
  timestamp %= 60000;

  long seconds = timestamp / 1000;
  long millis = timestamp % 1000;

  char buffer[19];
  sprintf(buffer, "|| %02lu:%02lu:%02lu,%03lu", hours, minutes, seconds, millis);

  return String(buffer);
}

bool check_pushbuttons(SdFile& subs, long& current_subtitle_index, long amount_of_subs, long periodic_times[PERIODIC_SIZE], long periodic_pos[PERIODIC_SIZE], subtitle& first_subtitle, subtitle& second_subtitle) {
  // PB_LEFT, PB_DOWN, PB_UP, PB_RIGHT, PB_B, PB_A, PB_NOT_PRESSED
  bool changed = false;  // This tells the device to stop tracking current subtitle timing and process the new subtitle
  int c_PB = checkButtons();

  switch (c_PB) {
  case PB_LEFT:
    if (current_subtitle_index > 0) {  // Prevent going before the first subtitle
      current_subtitle_index -= 1;
      changed = true;
    }
    break;
  case PB_RIGHT:
    if (current_subtitle_index < amount_of_subs - 1) {  // Prevent going after the last subtitle
      current_subtitle_index += 1;
      changed = true;
    }
    break;
  case PB_A:  // Pause
    int c_PB = checkButtons();
    if (c_PB != PB_A) {
      String paused_line = make_paused_line(periodic_times[current_subtitle_index - 1]);
      OLED_printLine(paused_line, MAX_ROWS - 1, "EN");
      while (c_PB != PB_A) {
        /* delay(1); */
        c_PB = checkButtons();
      }

      // Redraw current subtitle
      // current_subtitle_index -= 1;
      changed = true;
    }
    break;
  }

  // Repopulate first and second subtitles
  if (changed) {
    /* u8g2.clearDisplay(); */
    u8g2.clearBuffer();

    subs.seekSet(periodic_pos[current_subtitle_index]);
    read_subtitle(first_subtitle, subs);
    read_subtitle(second_subtitle, subs);
  }

  return changed;
}

void display_subs(SdFile& subs,
                  String locale,
                  long periodic_times[PERIODIC_SIZE],
                  long periodic_pos[PERIODIC_SIZE],
                  long amount_of_subs,
                  subtitle& first_subtitle,
                  subtitle& second_subtitle) {
  long playback_start = millis();
  long playback_offset = first_subtitle.from_time;
  long current_subtitle_index = first_subtitle.index;
  bool onscreen = false;

  while (current_subtitle_index < amount_of_subs) {
    long now = millis();
    long current_time = (now - playback_start) + playback_offset;

    // render subtitle
    if (!onscreen && current_time >= first_subtitle.from_time) {
      /* Serial.println(first_subtitle.dialogue); */
      first_subtitle.dialogue = clean_formatting(first_subtitle.dialogue);
      first_subtitle.dialogue = word_wrap(first_subtitle.dialogue);

      OLED_print(first_subtitle.dialogue, locale);
      onscreen = true;
    }

    // take subtitle off screen if it's time
    if (onscreen && current_time >= first_subtitle.to_time) {
      u8g2.clearDisplay();
      onscreen = false;

      transfer_subtitles(first_subtitle, second_subtitle);
      read_subtitle(second_subtitle, subs);

      ++current_subtitle_index;
    }

    // Handle buttons
    bool changed = check_pushbuttons(subs,
                                     current_subtitle_index,
                                     amount_of_subs,
                                     periodic_times,
                                     periodic_pos,
                                     first_subtitle,
                                     second_subtitle);

    if (changed) {
      /* Serial.println(current_subtitle_index); */
      playback_offset = first_subtitle.from_time;
      playback_start = millis();
      onscreen = false;

      /* u8g2.clearDisplay(); */
    }
  }
}
