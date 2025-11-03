#include <Preferences.h>

const int leds[] = {2, 4, 22, 5, 18, 19}; // 6 LED (GPIO22 = biru)
const int NUM_LEDS = sizeof(leds) / sizeof(leds[0]);
const int buttonPin = 23; // tombol wired ke GND (INPUT_PULLUP)

Preferences prefs;
const char* PREF_NAMESPACE = "led_toggle";
const char* PREF_KEY = "running_state";

bool running = false;

// Debounce variables
int lastReading = HIGH;           // last raw reading from pin
int stableState = HIGH;           // debounced stable state
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// LED animation
int ledIndex = 0;
int arah = 1; // 1 = ke kanan, -1 = ke kiri
unsigned long previousMillis = 0;
const unsigned long interval = 150; // ms

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== RUNNING LED TOGGLE (persistent) ===");

  // Setup LED pins
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  // Setup button with internal pull-up
  pinMode(buttonPin, INPUT_PULLUP);

  // Init Preferences and read saved state
  prefs.begin(PREF_NAMESPACE, false);
  running = prefs.getBool(PREF_KEY, false);
  Serial.print("Loaded saved state -> running = ");
  Serial.println(running ? "TRUE" : "FALSE");

  if (!running) {
    matikanSemuaLED();
  }

  Serial.println("Tekan tombol untuk toggle ON/OFF (disimpan ke NVS).");
}

void loop() {
  // --- Read button (raw) ---
  int reading = digitalRead(buttonPin);

  // If reading changed, reset debounce timer
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  // If enough time has passed, consider the reading stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableState) {
      stableState = reading;
      // Only toggle when stableState transitions to PRESSED (LOW)
      if (stableState == LOW) {
        // Toggle running
        running = !running;
        // Save to NVS
        prefs.putBool(PREF_KEY, running);
        if (running) {
          Serial.println(">>> LED DINYALAKAN (state disimpan)");
        } else {
          Serial.println(">>> LED DIMATIKAN (state disimpan)");
          matikanSemuaLED();
        }
      }
    }
  }

  lastReading = reading;

  // Run animation only when running = true
  if (running) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      updateRunningLED();
    }
  }

  // (loop ringan, non-blocking)
}

// Update LED with safe index management
void updateRunningLED() {
  // Matikan semua LED terlebih dahulu
  matikanSemuaLED();

  // Nyalakan LED sekarang
  digitalWrite(leds[ledIndex], HIGH);

  // Hitung index berikutnya tanpa melewati batas
  int nextIndex = ledIndex + arah;

  if (nextIndex >= NUM_LEDS) {
    // sudah melewati ujung kanan -> balik arah
    arah = -1;
    nextIndex = NUM_LEDS - 2; // agar lompat ke elemen satu di kiri
  } else if (nextIndex < 0) {
    // sudah melewati ujung kiri -> balik arah
    arah = 1;
    nextIndex = 1; // agar lompat ke elemen satu di kanan
  }

  ledIndex = nextIndex;
}

void matikanSemuaLED() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(leds[i], LOW);
  }
}