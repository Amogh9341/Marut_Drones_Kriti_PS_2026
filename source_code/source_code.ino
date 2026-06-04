#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

/* ------------------- PIN DEFINITIONS ------------------- */

// Agitator DC Motor (L298N)
#define AGITATOR_ENB   33
#define AGITATOR_IN4   32
#define AGITATOR_IN3   4

// Screw Stepper (L298N)
#define SCREW_ENA  25
#define SCREW_IN1  26
#define SCREW_IN2  27
#define SCREW_IN3  14
#define SCREW_IN4  12
#define SCREW_ENB  13

// Disc Stepper (L298N)
#define DISC_ENA  23
#define DISC_IN1  22
#define DISC_IN2  21
#define DISC_IN3  19
#define DISC_IN4  18
#define DISC_ENB  5

/* ------------------- SYSTEM VARIABLES ------------------- */

bool systemRunning = false;
int seedDroppingSpeed = 0;

/* ------------------- PWM CONFIG ------------------- */

#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

/* ------------------- STEPPER VARIABLES ------------------- */

unsigned long lastStepTimeScrew = 0;
unsigned long lastStepTimeDisc = 0;

unsigned long screwInterval = 2000;
unsigned long discInterval = 2000;

int screwStepIndex = 0;
int discStepIndex = 0;

unsigned long lastReverseWindow = 0;
bool reverseActive = false;

const unsigned long REVERSE_INTERVAL = 15000;   // 5 seconds
const unsigned long REVERSE_DURATION = 500;    // 200 ms

unsigned long lastReverseWindowDisc = 0;
bool reverseActiveDisc = false;

/* ------------------- SPEED MAPPING ------------------- */

unsigned long computeScrewInterval(int seedSpeed) {
  return map(seedSpeed, 1, 5, 2000, 10000);
}

unsigned long computeDiscInterval(int seedSpeed) {
  return map(seedSpeed, 1, 5, 2000, 10000);
}

/* ------------------- AGITATOR CONTROL ------------------- */

void controlAgitator() {

  if (systemRunning) {
    digitalWrite(AGITATOR_IN3, HIGH);
    digitalWrite(AGITATOR_IN4, LOW);
    ledcWrite(AGITATOR_ENB, 200);
  } else {
    ledcWrite(AGITATOR_ENB, 0);
  }
}

/* ------------------- STEPPER SEQUENCE ------------------- */

void stepMotor(int in1, int in2, int in3, int in4, int stepIndex) {

  switch (stepIndex % 4) {

    case 0:
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
      break;

    case 1:
      digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
      digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
      break;

    case 2:
      digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
      digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
      break;

    case 3:
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
      break;
  }
}

void stopStepper(int ena, int enb, int in1, int in2, int in3, int in4) {

  digitalWrite(ena, LOW);
  digitalWrite(enb, LOW);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

/* ------------------- SCREW CONTROL ------------------- */

void controlScrew() {

  if (!systemRunning) {
    stopStepper(SCREW_ENA, SCREW_ENB,
                SCREW_IN1, SCREW_IN2, SCREW_IN3, SCREW_IN4);
    return;
  }

  digitalWrite(SCREW_ENA, HIGH);
  digitalWrite(SCREW_ENB, HIGH);

  unsigned long currentMillis = millis();

  // Trigger reverse every 5 seconds
  if (currentMillis - lastReverseWindow >= REVERSE_INTERVAL) {
    lastReverseWindow = currentMillis;
    reverseActive = true;
  }

  // Stop reverse after 200 ms
  if (reverseActive && (currentMillis - lastReverseWindow >= REVERSE_DURATION)) {
    reverseActive = false;
  }

  screwInterval = computeScrewInterval(seedDroppingSpeed);

  if (micros() - lastStepTimeScrew >= screwInterval) {
    lastStepTimeScrew = micros();

    if (reverseActive) {
      screwStepIndex--;  // reverse direction
    } else {
      screwStepIndex++;  // forward direction
    }

    stepMotor(SCREW_IN1, SCREW_IN2, SCREW_IN3, SCREW_IN4, screwStepIndex);
  }
}

/* ------------------- DISC CONTROL ------------------- */

void controlDisc() {

  if (!systemRunning) {
    stopStepper(DISC_ENA, DISC_ENB,
                DISC_IN1, DISC_IN2, DISC_IN3, DISC_IN4);
    return;
  }

  digitalWrite(DISC_ENA, HIGH);
  digitalWrite(DISC_ENB, HIGH);

  unsigned long currentMillis = millis();

  // Trigger reverse every 5 seconds
  if (currentMillis - lastReverseWindowDisc >= REVERSE_INTERVAL) {
    lastReverseWindowDisc = currentMillis;
    reverseActiveDisc = true;
  }

  // Stop reverse after 200 ms
  if (reverseActiveDisc &&
      (currentMillis - lastReverseWindowDisc >= REVERSE_DURATION)) {
    reverseActiveDisc = false;
  }

  discInterval = computeDiscInterval(seedDroppingSpeed);

  if (micros() - lastStepTimeDisc >= discInterval) {

    lastStepTimeDisc = micros();

    if (reverseActiveDisc) {
      discStepIndex--;   // reverse
    } else {
      discStepIndex++;   // forward
    }

    stepMotor(DISC_IN1, DISC_IN2, DISC_IN3, DISC_IN4, discStepIndex);
  }
}

/* ------------------- BLUETOOTH HANDLING ------------------- */

void readBluetooth() {

  if (SerialBT.available()) {

    String command = SerialBT.readStringUntil('\n');
    command.trim();

    Serial.print("Received Command: ");
    Serial.println(command);

    if (command == "START") {
      systemRunning = true;
      Serial.println("System STARTED");
    }

    else if (command == "STOP") {
      systemRunning = false;
      Serial.println("System STOPPED");
    }

    else if (command.startsWith("SPEED:")) {
      seedDroppingSpeed = command.substring(6).toInt();
      seedDroppingSpeed = constrain(seedDroppingSpeed, 0, 100);

      Serial.print("Speed Set To: ");
      Serial.println(seedDroppingSpeed);
    }
  }
}

/* ------------------- SETUP ------------------- */

void setup() {

  Serial.begin(9600);
  SerialBT.begin("AKS");   // Bluetooth device name

  Serial.println("ESP32 TMP Controller Started");
  Serial.println("Bluetooth Device Name: AKS");

  pinMode(AGITATOR_IN3, OUTPUT);
  pinMode(AGITATOR_IN4, OUTPUT);
  pinMode(AGITATOR_ENB, OUTPUT);

  pinMode(SCREW_ENA, OUTPUT);
  pinMode(SCREW_ENB, OUTPUT);
  pinMode(SCREW_IN1, OUTPUT);
  pinMode(SCREW_IN2, OUTPUT);
  pinMode(SCREW_IN3, OUTPUT);
  pinMode(SCREW_IN4, OUTPUT);

  pinMode(DISC_ENA, OUTPUT);
  pinMode(DISC_ENB, OUTPUT);
  pinMode(DISC_IN1, OUTPUT);
  pinMode(DISC_IN2, OUTPUT);
  pinMode(DISC_IN3, OUTPUT);
  pinMode(DISC_IN4, OUTPUT);

  ledcAttach(AGITATOR_ENB, PWM_FREQ, PWM_RESOLUTION);
}

/* ------------------- MAIN LOOP ------------------- */

void loop() {

  readBluetooth();
  controlAgitator();
  controlScrew();
  controlDisc();
}




// repurpose code with TMC2209 instead of L289 for stepper