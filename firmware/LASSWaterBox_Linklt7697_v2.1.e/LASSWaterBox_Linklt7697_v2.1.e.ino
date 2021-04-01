/***** << WaterBox_V2.:LSleep with OLED and SD>> *****
  PCB borad：WaterBox V2.1
  功能：基本系統功能測試
  測試項目：
    1.OLED
    2.EEPROM
    3.LSleep
    4.SD ：使用前需留意LoRa的CS(P10)是否已經上拉，不然會抓到LoRa模組
    5.Temp
    6.pH EC
******************************************************/
/***** << WaterBox_V2.1:LinkIt 7697 EEPROM library >> *****/
#include <EEPROM.h>           // EEPROM library


/**** << WaterBox_V2.1:LinkIt 7697 Wifi library >> *****/
#include <LWiFi.h>

char ssid[] = "SmartCampus";      //  your network SSID (name)
char pass[] = "smartcampus206";  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
long wifi_rssi;
IPAddress wifi_ip;
String resultStrig;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(117,185,24,248);
char server[] = "download.labs.mediatek.com";   // http://download.labs.mediatek.com/linkit_7697_ascii.txt


// This is the root certificate for our host.
// Different host server may have different root CA.
static const char rootCA[] = "-----BEGIN CERTIFICATE-----\r\n"
                             "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n"
                             "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n"
                             "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n"
                             "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n"
                             "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n"
                             "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n"
                             "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n"
                             "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n"
                             "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n"
                             "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n"
                             "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n"
                             "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n"
                             "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n"
                             "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n"
                             "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n"
                             "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n"
                             "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n"
                             "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n"
                             "-----END CERTIFICATE-----\r\n";

// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
TLSClient client;    // HTTPS 用 Clinet
WiFiClient mqtt_client; // HTTP 用 Client


/**** << WaterBox_V2.1:PubSubClient >> *****
    功能：MQTT收發資料
    library：PubSubClient.h
    版本：ˊ2.8.0
**********************************************/
#include <PubSubClient.h>  // MQTT library
PubSubClient mqtt(mqtt_client);
#include <String.h>
#define SERVER_IP "0.0.0.0" #MQTT broker
#define SERVER_PORT 1883
#define USERNAME ""
#define PASSWORD ""
#define SUBTOPIC "NTU/TEST/Waterbox/#"

/***** << OLED library: u8g2 >> *****/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


/***** << 省電用 library: 未使用 >> *****/
#include "LSleep.h"
LSleepClass Sleep;
bool enableSleep = true;
long SleepSec = 10;


/***** << SD library >> *****/
#include <SPI.h>
#include <SD.h>
File myFile;
const int chipSelect = 4;


/**** << WaterBox_V2.1:DS3231 時間模組 >> *****
    功能：時間模組+EEPROM
    library：RTClib.h
    版本：1.2.0
**********************************************/
#include <RTClib.h>           // DS3231 library
RTC_DS3231 rtc;


/**** << WaterBox_V2.1:DS18B20 溫度模組 >> *****
    library：DS18B20.h
    版本：1.0.0
***********************************************************/
#define DS18B20_Pin 7
#include <DS18B20.h>
uint8_t address[] = {40, 250, 31, 218, 4, 0, 0, 52};
uint8_t selected;


/**** << WaterBox_V2.1:pin state test with ADS1115 >> *****
   ADC 數據及 switch切換
   使用Library Adafruit_ADS1X15 (Ver. 1.0.1)
***********************************************************/
#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;       /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
int16_t adc0, adc1, adc2, adc3;

#define pH_pin          0
#define EC_pin          1
#define Rotary_Pin_1    2  // VR1
#define Rotary_Pin_2    3  // VR2


/***** << GPIO設定 >> *****/
#define sensorSwitch 6
#define modeSwitch   5
#define pinLED       7
#define modulePower 14


/**** << WaterBox_V2.1:ArduinoJson >> *****
    功能：字串打包用
    library：ArduinoJson.h
    版本：ˊ6.15.2
**********************************************/
#include <ArduinoJson.h>    // DS3231 library
String JsonData;


/***** << firmware Version >> *****/
String _fw = "Ver 2.1.e";
String Description_Tittle = ">> LASS IS YOURS <<";

String Description_Firware = "The DEVICE FIRWARE: " + String(_fw);
String Description_Features = "THIS VERSION HAS THE　FOLLOWING FEATURES:\n\r"\
                              "\t 1.Get the EC and pH value via ADS1115 module\n\r"\
                              "\t 2.Get temptrue value via DS18B20 module\n\r"\
                              "\t 3.Save DATA with CSV format by Day\n\r"\
                              "\t 4.Upload Data to ThingSpeak cloud platform via WiFi\n\r";
String Description_Precautions = "<< PRECAUTIONS >>\n\r"\
                                 "\t 1.All module power controled by P14\n\r"\
                                 "\t 2.LinkIt 7697 was not into deep sleep mode\n\r"\
                                 "\t 3.LoRa module did not used in this version\n\r";


/***** << 系統參數: 系統設定相關 >> *****/
float pH_slop, pH_intercept, EC_slop, EC_intercept, pH_alarm, EC_alarm;
int print_interval = 1000;   // 系統設定時，把設定值顯示在序列埠用的時間間隔(1000 ms)

int chapter = 0;                               //  大項目：0~1(校正設定,時間設定)
int item = 0;                                  //  子項目：0~3(準備,參數1設定,參數2設定,參數3設定)
bool config_state = true;                      //  true時只能顯示目前設定，flase時可以改設定
bool _modeStatus = false;


/***** << 系統參數: 時間相關>> *****/
int _YY, _year_1, _year_2, _MM, _DD, _HH, _mm; //  時間日期調整用
int _year, _month, _day, _hour, _minute, _second;       //  系統時間用
unsigned long Tick = 0;


/***** << 系統參數: 感測相關>> *****/
int _c = 0;
float Temp_value, pH_value, EC_value, Tubidity_value;   // 感測器讀值
float SensorValue[7] = {0};                             // 感測數據彙整存放（LASS訊息區）

