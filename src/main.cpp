#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// WiFi credentials
#define WIFI_SSID "LagCT"
#define WIFI_PASSWORD "28052004"

// Firebase legacy
#define FIREBASE_HOST "https://hcmcity-afe25-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "lNM7wc3Zqh8Xu9GplYVppsDsBIlDv9sxkLDrFT1V"

// DHT
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// OLED I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// LED và buzzer
#define BUZZER 33

// Thêm khai báo chân cảm biến ánh sáng nếu cần
#define LIGHT_SENSOR_PIN 34

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

unsigned long lastSendTime = 0;
unsigned long blinkTime = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); // Đảm bảo buzzer tắt ban đầu


  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Không tìm thấy OLED");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("🔌 Kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n✅ WiFi đã kết nối");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("✅ Firebase đã sẵn sàng");

  pinMode(LIGHT_SENSOR_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSendTime > 2000) {
    lastSendTime = now;

    // Đọc giá trị cảm biến
    float nhietdo = dht.readTemperature();
    float doam = dht.readHumidity();
    int anhsang = analogRead(LIGHT_SENSOR_PIN);

    // Kiểm tra lỗi cảm biến DHT
    if (isnan(nhietdo) || isnan(doam)) {
      Serial.println("❌ Lỗi đọc cảm biến DHT!");
      return;
    }

    // In ra Serial
    Serial.println("===== Giá trị cảm biến =====");
    Serial.printf("🌡️ Nhiệt độ: %.2f °C\n", nhietdo);
    Serial.printf("💧 Độ ẩm   : %.2f %%\n", doam);
    Serial.printf("💡 Ánh sáng: %d\n", anhsang);

    // Gửi lên Firebase (nên kiểm tra trả về)
    if (!Firebase.setFloat(fbdo, "/nhietdo", nhietdo))
      Serial.println("❌ Lỗi gửi nhiệt độ lên Firebase");
    if (!Firebase.setFloat(fbdo, "/doam", doam))
      Serial.println("❌ Lỗi gửi độ ẩm lên Firebase");
    if (!Firebase.setInt(fbdo, "/anhsang", anhsang))
      Serial.println("❌ Lỗi gửi ánh sáng lên Firebase");

    // Hiển thị lên OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Ten: Nguyen Hoang Anh Tan");
    display.printf("Nhiet do: %.1f C\n", nhietdo);
    display.printf("Do am: %.1f %%\n", doam);
    display.printf("Anh sang: %d\n", anhsang);
    display.display();

    // Điều khiển buzzer
    if (anhsang > 1500) {
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(BUZZER, LOW);
    }
  }
}
