namespace Motion {

namespace {
  bool          isDetected = false;
  bool          seenLow    = false;  // OUT must drop LOW before a new trigger counts
  bool          wasHigh    = false;
  unsigned long highSince  = 0;
  bool          lastRaw    = false;  // only used to print changes of the raw signal
  unsigned long lastStatusAt = 0;    // only used to print the level regularly

  bool readRaw() {
    int reading = analogRead(PIN_PIR);
    bool high = reading >= PIR_ANALOG_THRESHOLD;
    if (high != lastRaw) { DBG("PIR analog value: "); DBGLN(reading); }
    return high;
  }
}

void begin() {
  pinMode(PIN_PIR, INPUT);
  lastRaw = readRaw();
  DBG("PIR warming up for "); DBG(PIR_WARMUP_MS / 1000); DBGLN(" s");
  DBG("PIR raw at start: "); DBGLN(lastRaw ? "HIGH" : "LOW");
}

void update(unsigned long now) {
  bool high = readRaw();
  if (high != lastRaw) {
    lastRaw = high;
    DBG("PIR raw: "); DBGLN(high ? "HIGH" : "LOW");
  }
  if (now - lastStatusAt >= PIR_STATUS_PRINT_MS) {
    lastStatusAt = now;
    DBG("PIR status: "); DBG(high ? "HIGH" : "LOW");
    DBG(", analog value "); DBG(analogRead(PIN_PIR));
    DBGLN(now < PIR_WARMUP_MS ? " (warming up, ignored)" : "");
  }

  if (now < PIR_WARMUP_MS) {
    seenLow = false;
    isDetected = false;
    wasHigh = high;
    highSince = now;
    return;
  }

  if (!high) {
    seenLow = true;
    wasHigh = false;
    isDetected = false;
    return;
  }

  if (!wasHigh) {
    wasHigh = true;
    highSince = now;
  }

  bool nowDetected = seenLow && now - highSince >= MOTION_HOLD_MS;
  if (nowDetected && !isDetected) DBGLN("Motion: detected");
  isDetected = nowDetected;
}

bool detected() {
  return isDetected;
}

void reset() {
  // Someone still standing in front keeps OUT HIGH, so wait for it to go LOW
  // before the next trigger. Stops the door reopening for the same player.
  seenLow = false;
  isDetected = false;
}

}
