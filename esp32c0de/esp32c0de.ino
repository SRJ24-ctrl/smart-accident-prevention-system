// -------- ESP32-CAM --------
#define HELMET_PIN 13
#define LED_PIN 33

#define USE_TEST_MODE 1
#define TOGGLE_INTERVAL 5000

bool helmetState = false;
unsigned long lastToggle = 0;

void setup() {
  pinMode(HELMET_PIN, OUTPUT);
  digitalWrite(HELMET_PIN, LOW); // safe default

  pinMode(LED_PIN, OUTPUT);
}

void loop() {

#if USE_TEST_MODE
  if (millis() - lastToggle > TOGGLE_INTERVAL) {
    helmetState = !helmetState;
    lastToggle = millis();
  }
#else
  helmetState = detectHelmet();
#endif

  digitalWrite(HELMET_PIN, helmetState ? HIGH : LOW);
  digitalWrite(LED_PIN, helmetState ? HIGH : LOW);
}

// Replace later with real detection
bool detectHelmet() {
  return false;
}