// libraries
#include <DHT.h>
#include <IOXhop_FirebaseESP32.h>
#include <TinyGPSPlus.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Firebase Endpoint/path & Authentication
#define FIREBASE_HOST " URL "   // End point
#define FIREBASE_AUTH " Key "  // Authentication key

// Defining Pins of esp32 to sensors
#define LED_PIN 2
#define DO_PIN 14  
#define AO_PIN 34
#define PIN_TO_SENSOR 19
DHT dht(26, DHT11);
int trigPin = 5;  // GPIO pin for trigger (example pin for ESP32)
int echoPin = 18;  // GPIO pin for echo (example pin for ESP32)
int ledPin = 4;      
int inputPin = 19;            
int pirState = LOW;            
int val = 0;

// UltraSoinc sensor constants
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// OLED display 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Objects
Servo servo1;  // Create a servo object
Servo servo2;  // Create a servo object
long distance;
long duration;
TinyGPSPlus gps;
float distanceCm;
float distanceInch;

// This function run only once 
void setup() {
  
  dht.begin();
  Serial.begin(9600);
  Serial2.begin(9600);
  
  servo1.attach(13);  // Attach servo to pin 13 
  servo2.attach(12);  // Attach servo to pin 12 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Wifi Credentials and message
  WiFi.begin("SSID", "Password");
 
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

   // OLED CODE
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  // Display static text
  display.println("Data Visualizer Rover");
  display.display(); 

  // ?
  pinMode(LED_PIN, OUTPUT);
  pinMode(DO_PIN, INPUT);

  pinMode(ledPin, OUTPUT);  
  pinMode(inputPin, INPUT);  
  delay(2000);
}

// This function runs continuously 
void loop() {

  // DHT11 Logic
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  Serial.print("Temp ");
  Serial.print(temp);
  Serial.print(" C ");
  Serial.print("Humidity ");
  Serial.print(humidity);
  Serial.print(" % ");

 // DHT11 readings collected & updating to firebase
  Firebase.setFloat("/temperature", temp);
  Firebase.setFloat("/humidity", humidity);

// Inbuilt LED Logic
  bool switchStatus = Firebase.getBool("/led");

  if (switchStatus) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED turned ON");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED turned OFF");
  }

// Ultra Function Called (Servo)
 ultra();
 // Sevo rotation logic
 servo1.write(0);  // Set servo to initial position
 if (distanceCm <= 10) {
    servo1.write(120);  // Move servo to 120 degrees if distance is less than or equal to 10
  }
  delay(1000);
//  
//  digitalWrite(trigPin, LOW);
//  delayMicroseconds(2);
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//  duration = pulseIn(echoPin, HIGH);
//  distanceCm = duration * SOUND_SPEED / 2;
//  distanceInch = distanceCm * CM_TO_INCH;
//
//  Serial.print("Distance (cm): ");
//  Serial.println(distanceCm);
//  Serial.print("Distance (inch): ");
//  Serial.println(distanceInch);

  Firebase.setFloat("/usensorCm", distanceCm);
//  Firebase.setFloat("/usensorInch", distanceInch);
// Gas Sensor Logic
  float gasState = digitalRead(DO_PIN);
  Firebase.setFloat("/gsensor", gasState);

  Serial.print("GasState is : " );
  Serial.println(gasState);
  if (gasState)
    Serial.println("The gas is NOT present");
  else
    Serial.println("The gas is present");

// Checking GPS connection
   while (Serial2.available() > 0)
   if (gps.encode(Serial2.read()))
      sendGPSData();
     
   if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
  }

 // PIR Sensor Logic
  val = digitalRead(inputPin);    
  if (val == HIGH)  
  {            
    digitalWrite(ledPin, HIGH);  
 
    if (pirState == LOW)
  {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  }
  else
  {
    digitalWrite(ledPin, LOW);
    if (pirState == HIGH)
  {
      Serial.println("Motion Detection ended!");  
      pirState = LOW;
    }
  }

  Firebase.setFloat("/pir", pirState);

  // servo
  cam_servo();
  delay(10000);
  // Loop Ends
}

// Function for printing updating GPS reading to Firebase
void sendGPSData() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()){
    Serial.print("Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print("Lng: ");
    Serial.print(gps.location.lng(), 6);
    Serial.println();

    // Send GPS data to Firebase
    Firebase.setFloat("/latitude", gps.location.lat());
    Firebase.setFloat("/longitude", gps.location.lng());
  } else {
    Serial.print(F("INVALID"));
  }
}

// Function for Servo rotation
void ultra() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.034 / 2;
}

void cam_servo() {
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo2.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }

  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo2.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }
}
