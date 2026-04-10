#include "KickSort.h"

#define MAX_FILES 8
#define MAX_FILENAME_LENGTH 100

void draw_files(String files[MAX_FILES], int files_count, String locale) {
  u8g2.clearBuffer();
  for (int i = 0; i < files_count; ++i) {
    OLED_print_line(files[i].c_str(), i, locale);
  }
}

int get_files(String files[MAX_ROWS]) {
  File dir;
  File file;
  dir.open("/");
  int files_count = 0;
  char buf[MAX_FILENAME_LENGTH];
  while (file.openNext(&dir, O_READ) && files_count < 30) {
    file.getName(buf, sizeof(buf));
    if (buf[0] != '.' && buf[0] != '\0' && strcmp(buf, "System Volume Information")) { // Ignore hidden files and System Volume Information folder
      /* Serial.println(buf); */
      files[files_count] = String(buf);
      ++files_count;
    }
  }
  dir.close();
  file.close();

  // Sort files
  KickSort<String>::insertionSort(files, files_count);

  // Strip .srt extensions
  for (int i = 0; i < files_count; ++i) {
    int len = files[i].length();
    if (files[i].substring(len - 4, len) == ".srt") {
      files[i] = files[i].substring(0, len - 4);
    }
  }

  return files_count;
}
