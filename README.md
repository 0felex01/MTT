# MTT
Offline SRT Reader for Arduino Uno R4 WiFi

# Implemented
- SD card menu with cursor and pushbuttons support to pick file
- Asks for time whenever picking a file to allow skipping to a time
- Navigation during subtitles view and pausing
- Blanks out display and waits between segments
- Word wrapping

# Things I could do
- Design simple PCB to replace protoboard mess, needed before 3D printing a case.
- Fix the crash if the SRT file doesn't end with a new line.
- Fix the word wrapping of consecutive dash lines.
- Add navigation when paused.
- Add proper reset function for Uno R4 WiFi.
- Possible support for italics and other formatting?
- Possible unicode font support for Japanese?
