#include <WiFi.h>

const char *ssid      = "EVHERO Design Lab";
const char *password  = "SampurasuN712";

void setup(){
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  Serial.begin(115200);
  Serial.print("\nDefault ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");      
  }

  // if (WiFi.status() == WL_CONNECTED){
    Serial.println("ESP32 Terhubung ke Internet");
    Serial.print("IP Station: ");
    Serial.println(WiFi.localIP());
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
  // }
}

void loop(){  }