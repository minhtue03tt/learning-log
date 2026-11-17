#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Preferences.h>

// Khai bao chan
#define DHT_PIN   4
#define LED_GREEN 26
#define LED_RED   27
#define BTN_UP    32
#define BTN_DOWN  33

// cac doi tuong va bien
DHT dht(DHT_PIN, DHT22);
LiquidCrystal_I2C lcd(0x3F, 16, 2);   // Neu LCD khong hien, doi 0x27 thanh 0x3F
Preferences prefs;

float tempOffset = 0.0;               // Gia tri can chinh
float t = NAN, h = NAN;               // Nhiet do, do am do duoc
unsigned long lastRead = 0;           // Moc thoi gian doc cam bien
unsigned long lastPress = 0;          // Moc thoi gian bam nut gan nhat
bool needSave = false;                // Co offset chua luu

// Hien thi va dieu khien led
void showData() {
  char line[17];

  if (isnan(t) || isnan(h)) {         // Cam bien loi
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien!   ");
    lcd.setCursor(0, 1);
    lcd.print("Kiem tra DHT22  ");
    return;
  }

  float tc = t + tempOffset;          // Nhiet do sau khi can chinh

  digitalWrite(LED_GREEN, tc < 30.0);  
  digitalWrite(LED_RED,   tc >= 30.0);  

  snprintf(line, sizeof(line), "T:%.1f\337C %+.1f   ", tc, tempOffset);
  lcd.setCursor(0, 0);
  lcd.print(line);
  snprintf(line, sizeof(line), "H:%.1f%%         ", h);
  lcd.setCursor(0, 1);
  lcd.print(line);
}

//
void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  dht.begin();
  Serial.begin(115200);
  Serial.println("Khoi dong...");

  prefs.begin("calib", false);
  tempOffset = prefs.getFloat("offset", 0.0);   // Doc offset da luu

  t = dht.readTemperature();
  h = dht.readHumidity();
  showData();
}

// 
void loop() {
  // 5a. Doc cam bien moi 2 giay
  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    t = dht.readTemperature();
    h = dht.readHumidity();
    showData();
  }

  // 5b. Doc 2 nut can chinh
  bool changed = false;
  if (digitalRead(BTN_UP) == LOW)   { tempOffset += 0.1; changed = true; }
  if (digitalRead(BTN_DOWN) == LOW) { tempOffset -= 0.1; changed = true; }

  if (changed) {
    tempOffset = constrain(tempOffset, -10.0, 10.0);
    tempOffset = round(tempOffset * 10) / 10.0;
    showData();
    lastPress = millis();
    needSave = true;
    delay(200);                       
  }

  // 5c. Luu Flash khi da ngung bam 1.5 giay
  if (needSave && millis() - lastPress > 1500) {
    prefs.putFloat("offset", tempOffset);
    needSave = false;
  }
}