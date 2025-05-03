#define PB_LEFT 5
#define PB_DOWN 4
#define PB_UP 3
#define PB_RIGHT 2
#define PB_B 1
#define PB_A 0
#define PB_NOT_PRESSED -1

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

    return PB_NOT_PRESSED; // Nothing pressed
}
