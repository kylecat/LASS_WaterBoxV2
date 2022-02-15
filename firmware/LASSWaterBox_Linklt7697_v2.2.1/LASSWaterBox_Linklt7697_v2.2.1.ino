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

const char* ssid = "";     //  your network SSID (name)
const char* pass = "";     // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                  // your network key Indefx number (needed only for WEP)

int status = WL_IDLE_STATUS;

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

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
TLSClient sslclient;
WiFiClient httpclient;

/**** << WaterBox_V2.1:LinkIt 7697 WatchDog library >> *****/
#include <LWatchDog.h>

/**** << WaterBox_V2.1 HY:MQTT library PubSubClient >> *****
    功能：MQTT Client
    library：PubSubClient.h
    版本：v2.7.0
***********************************************************/
#include <PubSubClient.h>
PubSubClient mqtt(httpclient); // 跟wifi的http client 一致
String mqtt_id;
String _mqtt_msg;
#define MQTT_MAX_PACKET_SIZE 512


/**** << WaterBox_V2.3:OLED library u8g2 >> *****
    功能：時間模組+EEPROM
    library：u8g2.h
    版本：LinkIt 7697內建
***********************************************************/
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



/**** << WaterBox_V2.1.HP:SD Card >> *****
    功能：儲存資料在Ｍicro SD卡內
    library：SD.h
    版本：LinkIt 7697內建]
***********************************************************/
#include <SPI.h>
#include <SD.h>
File myFile;
const int SC_CS = 4;


/**** << WaterBox_V2.1:Arduino JSON >> *****
    功能：資料處理用，統一轉換成JSON比較好做事情
    library：ArduinoJson.h
    版本：5.13.5
***********************************************************/
#include <ArduinoJson.h>

// 注意buffer大小，預設1024
StaticJsonBuffer<2048> _jsonBuffer;

struct CONFIG {
  const char* fw_ver;  // 韌體版本名稱
  String device_id; // 儀器序號
  int sampling_sec; // pump 啟動時間

  int ch1_sampling; // pump 啟動時間
  int ch1_cycle; // pump 啟動時間
  int ch2_sampling; // pump 啟動時間
  int ch2_cycle; // pump 啟動時間
  int ch3_sampling; // pump 啟動時間
  int ch3_cycle; // pump 啟動時間

  int analysis_cycle; // 分析週期
  int saving_cycle; // 存檔週期
  int upload_cycle; // 上傳週期

  String wifi_ssid; // wifi連線用
  String wifi_pass;

  String mqtt_broker; // mqtt連線用
  int mqtt_port;
  String mqtt_user;
  String mqtt_pass;
  String mqtt_pub_topic;
  String mqtt_sub_topic;

  String thingspeak_key; // ThingSpeak上傳

  String http_server;    // http/https 上傳
  String https_server;

  String restful_item1;   // RestFul用
  String restful_item2;
  String restful_item3;
  String restful_item4;
  String restful_item5;
  String restful_item6;
  String restful_item7;
  String restful_item8;
};


struct CAL_CONFIG
{
  float y_HIGH;
  float y_MID;
  float y_LOW;
  float x_HIGH;
  float x_MID;
  float x_LOW;
  float SLOP;
  float INTERCEPT;
  float R_2;
};


/**** << WaterBox_V2.1.HP:DS3231 時間模組 >> *****
    功能：時間模組+EEPROM
    library：RTClib.h
    版本：1.2.0
***********************************************************/
#include <RTClib.h>           // DS3231 library
RTC_DS3231 rtc;


/**** << WaterBox_V2.1.HP:UTC 時間自動更新 >> *****
    功能：到網站上抓UTC時間後，更新系統內時機ㄢ
    library：UTC_Converter.h
    版本：同個資料夾內的UTC_Converter.h UTC_Converter.cpp 檔案
***********************************************************/
#include <WiFiUdp.h>
#include <LRTC.h>

#include "UTC_Converter.h"           // UTC library

unsigned int localPort = 2390;                          // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28);                   // time.nist.gov NTP server
const char* NTP_server = "time-a.nist.gov";

const int NTP_PACKET_SIZE = 48;                         // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];                     // buffer to hold incoming and outgoing packets

WiFiUDP Udp;                                            // A UDP instance to let us send and receive packets over UDP
UTC_Converter cUTC(true);


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

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
int16_t adc0, adc1, adc2, adc3;

#define pH_pin          1
#define EC_pin          2
#define Rotary_Pin_1    3  // VR1
#define Rotary_Pin_2    0  // VR2


/**** << WaterBox_V2.1:Atlas Scientific >> *****
   Atlas Scientific 水質模組設定
***********************************************************/
#define addr_DO 97


/***** << GPIO設定 >> *****/
#define sensorSwitch 6
#define modeSwitch   5
#define pinLED       7
#define modulePower 14


/***** << 系統參數 >> *****/
float pH_slop, pH_intercept, EC_slop, EC_intercept, pH_alarm, EC_alarm;
int print_interval = 1000;

int _c = 0;
float Temp_value, pH_value, EC_value, Tubidity_value;
float DO_percent, DO_value;
float SensorValue[7] = {0};
bool _modeStatus = false;

int chapter = 0;                               //  大項目：0~1(校正設定,時間設定)
int item = 0;                                  //  子項目：0~3(準備,參數1設定,參數2設定,參數3設定)
bool config_state = true;                      //  true時只能顯示目前設定，flase時可以改設定
int _YY, _year_1, _year_2, _MM, _DD, _HH, _mm; //  時間日期調整用

int _year, _month, _day, _hour, _minute, _second;       //  系統時間用
unsigned long Tick = 0;

