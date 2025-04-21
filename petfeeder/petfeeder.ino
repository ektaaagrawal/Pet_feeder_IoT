// Include required libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Blynk Template Information
#define BLYNK_TEMPLATE_ID "TMPL3z5VSuva3"
#define BLYNK_TEMPLATE_NAME "Pet Feeder"
#define BLYNK_AUTH_TOKEN "-e3Gk1DabA0ZAmNYejSmkTnSDF0oJQuI"

// WiFi Credentials
char ssid[] = "jigar";      // Replace with your Wi-Fi SSID
char pass[] = "asdfghjkl";  // Replace with your Wi-Fi password

// Servo setupc:\Users\LENOVO\Pictures\Screenshots\Screenshot 2024-12-24 224513.png
Servo myservo;
const int servoPin = D3; // GPIO pin for servo control
const int OPEN_ANGLE = 90;  // Servo open position
const int CLOSE_ANGLE = 0;  // Servo closed position

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 LCD I2C address

// NTP Client Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // Time offset for IST (GMT +5:30)

// Blynk Timer for periodic tasks
BlynkTimer timer;

// Variables for feeding time
int feedHour = -1;  // Hour set by user (-1 indicates no time set)
int feedMinute = -1;  // Minute set by user
bool hasFedThisMinute = false;  // Flag to ensure feeding happens only once per minute

// Function prototypes
void updateTime();
void feedPet();

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  while (!Blynk.connected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nBlynk connected!");
  lcd.setCursor(0, 1);
  lcd.print("Blynk Connected!");

  // Initialize servo
  myservo.attach(servoPin);
  myservo.write(CLOSE_ANGLE); // Start with the servo in the closed position
  Serial.println("Servo initialized.");

  // Initialize NTP client
  timeClient.begin();
  Serial.println("NTP client initialized.");

  // Timer to update time every second
  timer.setInterval(1000L, updateTime);

  Serial.println("Setup complete!");
}

void loop() {
  Blynk.run();  // Handle Blynk events
  timer.run();  // Handle timer tasks
}

// Blynk virtual button to feed the pet immediately (V1)
BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    feedPet();
  }
}

// Blynk Time Input to set feeding time (V2)
BLYNK_WRITE(V2) {
  TimeInputParam t(param);

  if (t.hasStartTime()) {
    // Get start time from widget
    feedHour = t.getStartHour();
    feedMinute = t.getStartMinute();

    Serial.print("Feeding time set: ");
    Serial.print(feedHour);
    Serial.print(":");
    Serial.println(feedMinute);

    // Display feeding time on LCD
    lcd.setCursor(0, 1);
    lcd.print("Feed Time: ");
    lcd.print(feedHour);
    lcd.print(":");
    if (feedMinute < 10) lcd.print("0");
    lcd.print(feedMinute);
  } else {
    Serial.println("Feeding time not set!");
    feedHour = -1; // Reset feeding time if none is set
    feedMinute = -1;
  }
}

// Function to update and display time on LCD
void updateTime() {
  timeClient.update();
  String timeStr = timeClient.getFormattedTime();

  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(timeStr);
  Serial.println("Time updated: " + timeStr);

  // Compare current time with feeding time
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  if (feedHour != -1 && currentHour == feedHour && currentMinute == feedMinute) {
    if (!hasFedThisMinute) {
      feedPet();
      hasFedThisMinute = true;  // Mark feeding done for this minute
    }
  } else {
    hasFedThisMinute = false;  // Reset the flag if time no longer matches
  }
}

// Function to feed the pet
void feedPet() {
  Serial.println("Feeding pet...");
  lcd.setCursor(0, 1);
  lcd.print("Feeding...       ");

  myservo.write(OPEN_ANGLE);  // Open the servo
  delay(3000);               // Wait for 3 seconds
  myservo.write(CLOSE_ANGLE); // Close the servo

  lcd.setCursor(0, 1);
  lcd.print("Feed Complete!   ");
  Serial.println("Feeding complete.");
}
