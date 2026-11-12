namespace Slot {

namespace {
  bool          isOccupied     = false;  // debounced result used by the game
  bool          candidate      = false;  // latest raw decision
  unsigned long candidateSince = 0;
  unsigned long lastReadAt     = 0;
  float         smoothed       = 0;

  bool readRaw() {
    return digitalRead(PIN_SLOT) == LOW;
  }
}

void begin() {
  pinMode(PIN_SLOT, INPUT_PULLUP);
  candidate      = readRaw();
  isOccupied     = candidate;
  candidateSince = millis();
}

void update(unsigned long now) {
  if (now - lastReadAt < SLOT_READ_INTERVAL_MS) return;
  lastReadAt = now;

  bool raw = readRaw();
  if (raw != candidate) {
    candidate = raw;
    candidateSince = now;
  }
  if (candidate != isOccupied && now - candidateSince >= SLOT_DEBOUNCE_MS) {
    isOccupied = candidate;
    DBG("Slot: "); DBGLN(isOccupied ? "OCCUPIED" : "EMPTY");
  }
}

bool occupied() {
  return isOccupied;
}

}
