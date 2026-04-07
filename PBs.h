#define PB_LEFT 5
#define PB_DOWN 4
#define PB_UP 3
#define PB_RIGHT 2
#define PB_B 1
#define PB_A 0
#define PB_NOT_PRESSED -1

const int buttons[] = {PB_LEFT, PB_DOWN, PB_UP, PB_RIGHT, PB_B, PB_A};
const int numButtons = 6;

unsigned long lastPressTime = 0; // ms
const unsigned long debounceDelay = 150; // ms

int checkButtons() {
  unsigned long now = millis();

  if (now - lastPressTime < debounceDelay) {
    return PB_NOT_PRESSED;
  }

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      lastPressTime = now;
      return buttons[i];
    }
  }

  return PB_NOT_PRESSED;
}
