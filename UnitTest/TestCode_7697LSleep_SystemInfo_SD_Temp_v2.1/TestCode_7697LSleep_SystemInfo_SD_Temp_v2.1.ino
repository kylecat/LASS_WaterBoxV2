/***** << WaterBox_V2.:LSleep with OLED and SD>> *****
  PCB borad：WaterBox V2.1
  功能：每次 loop後睡眠5秒，啟動時依序測試模組
*/

#include <Arduino.h>
#include <U8g2lib.h>


/***** << OLED library: u8g2 >> *****/
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


/***** << 省電用 library >> *****/
#include "LSleep.h"
LSleepClass Sleep;


/***** << SD library >> *****/
#include <SPI.h>
#include <SD.h>

File myFile;
const int chipSelect = 4;


/**** << WaterBox_V2.1:DS3231 時間模組 >> *****
    功能：時間模組+EEPROM
    library：RTClib.h
    版本：1.2.0
***********************************************************/
#include <RTClib.h>           // DS3231 library
RTC_DS3231 rtc;


/**** << WaterBox_V2.1:DS18B20 溫度模組 >> *****
    library：OneWire.h
    版本：2.3.4
***********************************************************/
#include <OneWire.h>          // DS18B20 library
#define DS18B20_Pin 7
OneWire ds(DS18B20_Pin);


/**** << WaterBox_V2.1: pin state test with ADS1115 >> *****
     ADC 數據及 switch 切換
     使用Library Adafruit_ADS1X15 (Ver. 1.0.1)
***********************************************************/
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */


/***** << firmware Version >> *****/
String _fw = "Ver 2.1.a";
String Description_Tittle = ">> LASS IS YOURS <<";
String Description_Firware = "The DEVICE FIRWARE: " + String(_fw);

/***** << 系統參數 >> *****/
#define sensorSwitch 6
#define pinLED 7
#define modulePower 14

int _c = 0;
double EC, Tubidity;


/***** << SD function >> *****/
bool SD_checkDir(String _dirName)
{
  bool _stateCheck = SD.exists(_dirName);
  if (_stateCheck)
  {
    Serial.println("Directory <" + _dirName + "> exists");
  }
  else {
    Serial.println("Directory <" + _dirName + "> did not exist");
    _stateCheck = SD.mkdir(_dirName);
    Serial.print("Make a new directory:");
    if (_stateCheck) Serial.println("success");
    else             Serial.println("failed");
  }
  return _stateCheck;
}

/***** << OLED function >> *****/
void OLED_msg(String _verMsg, float _delay = 0.5)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 一個字母8 Pixel 寬，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // 16-10 = 6, 6*8=48 -> 24 開始

    u8g2.setFont(u8g2_font_timR08_tr);                 // 一個字母8 Pixel 寬，行高12
    u8g2.drawStr(80, 20, _fw.c_str());                 // 16-13 = 3, 3*8=24 -> 12 開始

    u8g2.setFont(u8g2_font_helvR14_te);                // 一個字母8 Pixel 寬，行高12
    u8g2.drawStr(0, 40, _verMsg.c_str());              // 16-13 = 3, 3*8=24 -> 12 開始
  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_content(String _msg1, String _msg2 = "", float _delay = 0.5)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 字母寬10Pixel，高13Pixel，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // LASS logo

    u8g2.setFont(u8g2_font_timR08_tr);                  // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(80, 20, _fw.c_str());                  // 韌體版本(靠右)

    u8g2.setFont(u8g2_font_helvR14_te);                 // 18x23 字母寬18Pixel，高23Pixel，行高20
    u8g2.drawStr(0, 40, _msg1.c_str());                 // 訊息A
    u8g2.drawStr(0, 60, _msg2.c_str());                 // 訊息B
  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_content_title(String _title, String _msg1, String _msg2 = "", float _delay = 0.5)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 字母寬10Pixel，高13Pixel，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // LASS logo

    u8g2.setFont(u8g2_font_timR08_tr);                  // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(0, 20, _title.c_str());                // 左邊的小title
    u8g2.drawStr(80, 20, _fw.c_str());                  // 韌體版本(靠右)

    u8g2.setFont(u8g2_font_helvR14_te);                 // 18x23 字母寬18Pixel，高23Pixel，行高20
    u8g2.drawStr(0, 40, _msg1.c_str());                 // 訊息A
    u8g2.drawStr(0, 60, _msg2.c_str());                 // 訊息B
  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_smallContent(String _msg1, String _msg2 = "", String _msg3 = "", float _delay = 0.5)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 字母寬10Pixel，高13Pixel，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // LASS logo

    u8g2.setFont(u8g2_font_timR08_tr);                  // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(80, 20, _fw.c_str());                  // 韌體版本(靠右)

    u8g2.setFont(u8g2_font_helvR12_te);                 // 15x20 字母寬15Pixel，高20Pixel，行高15
    u8g2.drawStr(0, 34, _msg1.c_str());                 // 訊息A
    u8g2.drawStr(0, 49, _msg2.c_str());                 // 訊息B

    u8g2.setFont(u8g2_font_8x13O_tr);                   // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(0, 62, _msg3.c_str());                 // 訊息C 最底層小字
  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  u8g2.setPowerSave(1); // 關閉螢幕
}


