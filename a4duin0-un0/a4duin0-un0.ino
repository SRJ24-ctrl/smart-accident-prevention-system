#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// -------- PINS --------
#define IR_PIN 2
#define BUZZER 9
#define LED 12
#define RELAY 7
#define HELMET_PIN 8

// -------- RELAY --------
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// -------- THRESHOLD --------
float tiltThreshold = 45.0;

// -------- DROWSINESS --------
unsigned long closedStart = 0;
bool eyeClosed = false;

// -------- IR FILTER --------
bool stableState = false;
bool lastRaw = false;
unsigned long lastChange = 0;

// -------- HELMET --------
bool helmetState = false;
bool helmetLastRaw = false;
unsigned long lastHelmetChange = 0;
const int debounceDelay = 150;

// -------- BUZZER --------
unsigned long buzzerTimer = 0;
bool buzzerState = false;

// -------- MPU --------
bool mpuConnected = false;

void setup() {
  Serial.begin(9600);

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(HELMET_PIN, INPUT);

  Wire.begin();
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected");
    mpuConnected = true;
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
  } else {
    Serial.println("MPU FAIL");
    mpuConnected = false;
  }

  digitalWrite(RELAY, RELAY_OFF);
}

// ------------------------------------------------


void loop() {
  Serial.println("LOOP RUNNING");

  // -------- HELMET --------
  bool rawHelmet = digitalRead(HELMET_PIN);

  if (rawHelmet != helmetLastRaw) {
    helmetLastRaw = rawHelmet;
    lastHelmetChange = millis();
  }

  if (millis() - lastHelmetChange > debounceDelay) {
    helmetState = helmetLastRaw;
  }

  // -------- IR FILTER --------
  bool raw = digitalRead(IR_PIN);   // invert if needed

  if (raw != lastRaw) {
    lastRaw = raw;
    lastChange = millis();
  }

  if (millis() - lastChange > 100) {
    stableState = lastRaw;
  }

  bool eyeClosedNow = stableState;

  // -------- DROWSINESS --------
  bool drowsy = false;

  if (eyeClosedNow) {
    if (!eyeClosed) {
      eyeClosed = true;
      closedStart = millis();
    }

    if (millis() - closedStart > 2000) {
      drowsy = true;
    }
  } else {
    eyeClosed = false;
  }

  // -------- TILT --------
  float angle = 0;
  bool unstable = false;

  if (mpuConnected) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    angle = atan2(ay, az) * 180 / PI;

    if (abs(angle) > tiltThreshold) {
      unstable = true;
    }
  }

  // -------- DECISION LOGIC --------
  if (!mpuConnected) {
    digitalWrite(RELAY, RELAY_OFF);
    digitalWrite(LED, HIGH);
    activateBuzzerContinuous();
    Serial.println("MPU FAILSAFE");
    return;
  }

  if (!helmetState) {
    digitalWrite(RELAY, RELAY_OFF);
    digitalWrite(LED, HIGH);
    activateBuzzerContinuous();
    Serial.println("NO HELMET");
  }

  else if (drowsy && unstable) {
    digitalWrite(RELAY, RELAY_OFF);
    digitalWrite(LED, HIGH);
    activateBuzzerFast();
    Serial.println("HIGH RISK");
  }

  else if (unstable) {
    digitalWrite(RELAY, RELAY_OFF);
    digitalWrite(LED, HIGH);
    activateBuzzerFast();
    Serial.println("TILT");
  }

  else if (drowsy) {
    digitalWrite(RELAY, RELAY_ON);
    digitalWrite(LED, HIGH);
    activateBuzzerSlow();
    Serial.println("DROWSY");
  }

  else {
    digitalWrite(RELAY, RELAY_ON);
    digitalWrite(LED, LOW);
    resetBuzzer();
    Serial.println("SAFE");
  }

  // -------- DEBUG --------
  Serial.print("Helmet: ");
  Serial.print(helmetState ? "YES" : "NO");
  Serial.print(" | IR: ");
  Serial.print(stableState);
  Serial.print(" | Angle: ");
  Serial.println(angle);
}

// -------- BUZZER --------

void activateBuzzerContinuous() {
  digitalWrite(BUZZER, HIGH);
}

void activateBuzzerFast() {
  if (millis() - buzzerTimer > 200) {
    buzzerState = !buzzerState;
    digitalWrite(BUZZER, buzzerState);
    buzzerTimer = millis();
  }
}

void activateBuzzerSlow() {
  if (millis() - buzzerTimer > 800) {
    buzzerState = !buzzerState;
    digitalWrite(BUZZER, buzzerState);
    buzzerTimer = millis();
  }
}

void resetBuzzer() {
  buzzerState = false;
  digitalWrite(BUZZER, LOW);
}