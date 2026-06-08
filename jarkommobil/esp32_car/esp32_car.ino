#include <WiFi.h>
#include <PubSubClient.h> // Library oleh Nick O'Leary (Install di Library Manager)

// ========== KONFIGURASI WIFI ==========
// GANTI DENGAN WIFI YANG MEMILIKI KONEKSI INTERNET
const char* ssid     = "Wifi_Kelompok_10";
const char* password = "burgerbang0r10";

// ========== KONFIGURASI MQTT (HIVEMQ PUBLIC BROKER) ==========
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883; 

// GANTI TOPIC INI SESUAI DENGAN YANG ANDA KETIK DI WEB APP
// Gunakan nama yang unik agar tidak bentrok dengan orang lain di internet!
const char* mqtt_topic = "rc_car/kendali_kelompok_10";

// Buat client WiFi dan MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// ========== KONFIGURASI PIN L298N ==========
const int pinENA = 5;  // PWM Speed Kiri
const int pinIN1 = 17; // Arah 1 Kiri
const int pinIN2 = 16; // Arah 2 Kiri
const int pinIN3 = 4;  // Arah 1 Kanan
const int pinIN4 = 2;  // Arah 2 Kanan
const int pinENB = 15; // PWM Speed Kanan

int currentSpeed = 255; 

void setupPins() {
  pinMode(pinENA, OUTPUT);
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  pinMode(pinENB, OUTPUT);
  stopMotor(); // Pastikan mati di awal
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi ini dipanggil setiap kali ESP32 menerima pesan dari Broker MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Parsing payload dari byte ke String
  String command = "";
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  
  Serial.print("MQTT Received [");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.println(command);
  
  // EKSEKUSI KONTROL MOTOR
  if(command == "F") {
    analogWrite(pinENA, currentSpeed);
    analogWrite(pinENB, currentSpeed);
    moveForward();
  } 
  else if(command == "B") {
    analogWrite(pinENA, currentSpeed);
    analogWrite(pinENB, currentSpeed);
    moveBackward();
  } 
  else if(command == "L") {
    analogWrite(pinENA, currentSpeed);
    analogWrite(pinENB, currentSpeed);
    turnLeft();
  } 
  else if(command == "R") {
    analogWrite(pinENA, currentSpeed);
    analogWrite(pinENB, currentSpeed);
    turnRight();
  } 
  else if(command == "S") {
    stopMotor();
  }
  else if(command.startsWith("V:")) {
    // Ekstrak angka kecepatan, misal "V:150" menjadi angka 150
    String valStr = command.substring(2);
    int val = valStr.toInt();
    setMotorSpeed(val);
  }
}

// Fungsi reconnect untuk menangani putus koneksi internet/broker
void reconnectMQTT() {
  // Terus mencoba hingga terkoneksi kembali
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to HiveMQ...");
    
    // Buat Client ID acak (Syarat beberapa broker publik)
    String clientId = "ESP32Client-RC-";
    clientId += String(random(0xffff), HEX);
    
    // Mencoba terhubung ke Broker
    if (client.connect(clientId.c_str())) {
      Serial.println("connected!");
      
      // Setelah berhasil terhubung, segera SUBSCRIBE ke Topik yang ditentukan
      client.subscribe(mqtt_topic);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  
  setupPins();
  setupWiFi();

  // Konfigurasi server MQTT HiveMQ
  client.setServer(mqtt_server, mqtt_port);
  // Set fungsi callback yang akan menangani pesan masuk
  client.setCallback(mqttCallback);
}

void loop() {
  // Cek apakah WiFi terputus
  if (WiFi.status() != WL_CONNECTED) {
    stopMotor(); // Safety stop
    setupWiFi(); // Coba konek WiFi lagi
  }
  
  // Cek apakah MQTT terputus
  if (!client.connected()) {
    stopMotor(); // Safety stop
    reconnectMQTT();
  }
  
  // Fungsi ini wajib dipanggil terus agar background process MQTT berjalan
  client.loop();
}

// ========== KUMPULAN FUNGSI KONTROL HARDWARE ==========

void setMotorSpeed(int speed) {
  currentSpeed = constrain(speed, 0, 255);
  analogWrite(pinENA, currentSpeed);
  analogWrite(pinENB, currentSpeed);
  Serial.print("Speed updated: ");
  Serial.println(currentSpeed);
}

void moveForward() {
  digitalWrite(pinIN1, HIGH);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, HIGH);
  digitalWrite(pinIN4, LOW);
}

void moveBackward() {
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, HIGH);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, HIGH);
}

void turnLeft() {
  // Motor Kiri Mundur, Motor Kanan Maju
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, HIGH);
  digitalWrite(pinIN3, HIGH);
  digitalWrite(pinIN4, LOW);
}

void turnRight() {
  // Motor Kiri Maju, Motor Kanan Mundur
  digitalWrite(pinIN1, HIGH);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, HIGH);
}

void stopMotor() {
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, LOW);
  analogWrite(pinENA, 0);
  analogWrite(pinENB, 0);
}
