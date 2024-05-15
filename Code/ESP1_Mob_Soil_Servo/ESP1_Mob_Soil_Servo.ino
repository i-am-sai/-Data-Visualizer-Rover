 #include <IOXhop_FirebaseESP32.h>
#include <ESP32Servo.h>

#define FIREBASE_HOST "https://m-testing-3373a-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "NH5RYKcbJzNML4QxsM9R1MxUg17929oENqboBm6U"
#define soil_mositure_pin 21  //change it to  21
static const int servoPin = 13;

Servo myServo;
static bool isServoOn = false; // Declare isServoOn as a static variable

const int motor1Pin1 = 5;   // in1 to GPIO 5 (d5)
const int motor1Pin2 = 17;  // in2 to GPIO 17 (tx2)
const int enable1Pin = 18;   // enA to GPIO 18 (d18)
const int motor2Pin1 = 4;   // in4 to GPIO 4 (d4)
const int motor2Pin2 = 16;  // in3 to GPIO 16 (rx2)
const int enable2Pin = 2;    // enB to GPIO 2 (d2)

const int pwmChannel1 = 0;   // PWM channel for motor 1
const int pwmChannel2 = 1;   // PWM channel for motor 2
const int freq = 30000;      // PWM frequency
const int resolution = 8;     // PWM resolution
int dutyCycle = 170;          // Initial duty cycle

bool backwardStatus = false;
bool forwardStatus = false;
bool rightStatus = false;
bool leftStatus = false;
bool servoStatus = false;
bool isSoilOn = false;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10; // Debounce time in milliseconds

void setup() {
  Serial.begin(115200);
  WiFi.begin("Sai", "saisaisai");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);
  myServo.attach(servoPin);

  ledcSetup(pwmChannel1, freq, resolution);
  ledcSetup(pwmChannel2, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel1);
  ledcAttachPin(enable2Pin, pwmChannel2);
  ledcWrite(pwmChannel1, dutyCycle);
  ledcWrite(pwmChannel2, dutyCycle);
}

void loop() {
  // Read Firebase boolean values with debounce
  if (millis() - lastDebounceTime > debounceDelay) {
    lastDebounceTime = millis();
    readFirebaseValues();
    handleMotionControl();
  }
}

void readFirebaseValues() {
  backwardStatus = Firebase.getBool("/back");
  forwardStatus = Firebase.getBool("/front");
  rightStatus = Firebase.getBool("/right");
  leftStatus = Firebase.getBool("/left");
  servoStatus = Firebase.getBool("/servo");
  isSoilOn = Firebase.getBool("/soilOn");
}

void handleMotionControl() {
  if (servoStatus==false) {
    callMotionControl(backwardStatus, forwardStatus, rightStatus, leftStatus);
  } else if (backwardStatus==false && forwardStatus==false && rightStatus==false && leftStatus ==false) {
    servoWithSoil(servoStatus,isSoilOn);
  }
}

void callMotionControl(bool backward, bool forward, bool right, bool left) {
  if (backward) {
    moveBackward();
  } else if (forward) {
    moveForward();
  } else if (right) {
    moveRight();
  } else if (left) {
    moveLeft();
  } else {
    stopMotion();
  }
}

void moveBackward() {
  digitalWrite(motor1Pin1, HIGH); // Motor 1 forward
  digitalWrite(motor2Pin1, HIGH); // Motor 2 forward
  for (dutyCycle = 170; dutyCycle < 255; dutyCycle++) {
    ledcWrite(pwmChannel1, dutyCycle);
    ledcWrite(pwmChannel2, dutyCycle);
  }
}

void moveForward() {
  digitalWrite(motor1Pin2, HIGH); // Motor 1 backward
  digitalWrite(motor2Pin2, HIGH); // Motor 2 backward
  for (dutyCycle = 170; dutyCycle < 255; dutyCycle++) {
    ledcWrite(pwmChannel1, dutyCycle);
    ledcWrite(pwmChannel2, dutyCycle);
  }
}

void moveRight() {
  digitalWrite(motor1Pin1, HIGH); // Motor 1 forward
  digitalWrite(motor2Pin2, HIGH); // Motor 2 backward
  for (dutyCycle = 170; dutyCycle < 255; dutyCycle++) {
    ledcWrite(pwmChannel1, dutyCycle);
    ledcWrite(pwmChannel2, dutyCycle);
  }
}

void moveLeft() {
  digitalWrite(motor1Pin2, HIGH); // Motor 1 backward
  digitalWrite(motor2Pin1, HIGH); // Motor 2 forward
  for (dutyCycle = 170; dutyCycle < 255; dutyCycle++) {
    ledcWrite(pwmChannel1, dutyCycle);
    ledcWrite(pwmChannel2, dutyCycle);
  }
}

void stopMotion() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

void servoWithSoil(bool servoStatus, bool isSoilOn) {
  static bool isServoOn = false;

  if (servoStatus == true) {
    Serial.println("Servo On");
    if (!isServoOn) {
      Serial.println("Turning Servo On");
      for (int i = 0; i <= 179; i++) {
        myServo.write(i);
        delay(5);
        Serial.print("Servo Position: ");
        Serial.println(i);
        isServoOn = true;
      }
    }
  }

  if (isSoilOn == true) {
    Serial.println(soil_mositure_pin);
    Firebase.setInt("/soilData", soil_mositure_pin);
  } else if (servoStatus == false) {
    if (isServoOn == true) {
      Serial.println("Turning Servo Off");
      for (int i = 179; i >= 0; i--) {
        myServo.write(i);
        delay(5);
        Serial.print("Servo Position: ");
        Serial.println(i);
        isServoOn = false;
      }
    }
  }
}
