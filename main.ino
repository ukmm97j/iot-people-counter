#include <WiFiEsp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>

// Setup serial communication with ESP8266 via D2 and D3
SoftwareSerial espSerial(2, 3); // RX, TX

// WiFi credentials
char ssid[] = "M97";
char pass[] = "10101010";

// ThingSpeak settings
unsigned long channelID = 2966182;
const char* writeAPIKey = "KQI3CL1K1IPSCGYO";

// Network client
WiFiEspClient client;

// LCD configuration (I2C address 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin configuration
int irPin = 8;        // IR sensor input pin
int ledPin = 7;       // LED output pin

// Variables
int motionCount = 0;
bool objectDetected = false;
unsigned long lastSendTime = 0;
unsigned long sendInterval = 15000; // 15 seconds interval for ThingSpeak

void setup() {
  pinMode(irPin, INPUT);
  pinMode(ledPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting...");

  Serial.begin(9600);
  espSerial.begin(9600);     // Start software serial for ESP
  WiFi.init(&espSerial);     // Initialize WiFi library

  if (WiFi.status() == WL_NO_SHIELD) {
    lcd.clear();
    lcd.print("No WiFi shield");
    while (true);
  }

  // Connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("Connecting WiFi");
    WiFi.begin(ssid, pass);
    delay(5000);
  }

  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(1000);

  ThingSpeak.begin(client);
}

void loop() {
  int motion = digitalRead(irPin);

  // Motion detected
  if (motion == LOW && !objectDetected) {
    objectDetected = true;
    motionCount++;

    digitalWrite(ledPin, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Object Detected");
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(motionCount);

    Serial.print("Detected! Count = ");
    Serial.println(motionCount);

    if (millis() - lastSendTime > sendInterval) {
      int statusCode = ThingSpeak.writeField(channelID, 1, motionCount, writeAPIKey);
      if (statusCode == 200) {
        Serial.println("Sent to ThingSpeak");
      } else {
        Serial.print("Failed. Status: ");
        Serial.println(statusCode);
      }
      lastSendTime = millis();
    }

    delay(1000); // Avoid rapid multiple counts
  }

  // Reset when object no longer present
  if (motion == HIGH && objectDetected) {
    objectDetected = false;
    digitalWrite(ledPin, LOW);
  }
}