String CSV_Header;   // 寫入CSV時的表頭
String CSV_fileName; // 檔案名稱(需少於8個字元)
String CSV_Data;     // CSV資料
String str_Time;     // 資料寫入時的時間  "YYYY-MM-DD mm:hh"

unsigned  long _SD_tag, _upload_tag, _delay_tag;  // 用來記錄要不要寫入SD卡/上傳雲端的時間戳
String alarmStr;                                  // 用來記錄要不要顯示alarm
String oledString;


/***** << OLED function >> *****/
void OLED_msg(String _verMsg, float _delay = 0.5, bool _savePower = true)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  if (_savePower) u8g2.setPowerSave(0); // 打開螢幕

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
  if (_savePower) u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_content(String _msg1, String _msg2 = "", float _delay = 0.5, bool _savePower = true)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  if (_savePower) u8g2.setPowerSave(0); // 打開螢幕

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
  if (_savePower) u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_content_title(String _title, String _msg1, String _msg2 = "", String _msg3 = "", float _delay = 0.5, bool _savePower = true)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  if (_savePower) u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 字母寬10Pixel，高13Pixel，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // LASS logo

    u8g2.setFont(u8g2_font_timR08_tr);                  // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(0, 20, _title.c_str());                // 左邊的小title
    u8g2.drawStr(80, 20, _fw.c_str());                  // 韌體版本(靠右)

    u8g2.setFont(u8g2_font_helvR12_te);                 // 18x23 字母寬18Pixel，高23Pixel，行高20
    u8g2.drawStr(0, 34, _msg1.c_str());                 // 訊息A
    u8g2.drawStr(0, 49, _msg2.c_str());                 // 訊息B

    u8g2.setFont(u8g2_font_6x13O_tf );                  // 5x8 字母寬5Pixel，高6Pixel，行高10
    u8g2.drawStr(0, 63, _msg3.c_str());                 // 訊息C 最底層小字

  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  if (_savePower) u8g2.setPowerSave(1); // 關閉螢幕
}

void OLED_smallContent(String _msg1, String _msg2 = "", String _msg3 = "", float _delay = 0.5, bool _savePower = true)
{
  u8g2.begin();
  u8g2.setFlipMode(0);                 // 是否翻轉螢幕
  if (_savePower)u8g2.setPowerSave(0); // 打開螢幕

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tf);                   // 字母寬10Pixel，高13Pixel，行高12
    u8g2.drawStr(24, 12, ">> LASS <<");                 // LASS logo

    u8g2.setFont(u8g2_font_timR08_tr);                  // 5x8 字母寬5Pixel，高8Pixel，行高10
    u8g2.drawStr(80, 20, _fw.c_str());                  // 韌體版本(靠右)

    u8g2.setFont(u8g2_font_helvR12_te);                 // 15x20 字母寬15Pixel，高20Pixel，行高15
    u8g2.drawStr(0, 34, _msg1.c_str());                 // 訊息A
    u8g2.drawStr(0, 49, _msg2.c_str());                 // 訊息B

    u8g2.setFont(u8g2_font_6x13O_tf );                  // 5x8 字母寬5Pixel，高6Pixel，行高10
    u8g2.drawStr(0, 62, _msg3.c_str());                 // 訊息C 最底層小字
  } while ( u8g2.nextPage() );

  delay(_delay * 1000);
  if (_savePower)u8g2.setPowerSave(1); // 關閉螢幕
}


/***** << SD function >> *****/
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

bool SD_SaveCSV(String _dirName, String _fileName, String _data)
{
  bool _stateCheck = SD_checkDir(_dirName);
  String fileLocation = _dirName + "/" + _fileName + ".CSV";

  if (_stateCheck)  {
    _stateCheck = SD_WriteData(fileLocation, _data);
  }
  else            {
    Serial.println("[SD ] Directory check failed");
    oledString = "No Dir";
  }

  return _stateCheck;
}

bool SD_WriteData(String _fileName, String _data)
{
  bool _stateCheck;
  bool _hasFile = SD.exists(_fileName);

  myFile = SD.open(_fileName, FILE_WRITE);
  if (myFile)
  {
    Serial.print("[SD ] Saving Data: ");

    if (!_hasFile) myFile.println(CSV_Header);

    myFile.println(_data);
    myFile.close();

    Serial.println("Done");
    oledString = "Done";
    _stateCheck = true;
  }
  else
  {
    myFile.close();
    Serial.println("[SD ] error opening " + _fileName);
    oledString = "File Error";
    _stateCheck = false;
  }
  Serial.println();

  return _stateCheck;
}

void SavingData(String _fileName, String _data)
{
  SD.begin(chipSelect);
  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  if (_fileName.length() > 8) {
    Serial.println("[SD ] 檔案名稱過長: " + _fileName);
    _fileName = _fileName.substring(0, 8);
  }
  SD.begin(chipSelect);
  SD_SaveCSV("DATA", _fileName, _data);  // 寫入資料
  delay(50);
}


/***** << RTC function >> *****/
bool initRTC(bool _setTime = false)
{
  bool _state = true;
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    _state = false;
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  if (_setTime) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  return _state;
}


/***** << GPIO function >> *****
   計算某pin上拉一次花多少秒(0.1)
   並設定上限秒數超過自動跳出回傳
********************************/
float pull_time(uint8_t pin, int _limit)
{
  bool _virutal_button = false;             // 紀錄按鈕狀態用
  int _while_i = 0;                         // 中斷while用
  unsigned long _push_time = 0;             // 按下時的時間(暫存)
  float _duration_time = 0;                 // 總經歷秒數
  bool _btn_state = digitalRead(pin);       // 確認按鈕有沒有被按下

  for (int _i = 0 ; _i < 2; _i++) {         //  跑兩遍，第一遍用來確認按鈕狀態，第二遍用來結算時間
    if (_btn_state) {                       // 按鈕被按下的話，確認虛擬按鈕的狀態
      _virutal_button = true;               // 按下虛擬按鈕
      _push_time = millis();                // 紀錄按下開始時間(millisecond)

      while (_btn_state) {                                                 // 當按鈕被按下時進入while迴圈，超過_limit秒數(*10ms)後自動中斷
        if (_while_i < _limit * 100)  _btn_state = digitalRead(pin);        // 用while停住程式，並持續更新按鈕狀態
        else {
          _btn_state = false;                                              // 跳出前先把按鈕狀態關掉，避免再次進入while
        }
        delay(10);
        _while_i++;
      } // end of while (按鈕確認)

    }
    else {                           // 按鈕彈起時，結算按住時間
      if (_virutal_button) {         // 如果是按鈕彈起後還沒更新虛擬按鈕，結算時間
        _duration_time = (millis() - _push_time) * 0.001;
      }
    }
  }
  return _duration_time;
}


