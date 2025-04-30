#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ======= WiFi & Ubidots =======
const char* ssid = "R LAB FISIKA";
const char* password = "l4b.f1s1k4";
const char* TOKEN = "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ";
const char* DEVICE_LABEL = "saring";
const char* VARIABLE_LABEL = "kelembapan";

// ======= Pin Setup =======
const int soilPin = 34;
const int relayPin = 27;
const int LED = 2;
const int redPin = 18;
const int yellowPin = 19;
const int greenPin = 23;

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variabel
int kelembapan = 0;
bool pumpOn = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA = 21, SCL = 22

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED GAGAL");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Mulai...");
  display.display();
  delay(1000);

  // Pin
  pinMode(relayPin, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Terhubung");
  display.display();
  delay(1000);
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
    Serial.print("Data terkirim. Code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Gagal kirim. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
    return;
  }

  int kelembapanRaw = readSoil();
  kelembapan = map(kelembapanRaw, 4095, 0, 0, 100);

  Serial.print("Moisture: ");
  Serial.print(kelembapan);
  Serial.println(" %");

  // === Kontrol Pompa ===
  if (kelembapan < 40) {
    digitalWrite(relayPin, HIGH);
    pumpOn = true;
  } else {
    digitalWrite(relayPin, LOW);
    pumpOn = false;
  }

  // === LED Indikator ===
  if (kelembapan < 40) {
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
  } else if (kelembapan >= 40 && kelembapan <= 60) {
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, HIGH);
    digitalWrite(greenPin, LOW);
  } else {
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
  }

  digitalWrite(LED, pumpOn ? HIGH : LOW); // LED onboard menandakan pompa aktif

  // === OLED ===
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SARING - MONITORING");
  display.setCursor(0, 15);
  display.print("Moisture: ");
  display.print(kelembapan);
  display.println(" %");
  display.setCursor(0, 30);
  display.print("Pump: ");
  display.println(pumpOn ? "ON" : "OFF");
  display.display();

  // === Kirim ke Ubidots ===
  sendToUbidots(kelembapan, pumpOn);

  delay(10000); // delay 10 detik
}
