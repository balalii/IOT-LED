#include <Preferences.h>

int button = 15;                         // pin tombol (sesuai JSON)
int leds[] = {2, 4, 5, 17, 18, 19};      // pin LED (termasuk biru di 17)
int totalLeds = 6;

bool running = false;                    // status LED berjalan
int currentLed = 0;
int lastButtonState = LOW;               // aktif HIGH (default LOW)
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 200;

Preferences prefs;

void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT);                // tombol aktif HIGH, tidak gunakan PULLUP
  for (int i = 0; i < totalLeds; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  prefs.begin("led-state", false);
  running = prefs.getBool("running", false);
  Serial.println(running ? "MODE: ON (Saved)" : "MODE: OFF (Saved)");
}

void loop() {
  int reading = digitalRead(button);

  // deteksi tepi naik (LOW -> HIGH)
  if (reading == HIGH && lastButtonState == LOW && (millis() - lastDebounce) > debounceDelay) {
    running = !running;
    prefs.putBool("running", running);
    Serial.println(running ? "MODE: ON" : "MODE: OFF");
    lastDebounce = millis();

    if (!running) {
      // matikan semua LED jika OFF
      for (int i = 0; i < totalLeds; i++) {
        digitalWrite(leds[i], LOW);
      }
    }
  }

  lastButtonState = reading;

  if (running) {
    static unsigned long lastLedTime = 0;
    if (millis() - lastLedTime >= 200) {
      lastLedTime = millis();
      // matikan semua LED
      for (int i = 0; i < totalLeds; i++) {
        digitalWrite(leds[i], LOW);
      }
      // nyalakan LED berikutnya
      digitalWrite(leds[currentLed], HIGH);
      currentLed = (currentLed + 1) % totalLeds;
    }
  }
}
