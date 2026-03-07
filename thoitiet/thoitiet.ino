#include <WiFi.h>             // Dùng cho ESP32
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ScioSense_ENS160.h>

// --- 1. CẤU HÌNH WIFI & MQTT ---
const char* ssid = "Huflit-CB-NV";
const char* password = "huflitcbnv@123";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Các Topic MQTT (Bạn có thể đổi tên "simon_house" tùy ý)
const char* topic_alert = "simon_house/air/alert"; // Topic cảnh báo
const char* topic_aqi   = "simon_house/air/aqi";
const char* topic_eco2  = "simon_house/air/eco2";
const char* topic_tvoc  = "simon_house/air/tvoc";
const char* topic_temp  = "simon_house/air/temp";
const char* topic_hum   = "simon_house/air/hum";

// --- 2. KHỞI TẠO ĐỐI TƯỢNG ---
WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // Địa chỉ 0x53

// --- 3. BIẾN LOGIC (Mang từ bài Gas sang) ---
unsigned long lastMsg = 0;
unsigned long interval = 5000; // Mặc định đo chậm 5s
bool isDanger = false;         // Cờ đánh dấu trạng thái nguy hiểm

// Ngưỡng cảnh báo: AQI >= 3 (1-2 là tốt, 3-5 là tệ)
// Hoặc eCO2 > 1000 ppm
const int NGUONG_AQI = 3; 
const int NGUONG_CO2 = 1000;

// --- HÀM KẾT NỐI WIFI ---
void setup_wifi() {
  delay(10);
  Serial.print("\nDang ket noi Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi da ket noi!");
}

// --- HÀM KẾT NỐI MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Dang ket noi MQTT...");
    String clientId = "ESP32_AirClient_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Thanh cong!");
      client.publish(topic_alert, "He thong khoi dong - An toan");
    } else {
      Serial.print("That bai, rc=");
      Serial.print(client.state());
      Serial.println(" thu lai sau 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Khởi động I2C
  Wire.begin(); 

  // Setup AHT
  if (!aht.begin()) {
    Serial.println("Loi: AHT khong phan hoi!");
    while (1) delay(10);
  }
  Serial.println("AHT OK!");

  // Setup ENS160
  if (!ens160.begin()) {
    Serial.println("Loi: ENS160 khong phan hoi!");
    while (1) delay(10);
  }
  ens160.setMode(ENS160_OPMODE_STD);
  Serial.println("ENS160 OK!");

  // Setup Mạng
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Giữ kết nối MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- LOGIC ĐO THÔNG MINH ---
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // 1. Đọc dữ liệu cảm biến
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    // Gửi nhiệt/ẩm vào ENS160 để bù trừ (QUAN TRỌNG)
    ens160.set_envdata(temp.temperature, humidity.relative_humidity);
    ens160.measure(true);
    ens160.measure(false);

    int currentAQI = ens160.getAQI();
    int currentCO2 = ens160.geteCO2();
    int currentTVOC = ens160.getTVOC();

    // 2. Gửi dữ liệu số liệu lên MQTT (Luôn gửi dù an toàn hay nguy hiểm)
    client.publish(topic_temp, String(temp.temperature).c_str());
    client.publish(topic_hum,  String(humidity.relative_humidity).c_str());
    client.publish(topic_aqi,  String(currentAQI).c_str());
    client.publish(topic_eco2, String(currentCO2).c_str());
    client.publish(topic_tvoc, String(currentTVOC).c_str());

    // In ra Serial để kiểm tra
    Serial.print("AQI: "); Serial.print(currentAQI);
    Serial.print(" | CO2: "); Serial.print(currentCO2);
    Serial.print(" | Temp: "); Serial.println(temp.temperature);

    // 3. LOGIC CẢNH BÁO (Logic của Gas Sensor áp dụng sang đây)
    // Nếu AQI >= 3 (Kém) HOẶC CO2 > 1000 (Ngột ngạt)
    if (currentAQI >= NGUONG_AQI || currentCO2 > NGUONG_CO2) {
      
      // Nếu trước đó chưa báo nguy hiểm -> Báo ngay
      if (!isDanger) {
        client.publish(topic_alert, "CANH BAO: KHONG KHI O NHIEM!");
        interval = 1000; // Tăng tốc độ đo lên 1s/lần
        isDanger = true;
        Serial.println("-> Nguy hiem! Chuyen sang do nhanh 1s");
      }
      
    } else {
      // Nếu không khí đã an toàn trở lại
      if (isDanger) {
        client.publish(topic_alert, "Khong khi da an toan");
        interval = 5000; // Quay về đo chậm 5s/lần
        isDanger = false;
        Serial.println("-> An toan. Chuyen sang do cham 5s");
      }
    }
  }
}
