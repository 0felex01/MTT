#define PB_LEFT 5
#define PB_DOWN 4
#define PB_UP 3
#define PB_RIGHT 2
#define PB_B 1
#define PB_A 0
#define PB_NOT_PRESSED -1

const int buttons[] = {PB_LEFT, PB_DOWN, PB_UP, PB_RIGHT, PB_B, PB_A};
const int num_buttons = 6;

unsigned long last_press_time = 0; // ms
const unsigned long debounce_delay = 150; // ms

int check_buttons() {
  unsigned long now = millis();

  if (now - last_press_time < debounce_delay) {
    return PB_NOT_PRESSED;
  }

  for (int i = 0; i < num_buttons; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      last_press_time = now;
      return buttons[i];
    }
  }

  return PB_NOT_PRESSED;
}
