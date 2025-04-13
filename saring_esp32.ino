#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Redmi 10 2022";
const char* password = "23232323";
const char* TOKEN = "BBUS-N7BD5c1zWnvSXahWY92pV7DJniZDtQ"; // Token Ubidots kamu
const char* DEVICE_LABEL = "saring";
const char* VARIABLE_LABEL = "kelembapan";

const int soilPin = 34;
const int relayPin = 27;
const int LED = 2;

int kelembapan = 0;
bool pumpOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(LED, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
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
  http.POST(payload);
  http.end();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
    return;
  }

  kelembapan = readSoil();
  Serial.print("Soil Moisture: "); Serial.println(kelembapan);

  if (kelembapan < 2000) {
    digitalWrite(relayPin, HIGH);
    pumpOn = true;
  } else {
    digitalWrite(relayPin, LOW);
    pumpOn = false;
  }

  digitalWrite(LED, pumpOn ? HIGH : LOW);
  sendToUbidots(kelembapan, pumpOn);
  delay(10000); // 10 detik
}