String CSV_Header;   // 寫入CSV時的表頭
String CSV_fileName; // 檔案名稱(需少於8個字元)
String CSV_Data;     // CSV資料
String str_Time;     // 資料寫入時的時間  "YYYY-MM-DD mm:hh"

unsigned  long _SD_tag, _upload_tag, _utp_tag, _analysis_tag; // 用來記錄要不要寫入SD卡/上傳雲端的時間戳
bool _saving = true;
bool _upload = true;
bool _analysis = true;
bool _updateNTP = true;

String alarmStr;                      // 用來記錄要不要顯示alarm

CONFIG sysConfig;                     // 存放資料的stuct
CAL_CONFIG cal_ph;
CAL_CONFIG cal_ec;


/***** << firmware Version >> *****/
String _fw = "Ver 2.1.Hydroponic";
const String Description_Tittle = ">> LASS IS YOURS <<";

String Description_Firware = "The DEVICE FIRWARE: " + String(_fw);
const String Description_Features = "THIS VERSION HAS THE　FOLLOWING FEATURES:\n\r"\
                                    "\t 1.Get the EC and pH value via ADS1115 module\n\r"\
                                    "\t 2.Get temptrue value via DS18B20 module\n\r"\
                                    "\t 3.Save DATA with CSV format by Day\n\r"\
                                    "\t 4.Upload Data to ThingSpeak cloud platform via WiFi\n\r";
