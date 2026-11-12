namespace Leds {

void begin() {
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
}

void set(bool green, bool red) {
  digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
  digitalWrite(PIN_LED_RED,   red   ? HIGH : LOW);
}

void off() {
  set(false, false);
}

void flickerRed(unsigned long now) {
  bool on = ((now / ALARM_BLINK_MS) % 2) == 0;
  set(false, on);
}

}