/***** << ADS1115 function >> *****/
void initADC(void)
{
  //                                                             ADS1015  ADS1115
  //                                                             -------  -------
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();
}

int _getVoltage(int _ch)
{
//  initADC();
  delay(1000);
  int adc = ads.readADC_SingleEnded(_ch);
  int mV = adc * 0.125;
  return mV;
}


/***** << GPIO function >> *****/
void SystemTest_GPIO(int _loop)
{
  Serial.print("LOOP:\t" + String(_loop));
  Serial.print("\tOn");
  delay(1000);

  digitalWrite(sensorSwitch, LOW);
  Serial.print("\tEC");
  delay(1000);
  digitalWrite(sensorSwitch, HIGH);
  Serial.print("\tpH");
  delay(1000);
  Serial.println("\tOff");
}

void SystemTest_SD(int _loop)
{
  SD.begin(chipSelect);
  Serial.println("建立資料夾");
  String _dirName = "Dir-" + String(_loop);
  SD_checkDir(_dirName);
  Serial.print("確認資料夾(" + _dirName + "): ");
  Serial.println(SD.exists(_dirName));
  delay(5000);
}

void SystemTest_Sleep(int _ms, int _mode)
{
  delay(500);
  Sleep.init(false);
  Sleep.setTime(_ms);       // 設定時間
  Sleep.setMode(_mode);     // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  Serial.println("晚安");
  Sleep.sleep();
  Serial.println("起床");
}

/***** << DS18B20 function >> *****/
float getTemperture(void)
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if ( !ds.search(addr)) {
    Serial.println("[DS18B20] No more addresses.");
    ds.reset_search();
    delay(250);
    return 0;
  }

  switch (addr[0]) {
    case 0x10:
      //      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("[DS18B20] Device is not a DS18x20 family device.");
      return 0;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

  Serial.print("[DS18B20] Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  ds.depower();

  return celsius;
}


/***** << DFRobot function >> *****/
double getTubidity(int _mV)
{
  double voltage = _mV * 0.001;
  //  double tubidity = -1120.4 * voltage * voltage + 5742.3 * voltage - 4352.9;    // 官方公式
  double tubidity = 1071.2 * voltage * voltage - 6418.5 * voltage + 9952.1;     // 實驗結果 Tubidity<800 NTU

  return tubidity;
}

double Tubidity_value()
{
  int _ads_Volate = _getVoltage(3);
  return getTubidity(_ads_Volate);
}


/***** << Main function: Setup >> *****/
void setup(void)
{
  Serial.begin(9600);
  Serial.println("System Start");

  pinMode(modulePower, OUTPUT);     //
  digitalWrite(modulePower, HIGH);  // 開啟模組電源
  pinMode(sensorSwitch, OUTPUT);    // 設定EC/pH切換控制IO

  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  Serial.print("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  OLED_content("OLED", "Init", 1);
  
  initADC();
  OLED_content("ADS1115", "Init", 1);

  pinMode(pinLED, OUTPUT);
  Serial.println(F("倒數三秒"));
  for (int _i = 0; _i < 3; _i++)
  {
    Serial.print(3 - _i);
    digitalWrite(pinLED, HIGH);
    delay(500);
    digitalWrite(pinLED, LOW);
    delay(500);
  }
  Serial.println();

  // Sleep功能初始化設定
  Serial.println("休眠測試");
  SystemTest_Sleep(1000 , 3);
  digitalWrite(pinLED, HIGH);
  Serial.println("System Init Done");

}


/***** << Main function: Setup >> *****/
void loop(void) {
  digitalWrite(modulePower, HIGH); // 開幾電源

  OLED_content("Hello", "LASS", 1);
  SystemTest_GPIO(_c);
  SystemTest_SD(_c);

  //  float Temp = getTemperture();
  //  if(Temp==0) Temp = getTemperture();

  digitalWrite(sensorSwitch, LOW);
  EC = _getVoltage(2);
  Serial.println("EC: " + String(EC));
  
  digitalWrite(sensorSwitch, HIGH);
  Tubidity = Tubidity_value();
  Serial.println("Tubidity: " + String(Tubidity));
  delay(1000);

  digitalWrite(modulePower, LOW);  // 關閉電源
  delay(500);

  Serial.print("睡覺Loop:(");
  Serial.print(_c);
  Serial.println(")(10 sec)");

  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);
  SystemTest_Sleep(10000, 3);
  digitalWrite(pinLED, HIGH);

  _c++;
}
