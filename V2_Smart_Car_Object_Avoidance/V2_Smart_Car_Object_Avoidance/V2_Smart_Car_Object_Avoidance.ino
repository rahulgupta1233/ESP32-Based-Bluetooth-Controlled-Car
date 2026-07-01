#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo scanServo;

// ---------------- Motor Pins ----------------
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

// ------------ Ultrasonic Sensor Pins --------
#define TRIG_PIN 5
#define ECHO_PIN 18

// ---------------- Servo Pin -----------------
#define SERVO_PIN 13

bool autoMode = false;

// ------------------- Setup -------------------
void setup() {
  // Initialize Bluetooth
  SerialBT.begin("ESP32_CAR"); 

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  scanServo.attach(SERVO_PIN, 500, 2400);
  scanServo.write(90);

  stopCar();
}

// ------------------- Main Loop -------------------
void loop() {

  // Read Bluetooth Commands
  if (SerialBT.available()) {
    char cmd = SerialBT.read();

    // Change Modes
    if (cmd == '1') { 
      autoMode = true; //Switch to autoMode 
      stopCar();
    }
    else if (cmd == '0') {
      autoMode = false; // Switch to Manual Mode
      stopCar();
    }

    // Manual Controls
    else if (!autoMode) {
      switch (cmd) {
        case 'F':
          forward();
          break;
        case 'B':
          backward();
          break;
        case 'L':
          left();
          break;
        case 'R':
          right();
          break;
        case 'S':
          stopCar();
          break;
      }
    }
  }

  // Run Obstacle Avoidance
  if (autoMode) {
    obstacleAvoid();
  }

  // CRITICAL: Small delay yields time to the ESP32's background Bluetooth task.
  // Without this, the tight loop can starve the radio and prevent connection.
  delay(10); 
}

// ---------------- Distance Function ----------------
long getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
    return 400;

  long distance = duration * 0.034 / 2;

  return distance;
}

// --------------- Obstacle Avoidance ----------------
void obstacleAvoid() {

  long frontDist = getDistance();

  if (frontDist >= 20) {
    forward();
  }
  else {
    stopCar();
    delay(300);

    backward();
    delay(400);

    stopCar();
    delay(200);

    // Scan Left
    scanServo.write(150);
    delay(500);
    long leftDist = getDistance();

    // Scan Right
    scanServo.write(30);
    delay(500);
    long rightDist = getDistance();

    // Return Servo to Center
    scanServo.write(90);
    delay(300);

    if (leftDist > rightDist && leftDist >= 20) {
      left();
      delay(500);
      stopCar();
    }
    else if (rightDist >= 20) {
      right();
      delay(500);
      stopCar();
    }
    else {
      backward();
      delay(700);

      right();
      delay(700);

      stopCar();
    }
  }
}

// ---------------- Motor Functions ----------------
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}