/***** << ADS1115 function >> *****/
void initADC(void)
{
  ads.begin();
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
}

float _getVoltage(int _ch)
{
  delay(1000);
  int adc = ads.readADC_SingleEnded(_ch);
  int mV = adc * 0.125;
  return mV;
}

float _analog_convert(int _r_val, int _C_val)
{
  if (_r_val < 0) _r_val = 0;
  float _value = _C_val * (_r_val * 0.125) / 3335.0;
  return _value;
}


/***** << fromate function >> *****
   把int轉成2位數String，自動補0
***********************************/
String convert_2digits(int i)
{
  String number;
  if (i < 10) number = "0" + (String)i;
  else number = (String)i;
  return number;
}


/***** << DFRobot function >> *****
   DFRobot 模組用
   getPH  // 讀取pH
   getEC  // 讀取EC
***********************************/
float getPH(float slope = 1.0, float intercept = 0.0)
{

  // 因為板子的電路會進行分壓，所以ADS1115量出來的電壓要再乘上2才會是電錶的電壓
  float _ads_Volate = 2 * _getVoltage(pH_pin) * slope + intercept;
  float pH_Value = 3.5 * _ads_Volate * 0.001;                                   // 分壓(5V->2.5V)後，用電壓換算成pH (斜率3.5的出處待確認)
  return pH_Value;
}

