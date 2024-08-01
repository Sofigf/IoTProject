#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>

const char *ssid = "LAPAN CUAN";
const char *password = "sebentaryakak";

WiFiClient esp_client;
HTTPClient http;

String Host = "http://monitoringB[a]P.my.id";
String dataSendUrl = Host + "/index.php";

float MQ9;
int sampling;  
long rssi = WiFi.RSSI();
unsigned long CurrentMillis, PreviousMillis, DataSendingTime = (unsigned long)1000 * 30;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Terhubung ke jaringan WiFi dengan alamat IP: " + WiFi.localIP().toString());
  PreviousMillis = millis();
}

String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void loop() {
  CurrentMillis = millis();

  if (CurrentMillis - PreviousMillis >= DataSendingTime) {
      String msg = "";
      while (Serial.available()) {
        msg += char(Serial.read());
        delay(50);
      }

      MQ9 = splitString(msg, ';', 0).toFloat();
      sampling = splitString(msg, ';', 4).toInt();
      rssi = WiFi.RSSI(); 
    

      Serial.print("RSSI (dBm): ");
      Serial.println(rssi);
      Serial.print("MQ9: ");
      Serial.println(MQ9);
      Serial.println(sampling);

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        
        http.begin(client, dataSendUrl);
        http.addHeader("Content-Type", "application/json");

        // Construct the JSON payload
        String httpRequestData = "{";
        httpRequestData += "\"Sampling\": " + String(sampling) + ", ";
        httpRequestData += "\"MQ9\": " + String(MQ9) + ", ";
        httpRequestData += "\"RSSI\": " + String(rssi);
        httpRequestData += "}";

        Serial.print("HTTP Request Data: ");
        Serial.println(httpRequestData);

        int httpResponseCode = http.POST(httpRequestData);

        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
          // Successfully sent
          Serial.println("HTTP Request Successful");
        } else {
          // Failed to send
          Serial.println("HTTP Request Failed");
        }

        http.end();
      } 
      else {
        Serial.println("WiFi Disconnected");
      }
    PreviousMillis = CurrentMillis;
  }
  delay (30000);
}
