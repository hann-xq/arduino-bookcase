namespace Audio {

namespace {
  struct Note { uint16_t freq; uint16_t ms; };
  const Note SUCCESS_TUNE[] = { {523, 150}, {659, 150}, {784, 150}, {1047, 450} };
  const Note FAIL_TUNE[]    = { {392, 300}, {330, 300}, {262, 300}, {196, 700} };

  bool          alarmActive   = false;
  bool          sirenHigh     = false;
  unsigned long lastSirenFlip = 0;

  const Note*   tune        = nullptr;
  uint8_t       tuneLen     = 0;
  uint8_t       tuneIdx     = 0;
  unsigned long noteStarted = 0;

  void startTune(const Note* t, uint8_t len) {
    alarmActive = false;
    tune = t; tuneLen = len; tuneIdx = 0;
    noteStarted = millis();
    tone(PIN_BUZZER, tune[0].freq);
  }

  void updateSiren(unsigned long now) {
    if (now - lastSirenFlip < SIREN_FLIP_MS) return;
    lastSirenFlip = now;
    sirenHigh = !sirenHigh;
    tone(PIN_BUZZER, sirenHigh ? SIREN_HIGH_HZ : SIREN_LOW_HZ);
  }

  void updateTune(unsigned long now) {
    if (tune == nullptr || now - noteStarted < tune[tuneIdx].ms) return;
    tuneIdx++;
    if (tuneIdx >= tuneLen) {
      tune = nullptr;
      noTone(PIN_BUZZER);
    } else {
      noteStarted = now;
      tone(PIN_BUZZER, tune[tuneIdx].freq);
    }
  }
}

void begin() { pinMode(PIN_BUZZER, OUTPUT); }

void startAlarm() {
  tune = nullptr;
  alarmActive = true;
  lastSirenFlip = millis();
  sirenHigh = true;
  tone(PIN_BUZZER, SIREN_HIGH_HZ);
}

void playSuccess() { startTune(SUCCESS_TUNE, sizeof(SUCCESS_TUNE) / sizeof(Note)); }
void playFail()    { startTune(FAIL_TUNE, sizeof(FAIL_TUNE) / sizeof(Note)); }
void stop()        { alarmActive = false; tune = nullptr; noTone(PIN_BUZZER); }

void update(unsigned long now) {
  if (alarmActive) updateSiren(now);
  else             updateTune(now);
}

}
