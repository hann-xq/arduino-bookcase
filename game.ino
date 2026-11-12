namespace Game {

namespace {

enum State {
  IDLE,
  DOOR_OPENING,
  DOOR_OPEN,
  ALARM_DOOR_CLOSING,
  WIN,
  LOSE,
  RESETTING
};

enum OpeningStage { TO_PARTIAL, PARTIAL_PAUSE, TO_FULL };

enum LoseStage { ALARM_TAIL, FAIL_HOLD };

State         state        = IDLE;
unsigned long stateStart   = 0;
OpeningStage  openingStage = TO_PARTIAL;
LoseStage     loseStage    = ALARM_TAIL;
unsigned long stageStart   = 0;
bool          armed        = false;  // true once the slot has been seen occupied while the door is open

const char* stateName(State s) {
  switch (s) {
    case IDLE:               return "IDLE";
    case DOOR_OPENING:       return "DOOR_OPENING";
    case DOOR_OPEN:          return "DOOR_OPEN";
    case ALARM_DOOR_CLOSING: return "ALARM_DOOR_CLOSING";
    case WIN:                return "WIN";
    case LOSE:               return "LOSE";
    case RESETTING:          return "RESETTING";
  }
  return "?";
}

// ---- Entering a state ----

void enterState(State next) {
  state        = next;
  stateStart   = millis();
  stageStart   = stateStart;
  openingStage = TO_PARTIAL;
  loseStage    = ALARM_TAIL;
  DBG("State -> "); DBGLN(stateName(next));

  switch (next) {
    case IDLE:
      Leds::off();
      Motion::reset();
      break;

    case DOOR_OPENING:
      Leds::off();
      Door::moveTo(DOOR_PARTIAL_ANGLE, DOOR_OPEN_STEP_MS);
      break;

    case DOOR_OPEN:
      // Only arm if the book is actually in the slot. If the slot is empty
      // (e.g. after a lost round), the game arms as soon as a book goes back in.
      armed = Slot::occupied();
      DBG("Armed: "); DBGLN(armed ? "yes" : "no, waiting for a book in the slot");
      break;

    case ALARM_DOOR_CLOSING:
      Audio::startAlarm();
      Door::moveTo(DOOR_CLOSED_ANGLE, DOOR_CLOSE_STEP_MS);
      break;

    case WIN:
      Audio::playSuccess();
      Leds::set(true, false);
      Door::moveTo(DOOR_OPEN_ANGLE, DOOR_OPEN_STEP_MS);
      break;

    case LOSE:
      break;

    case RESETTING:
      Leds::off();
      Door::moveTo(DOOR_CLOSED_ANGLE, DOOR_RESET_STEP_MS);
      break;
  }
}

// ---- Inside a state ----

void updateIdle(unsigned long now) {
  Motion::update(now);
  if (now - stateStart >= IDLE_COOLDOWN_MS && Motion::detected()) {
    enterState(DOOR_OPENING);
  }
}

void updateDoorOpening(unsigned long now) {
  switch (openingStage) {
    case TO_PARTIAL:
      if (Door::atTarget()) {
        openingStage = PARTIAL_PAUSE;
        stageStart = now;
      }
      break;

    case PARTIAL_PAUSE:
      if (now - stageStart >= DOOR_PARTIAL_PAUSE_MS) {
        Door::moveTo(DOOR_OPEN_ANGLE, DOOR_OPEN_STEP_MS);
        openingStage = TO_FULL;
      }
      break;

    case TO_FULL:
      if (Door::atTarget()) enterState(DOOR_OPEN);
      break;
  }
}

void updateDoorOpen(unsigned long now) {
  if (!armed && Slot::occupied()) {
    armed = true;
    DBGLN("Armed: book detected in slot");
  }
  if (armed && !Slot::occupied()) {
    enterState(ALARM_DOOR_CLOSING);
  } else if (now - stateStart >= DOOR_OPEN_TIMEOUT_MS) {
    DBGLN("Timeout: nobody took the book");
    enterState(RESETTING);
  }
}

void updateAlarmDoorClosing(unsigned long now) {
  Leds::flickerRed(now);
  if (Slot::occupied()) {
    enterState(WIN);
  } else if (Door::atTarget()) {
    enterState(LOSE);
  }
}

void updateWin(unsigned long now) {
  if (now - stateStart >= WIN_HOLD_MS && Door::atTarget()) {
    enterState(RESETTING);
  }
}

void updateLose(unsigned long now) {
  switch (loseStage) {
    case ALARM_TAIL:
      Leds::flickerRed(now);
      if (now - stageStart >= LOSE_ALARM_EXTRA_MS) {
        Audio::playFail();
        Leds::set(false, true);
        loseStage = FAIL_HOLD;
        stageStart = now;
      }
      break;

    case FAIL_HOLD:
      if (now - stageStart >= LOSE_HOLD_MS) enterState(RESETTING);
      break;
  }
}

void updateResetting() {
  if (Door::atTarget()) {
    Audio::stop();
    enterState(IDLE);
  }
}

}

void begin() {
  enterState(IDLE);
}

void update(unsigned long now) {
  switch (state) {
    case IDLE:               updateIdle(now);             break;
    case DOOR_OPENING:       updateDoorOpening(now);      break;
    case DOOR_OPEN:          updateDoorOpen(now);         break;
    case ALARM_DOOR_CLOSING: updateAlarmDoorClosing(now); break;
    case WIN:                updateWin(now);              break;
    case LOSE:               updateLose(now);             break;
    case RESETTING:          updateResetting();           break;
  }
}

}
