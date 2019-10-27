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
String DeviceID = "B1";

/***** << 系統參數 >> *****/
#define sensorSwitch 6
#define pinLED 7
#define modulePower 14

int _c = 0;
double EC, Tubidity;


/***** << SD function >> *****/
bool SD_SaveCSV(String _dirName, String _fileName, String _data)
{
  bool _stateCheck = SD_checkDir(_dirName);
  String fileLocation = _dirName + "/" + _fileName + ".CSV";

  if (_stateCheck)  _stateCheck = SD_WriteData(fileLocation, _data);
  else              Serial.println("[SD ] Directory check failed");

  return _stateCheck;
}

bool SD_checkDir(String _dirName)
{
  bool _stateCheck = SD.exists(_dirName);
  if (_stateCheck)
  {
    Serial.println("[SD ] Directory <" + _dirName + "> exists");
  }
  else {
    Serial.println("[SD ] Directory <" + _dirName + "> did not exist");
    _stateCheck = SD.mkdir(_dirName);
    Serial.print("[SD ] Make a new directory:");
    if (_stateCheck) Serial.println("success");
    else             Serial.println("failed");
  }
  return _stateCheck;
}

bool SD_WriteData(String _fileName, String _data)
{
  bool _stateCheck;

  myFile = SD.open(_fileName, FILE_WRITE);
  if (myFile)
  {
    Serial.print("[SD ] Saving Data: ");

    myFile.println(_data);
    myFile.close();

    Serial.println("Done");
    _stateCheck = true;
  }
  else
  {
    myFile.close();
    Serial.println("[SD ] error opening " + _fileName);
    _stateCheck = false;
  }
  return _stateCheck;
}

void SavingSD(String _fileName, String _data)
{
  SD.begin(chipSelect);
  SD_SaveCSV("DATA", _fileName, _data);  // 寫入資料
  delay(500);
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
    u8g2.drawStr(24, 12, " >> LASS << ");                 // 16-10 = 6, 6*8=48 -> 24 開始

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
    u8g2.drawStr(24, 12, ">> LASS << ");                 // LASS logo

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
    u8g2.drawStr(24, 12, ">> LASS << ");                 // LASS logo

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
    u8g2.drawStr(24, 12, ">> LASS << ");                 // LASS logo

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
  Serial.print("LOOP: \t" + String(_loop));
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
  //  Serial.println("建立資料夾");
  String _dirName = "Dir-" + String(_loop);
  String _fileName = "D-" + String(_loop);

  String _data = String(_loop) + ",test";
  SD_SaveCSV(_dirName, _fileName, _data);  // 寫入資料
  //  SD_checkDir(_dirName);               // 確認資料夾

  //  Serial.print("確認資料夾(" + _dirName + "): ");
  //  Serial.println(SD.exists(_dirName));
  delay(2000);
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

double TubidityValue()
{
  delay(1000);
  int _ads_Volate = _getVoltage(0);
  return getTubidity(_ads_Volate);
}

double getEC(int _mV)
{
  // 溫度補償係數
  float _value;
  float _temp_coefficient = 1.0 + 0.0185 * (22.0 - 25.0);      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  float _coefficient_volatge = _mV / _temp_coefficient;  // 電壓係數

  // 三個不同區間的導電度換算
  if (_coefficient_volatge < 150)
  {
    _value = 6.84 * _coefficient_volatge - 200; // EC <= 1ms/cm (暫定)

    Serial.print("[EC ]No solution!(");
    Serial.print(_value);
    Serial.println(")");

    if (_value < 0) _value = 0;
  }
  else if (_coefficient_volatge > 3300)
  {
    _value = 5.3 * _coefficient_volatge + 2278; // 20ms/cm<EC (暫定)
    Serial.print("[EC ]Out of the range!(");
    Serial.print(_value);
    Serial.println(")");
  }
  else
  {
    if (_coefficient_volatge <= 448)        _value = 6.84 * _coefficient_volatge - 64.32; // 1ms/cm<EC<=3ms/cm
    else if (_coefficient_volatge <= 1457)  _value = 6.98 * _coefficient_volatge - 127;   // 3ms/cm<EC<=10ms/cm
    else                                    _value = 5.3 * _coefficient_volatge + 2278;   // 10ms/cm<EC<20ms/cm
  }

  return _value;
}

double ECValue()
{
  delay(1000);
  int _ads_Volate = _getVoltage(1);
  return getEC(_ads_Volate);
}


/**** << WaterBox_V2.1:數值處理用funtion >> *****
   把int轉成2位數String，自動補0
********************************************************/
String convert_2digits(int i)
{
  String number;
  if (i < 10) number = "0" + (String)i;
  else number = (String)i;
  return number;
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
    Serial.println("initialization failed. Things to check: ");
    Serial.println("* is a card inserted ? ");
    Serial.println("* is your wiring correct ? ");
    Serial.println("* did you change the chipSelect pin to match your shield or module ? ");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  OLED_content("OLED", "Init", 1);

  initADC();
  OLED_content("ADS1115", "Init", 1);


  // RTC 初始化
  if (rtc.lostPower()) {
    Serial.println("[RTC ]RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  OLED_content("RTC", "Init", 1);

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

  OLED_content("Deep Sleep", "Test", 1);
  Serial.println("休眠測試");
  SystemTest_Sleep(1000 , 3);
  digitalWrite(pinLED, HIGH);

  OLED_content("System", "Ready", 1);
  Serial.println("System Init Done");

}


/***** << Main function: Setup >> *****/
void loop(void) {
  digitalWrite(modulePower, HIGH); // 開幾電源

  DateTime now = rtc.now();          // 取得目前時間
  int _year = now.year();
  int _month = now.month();
  int _day = now.day();
  int _hour = now.hour();
  int _minute = now.minute();
  String _Date = _year + convert_2digits(_month) + convert_2digits(_day);
  String _Time = convert_2digits(_hour) + ":" + convert_2digits(_minute);

  //  OLED_content("Hello", "LASS", 1);
  //  SystemTest_GPIO(_c);
  //  SystemTest_SD(_c);

  OLED_content_title(_Time, _Date, String(_c), 2);

  //  float Temp = getTemperture();
  //  if(Temp==0) Temp = getTemperture();

  digitalWrite(sensorSwitch, LOW);

  EC = ECValue();
  Serial.println("EC : " + String(EC));

  digitalWrite(sensorSwitch, HIGH);
  Tubidity = TubidityValue();
  Serial.println("Tubidity : " + String(Tubidity));
  OLED_content_title(_Time, "EC:" + String(EC), "Tub:" + String(Tubidity), 2);

  String _Data = DeviceID + "," + _Date + "," + _Time + "," + String(EC) + "," + String(Tubidity); // 製作要儲存的資料String
  _Date = convert_2digits(_month) + convert_2digits(_day);                            // 把日期更新成MMDD
  SavingSD(DeviceID + "_" + _Date, _Data);
  OLED_content_title(_Time, DeviceID, "Data Saved", 2);


  OLED_content("Prepare", "Deep Sleep", 0.5);
  Serial.print("睡覺Loop : (");
  Serial.print(_c);
  Serial.println(")(300 sec)");

  digitalWrite(modulePower, LOW);  // 關閉電源
  delay(500);

  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);
  SystemTest_Sleep(300000, 3);
  digitalWrite(pinLED, HIGH);

  _c++;
}
