#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_netif.h>

// =========================================================
// 1. KONFIGURASI WIFI STATION (Sumber Internet)
// =========================================================
const char *ssid_sta      = "Starlink_IoT_Kelompok-10";
const char *password_sta  = "burgerbang0r";

// =========================================================
// 2. KONFIGURASI WIFI ACCESS POINT (Hotspot ESP32)
// =========================================================
const char *ssid_ap       = "Wifi_Kelompok_10";
const char *password_ap   = "burgerbang0r10";
const int channel         = 1;
const int hidden          = 0;
const int MAX_ALLOWED     = 3; // Maksimal perangkat terhubung bersamaan

// =========================================================
// 3. KONFIGURASI MAC ADDRESS WHITELIST
// =========================================================
struct MacAddress {
  uint8_t bytes[6];
};

MacAddress whitelist[MAX_ALLOWED] = {
  {{0x1C, 0xC3, 0xAB, 0xBF, 0x7D, 0x58}}, // Contoh Device 1
  {{0x00, 0x4B, 0x12, 0x37, 0xD7, 0xD4}}  // Contoh Device 2
};

bool isMacEqual(const uint8_t* mac1, const uint8_t* mac2) {
  for (int i = 0; i < 6; i++) {
    if (mac1[i] != mac2[i]) return false;
  }
  return true;
}

// =========================================================
// 4. EVENT HANDLER (Mengatur Perangkat Masuk/Keluar)
// =========================================================
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED: {
      uint8_t* connected_mac = info.wifi_ap_staconnected.mac;
      uint16_t connected_aid = info.wifi_ap_staconnected.aid;
      bool access_granted = false;

      Serial.print("\n[+] Perangkat mencoba terhubung... MAC: ");
      for(int i = 0; i < 6; i++){
        Serial.printf("%02X", connected_mac[i]);
        if(i < 5) Serial.print(":");
      }
      Serial.println();

      // Cek apakah MAC Address ada di Whitelist
      for (int i = 0; i < MAX_ALLOWED; i++) {
        if (isMacEqual(connected_mac, whitelist[i].bytes)) {
          access_granted = true;
          break;
        }
      }

      if (access_granted) {
        Serial.println("    -> AKSES DITERIMA! Perangkat terdaftar.");
      } else {
        Serial.println("    -> AKSES DITOLAK! Menendang perangkat (Deauth)...");
        // PERBAIKAN: Gunakan 'connected_aid', bukan 'connected_mac'
        esp_wifi_deauth_sta(connected_aid);
      }
      break;
    }

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("[-] Sebuah perangkat telah terputus dari Hotspot ESP32.");
      break;

    default:
      break;
  }
}

// =========================================================
// 5. SETUP
// =========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Memulai Sistem ESP32 Router ---");

  // Set mode ke AP dan Station secara bersamaan
  WiFi.mode(WIFI_AP_STA);

  // Daftarkan Event Handler untuk memantau Hotspot
  WiFi.onEvent(WiFiEvent);

  // --- A. KONEKSI KE INTERNET (Station) ---
  Serial.printf("Menghubungkan ke %s ", ssid_sta);
  WiFi.begin(ssid_sta, password_sta);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");      
  }
  Serial.println("\n[Berhasil] ESP32 Terhubung ke Internet!");
  Serial.print("IP Station: ");
  Serial.println(WiFi.localIP());

  // --- B. MEMBUAT HOTSPOT (Access Point) ---
  Serial.println("Mengkonfigurasi Access Point...");
  if (!WiFi.softAP(ssid_ap, password_ap, channel, hidden, MAX_ALLOWED)) {
    Serial.println("Pembuatan Soft AP Gagal!");
    while (1); // Berhenti di sini jika gagal
  }
  Serial.print("IP Hotspot: ");
  Serial.println(WiFi.softAPIP());

  // --- C. MENGAKTIFKAN INTERNET SHARING (NAT) ---
  // Kode ini menjembatani internet dari Station (Starlink) ke Hotspot (Kelompok 10)
  esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (ap_netif) {
      esp_netif_napt_enable(ap_netif);
      Serial.println("[Berhasil] Fitur Internet Sharing (NAT) Aktif!");
  } else {
      Serial.println("[Gagal] Tidak dapat mengaktifkan NAT.");
  }
  
  Serial.println("--- Sistem Siap Digunakan ---");
}

// =========================================================
// 6. MAIN LOOP
// =========================================================
void loop() {
  // Kosongkan loop jika ESP32 hanya berfungsi sebagai router.
  // Jika kamu ingin menambahkan fitur Web Server / kontrol LED seperti di Kode 2,
  // kamu bisa memasukkan logika NetworkClient di sini.
}