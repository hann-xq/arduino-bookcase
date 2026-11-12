namespace Door {

namespace {
  Servo         servo;
  int           angle      = DOOR_CLOSED_ANGLE;
  int           target     = DOOR_CLOSED_ANGLE;
  unsigned long stepMs     = DOOR_OPEN_STEP_MS;
  unsigned long lastStepAt = 0;
}

void begin() {
  servo.attach(PIN_SERVO,500, 2600);
  angle  = DOOR_CLOSED_ANGLE;
  target = DOOR_CLOSED_ANGLE;
  servo.write(angle);
}

void moveTo(int newTarget, unsigned long msPerDegree) {
  target = newTarget;
  stepMs = msPerDegree;
}

bool atTarget() {
  return angle == target;
}

void update(unsigned long now) {
  if (angle == target) return;
  if (now - lastStepAt < stepMs) return;
  lastStepAt = now;
  angle += (target > angle) ? 1 : -1;
  servo.write(angle);
}

}
