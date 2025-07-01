// This file is provided with a youtube tutorial made by Al Moeid

#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// --- WiFi ---
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// --- State ---
volatile boolean state = false;

// --- Pins ---
const int switchPin = 3;         // Switch input
const int buzzerPin = 4;          // Output pin to go HIGH on internet down
const int neopixelPin = 1;        // NeoPixel data pin
const int numPixels = 1;          // Number of NeoPixels

// --- NeoPixel ---
Adafruit_NeoPixel pixels(numPixels, neopixelPin, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  pinMode(switchPin, INPUT_PULLUP);
  ledcAttach(buzzerPin, 2000, 8);
  ledcWriteTone(buzzerPin, 0);  

  pixels.begin();
  pixels.setBrightness(50);
  pixels.show(); // Clear LED initially

  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  // Start tasks
  xTaskCreatePinnedToCore(internetIndicatorTask, "NeoPixel Blink Task", 1000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(switchMonitorTask, "Switch Monitor", 1000, NULL, 1, NULL, 1);
}

void loop() {
  WiFiClient client;
  if (client.connect("8.8.8.8", 53)) {
    Serial.println("Internet is UP");
    client.stop();
    state = false;
  } else {
    Serial.println("Internet is DOWN");
    state = true;
  }

  delay(10000); // Check every 60 seconds
}

void internetIndicatorTask(void* parameter) {
  bool toggle = false;
  while (true) {
    if (state) {
      ledcWriteTone(buzzerPin, 2000);  
      if (toggle) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue
      }
      pixels.show();
      toggle = !toggle;
    } else {
      ledcWriteTone(buzzerPin, 0);  
      pixels.clear();  // Turn off LED
      pixels.show();
    }
    delay(500); // Blink rate
  }
}

void switchMonitorTask(void* parameter) {
  while (true) {
    if (digitalRead(switchPin) == LOW) {
      Serial.println("Switch pressed → resetting state");
      state = false;
    }
    delay(200); // Polling interval
  }
}
