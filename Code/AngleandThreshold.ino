#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int buzzerPin = 9;              // active buzzer via transistor
const int threshold = 15;             // degrees deviation allowed
const unsigned long cooldown = 5000;  // 5 sec pause between alerts

unsigned long lastBuzz = 0;
float baselinePitch = 0;              // reference upright angle

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println("=== Posture Detection with Active Buzzer ===");
  Serial.println("Initializing MPU6050...");

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful!");
  } else {
    Serial.println("MPU6050 connection failed. Check wiring!");
    while (1); // stop program
  }

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Calibrate upright posture
  Serial.println("Calibrating posture baseline... Sit straight now!");
  baselinePitch = calibratePosture();
  Serial.print("Baseline pitch set to: ");
  Serial.println(baselinePitch);

  Serial.println("Setup complete. Starting loop...\n");
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Calculate pitch (forward/back tilt in degrees)
  float pitch = atan2(ay, sqrt((long)ax * ax + (long)az * az)) * 180.0 / PI; 
  float deviation = pitch - baselinePitch;

  // Debug
  Serial.print("Pitch = ");
  Serial.print(pitch);
  Serial.print(" | Deviation = ");
  Serial.print(deviation);

  // Posture check
  if (abs(deviation) > threshold) {
    Serial.print(" | Status: BAD posture");

    unsigned long now = millis();
    if (now - lastBuzz > cooldown) {
      Serial.print(" --> Triggering buzzer");
      buzzAlert();
      lastBuzz = now;
    } else {
      Serial.print(" | Cooling down, no buzz");
    }
  } else {
    Serial.print(" | Status: GOOD posture");
  }

  Serial.println();
  delay(500);
}

void buzzAlert() {
  Serial.println("\n*** ALERT: Bad posture detected! Buzzer ON ***");
  digitalWrite(buzzerPin, HIGH);
  delay(1000); // buzz 1 sec
  digitalWrite(buzzerPin, LOW);
  Serial.println("*** Buzzer OFF ***\n");
}

// Take average upright posture value
float calibratePosture() {
  long sum = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    float pitch = atan2(ay, sqrt((long)ax * ax + (long)az * az)) * 180.0 / PI;
    sum += pitch;
    delay(20);
  }
  return (float)sum / samples;
}
