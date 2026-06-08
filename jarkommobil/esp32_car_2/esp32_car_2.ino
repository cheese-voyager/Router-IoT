#include <WiFi.h>
#include <PubSubClient.h> 

// ========== KONFIGURASI WIFI (MINTA HOTSPOT KE MOBIL 1) ==========
// Anda harus memastikan ESP jaringan pada Mobil 1 memancarkan SSID dan Password di bawah ini.
// Ganti nilainya sesuai dengan konfigurasi Access Point (SoftAP) yang dibuat oleh ESP Jaringan Mobil 1.
const char* ssid     = "HOTSPOT_MOBIL_1";
const char* password = "PASSWORD_MOBIL_1";

// ========== KONFIGURASI MQTT (HIVEMQ PUBLIC BROKER) ==========
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883; 

// Topik MQTT disamakan persis dengan Mobil 1 agar Mobil 2 bergerak mengikuti Mobil 1
const char* mqtt_topic = "rc_car/kendali_kelompok_10";

WiFiClient espClient;
PubSubClient client(espClient);

// ========== KONFIGURASI PIN L298N ==========
const int pinENA = 14;  
const int pinIN1 = 27; 
const int pinIN2 = 26; 
const int pinIN3 = 25;  
const int pinIN4 = 33;  
const int pinENB = 32; 

int currentSpeed = 255; 

void setupPins() {
  pinMode(pinENA, OUTPUT);
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  pinMode(pinENB, OUTPUT);
  stopMotor(); 
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke Hotspot Mobil 1: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nBerhasil terhubung ke Hotspot Mobil 1!");
  Serial.print("Alamat IP Mobil 2: ");
  Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String command = "";
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  
  Serial.print("Pesan MQTT Diterima [");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.println(command);
  
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
    String valStr = command.substring(2);
    int val = valStr.toInt();
    setMotorSpeed(val);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    
    String clientId = "ESP32-Mobil2-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung!");
      client.subscribe(mqtt_topic);
      Serial.print("Berlangganan pada topik: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("Gagal, status=");
      Serial.print(client.state());
      Serial.println(" Mencoba lagi dalam 5 detik.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  
  setupPins();
  setupWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    stopMotor(); 
    setupWiFi(); 
  }
  
  if (!client.connected()) {
    stopMotor(); 
    reconnectMQTT();
  }
  
  client.loop();
}

void setMotorSpeed(int speed) {
  currentSpeed = constrain(speed, 0, 255);
  analogWrite(pinENA, currentSpeed);
  analogWrite(pinENB, currentSpeed);
  Serial.print("Kecepatan diperbarui: ");
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
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, HIGH);
  digitalWrite(pinIN3, HIGH);
  digitalWrite(pinIN4, LOW);
}

void turnRight() {
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
