#include <Servo.h>

// =====================================================================
//                               CONFIG
// =====================================================================

#define DEBUG_SERIAL           1   // 1 = print state changes and sensor values to USB serial

// ---- Pins ----
const uint8_t PIN_PIR       = A1;
const uint8_t PIN_SLOT      = A0;
const uint8_t PIN_SERVO     = 9;
const uint8_t PIN_LED_GREEN = 5;
const uint8_t PIN_LED_RED   = 6;
const uint8_t PIN_BUZZER    = 3;

// ---- Motion sensor ----
const unsigned long PIR_WARMUP_MS           = 30000;
const unsigned long MOTION_HOLD_MS          = 100;    // OUT must stay HIGH this long (filters glitches)
const int           PIR_ANALOG_THRESHOLD    = 400;
const unsigned long PIR_STATUS_PRINT_MS     = 2000;   // for debugging: how often the current PIR level is printed while idle

// ---- Slot sensor ----
const unsigned long SLOT_READ_INTERVAL_MS   = 10;
const unsigned long SLOT_DEBOUNCE_MS        = 250;

// ---- Servo ----
const int           DOOR_CLOSED_ANGLE       = 180;
const int           DOOR_PARTIAL_ANGLE      = 140;
const int           DOOR_OPEN_ANGLE         = 90;
const unsigned long DOOR_OPEN_STEP_MS       = 25;     // ms per degree while opening
const unsigned long DOOR_PARTIAL_PAUSE_MS   = 2000; 
const unsigned long DOOR_CLOSE_STEP_MS      = 200;    // ms per degree during alarm (90 deg * 200 ms = 18 s)
const unsigned long DOOR_RESET_STEP_MS      = 40;

// ---- Game timings ----
const unsigned long DOOR_OPEN_TIMEOUT_MS    = 90000;
const unsigned long WIN_HOLD_MS             = 6000;
const unsigned long LOSE_ALARM_EXTRA_MS     = 2000;
const unsigned long LOSE_HOLD_MS            = 5000;
const unsigned long IDLE_COOLDOWN_MS        = 3000;   // ignore motion right after a reset
const unsigned long ALARM_BLINK_MS          = 150;
const unsigned long LED_SELFTEST_MS         = 500;

// ---- Passive buzzer ----
const uint16_t      SIREN_HIGH_HZ           = 1200;
const uint16_t      SIREN_LOW_HZ            = 800;
const unsigned long SIREN_FLIP_MS           = 300;    // time per siren tone

// =====================================================================
//                               DEBUG
// =====================================================================

// Debug printing that compiles away completely when DEBUG_SERIAL is 0.

#if DEBUG_SERIAL
  #define DBG(x)   Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif

void debugBegin() {
#if DEBUG_SERIAL
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) { }   // wait briefly for USB serial, don't hang without it
#endif
}

// =====================================================================
//                    MODULES
// =====================================================================

namespace Door {
  void begin();
  void moveTo(int angle, unsigned long msPerDegree);
  bool atTarget();
  void update(unsigned long now);
}

namespace Motion {
  void begin();
  void update(unsigned long now);
  bool detected();
  void reset();
}

namespace Slot {
  void begin();
  void update(unsigned long now);
  bool occupied();
}

namespace Audio {
  void begin();
  void update(unsigned long now);
  void startAlarm();
  void playSuccess();
  void playFail();
  void stop();
}

namespace Leds {
  void begin();
  void set(bool green, bool red);
  void off();
  void flickerRed(unsigned long now);
}

namespace Game {
  void begin();
  void update(unsigned long now);
}

// =====================================================================
//                            SETUP / LOOP
// =====================================================================

void setup() {
  debugBegin();

  Leds::begin();
  Leds::set(true, true);   // LED test to see the board booted
  delay(LED_SELFTEST_MS);

  Door::begin();
  Motion::begin();
  Slot::begin();
  Audio::begin();

  DBGLN("Harry Potter bookcase ready");
  DBG("Slot at start: "); DBGLN(Slot::occupied() ? "OCCUPIED" : "EMPTY");

  Game::begin();
}

void loop() {
  unsigned long now = millis();

  Slot::update(now);    // always watch the slot
  Door::update(now);    // door keeps moving while everything else runs
  Audio::update(now);
  Game::update(now);
}
