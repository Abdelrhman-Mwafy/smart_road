#include <BluetoothSerial.h>

#define TRIG_FRONT 5
#define ECHO_FRONT 18
#define TRIG_REAR 13
#define ECHO_REAR 12
#define LED_PIN 2

BluetoothSerial SerialBT;

float frontDistance = 400;
float rearDistance  = 400;

float lastFront = 400;
float lastRear  = 400;

String roadStatus = "EMPTY";
String lastStatus = "EMPTY";

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_REAR, OUTPUT);
  pinMode(ECHO_REAR, INPUT);
  pinMode(LED_PIN, OUTPUT);

  SerialBT.begin("SmartRoad_ESP32");
  Serial.println("Smart Road Started");
}

void loop() {
  frontDistance = measureDistance(TRIG_FRONT, ECHO_FRONT, lastFront);
  rearDistance  = measureDistance(TRIG_REAR, ECHO_REAR, lastRear);

  lastFront = frontDistance;
  lastRear  = rearDistance;

  roadStatus = analyzeRoadStatus(frontDistance, rearDistance);

  controlLED(roadStatus);

  if (millis() - lastSend > 200) {
    sendBluetoothData();
    debugSerial();
    lastSend = millis();
  }
}

// ================= FUNCTIONS =================

float measureDistance(int trig, int echo, float lastValue) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return lastValue;

  float d = (duration * 0.0343) / 2;
  if (d < 2 || d > 300) return lastValue;

  return d;
}

String analyzeRoadStatus(float front, float rear) {
  String newStatus = lastStatus;

  if (front < 5 && rear < 5)
    newStatus = "CROWDED";
  else if (front < 5 && rear <= 15)
    newStatus = "SEMI_CROWDED";
  else if (front < 5 && rear > 15)
    newStatus = "GOOD";
  else if (front > 80 && rear > 80)
    newStatus = "EMPTY";

  lastStatus = newStatus;
  return newStatus;
}

void sendBluetoothData() {
  SerialBT.print(frontDistance, 1);
  SerialBT.print(",");
  SerialBT.print(rearDistance, 1);
  SerialBT.print(",");
  SerialBT.print(roadStatus);
  SerialBT.print("\n");
}

void controlLED(String status) {
  unsigned long t = millis();

  if (status == "CROWDED")
    digitalWrite(LED_PIN, (t % 200) < 100);
  else if (status == "SEMI_CROWDED")
    digitalWrite(LED_PIN, (t % 600) < 300);
  else if (status == "GOOD")
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);
}

void debugSerial() {
  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print("  Rear: ");
  Serial.print(rearDistance);
  Serial.print("  Status: ");
  Serial.println(roadStatus);
}