float getECmV(float _mV)
{
  // 溫度補償係數
  float _value;

  // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  // 先用22度做溫度補償
  float _temp_coefficient = 1.0 + 0.0185 * (22.0 - 25.0);
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

float getEC(float slope = 1.0, float intercept = 0.0)
{
  // 因為板子的電路會進行分壓，所以ADS1115量出來的電壓要再乘上2才會是電錶的電壓
  float _ads_Volate = 2 * _getVoltage(EC_pin) * slope + intercept;
  return getECmV(_ads_Volate);
}

float getTemp(void)
{
  float _result = 0;
  DS18B20 ds(DS18B20_Pin);

  selected = ds.select(address);

  if (selected) {
    _result = ds.getTempC();
  } else {
    Serial.println("[Temp] Device not found!");
    oledString = "Temp Error";
  }

  return _result;
}


/***** << LinkIt 7697 wifi function >> *****
   LinkIt 7697 WIFI 連線用
***********************************/
int connectWifi(bool _debug = false)
{
  status = WiFi.status();  // 更新wifi目前的狀況

  for (int _c = 0; _c < 5; _c++) {
    if (status != WL_CONNECTED)
    {
      if (_debug) Serial.print("[Wifi State] Attempting to connect to SSID: ");
      if (_debug) Serial.println(ssid);
      if (_debug) Serial.println(); // 多空一行
      status = WiFi.begin(ssid, pass);
    }
  }

  switch (status) {
    case WL_CONNECTED:
      if (_debug) Serial.println("[Wifi State] connected to a WiFi network");
      oledString = "Connected";
      break;
    case WL_NO_SHIELD:
      if (_debug) Serial.println("[Wifi State] no WiFi shield is present");
      break;
    case WL_IDLE_STATUS:
      if (_debug) Serial.println("[Wifi State] waiting to update WiFi conncet state");
      oledString = "Waiting";
      break;
    case WL_NO_SSID_AVAIL:
      if (_debug) Serial.println("[Wifi State] no SSID are available");
      oledString = "SSID ERROR";
      break;
    case WL_SCAN_COMPLETED:
      if (_debug) Serial.println("[Wifi State] the scan networks is completed");
      break;
    case WL_CONNECT_FAILED:
      if (_debug) Serial.println("[Wifi State] the connection fails for all the attempts");
      break;
    case WL_CONNECTION_LOST:
      if (_debug) Serial.println("[Wifi State] the connection is lost");
      oledString = "Connect Lost";
      break;
    case WL_DISCONNECTED:
      if (_debug) Serial.println("[Wifi State] disconnected from a network");
      break;
    default:
      break;
  }

  return WiFi.status();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("[WIFI] SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  wifi_ip = WiFi.localIP();
  Serial.print("[WIFI] IP Address: ");
  Serial.println(wifi_ip);
  oledString = String((byte)wifi_ip[0]) + "." + String((byte)wifi_ip[1]) + "." + String((byte)wifi_ip[2]) + "." + String((byte)wifi_ip[3]);

  // print the received signal strength:
  wifi_rssi = WiFi.RSSI();
  Serial.print("[WIFI] signal strength (RSSI):");
  Serial.print(wifi_rssi);
  Serial.println(" dBm");
}

String WifiMac(bool _hasDash = false, bool _fullMac = false)
{
  String _dash = "";
  if (_hasDash) _dash = "-";

  byte mac[6];
  WiFi.macAddress(mac);

  String _macAddress;

  int _stop = 2;
  if (_fullMac) _stop = -1;

  for (int _i = 5 ; _i > _stop; _i--)
  {
    if (_i == _stop + 1) _dash = "";
    _macAddress = _macAddress + String(mac[_i], HEX) + _dash;
  }

  _macAddress.toUpperCase(); _macAddress.toUpperCase();
//  Serial.println("MAC Address: " + _macAddress); // 檢查用

  return _macAddress;
}


/***** << LinkIt 7697 upload function >> *****
   LinkIt 7697 WIFI 上傳資料用
***********************************/
bool getLinkItLogo( bool _readResponse)
{
  client.setRootCA(rootCA, sizeof(rootCA));
  if (client.connect(server, 443)) {
    Serial.println("[WIFI] connected to LinkIt Server (GET)");
    // Make a HTTP request:
    client.println("GET /linkit_7697_ascii.txt HTTP/1.0");
    client.println("Host: download.labs.mediatek.com");
    client.println("Accept: */*");
    client.println("Connection: close");
    client.println();
    delay(500);
  }
  else Serial.println();

  if (_readResponse)
  {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }
  Serial.println("\r\n");
  client.stop();
}

bool updateThingSpeak(String _api, String _field1Value, String _field2Value, String _field3Value)
{
  String server = "api.thingspeak.com";
  String _field1 = "&field1=" + _field1Value;
  String _field2 = "&field2=" + _field2Value;
  String _field3 = "&field3=" + _field3Value;
  String getStr = "GET /update?api_key=" + _api + _field1 + _field2 + _field3;

  bool _uploadStatus = false;

  client.setRootCA(rootCA, sizeof(rootCA));

  if (client.connect(server.c_str(), 443))
  {
    Serial.println("[UPLOAD] Update to ThingSpeak (GET)");
    // Make a HTTP request:
    client.println(getStr);
    client.println("Host: " + server);
    client.println("Accept: */*");
    client.println("Connection: close");
    client.println();
    delay(500);
    _uploadStatus = true;
    resultStrig = "";
  }
  else {
    Serial.print("[UPLOAD] Update to ThingSpeak: ");
    //    Serial.print(server);
    Serial.print(getStr);
    Serial.println(" ERROR");
  }


  while (client.available()) {
    char _c = client.read();
    Serial.write(_c);
    resultStrig += (String)_c;
  }
  Serial.println("\r\n");


  if (_uploadStatus) oledString = resultStrig;
  else               oledString = "Error";


  client.stop(); // 中斷連線
  return _uploadStatus;
}

String addJSON_msg(float _value[], bool _debug = false)
{
  String _strBuffer;

  StaticJsonDocument<300> DocJSON;

  DateTime now = rtc.now();          // 取得目前時間
  _year = now.year();
  _month = now.month();
  _day = now.day();
  _hour = now.hour();
  _minute = now.minute();
  _second = now.second();

  DocJSON["Device_ID"] = WifiMac();
  DocJSON["Device"] = "WaterBox";
  DocJSON["Firmware_Ver"] = _fw;
  DocJSON["MCU"] = "LinkIt7697";

  DocJSON["Date"] = String(_year) + "-" + convert_2digits(_month) + "-" + convert_2digits(_day);
  DocJSON["Time"] = convert_2digits(_hour) + ":" + convert_2digits(_minute) + ":" + convert_2digits(_second) ;

  DocJSON["s_t0"] = String(_value[0]);
  DocJSON["s_ph"] = String(_value[1]);
  DocJSON["s_ec"] = String(_value[2]);
  DocJSON["s_Tb"] = String(_value[3]);
  DocJSON["s_Lv"] = String(_value[4]);
  DocJSON["s_DO"] = String(_value[5]);
  DocJSON["s_orp"] = String(_value[6]);

  serializeJson(DocJSON, _strBuffer);

  if (_debug) {
    Serial.println("[SYSTEM] JSON Data:");
    serializeJsonPretty(DocJSON, Serial);
    Serial.println();
  }

  return _strBuffer;
}


/***** << LinkIt 7697 LASS upload function >> *****
   LinkIt 7697 WIFI 上傳LASS資料用
***********************************/
String addLASS_msgTime()
{
  DateTime now = rtc.now();          // 取得目前時間
  _year = now.year();
  _month = now.month();
  _day = now.day();
  _hour = now.hour();
  _minute = now.minute();
  _second = now.second();
  Tick = now.unixtime() - Tick;

  String _strBuffer = "date=" + String(_year) + "-" + convert_2digits(_month) + "-" + convert_2digits(_day) + "|time=" + convert_2digits(_hour) + ":" + convert_2digits(_minute) + ":" + convert_2digits(_second) + "|tick=" + String(Tick);
  return _strBuffer;
}

String addLASS_msgValue(float _value[], bool _debug = false)
{
  if (_debug)
  {
    Serial.print("     Temp: "); Serial.println(_value[0]);
    Serial.print("       pH: "); Serial.println(_value[1]);
    Serial.print("       EC: "); Serial.println(_value[2]);
    Serial.print("Turbidity: "); Serial.println(_value[3]);
    Serial.print("    Level: "); Serial.println(_value[4]);
    Serial.print("       DO: "); Serial.println(_value[5]);
    Serial.print("      ORP: "); Serial.println(_value[6]);
  }
  String _strBuffer = "s_t0=" + String(_value[0]) + "|s_ph=" + String(_value[1]) + "|s_ec=" + String(_value[2]) + "|s_Tb=" + String(_value[3]) + "|s_Lv=" + String(_value[4]) + "|s_DO =" + String(_value[5]) + "|s_orp=" + String(_value[6]);
  return _strBuffer;
}

bool updateLASS(String _msgTime, String _msgValue)
{
  //  "https://pm25.lass-net.org/Upload/waterbox_tw.php"
  //  "?topic=LASS/Test/WaterBox_TW&device_id=XXXXXXXXXXXX&key=NoKey&msg="
  //  "|device=Linkit7697|device_id=9C65F920C020|ver_app=1.1.0|app=WaterBox_TW"
  //  "|FAKE_GPS=1|gps_lat=25.1933|gps_lon = 121.787|"
  //  "date=2019-03-21|time=06:53:55|tick=714436.97"
  //  "|s_t0=20.00|s_ph=7.00|s_ec=200.0|s_Tb=500|s_Lv=|s_DO=8.0|s_orp=0.0|"

  String Host = "pm25.lass-net.org";
  String url = "https://" + Host + "/Upload/waterbox_tw.php";
  String DeviceID = "Field_D01_" + WifiMac();
  String DeviceInfo = "device=Linkit7697|device_id=" + DeviceID + "|ver_app=" + _fw + "|app=WaterBox_TW";
  String Location = "FAKE_GPS=1|gps_lat=25.029387|gps_lon=121.579060";
  String getStr = "GET " + url + "?topic=WaterBox_TW&device_id=" + DeviceID + "&key=NoKey&msg=|" + DeviceInfo + "|" + Location + "|" + _msgTime + "|" + _msgValue + "|";

  client.setRootCA(rootCA, sizeof(rootCA));

  if (client.connect(Host.c_str(), 443))
  {
    Serial.println("[UPLOAD] Update to LASS (GET)");
    Serial.println(getStr);
    // Make a HTTP request:
    client.println(getStr);
    client.println("Host : " + Host);
    client.println("Accept : */*");
    client.println("Connection: close");
    client.println();
    delay(500);
  }
  else {
    Serial.println("[UPLOAD] WIFI Server connect error");
    oledString = "Update Error";
  }

  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  if (client.available()) {
    oledString = "Update Done";
  }

  client.stop();
  Serial.println("\r\n");
}


/***** << LinkIt 7697 upload function >> *****
   LinkIt 7697 WIFI 用MQTT收發資料用
***********************************/
void reconnect(String _id)
{

  // Loop until we're reconnected
  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(_id.c_str(), USERNAME, PASSWORD))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  //  mqtt.subscribe(SUBTOPIC);  // 只要連線後就訂閱Topic
  //  mqtt.unsubscribe(SUBTOPIC);  // 取消訂閱Topic
}


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqtt_publish(String _msg)
{
  String _id = "WaterBox_" + WifiMac();
  String _topic = "NTU/TEST/Waterbox/TaichungEPB/" + _id;

  if (mqtt.connect(_id.c_str(), USERNAME, PASSWORD)) {
    reconnect(_id);
  }

  if (mqtt.connected()) {
    mqtt.publish(_topic.c_str(), _msg.c_str());
    mqtt.disconnect();
  }
  else {
    Serial.println("MQTT ERROR");
  }

}


/***** << LinkIt 7697 EEPROM function >> *****
  LinkIt 7697 內部記憶體的讀寫
  bool EEPROM_write(char* _str, int _page, int _length)
***********************************/
bool EEPROM_write(char* _str, int _page, int _length) // 寫入資料，1頁 32 bytes
{
  int _address = _page * 32;
  if (_length > 31) {                // 超出頁面
    Serial.println("[EEPROM] Out Of Pages");
    return false;
  }
  else {
    Serial.print("[EEPROM] Writing data：");
    for ( int _i = 0; _i < _length; _i++ ) {
      EEPROM.update(_address + _i, _str[_i]);
      Serial.print(_str[_i]);
    }
    Serial.println();
    return true;
  } // end of if
} // end of EEPROM_write()

String EEPROM_read(int _page, int _length) // 讀取資料，1頁 30 bytes
{
  int _address = _page * 32;
  char _str;
  String read_buffer = "";

  if (_length > 31) {                         // 超出頁面
    Serial.println("[EEPROM] Out Of Pages");
  }
  else {
    for ( int _i = 0; _i < _length; _i++ ) {
      _str = EEPROM.read(_address + _i);
      read_buffer += (String)_str;
    }
  }
  return read_buffer;
} // end of EEPROM_read()


/***** << System config function >> *****/
void _config(bool _state = true)
{
  String _buffer;
  char _char_buffer[30];

  if (_state)  Serial.print("[System] 讀取設定....");
  else         Serial.println("[System] 開始更新設定");

  for (int _i = 0; _i < 6; _i++) {

    if (_state) {
      _buffer = EEPROM_read(_i, 10);    // Linklt 7697 內部的EE{ROM)
      Serial.print(_buffer);
    }

    switch (_i) {
      case 0:
        if (_state) pH_slop = _buffer.toFloat();
        else _buffer = (String)pH_slop;
        break;
      case 1:
        if (_state) pH_intercept = _buffer.toFloat();
        else       _buffer = (String)pH_intercept;
        break;
      case 2:
        if (_state) EC_slop = _buffer.toFloat();
        else       _buffer = (String)EC_slop;
        break;
      case 3:
        if (_state) EC_intercept = _buffer.toFloat();
        else       _buffer = (String)EC_intercept;
        break;
      case 4:
        if (_state) pH_alarm = _buffer.toFloat();
        else       _buffer = (String)pH_alarm;
        break;
      case 5:
        if (_state) EC_alarm = _buffer.toFloat();
        else       _buffer = (String)EC_alarm;
        break;
    }// end of switch

    if (!_state) {
      _buffer.toCharArray(_char_buffer, _buffer.length() + 1);

      Serial.print("寫入資料");
      Serial.println(_char_buffer);
      EEPROM_write(_char_buffer, _i, _buffer.length() + 1);
      delay(50);
    }
    delay(100);
  }
  Serial.println("設定完成");
}

bool alarm_check(float pH, float EC)
{
  bool _pH_alarm = false, _EC_alarm = false;
  if (pH < 7 - pH_alarm || pH > 7 + pH_alarm)  _pH_alarm = true;
  if (EC > EC_alarm)  _EC_alarm = true;
  return _pH_alarm || _EC_alarm;
}

bool CheckTag(unsigned long * r_tag, unsigned long _sec , bool _debug = false)
{
  bool _result = false;
  DateTime now = rtc.now();      // 取得目前的時間

  if (_debug)
  {
    Serial.println("[Tag] Unixtime:" + (String)now.unixtime());
    Serial.println("[Tag] Tag time:" + (String)*r_tag);
  }

  if (now.unixtime() > *r_tag)
  {
    _result = true;                // 更新比對結果
    *r_tag = now.unixtime() + _sec; // 更新時間戳
    if (_debug) Serial.println("[Next Tag] " + (String)*r_tag + " (seconds):new");
  }

  else
  {
    _result = false;
    unsigned long _interval = *r_tag - now.unixtime();
    if (_debug) Serial.println("[Next Tag] " + (String)*r_tag + " (seconds):" + (String)_interval + " (seconds missing)");
  }

  return _result;
}

void SystemTest_SD(int _loop)
{
  SD.begin(chipSelect);
  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  Serial.println("建立資料夾");
  String _dirName = "Dir-" + String(_loop);
  SD_checkDir(_dirName);
  Serial.print("尋找資料夾(" + _dirName + "): ");
  Serial.println(SD.exists(_dirName));
  delay(5000);
}

void SystemInitSystem(bool _loadConfig = false)
{

  Serial.begin(9600);
  pinMode(modulePower, OUTPUT);     // power_pin 初始化
  pinMode(modeSwitch, INPUT);       // 模式切換用
  u8g2.begin();
  initRTC(false);
  initADC();
  delay(100);
}

void SystemTest_Sleep(int _ms, int _mode)
{
  delay(500);
  int _result = 0;
  Sleep.init(true);

  _result = Sleep.setTime(_ms);                   // 設定時間
  if (_result) _result = Sleep.setMode(_mode);    // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  else Serial.println("鬧鐘設定錯誤");

  if (_result) {
    Serial.println("晚安");
    Sleep.sleep();
    delay(500);
    Serial.println("起床");
  }
}



/***** << Main function: Setup >> *****/
void setup(void)
{
  //power_pin 初始化
  pinMode(modulePower, OUTPUT);     //
  digitalWrite(modulePower, HIGH);  // 開啟模組電源

  pinMode(modeSwitch, INPUT);       // 模式切換用

  // OLED 初始化
  u8g2.begin();
  OLED_content("System", "Initialize", 1.0, false);


  Serial.begin(9600);
  Serial.println("[System] System Start");
  OLED_content("Serial Port", "CHECKED", 1.0, false);

  initRTC(true);
  Serial.println("[System] DS3231 初始化完成");

  // ADS1115 初始化
  initADC();
  OLED_content("ADS1115", "Init", 1, false);
  Serial.println("[System] ADS1115 初始化完成");

  OLED_content("Loading", "System Config", 1.0, false);
  Serial.println("[System] 讀取系統設定");
  _config(true); // 讀取目前的設定

  Serial.println("[System] 系統設定初始化完成");
  OLED_content("System", "DONE" , 1.0, false);

  pinMode(pinLED, OUTPUT);

  Serial.print("[System] 倒數三秒");
  OLED_content("Sleep", "TEST(3)" , 1.0, true);
  for (int _i = 0; _i < 3; _i++)
  {
    Serial.print(3 - _i);
    digitalWrite(pinLED, HIGH);
    delay(500);
    digitalWrite(pinLED, LOW);
    delay(500);
  }
  Serial.println();

  digitalWrite(modulePower, LOW);  // 關閉模組電源
  // Sleep功能初始化設定
  // Serial.println("[System] 休眠測試");
  // SystemTest_Sleep(5000, 3);
  digitalWrite(pinLED, HIGH);
  Serial.println("[System] System Init Done");

  digitalWrite(modulePower, HIGH);  // 開啟模組電源
  OLED_content("Hello", "LASS", 1, false);
  Serial.println();
  Serial.println("**************************************");
  Serial.println();
  Serial.println(Description_Tittle);
  Serial.println(Description_Firware);
  Serial.println("版本功能：");
  Serial.println("\t" + Description_Features);
  Serial.println("注意事項：");
  Serial.println("\t" + Description_Precautions);
  Serial.println();
  Serial.println("**************************************");
  Serial.println();


  OLED_content("Connect", "Wifi", 1, false);
  status = connectWifi(true);
  if (status == WL_CONNECTED)
  {
    printWifiStatus();
    getLinkItLogo(true);
  }
  OLED_content("WiFi", oledString, 1, false);

  JsonData = "WaterBox_" + WifiMac() + " Online";
  mqtt.setServer(SERVER_IP, SERVER_PORT);
  mqtt.setCallback(callback);
  mqtt_publish(JsonData);

  WiFi.disconnect();

  Serial.println("[System] End of Setup");
}



/***** << Main function: Loop >> *****/
void loop()
{
  digitalWrite(modulePower, HIGH);    // 開啟模組電源
  /*****<< 取得時間資料 >>*****/
  DateTime now = rtc.now();          // 取得目前時間
  _year = now.year();
  _month = now.month();
  _day = now.day();
  _hour = now.hour();
  _minute = now.minute();

  bool _mode = digitalRead(modeSwitch);

  if (_mode)
  {
    /*****<< 進入分析運作模式 >>*****/
    if (_modeStatus != _mode) {
      Serial.println("\r\n");
      Serial.println("**********<< 進入分析模式 >>**********");
      _modeStatus = _mode;
    }

    /*****<< 更新時間資料 >>*****/
    str_Time = convert_2digits(_hour) + ":" + convert_2digits(_minute);

    bool _SD_save = CheckTag( &_SD_tag, 60, false);        //  5分        存檔一次
    bool _upload =  CheckTag( &_upload_tag, 60, false);    // 10分        上傳一次
    bool _delay = CheckTag( &_delay_tag, 150, false);      //  2分 30秒   醒來一次

    if (_delay)
    {
      pinMode(sensorSwitch, OUTPUT);        // 用USR pin 控制 pH & EC 模組電源切換
      delay(500);                           // 先等 0.5秒
      digitalWrite(sensorSwitch, HIGH);     // 切換到 pH

      Temp_value = getTemp();
      pH_value = getPH(pH_slop, pH_intercept);

      OLED_content_title(str_Time, "pH: " + String(pH_value), "Temp: " + String(Temp_value), "Analysis Mode", 1.5, false);

      digitalWrite(sensorSwitch, LOW);     // 切換到 EC
      EC_value = getEC(EC_slop, EC_intercept);
      if ( alarm_check(pH_value, EC_value))  alarmStr = "Alarm";
      else                                   alarmStr = "";

      Serial.println();
      Serial.print("***** 測值@");
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(" ");
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.println(" *****");
      Serial.println("pH\t" + String(pH_value));
      Serial.println("Temp\t" + String(Temp_value));
      Serial.println("EC\t" + String(EC_value));
      Serial.println("Alarm\t" + alarmStr);
      Serial.println("********************");
      Serial.println();

      OLED_content_title(str_Time, "EC: " + String(EC_value), alarmStr, "Analysis Mode", 1.5, true);

    }

    if (_SD_save)
    {
      OLED_content_title(str_Time, "SD", "Saving", "Analysis Mode", 1.0, false);

      str_Time = (String)_year + "-" + convert_2digits(_month) + "-" + convert_2digits(_day) + " " + str_Time;
      CSV_fileName = convert_2digits(_month) + convert_2digits(_day);
      CSV_Header   = "Date,Temp,pH,EC";
      CSV_Data = str_Time + "," + String(Temp_value) + "," + String(pH_value) + "," + String(EC_value);
      SavingData(CSV_fileName, CSV_Data);     //  寫入CSV

      OLED_content_title(str_Time, "SD", oledString, "Analysis Mode", 1.0, false);
    }

    if (_upload)
    {
      connectWifi(true);
      OLED_content_title(str_Time, "Upldate via", String(ssid), "Analysis Mode", 1.0, false);

      SensorValue[0] = Temp_value;
      SensorValue[1] = pH_value;
      SensorValue[2] = EC_value;
      SensorValue[3] = 0.0;
      SensorValue[4] = 0.0;
      SensorValue[5] = 0.0;
      SensorValue[6] = 0.0;
      SensorValue[7] = 0.0;

      // 打包上傳LASS
      // String MQTT_Time = addLASS_msgTime();
      // String MQTT_Value = addLASS_msgValue(SensorValue, false);

      // OLED_content_title(str_Time, "LASS", "Upload", "Analysis Mode", 1.0, false);
      // updateLASS(MQTT_Time, MQTT_Value);
      // OLED_content_title(str_Time, "LASS", oledString, "Analysis Mode", 1.0, false);

      // 打包成JSON格式
      JsonData = addJSON_msg(SensorValue, true);

      // 上傳MQTT
      mqtt_publish(JsonData);

      // 上傳ThingSpeak
      OLED_content_title(str_Time, "ThingSpeak", "Upload", "Analysis Mode", 1.0, false);
      updateThingSpeak("3HM7DXT2QAVJMB6C", String(Temp_value), String(pH_value), String(EC_value));
      OLED_content_title(str_Time, "ThingSpeak", oledString, "Analysis Mode", 1.0, false);
      WiFi.disconnect();
    }

    digitalWrite(modulePower, LOW);         // 關閉電源
    int _delayTime = (4 * 60 + 40) * 1000;  // 4分40秒
    delay(_delayTime);

  } // end of if (_mode)


  /*****<< 進入系統設定模式 >>*****/
  else {
    if (_modeStatus != _mode) {
      Serial.println("**********<< 進入設定模式 >>**********");
      _modeStatus = _mode;
    }

    pinMode(sensorSwitch, INPUT);                                  // 用USR pin 作為按鈕輸入
    unsigned long printTime = millis();                            // Serial印出時間計時器
    float _time = pull_time(sensorSwitch, 3);                      // 設定按鈕計時器
    String _state_str, _config_str;                                // Serial輸出用String
    String _state_OLDE_1, _state_OLDE_2, _state_str_OLED;          // OLED 顯示用String

    /*****<< 取得ADS1115狀態 >>*****/

    adc0 = ads.readADC_SingleEnded(EC_pin);
    delay(100);
    adc1 = ads.readADC_SingleEnded(pH_pin);
    delay(100);
    adc2 = ads.readADC_SingleEnded(Rotary_Pin_1);
    delay(100);
    adc3 = ads.readADC_SingleEnded(Rotary_Pin_2);
    delay(100);
    //    Serial.print("AIN0: "); Serial.println(adc0);
    //    Serial.print("AIN1: "); Serial.println(adc1);
    //    Serial.print("AIN2: "); Serial.println(adc2);
    //    Serial.print("AIN3: "); Serial.println(adc3);
    //    Serial.println();

    /***************************
       檢查按鈕按下的時間，切換模式
     ***************************/
    if (_time > 0 && _time < 1) {       // 按下時間大於<1秒時，切換子項目
      if (item == 3) item = 0;          // 到最後一項時跳回第一項
      else item += 1;                   // 跳到下一項
    }
    else if (_time > 1 && _time < 2) {  // 按下時間大於1~2秒時，切換大項目
      if (chapter == 1) chapter = 0;    // 到最後一項時跳回第一項
      else chapter += 1;                // 跳到下一項
    }
    else if (_time >= 3) {              // 按下時間大於3秒時，儲存設定資料 / 進入設定模式
      if (config_state) config_state = false;                          // 切換到設定調整狀態
      else  {                                                          // 切回設定顯示狀態
        config_state = true;
        _config(false);                                                // 儲存目前的設定
        if (_YY != 0 && _MM != 0 && _DD != 0 && _HH != 0 && _mm != 0)  rtc.adjust(DateTime(_YY, _MM, _DD, _HH, _mm, 0)); // 設定時間
      }
    }

    /***************************
       檢查是不是要在監控埠顯示訊息
     ***************************/
    if (printTime % print_interval <= print_interval * 0.05)
    {
      if (config_state) {
        Serial.println("設定模式：顯示目前設定");
      }
      else {
        Serial.println("設定模式：調整設定");
      }
    }

    /***************************
       更新要顯示的String內容
     ***************************/
    switch (item) {
      case 0:
        if (chapter) {
          _state_str = "時間設定頁面";
          _state_str_OLED = "Setting Time";
        }
        else {
          _state_str = "參數設定頁面";
          _state_str_OLED = "Setting Parameters";
        }
        break;
      case 1:
        if (chapter) {          // 年份的設定
          if (config_state) {   // 顯示年分
            _state_str = "系統年分";
            _config_str = (String) _year;
            _state_str_OLED = "This Year";
            _state_OLDE_1 = (String)_year;
          }
          else {                // 用可變電阻調整目前年分
            _state_str = "設定年分";
            int _year_1 = (int)_analog_convert(adc2, 9);         // VR1設定 年分 10位數
            int _year_2 = (int)_analog_convert(adc3, 9);         // VR2設定 年分 個位數
            _YY = 2000 + _year_1 * 10 + _year_2;
            _config_str = (String) _YY;
            _state_str_OLED = "Setting Year";
            _state_OLDE_1 = (String)_YY;
          }
        }
        else {                  // pH斜率截距的設定
          if (config_state) {   // 顯示目前設定
            _state_str = "pH 目前的斜率截距";
            _config_str = "pH slope:" + (String)pH_slop + "\t pH intercept：" + (String)pH_intercept;
            _state_str_OLED = "pH's config";
            _state_OLDE_1 = "slop: " + (String)pH_slop;
            _state_OLDE_2 = "intercept: " + (String)pH_intercept;
          }
          else {                // 用可變電阻設定參數
            _state_str = "設定pH斜率截距";
            pH_slop = 0.8 + _analog_convert(adc2, 400) * 0.001;         // VR1設定 pH 斜率
            pH_intercept = -0.5 + _analog_convert(adc3, 100) * 0.01;    // VR2設定 pH 截距
            _config_str = "pH slope:" + (String)pH_slop + "\t pH intercept：" + (String)pH_intercept;
            _state_str_OLED = "Setting pH's config";
            _state_OLDE_1 = "slop: " + (String)pH_slop;
            _state_OLDE_2 = "intercept: " + (String)pH_intercept;
          }
        }
        break;
      case 2:
        if (chapter) {          // 日期的設定
          if (config_state) {   // 顯示目前日期
            _state_str = "日期顯示";
            _config_str = convert_2digits(_month) + "/" + convert_2digits(_day);
            _state_str_OLED = "Date Now";
            _state_OLDE_1 = "Month: " + convert_2digits(_month);
            _state_OLDE_2 = "Day: " + convert_2digits(_day);
          }
          else {                // 用可變電阻調整日期
            _state_str = "日期設定(MM/DD)";
            _MM = (int)_analog_convert(adc2, 11) + 1;               // VR1設定 月份 MM
            _DD = (int)_analog_convert(adc3, 30) + 1;               // VR2設定 日期 DD
            _config_str = convert_2digits(_MM) + "/" + convert_2digits(_DD);
            _state_str_OLED = "Setting Date";
            _state_OLDE_1 = "Month: " + convert_2digits(_MM);
            _state_OLDE_2 = "Day: " + convert_2digits(_DD);
          }
        }
        else {                  // EC斜率截距的設定
          if (config_state) {   // 顯示目前設定
            _state_str = "EC目前的斜率截距";
            _config_str = "EC斜率:" + (String)EC_slop + "\t EC截距：" + (String)EC_intercept;
            _state_str_OLED = "EC's config";
            _state_OLDE_1 = "slop: " + (String)EC_slop;
            _state_OLDE_2 = "intercept: " + (String)EC_intercept;
          }
          else {                // 用可變電阻設定參數
            _state_str = "設定EC斜率截距";
            EC_slop = 0.8 + _analog_convert(adc2, 400) * 0.001; // VR1設定 EC 斜率
            EC_intercept = _analog_convert(adc3, 1000) - 500;   // VR2設定 EC 截距
            _config_str = "EC斜率:" + (String)EC_slop + "\t EC截距：" + (String)EC_intercept;
            _state_str_OLED = "Setting EC's config";
            _state_OLDE_1 = "slop: " + (String)EC_slop;
            _state_OLDE_2 = "intercept: " + (String)EC_intercept;
          }
        }
        break;
      case 3:
        if (chapter) {          // 時間的設定
          if (config_state) {   // 顯示目前時間
            _state_str = "時間顯示";
            _config_str = convert_2digits(_hour) + ":" + convert_2digits(_minute);
            _state_str_OLED = "Time Now";
            _state_OLDE_1 = "Hours Now: " + (String)_hour;
            _state_OLDE_2 = "Minutes Now: " + (String)_minute;
          }
          else {                // 用可變電阻調整目前時間
            _state_str = "時間設定(HH:MM)";
            _HH = (int)_analog_convert(adc2, 23);           // VR1設定 小時HH
            _mm = (int)_analog_convert(adc3, 59) + 1;       // VR2設定 分鐘mm
            _config_str = convert_2digits(_HH) + ":" + convert_2digits(_mm);
            _state_str_OLED = "SettingTime(HH:MM)";
            _state_OLDE_1 = "Hours: " + (String)_HH;
            _state_OLDE_2 = "Minutes: " + (String)_mm;
          }
        }
        else {                  // 偵測警報設定
          if (config_state) {
            _state_str = "上下限顯示";
            _config_str = "pH上下限:±" + (String)pH_alarm + "\t EC上限：" + (String)EC_alarm;
            _state_str_OLED = "Show the limit";
            _state_OLDE_1 = "pH's : " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's : " + (String)EC_alarm;
          }
          else {
            _state_str = "上下限設定";
            pH_alarm = _analog_convert(adc2, 20) * 0.1; // VR1設定 pH 上限
            EC_alarm = _analog_convert(adc3, 1000);     // VR2設定 EC 上限
            _config_str = "pH上下限:7.00±" + (String)pH_alarm + "\t EC上限：" + (String)EC_alarm;
            _state_str_OLED = "Setting limit";
            _state_OLDE_1 = "pH's : " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's : " + (String)EC_alarm;
          }
        }
        break;
    }

    /***************************
       每1秒顯示依次目前設定
     ***************************/
    if (printTime % print_interval <= print_interval * 0.05 ) {
      Serial.println("目前頁面:" + (String)chapter + "-" + (String)item);
      Serial.println("\t " + _state_str);
      Serial.println("\t 目前設定：" + _config_str);
      Serial.println("*******************************\n\r\n\r ");
    }
    OLED_smallContent(_state_OLDE_1, _state_OLDE_2, _state_str_OLED, 0.1, false);
  } //end of 設定模式
}
