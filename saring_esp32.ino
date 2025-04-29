#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfigurasi WiFi dan Ubidots
const char* ssid = "R LAB FISIKA";
const char* password = "l4b.f1s1k4";
const char* TOKEN = "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ"; // Token Ubidots kamu
const char* DEVICE_LABEL = "saring";
const char* VARIABLE_LABEL = "kelembapan";

// Pin Setting
const int soilPin = 34;   // Sensor kelembapan tanah
const int relayPin = 27;  // Relay untuk pompa
const int LED = 2;        // LED indikator

// OLED Display Setting
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variabel global
int kelembapan = 0;
bool pumpOn = false;

void setup() {
  Serial.begin(115200);

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Gagal inisialisasi OLED"));
    for (;;); // Berhenti jika OLED gagal
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Mulai...");
  display.display();
  delay(1000);

  // Inisialisasi pin
  pinMode(relayPin, OUTPUT);
  pinMode(LED, OUTPUT);

  // Koneksi WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

int readSoil() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(soilPin);
    delay(20);
  }
  return total / 10;
}

void sendToUbidots(int value, int status) {
  HTTPClient http;
  String url = "http://industrial.api.ubidots.com/api/v1.6/devices/";
  url += DEVICE_LABEL;

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Auth-Token", TOKEN);

  String payload = "{\"" + String(VARIABLE_LABEL) + "\":" + String(value) + ",\"pompa\":" + String(status) + "}";
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.print("Data terkirim. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error kirim data. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void loop() {
  // Pastikan WiFi tetap terkoneksi
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
    return;
  }

  kelembapan = readSoil();
  Serial.print("Soil Moisture: ");
  Serial.println(kelembapan);

  // Kontrol Pompa berdasarkan kelembapan
  if (kelembapan < 3000) {
    digitalWrite(relayPin, HIGH);
    pumpOn = true;
  } else {
    digitalWrite(relayPin, LOW);
    pumpOn = false;
  }
  digitalWrite(LED, pumpOn ? HIGH : LOW);

  // Update OLED
  display.clearDisplay();
  display.setTextSize(20);
  display.setCursor(0,0);
  display.print("Moisture: ");
  display.println(kelembapan);

  display.setCursor(0, 20);
  display.print("Pump: ");
  display.println(pumpOn ? "ON" : "OFF");

  display.display();

  // Kirim data ke Ubidots
  sendToUbidots(kelembapan, pumpOn);

  delay(10000); // Delay 10 detik
}