const String Description_Precautions = "<< PRECAUTIONS >>\n\r"\
                                       "\t 1.All module power controled by P14\n\r"\
                                       "\t 2.LinkIt 7697 was not into deep sleep mode\n\r"\
                                       "\t 3.LoRa module did not used in this version\n\r";


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
    u8g2.drawStr(80, 20, sysConfig.fw_ver);                 // 16-13 = 3, 3*8=24 -> 12 開始

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
    u8g2.drawStr(80, 20, sysConfig.fw_ver);                  // 韌體版本(靠右)

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
    u8g2.drawStr(80, 20, sysConfig.fw_ver);             // 韌體版本(靠右)

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
    u8g2.drawStr(80, 20, sysConfig.fw_ver);                  // 韌體版本(靠右)

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
void initSD(void)
{
  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa
  SD.begin(SC_CS);
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

void SD_showFile(File dir, int numTabs)
{
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      SD_showFile(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

String SD_readFile(String _fileName)
{
  initSD();
  String _buffer = "";

  File _file = SD.open(_fileName, FILE_READ);

  if (_file) {
    Serial.println("Open File: " + _fileName);
    while (_file.available()) {
      char _c = (char)_file.read();
      _buffer += _c;
    }
    Serial.println(_buffer);
    _file.close();
  }
  else
  {
    Serial.println("[ERROR] File: " + _fileName + " open error");
  }
  return _buffer;
}

bool SD_SaveCSV(String _dirName, String _fileName, String _data)
{
  bool _stateCheck = SD_checkDir(_dirName);
  String fileLocation = _dirName + "/" + _fileName + ".CSV";

  if (_stateCheck)  {
    _stateCheck = SD_WriteData(fileLocation, _data);
  }
  else            Serial.println("[SD ] Directory check failed");

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

void SavingData(String _fileName, String _data)
{
  if (_fileName.length() > 8) {
    Serial.println("[SD ] 檔案名稱過長: " + _fileName);
    _fileName = _fileName.substring(0, 8);
  }
  initSD();
  SD_SaveCSV("DATA", _fileName, _data);  // 寫入資料
  delay(50);
}


/***** << JSON function >> *****/
void loadConfig(String _fileName)
{
  initSD();

  bool _stateCheck = SD.exists(_fileName);
  if (_stateCheck)
  {
    Serial.println("[SD] File <" + _fileName + "> exists");
  }
  else {
    Serial.println("[SD] File <" + _fileName + "> did not exist");
  }

  File _configFile = SD.open(_fileName);
  int _size = _configFile.size();

  if (_size > 2048) Serial.println("[ERROR] the size of config > " + (String)_size);
  else              Serial.println("[SYSTEM] config size: " + (String)_size);

  // Deserialize the JSON document

  JsonObject &_json = _jsonBuffer.parseObject(_configFile);

  if (!_json.success()) {
    Serial.println(F("[JSON] Failed to read file, using default configuration"));
  }
  else {
    Serial.println("[SYSTEM] Reading JSON Config File\r\n==========");
    appendConfig(_json);
    Serial.println("\r\n==========");
  }
  _configFile.close();
}

void appendConfig(JsonObject &_json)
{
  //  _json.prettyPrintTo(Serial);
  JsonVariant _buffer;

  sysConfig.fw_ver = _json["fw_ver"].as<const char*>();
  sysConfig.device_id = _json["device_id"].as<String>();
  sysConfig.sampling_sec = _json["sampling_sec"].as<int>();

  _buffer = _json.get<JsonVariant>("pump_ch1_sec");
  sysConfig.ch1_sampling = _buffer["sampling"].as<int>();
  sysConfig.ch1_cycle = _buffer["cycle"].as<int>();

  _buffer = _json.get<JsonVariant>("pump_ch2_sec");
  sysConfig.ch2_sampling = _buffer["sampling"].as<int>();
  sysConfig.ch2_cycle = _buffer["cycle"].as<int>();

  _buffer = _json.get<JsonVariant>("pump_ch3_sec");
  sysConfig.ch3_sampling = _buffer["sampling"].as<int>();
  sysConfig.ch3_cycle = _buffer["cycle"].as<int>();

  _buffer = _json.get<JsonVariant>("interval_sec");
  sysConfig.analysis_cycle = _buffer["analysis"].as<int>();
  sysConfig.saving_cycle = _buffer["saving"].as<int>();
  sysConfig.upload_cycle = _buffer["upload"].as<int>();

  _buffer = _json.get<JsonVariant>("wifi");
  sysConfig.wifi_ssid = _buffer["wifi_ssid"].as<String>();
  sysConfig.wifi_pass = _buffer["wifi_pass"].as<String>();

  _buffer = _json.get<JsonVariant>("mqtt");
  sysConfig.mqtt_broker = _buffer["mqtt_broker"].as<String>();
  sysConfig.mqtt_port = _buffer["mqtt_port"].as<int>();
  sysConfig.mqtt_user = _buffer["mqtt_user"].as<String>();
  sysConfig.mqtt_pass = _buffer["mqtt_pass"].as<String>();
  sysConfig.mqtt_pub_topic = _buffer["mqtt_pub_topic"].as<String>();
  sysConfig.mqtt_sub_topic = _buffer["mqtt_sub_topic"].as<String>();

  sysConfig.thingspeak_key = _json["thingspeak_key"].as<String>();

  _buffer = _json.get<JsonVariant>("http");
  sysConfig.http_server = _buffer["http_server"].as<String>();
  sysConfig.https_server = _buffer["https_server"].as<String>();


  sysConfig.restful_item1 = _json["restful_item"][0].as<String>();
  sysConfig.restful_item2 = _json["restful_item"][1].as<String>();
  sysConfig.restful_item3 = _json["restful_item"][2].as<String>();
  sysConfig.restful_item4 = _json["restful_item"][3].as<String>();
  sysConfig.restful_item5 = _json["restful_item"][4].as<String>();
  sysConfig.restful_item6 = _json["restful_item"][5].as<String>();
  sysConfig.restful_item7 = _json["restful_item"][6].as<String>();
  sysConfig.restful_item8 = _json["restful_item"][7].as<String>();

  // 基本檢查
  if (sysConfig.analysis_cycle == 0) sysConfig.analysis_cycle = 150;
  if (sysConfig.saving_cycle == 0) sysConfig.saving_cycle = 300;
  if (sysConfig.upload_cycle == 0) sysConfig.upload_cycle = 600;

}

void showConfig()
{
  Serial.println("----- System Config -----");

  Serial.println("[ Device ]");
  Serial.print("\t fw_ver:\t"); Serial.println(sysConfig.fw_ver);
  Serial.print("\t device_id:\t"); Serial.println(sysConfig.device_id);
  Serial.print("\t sampling_sec:\t"); Serial.println(sysConfig.sampling_sec);


  Serial.println("[ Ch_1 (sec) ]");
  Serial.print("\t Sampling:\t"); Serial.println(sysConfig.ch1_sampling);
  Serial.print("\t cycle:\t\t"); Serial.println(sysConfig.ch1_cycle);

  Serial.println("[ Ch_2 (sec) ]");
  Serial.print("\t Sampling:\t"); Serial.println(sysConfig.ch2_sampling);
  Serial.print("\t cycle:\t\t"); Serial.println(sysConfig.ch2_cycle);

  Serial.println("[ Ch_3 (sec) ]");
  Serial.print("\t Sampling:\t"); Serial.println(sysConfig.ch3_sampling);
  Serial.print("\t cycle:\t\t"); Serial.println(sysConfig.ch3_cycle);

  Serial.println("[ Interval(sec) ]");
  Serial.print("\t analysis:\t"); Serial.println(sysConfig.analysis_cycle);
  Serial.print("\t saving:\t"); Serial.println(sysConfig.saving_cycle);
  Serial.print("\t upload:\t"); Serial.println(sysConfig.upload_cycle);

  Serial.println("[ WIFI ]");
  Serial.print("\t wifi_ssid:\t"); Serial.println(sysConfig.wifi_ssid);
  Serial.print("\t wifi_pass:\t"); Serial.println(sysConfig.wifi_pass);

  Serial.println("[ MQTT ]");
  Serial.print("\t mqtt_broker:\t"); Serial.println(sysConfig.mqtt_broker);
  Serial.print("\t mqtt_port:\t"); Serial.println(sysConfig.mqtt_port);
  Serial.print("\t mqtt_user:\t"); Serial.println(sysConfig.mqtt_user);
  Serial.print("\t mqtt_pass:\t"); Serial.println(sysConfig.mqtt_pass);
  Serial.print("\t pub_topic:\t"); Serial.println(sysConfig.mqtt_pub_topic);
  Serial.print("\t sub_topic:\t"); Serial.println(sysConfig.mqtt_sub_topic);

  Serial.println("[ ThingSpeak ]");
  Serial.print("\t thingspeak_key:\t"); Serial.println(sysConfig.thingspeak_key);

  Serial.println("[ HTTP ]");
  Serial.print("\t http_server:\t"); Serial.println(sysConfig.http_server);
  Serial.print("\t https_server:\t"); Serial.println(sysConfig.https_server);

  Serial.println("[ RestFul API Item ]");
  Serial.print("\t restful_item1:\t"); Serial.println(sysConfig.restful_item1);
  Serial.print("\t restful_item2:\t"); Serial.println(sysConfig.restful_item2);
  Serial.print("\t restful_item3:\t"); Serial.println(sysConfig.restful_item3);
  Serial.print("\t restful_item4:\t"); Serial.println(sysConfig.restful_item4);
  Serial.print("\t restful_item5:\t"); Serial.println(sysConfig.restful_item5);
  Serial.print("\t restful_item6:\t"); Serial.println(sysConfig.restful_item6);
  Serial.print("\t restful_item7:\t"); Serial.println(sysConfig.restful_item7);
  Serial.print("\t restful_item8:\t"); Serial.println(sysConfig.restful_item8);

  Serial.println("-------------------------");
}


/***** << RTC function >> *****/
bool initRTC(bool _setTime = false,
             int _year_rtc = 2000, int _month_rtc = 1, int _day_rtc = 1,
             int _Hour_rtc = 1, int _Minute_rtc = 0, int _Second_rtc = 0)
{
  bool _state = true;
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    _state = false;
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     rtc.adjust(DateTime(2021, 9, 21, 1, 0, 0));
  }

  if (_setTime) {
    if (_year_rtc == 2000) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    else rtc.adjust(DateTime(_year_rtc, _month_rtc, _day_rtc, _Hour_rtc, _Minute_rtc, _Second_rtc));
  }
  return _state;
}


/***** << UTC function >>  ************************
   數字轉換 用function
***************************************************/
String padding(int _d)
{
  String _str;
  if (_d < 10) _str = "0" + String(_d);
  else _str = String(_d);
  return _str;
}

bool compare(int _a, int _b)
{
  bool _result = true;
  if (_a != _b)
  {
    _result = false;
    Serial.println("錯誤: " + String(_a) + "-" + String(_b));
  }
  return _result;
}


/***** << UTC function >>  ************************
   取得網路上時間資料
***************************************************/
// 取得NTP的封包
unsigned long sendNTPpacket(const char* host)
{
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(host, 123); //NTP requests are to port 123

  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);

  //Serial.println("5");
  Udp.endPacket();

  //Serial.println("6");
  return 0;
}

// 取得NTP的資料同時更新
void getNTP()
{
  Udp.begin(localPort);
  sendNTPpacket(NTP_server); // send an NTP packet to a time server
  delay(5000);

  if (Udp.parsePacket())
  {
    Serial.println("packet received");                  // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long secs1900 = calculateNTPTime();
    cUTC.setUTC(secs1900);
    //cUTC.setTimeZone(8);
  }
  else
  {
    Serial.println("No Packet recevied");
  }
}

// 計算出自1900.1.1 00:00:00以來的秒數(NTP都統一用這當基準點)
unsigned long calculateNTPTime()
{
  // the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  //Serial.print("Seconds since Jan 1 1900 = ");
  //Serial.println(secsSince1900);


  // now convert NTP time into everyday time:
  //Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:

  //const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  //unsigned long _epoch = secsSince1900 - seventyYears;
  // print Unix time:
  //Serial.println(_epoch);

  return secsSince1900;
}

// 顯示NTP的內容，_new 控制要不要再更新一次NTP
void showNTP(bool _new = false)
{
  if (_new)  getNTP();
  Serial.print(cUTC.year);  Serial.print("/");  Serial.print(cUTC.month); Serial.print("/");  Serial.print(cUTC.day);
  Serial.print(" ");
  Serial.println(padding(cUTC.hour) + ":" + padding(cUTC.minute) + ":" + padding(cUTC.second));

  Serial.print(cUTC.WeeksOfYear);
  Serial.print(" Weeks and ");
  Serial.print(cUTC.DaysOfYear);
  Serial.print(" Days in this year, ");
  Serial.print("Day of the week: ");
  Serial.println(cUTC.DaysOfWeek);
}

// 檢察NTP跟系統時間，_reset 控制要不要更新系統時間
bool checkNTP(bool _reset = false)
{
  bool _result = true;

  getNTP();
  LRTC.get();

  _result = _result && compare(cUTC.year, LRTC.year());
  _result = _result && compare(cUTC.month, LRTC.month());
  _result = _result && compare(cUTC.day, LRTC.day());
  _result = _result && compare(cUTC.hour, LRTC.hour());
  _result = _result && compare(cUTC.minute, LRTC.minute());
  _result = _result && compare(cUTC.second, LRTC.second());

  if (_reset && !_result) {
    Serial.println("重設系統時間");
    LRTC.set(cUTC.year, cUTC.month, cUTC.day, cUTC.hour, cUTC.minute, cUTC.second);
    showNTP();
  }
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


String atlas_cmd(String _cmd, uint16_t _delayMS, uint8_t _addr, bool _info = false)
{

  byte _char = 0;                //used as a 1 byte buffer to store inbound bytes from the D.O. Circuit.
  String _responce = "";

  Wire.beginTransmission(_addr);
  Wire.write(_cmd.c_str());
  Wire.endTransmission();

  delay(_delayMS);

  Wire.requestFrom(_addr, 20, 1); //call the circuit and request 20 bytes (this is more than we need)
  byte code = Wire.read();

  switch (code) {                           //switch case based on what the response code is.
    //    case 1:                                 //decimal 1.
    //      Serial.println("[Atlas] command Success");            //means the command was successful.
    //      break;                                //exits the switch case.
    case 2:                                 //decimal 2.
      Serial.println("[Atlas] command Failed");             //means the command has failed.
      break;                                //exits the switch case.
    case 254:                               //decimal 254.
      Serial.println("[Atlas] command Pending");            //means the command has not yet been finished calculating.
      break;                                //exits the switch case.
    case 255:                               //decimal 255.
      Serial.println("[Atlas] command No Data");            //means there is no further data to send.
      break;                                //exits the switch case.
  }


  while (Wire.available()) {                 //are there bytes to receive.
    _char = Wire.read();                   //receive a byte.
    _responce += (char) _char;                    //load this byte into our array.
  }

  if (_info) {
    Serial.print("[Atlas] ");
    Serial.println(_responce);
  }

  return _responce;
}

void atlas_sleep(uint8_t _addr) {
  Wire.beginTransmission(_addr);
  Wire.write("Sleep");
  Wire.endTransmission();
}

float get_DO(float _temp = 20.0) {
  String _cmd = "rt," + String(_temp, 1);

  atlas_cmd("O,mg,1", 600, addr_DO); // 設定顯示 mg/L
  atlas_cmd("O,%,1", 600, addr_DO);  // 設定顯示 %

  String _result = atlas_cmd(_cmd, 600, addr_DO);

  uint8_t _i = _result.indexOf(",");
  String _value_mg = _result.substring(0, _i);
  String _value_percent = _result.substring(_i + 1);

  DO_percent = _value_percent.toFloat();

  return _value_mg.toFloat();
}


/***** << Serial function >> ******
    取得序列舖輸入資料
    DO,<cmd>    // DO模組用的指令
    TEMP,<cmd>  // Temp模組用的指令
    PH,<cmd>    // pH模組用的指令
    EC,<cmd>    // EC模組用的指令
    SYS,<cmd>   // 系統設定指令：如讀去目前的資料
***********************************/
String getSerial(void)
{
  String _result;
  while (Serial.available())
  {
    char _c = Serial.read();
    _result += String(_c);
  }
  _result.replace("\r", "");
  _result.replace("\n", "");
  return _result;
}

void sysCMD(String _cmd) {
  _cmd.toUpperCase();
  int _index = _cmd.indexOf(",");
  String _cmd_type = _cmd.substring(0, _index);
  _cmd =  _cmd.substring(_index + 1);
  Serial.println("[CMD] " + _cmd_type + " -> " + _cmd + ":" + String(_cmd.length()));

  if (_cmd_type == "DO")    atlas_cmd(_cmd, 600, 97, true);
  if (_cmd_type == "TEMP")  Serial.println("[TEMP] " + _cmd + " NOT Executed");
  if (_cmd_type == "PH")    Serial.println("[PH]   " + _cmd + " NOT Executed");
  if (_cmd_type == "EC")    Serial.println("[EC]   " + _cmd + " NOT Executed");
  if (_cmd_type == "SYS")   Serial.println("[SYS]  " + _cmd + " NOT Executed");
  if (_cmd_type == "DATE")   Serial.println("[SYS]  " + _cmd + " NOT Executed");
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

  // 因為板子的電路會進行分壓 0~5V -> 0~2.5V
  // 所以ADS1115量出來的電壓要再乘上2才會是電錶的電壓
  // 手動輸入
  //  slope = 0.6024;
  //  intercept = 3.3643;

  float _ads_Volate = 2 * _getVoltage(pH_pin) * slope + intercept;
  float pH_Value = 3.5 * _ads_Volate * 0.001;                                   // 分壓(5V->2.5V)後，用電壓換算成pH (斜率3.5的出處待確認)
  //  pH_Value = pH_Value * 0.5063 + 3.3308; // 舊的斜率斜率截距
  pH_Value = pH_Value * 1.0 + 0.0; // 校正用
//  pH_Value = pH_Value * 0.6584 + 2.7574; // 校正用
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

    Serial.print("[EC ] Below range!(");
    Serial.print(_value);
    Serial.println(")");
    //    if (_value < 0) _value = 0;

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
  _value = 0.94 * _value + 2300;
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
  DS18B20 ds(DS18B20_Pin);
  float _result = ds.getTempC();

  return _result;
}


/***** << LinkIt 7697 wifi function >> *****
   LinkIt 7697 WIFI 連線用
***********************************/
void showWiFiConcentStatus(void)
{
  int _status = WiFi.status();  // 更新wifi目前的狀況
  switch (_status) {
    case WL_CONNECTED:
      Serial.println("[Wifi State] connected to a WiFi network");
      break;
    case WL_NO_SHIELD:
      Serial.println("[Wifi State] no WiFi shield is present");
      break;
    case WL_IDLE_STATUS:
      Serial.println("[Wifi State] waiting to update WiFi conncet state");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("[Wifi State] no SSID are available");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("[Wifi State] the scan networks is completed");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("[Wifi State] the connection fails for all the attempts");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("[Wifi State] the connection is lost");
      break;
    case WL_DISCONNECTED:
      Serial.println("[Wifi State] disconnected from a network");
      break;
    default:
      break;
  }
}

int connectWifi(bool _debug = false)
{
  if (sysConfig.wifi_ssid.length() != 0) ssid = sysConfig.wifi_ssid.c_str();
  if (sysConfig.wifi_pass.length() != 0) pass = sysConfig.wifi_pass.c_str();

  int _status = WiFi.status();
  if (_debug) {
    showWiFiConcentStatus();
  }

  for (int _c = 0; _c < 5; _c++) {
    if (_status != WL_CONNECTED)
    {
      Serial.print("Attempting to connect to SSID: ");

      Serial.print(ssid);
      Serial.print("\t");
      Serial.println(pass);
      _status = WiFi.begin(ssid, pass);

      Serial.print("reCheck SSID: ");
      Serial.println(WiFi.SSID());
      if (_debug) {
        showWiFiConcentStatus();
      }
    }
    else {
      break;
    }

  }
  return WiFi.status();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


/***** << LinkIt 7697 upload function >> *****
   LinkIt 7697 WIFI 上傳資料用
***********************************/
bool getLinkItLogo( bool _readResponse)
{
  sslclient.setRootCA(rootCA, sizeof(rootCA));
  if (sslclient.connect(server, 443)) {
    Serial.println("connected to server (GET)");
    // Make a HTTP request:
    sslclient.println("GET /linkit_7697_ascii.txt HTTP/1.0");
    sslclient.println("Host: download.labs.mediatek.com");
    sslclient.println("Accept: */*");
    sslclient.println("Connection: close");
    sslclient.println();
    delay(500);
  }
  else Serial.println("Web Conncet Error");

  if (_readResponse)
  {
    Serial.println("\r\n");
    Serial.println("********************* LINKIT 7697 LOGO Test *********************");
    while (sslclient.available()) {
      char c = sslclient.read();
      Serial.write(c);
    }
    Serial.println();
    Serial.println("*****************************************************************");
  }
  Serial.println("\r\n");
  sslclient.stop();
}

bool updateThingSpeak(String _api, String _field1Value, String _field2Value, String _field3Value)
{
  String server = "api.thingspeak.com";
  String _field1 = "&field1=" + _field1Value;
  String _field2 = "&field2=" + _field2Value;
  String _field3 = "&field3=" + _field3Value;
  String getStr = "GET /update?api_key=" + _api + _field1 + _field2 + _field3;

  sslclient.setRootCA(rootCA, sizeof(rootCA));
  if (sslclient.connect(server.c_str(), 443))
  {
    Serial.println("Update to ThingSpeak (GET)");
    // Make a HTTP request:
    sslclient.println(getStr);
    sslclient.println("Host: " + server);
    sslclient.println("Accept: */*");
    sslclient.println("Connection: close");
    sslclient.println();
    delay(500);
  }

  while (sslclient.available()) {
    char c = sslclient.read();
    Serial.write(c);
  }
  sslclient.stop();
  Serial.println();
}


/***** << LinkIt 7697 MQTT function >> *****
   LinkIt 7697 MQTT傳資料用
***********************************/
String WifiMac(bool _hasDash = false, bool _fullMac = false)
{
  String _dash;
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

  //  Serial.println("MAC Address: "+_macAddress);   // 檢查用

  return _macAddress;
}


/***** << LinkIt 7697 IBP upload function >> *****
   LinkIt 7697 WIFI 上傳IBP資料用
***********************************/
void mqtt_reconnect()
{
  //  const char* _USERNAME = sysConfig.mqtt_user.c_str();
  //  const char* _PASSWORD = sysConfig.mqtt_pass.c_str();
  const char* _TOPIC = sysConfig.mqtt_sub_topic.c_str();

  for (int _i = 0 ; _i < 5; _i++)
  {
    Serial.print("[MQTT] Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(mqtt_id.c_str(), "", ""))
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
    if (mqtt.connected()) break;
  }

  if (sysConfig.mqtt_sub_topic.length() > 0) {
    mqtt.subscribe(_TOPIC);  // 訂閱Topic
  }
  //  mqtt.unsubscribe(TOPIC);  // 取消訂閱Topic
}

void mqtt_disconnect()
{
  Serial.println("[MQTT] MQTT Disconnect");
  mqtt.unsubscribe(sysConfig.mqtt_sub_topic.c_str()); // 先取消訂閱TOPIC
  mqtt.disconnect();
}

void mqtt_publish(String _msg)
{
  //  const char* _TOPIC = sysConfig.mqtt_pub_topic.c_str();

  if (mqtt.connected()) {
    mqtt_disconnect();
  }
  mqtt_reconnect();

  if (mqtt.connected()) {
    Serial.println("[MQTT] MQTT Connect Success");
    mqtt.publish(sysConfig.mqtt_pub_topic.c_str(), _msg.c_str());
  }
  else {
    Serial.println("[MQTT] MQTT Connect Fail");
  }
  Serial.println("[MQTT] Publish done");
  mqtt_disconnect();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("[MQTT] Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


/***** << LinkIt 7697 EEPROM function >> *****
  LinkIt 7697 內部記憶體的讀寫
  bool EEPROM_write(char* _str, int _page, int _length)
***********************************/
bool EEPROM_write(char* _str, int _page, int _length) // 寫入資料，1頁 32 bytes
{
  int _address = _page * 32;
  if (_length > 31) {                // 超出頁面
    Serial.println("Out Of Pages");
    return false;
  }
  else {
    Serial.print("Writing data：");
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
    Serial.println("Out Of Pages");
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

  if (!_state)  Serial.println("開始更新設定");

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


bool CheckTag(unsigned long *r_tag, unsigned long _ms , bool _debug = false)
{
  bool _result = false;
  DateTime now = rtc.now();      // 取得目前的時間

  unsigned long _now = now.unixtime();

  if (_debug)
  {
    Serial.println("[Tag] \t Unixtime:" + (String)_now);
    Serial.println("[Tag] \t Tag time:" + (String)*r_tag);
  }

  if (_now > *r_tag)
  {
    _result = true;                // 更新比對結果
    *r_tag = _now + _ms; // 更新時間戳
    if (_debug) Serial.println("[Next Tag] " + (String)*r_tag + " (seconds):new");
  }

  else
  {
    _result = false;
    unsigned long _interval = *r_tag - _now;
    if (_debug) Serial.println("[Next Tag] " + (String)*r_tag + " (seconds):" + (String)_interval + " (seconds missing)");
  }

  return _result;
}


void SystemTest_SD(int _loop)
{
  SD.begin(SC_CS);
  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  Serial.println("建立資料夾");
  String _dirName = "Dir-" + String(_loop);
  SD_checkDir(_dirName);
  Serial.print("尋找資料夾(" + _dirName + "): ");
  Serial.println(SD.exists(_dirName));
  delay(5000);
}

// 目前沒有用到
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
  Serial.println("System Start");
  OLED_content("Serial Port", "CHECKED", 1.0, false);

  // ADS1115 初始化
  initADC();
  OLED_content("ADS1115", "Init", 1, false);
  Serial.println("ADS1115 初始化完成");

  OLED_content("Loading", "System Config", 1.0, false);
  Serial.println("讀取系統設定");
  _config(true); // 讀取目前的設定

  OLED_content("Loading", "SD Config", 1.0, false);
  loadConfig("/config");
  showConfig();

  Serial.println("系統設定初始化完成");
  OLED_content("System", "DONE" , 1.0, false);

  pinMode(pinLED, OUTPUT);

  Serial.println("倒數三秒");
  OLED_content("Sleep", "TEST(3)" , 1.0, true);
  atlas_sleep(97);

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

  digitalWrite(pinLED, HIGH);
  Serial.println("System Init Done");

  digitalWrite(modulePower, HIGH);  // 開啟模組電源
  OLED_content("Hello", "LASS", 1, false);
  Serial.println();
  Serial.println(F("**************************************"));
  Serial.println();
  Serial.println(Description_Tittle);
  Serial.println(Description_Firware);
  Serial.println(F("版本功能："));
  Serial.println("\t" + Description_Features);
  Serial.println(F("注意事項："));
  Serial.println("\t" + Description_Precautions);
  Serial.println();
  Serial.println(F("**************************************"));
  Serial.println();


  OLED_content("WatechDog", "20s Ready", 1, false);
  LWatchDog.begin(20);
  Serial.println("[Watech Dog] init dog....");

  OLED_content("Connect", "Wifi", 1, false);
  status = connectWifi(true);
  if (status == WL_CONNECTED)
  {
    mqtt_id = "METL_WaterBoxHP_" + WifiMac();
    Serial.println("[MQTT] MQTT Client ID: " + mqtt_id);
    printWifiStatus();

    LWatchDog.feed();
    Serial.println("[RTC] init RTC....");
    checkNTP(true);
    LRTC.get();
    initRTC(true, LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());
    Serial.println("DS3231 初始化完成");

    LWatchDog.feed();
    getLinkItLogo(true);
  }

  if (sysConfig.mqtt_broker.length() > 0) {
    mqtt.setServer(sysConfig.mqtt_broker.c_str(), sysConfig.mqtt_port);
    mqtt.setCallback(mqtt_callback);
    mqtt_publish("Hi TUVALU!");    // MQTT 測試用
  }
  WiFi.disconnect();


  LWatchDog.feed();
  pinMode(sensorSwitch, OUTPUT);        // 用USR pin 控制 pH & EC 模組電源切換

  digitalWrite(sensorSwitch, HIGH);     // 切換到 pH
  delay(1000); // 讓Sensor 完成積分
  Temp_value = getTemp();
//  pH_value = getPH(pH_slop, pH_intercept);
  pH_value = getPH(); // 使用斜率=1, 截距=0
  OLED_content_title(str_Time, "pH: " + String(pH_value), "Temp: " + String(Temp_value), "Analysis Mode", 1.5, false);

  digitalWrite(sensorSwitch, LOW);     // 切換到 EC
  delay(1000); // 讓Sensor 完成積分
  LWatchDog.feed();
//  EC_value = getEC(EC_slop, EC_intercept);
  EC_value = getEC(); // 使用斜率=1, 截距=0
  DO_value = get_DO(Temp_value);
  atlas_sleep(97); // 關閉DO 模組的電源

  if ( alarm_check(pH_value, EC_value))  alarmStr = "Alarm";
  else                                   alarmStr = "";

  OLED_content_title(str_Time, "EC: " + String(EC_value), "DO: " + String(DO_value), "Analysis Mode", 1.5, true);

  DateTime now = rtc.now();          // 取得目前時間
  Serial.print("***** [Setup] 測值");
  Serial.print(now.year(), DEC); Serial.print('/'); Serial.print(now.month(), DEC); Serial.print('/'); Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC);
  Serial.println(" *****");
  Serial.println("pH\t" + String(pH_value));
  Serial.println("Temp\t" + String(Temp_value) + " C");
  Serial.println("EC\t" + String(EC_value) + " uS");
  Serial.println("DO\t" + String(DO_value) + " mg/L");
  Serial.println("DO\t" + String(DO_percent) + " %");
  Serial.println("Alarm\t" + alarmStr);

  digitalWrite(modulePower, LOW);         // 關閉電源

  Serial.println("End of Setup");
}


/***** << Main function: Loop >> *****/
void loop()
{
  LWatchDog.feed();
  digitalWrite(modulePower, HIGH);    // 開啟模組電源
  /*****<< 取得時間資料 >>*****/
  DateTime now = rtc.now();          // 取得目前時間
  _year = now.year();
  _month = now.month();
  if (now.hour() + 8 > 23) {
    _hour = now.hour() + 8 - 23;
    _day = now.day() + 1;
  }
  else
  {
    _hour = now.hour() + 8;
    _day = now.day();
  }

  _minute = now.minute();
  str_Time = convert_2digits(_hour) + ":" + convert_2digits(_minute);

  bool _mode = digitalRead(modeSwitch);

  if (_mode)
    /*****<< 進入分析運作模式 >>*****/
  {
    if (_modeStatus != _mode) {
      Serial.println("**********<< 進入分析模式 >>**********");
      _modeStatus = _mode;
    }

    /*****<< 更新時間資料 >>*****/
    if (_year != 2000) {
      _analysis = CheckTag( &_analysis_tag, sysConfig.analysis_cycle, false);      // 1分鐘醒來一次
      _saving = CheckTag( &_SD_tag, sysConfig.saving_cycle, false);                //  5分鐘存檔一次
      _upload =  CheckTag( &_upload_tag, sysConfig.upload_cycle, false);           // 10分鐘上傳一次
      _updateNTP =  CheckTag( &_utp_tag, 3600, false);                             // 60分鐘更新時間一次
    }

    /*****<< 按造各tag檢查結果執行內容 >>*****/
    /*****<< 分析水質 >>*****/
    if (_analysis)
    {
      LWatchDog.feed();
      pinMode(sensorSwitch, OUTPUT);        // 用USR pin 控制 pH & EC 模組電源切換

      digitalWrite(sensorSwitch, HIGH);     // 切換到 pH
      delay(1000); // 讓Sensor 完成積分
      Temp_value = getTemp();
      pH_value = getPH(pH_slop, pH_intercept);
      OLED_content_title(str_Time, "pH: " + String(pH_value), "Temp: " + String(Temp_value), "Analysis Mode", 1.5, false);

      digitalWrite(sensorSwitch, LOW);     // 切換到 EC
      delay(1000); // 讓Sensor 完成積分
      LWatchDog.feed();
      EC_value = getEC(EC_slop, EC_intercept);
      //      DO_value = get_DO(Temp_value);
      //      atlas_sleep(addr_DO); // 關閉DO 模組的電源

      if ( alarm_check(pH_value, EC_value))  alarmStr = "Alarm";
      else                                   alarmStr = "";

      //      OLED_content_title(str_Time, "EC: " + String(EC_value), "DO: " + String(DO_value), "Analysis Mode", 1.5, true);
      OLED_content_title(str_Time, "EC: " + String(EC_value), "", "Analysis Mode", 1.5, true);

      Serial.print("***** 測值 @");
      Serial.print(_year, DEC);
      Serial.print('/');
      Serial.print(_month, DEC);
      Serial.print('/');
      Serial.print(_day, DEC);
      Serial.print(" ");
      Serial.print(str_Time);
      Serial.print(" UTC+8");
      Serial.println(" *****");
      Serial.println("pH\t" + String(pH_value));
      Serial.println("Temp\t" + String(Temp_value) + " C");
      Serial.println("EC\t" + String(EC_value) + " uS");
      //      Serial.println("DO\t" + String(DO_value) + " mg/L");
      //      Serial.println("DO\t" + String(DO_percent) + " %");
      Serial.println("Alarm\t" + alarmStr);
    }

    /*****<< 把目前量測結果儲存至SD卡 >>*****/
    if (_saving)
    {
      LWatchDog.feed();

      OLED_content_title(str_Time, "SD", "Saving", "Analysis Mode", 1.0, false);
      String _str_Time = (String)_year + "-" + convert_2digits(_month) + "-" + convert_2digits(_day) + "," + str_Time;
      CSV_fileName = convert_2digits(_month) + convert_2digits(_day);

      CSV_Header   = "Date,Temp,pH,EC,DO,DO_%";
      CSV_Data = _str_Time + "," +
                 String(Temp_value) + "," +
                 String(pH_value) + "," +
                 String(EC_value) + "," +
                 String(DO_value) + "," +
                 String(DO_percent);

      SavingData(CSV_fileName, CSV_Data);     //  寫入CSV
      OLED_content_title(str_Time, "SD", "Done", "Analysis Mode", 1.0, false);
      Serial.println("[SD] 完成寫入SD");
    }

    /*****<< 上傳至ThingSpeak或是LASS >>*****/
    if (_upload)
    {
      LWatchDog.feed();
      OLED_content_title(str_Time, "Upload", "prepare", "Analysis Mode", 1.0, false);
      OLED_content_title(str_Time, "WiFi", "Connect", "Analysis Mode", 1.0, false);

      connectWifi();
      String _str_Time = String(_year) + "-" + convert_2digits(_month) + "-" + convert_2digits(_day) + " " +
                         convert_2digits(_hour) + ":" + convert_2digits(_minute) + ":" + convert_2digits(now.second());



      //      {\"dataTime\":\"2020-11-16 20:36:00\",\"dCode_temp\":130.0}

      //            String MQTT_Time = addLASS_msgTime();
      //            String MQTT_Value = addLASS_msgValue(SensorValue, true);
      //            updateLASS(MQTT_Time, MQTT_Value);

      updateThingSpeak(sysConfig.thingspeak_key, String(Temp_value), String(pH_value), String(EC_value));

      /* MQTT 發資料*/

      if (sysConfig.mqtt_broker.length() > 0) {
        _mqtt_msg = "{\"dataTime\":\"" + _str_Time +
                           "\",\"Temp\":" + Temp_value +
                           ",\"pH\":" + pH_value +
                           ",\"EC\":" + EC_value +
                           ",\"DO_mg\":" + DO_value +
                           ",\"DO_percent\":" + DO_percent +
                           "}";      String _mqtt_msg = "{\"dataTime\":\"" + _str_Time +
                               "\",\"Temp\":" + Temp_value +
                               ",\"pH\":" + pH_value +
                               ",\"EC\":" + EC_value +
                               ",\"DO_mg\":" + DO_value +
                               ",\"DO_percent\":" + DO_percent +
                               "}";
        Serial.println("[MQTT Publish] Message:" + _mqtt_msg);
        Serial.println("[MQTT Publish] Message size:" + String(_mqtt_msg.length()));

        OLED_content_title(str_Time, "MQTT", "Publish", "Analysis Mode", 1.0, false);

        LWatchDog.feed();
        mqtt_publish(_mqtt_msg);
        Serial.println("[MQTT] Publish done");
        mqtt_disconnect();
      }

      WiFi.disconnect();
      OLED_content_title(str_Time, "Upload", "Finish", "Analysis Mode", 1.0, false);
      Serial.println(" 完成上傳");
    }

    /*****<< 更新網路時間NTP Time >>*****/
    if (_updateNTP) {
      LWatchDog.feed();
      Serial.println("[System] 更新NTP");
      connectWifi();
      OLED_content_title(str_Time, "NTP", "Check", "Analysis Mode", 1.0, false);

      LWatchDog.feed();
      checkNTP(true);
      //      showNTP();
      LRTC.get();
      initRTC(true, LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());
      WiFi.disconnect();
    }

    //    digitalWrite(modulePower, LOW);         // 關閉電源

  } // end of if (_mode)
  /*****<< 進入系統設定模式 >>*****/
  else {
    if (_modeStatus != _mode) {
      Serial.println("**********<< 進入設定模式 >>**********");
      _modeStatus = _mode;
    }

    String newCMD = getSerial();
    if (newCMD.length() > 0) {
      sysCMD(newCMD);
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
