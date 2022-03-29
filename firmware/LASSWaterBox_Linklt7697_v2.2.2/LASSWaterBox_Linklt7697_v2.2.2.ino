/***** << WaterBox_V2.:LSleep with OLED and SD>> *****
  PCB borad：WaterBox V2.2
  功能：基本系統功能測試
  測試項目：
    1.OLED
    2.EEPROM
    3.LSleep
    4.SD ：使用前需留意LoRa的CS(P10)是否已經上拉，不然會抓到LoRa模組
    5.Temp
    6.pH EC
******************************************************/
/***** << WaterBox_V2.2:LinkIt 7697 EEPROM library >> *****/
#include <EEPROM.h>           // EEPROM library


/**** << WaterBox_V2.2:LinkIt 7697 Wifi library >> *****/
#include <LWiFi.h>

const char* ssid = "";     //  your network SSID (name)
const char* pass = "";     // your network password (use for WPA, or use as key for WEP)
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
#define SC_CS 4
#define LoRa_CS 10


/**** << WaterBox_V2.1:Arduino JSON >> *****
    功能：資料處理用，統一轉換成JSON比較好做事情
    library：ArduinoJson.h
    版本：5.13.5
***********************************************************/
#include <ArduinoJson.h>

// 注意buffer大小，預設1024
String CONFIG_PATH = "/config";
//StaticJsonBuffer<2048> _jsonBuffer;
DynamicJsonBuffer _jsonBuffer;         // ArduinoJson 5, 動態的Buffer

struct CONFIG {
  const char* fw_ver;  // 韌體版本名稱
  String device_id;    // 儀器序號: MQTT用

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

  String thingspeak_key; // ThingSpeak上傳用的API Key

  String http_server;      // http 上傳用Server
  String https_server;     // /https 上傳
  String restful_fromat;   // RestFul用 // 放數值的保留字(Reserve Key):r_ +item, ex:r_PH,r_EC,r_DO,r_ORP,r_TEMP
  int vr1_offset;
  int vr1_score;
  int vr2_offset;
  int vr2_score;
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

#define pH_pin          0
#define EC_pin          1
#define Rotary_Pin_1    2  // VR1
#define Rotary_Pin_2    3  // VR2


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
String _fw = "Ver 2.2";
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
    u8g2.drawStr(0, 20, "Test Mode");                   // 左邊的小title
    u8g2.drawStr(80, 20, sysConfig.fw_ver);             // 韌體版本(靠右)

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
  pinMode(LoRa_CS, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(LoRa_CS, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa
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
  JsonVariant _buffer; // 是個 reference, 而不是實際ㄉ植
  sysConfig.fw_ver = _json["fw_ver"].as<const char*>();
  sysConfig.device_id = _json["device_id"].as<String>();

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

  _buffer = _json.get<JsonVariant>("thingspeak");
  sysConfig.thingspeak_key = _buffer["api_key"].as<String>();

  _buffer = _json.get<JsonVariant>("restful_api");
  sysConfig.http_server = _buffer["http_server"].as<String>();
  sysConfig.https_server = _buffer["https_server"].as<String>();
  sysConfig.restful_fromat = _buffer["restful_fromat"].as<String>(); // 放數值的保留字(Reserve Key):r_ +item, ex:r_PH,r_EC,r_DO,r_ORP,r_TEMP

  _buffer = _json.get<JsonVariant>("cal_ph");
  cal_ph.x_HIGH = _buffer["raw_HIGH"].as<float>();
  cal_ph.x_MID = _buffer["raw_MID"].as<float>();
  cal_ph.x_LOW = _buffer["raw_LOW"].as<float>();
  cal_ph.y_HIGH = _buffer["std_HIGH"].as<float>();
  cal_ph.y_MID = _buffer["std_MID"].as<float>();
  cal_ph.y_LOW = _buffer["std_LOW"].as<float>();

  _buffer = _json.get<JsonVariant>("cal_ec");
  cal_ec.x_HIGH = _buffer["raw_HIGH"].as<float>();
  cal_ec.x_MID = _buffer["raw_MID"].as<float>();
  cal_ec.x_LOW = _buffer["raw_LOW"].as<float>();
  cal_ec.y_HIGH = _buffer["std_HIGH"].as<float>();
  cal_ec.y_MID = _buffer["std_MID"].as<float>();
  cal_ec.y_LOW = _buffer["std_LOW"].as<float>();

  _buffer = _json.get<JsonVariant>("vr_1");
  sysConfig.vr1_offset = _buffer["offset"].as<int>();
  sysConfig.vr1_score = _buffer["score"].as<int>();

  _buffer = _json.get<JsonVariant>("vr_2");
  sysConfig.vr2_offset = _buffer["offset"].as<int>();
  sysConfig.vr2_score = _buffer["score"].as<int>();


  // 基本檢查
  if (sysConfig.analysis_cycle == 0) sysConfig.analysis_cycle = 150;
  if (sysConfig.saving_cycle == 0) sysConfig.saving_cycle = 300;
  if (sysConfig.upload_cycle == 0) sysConfig.upload_cycle = 600;
  if (sysConfig.mqtt_broker == "") Serial.println("No MQTT Broker Config");
  if (sysConfig.http_server == "") Serial.println("No Restful API Config");
  if (sysConfig.https_server == "") Serial.println("No Restful APIs Config");

}

void showConfig(void)
{
  Serial.println("----- System Config -----");

  Serial.println("[ Device ]");
  Serial.print("\t fw_ver:\t"); Serial.println(sysConfig.fw_ver);
  Serial.print("\t device_id:\t"); Serial.println(sysConfig.device_id);

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
  Serial.print("\t RestFul Fromat:\t"); Serial.println(sysConfig.restful_fromat);


  Serial.println("[ Knob Setting: VR1 ]");
  Serial.print("\t offset:\t"); Serial.println(sysConfig.vr1_offset);
  Serial.print("\t score: \t"); Serial.println(sysConfig.vr1_score);

  Serial.println("[ Knob Setting: VR2 ]");
  Serial.print("\t offset:\t"); Serial.println(sysConfig.vr2_offset);
  Serial.print("\t score: \t"); Serial.println(sysConfig.vr2_score);

  Serial.println("-------------------------\r\n");
}

void showCalConfig(String _name, CAL_CONFIG &_config)
{
  Serial.print("****** ");
  Serial.print(_name);
  Serial.println(" Calibration Config ******");

  Serial.println("[ Data ]");
  Serial.print("\t [High]  raw/std: "); Serial.print(_config.x_HIGH); Serial.print("/"); Serial.println(_config.y_HIGH);
  Serial.print("\t [Mid ]  raw/std: "); Serial.print(_config.x_MID); Serial.print("/"); Serial.println(_config.y_MID);
  Serial.print("\t [Low ]  raw/std: "); Serial.print(_config.x_LOW); Serial.print("/"); Serial.println(_config.y_LOW);

  Serial.println("[ Calibration ]");
  Serial.print("\t SLOP:"); Serial.println(_config.SLOP, 4);
  Serial.print("\t INTERCEPT:"); Serial.println(_config.INTERCEPT, 4);
  Serial.print("\t R_2:"); Serial.println(_config.R_2, 8);
  Serial.println("************************");
}


// simple Linear Regression
// reference: https://www.statisticshowto.com/probability-and-statistics/regression-analysis/find-a-linear-regression-equation/
void simpleLinearRegression(CAL_CONFIG &_config)
{
  int _n = 3;
  float _sum_y = _config.y_HIGH + _config.y_MID + _config.y_LOW;
  float _sum_x = _config.x_HIGH + _config.x_MID + _config.x_LOW;
  float _sum_xy = _config.x_HIGH * _config.y_HIGH +
                  _config.x_MID * _config.y_MID +
                  _config.x_LOW * _config.y_LOW;

  float _sum_y2 = pow(_config.y_HIGH, 2) + pow(_config.y_MID, 2) + pow(_config.y_LOW, 2);
  float _sum_x2 = pow(_config.x_HIGH, 2) + pow(_config.x_MID, 2) + pow(_config.x_LOW, 2);

  _config.SLOP =  (_n * _sum_xy - _sum_x * _sum_y) /
                  (_n * _sum_x2 - pow(_sum_x, 2));           // beta_1

  _config.INTERCEPT = (_sum_y * _sum_x2 - _sum_x * _sum_xy) /
                      (_n * _sum_x2 - pow(_sum_x, 2));       // beta_0

  double _r2_denominator = (_n * _sum_x2 - pow(_sum_x, 2)) * (_n * _sum_y2 - pow(_sum_y, 2));
  _config.R_2 = (_n * _sum_xy - _sum_x * _sum_y) / sqrt(_r2_denominator); // Pearson correlation coefficient
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
    Serial.println("[NTP] 錯誤: " + String(_a) + "-" + String(_b));
  }
  return _result;
}


/***** << UTC function >>  ************************
   取得網路上時間資料
***************************************************/
// 取得NTP的封包
unsigned long sendNTPpacket(const char* host)
{
  //Serial.println("NTP Step.1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("NTP Step.2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  //Serial.println("NTP Step.3");
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(host, 123); //NTP requests are to port 123

  //Serial.println("NTP Step.4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);

  //Serial.println("NTP Step.5");
  Udp.endPacket();

  //Serial.println("NTP Step.6");
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
    Serial.print("[NTP] packet received");                  // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long secs1900 = calculateNTPTime();
    cUTC.setUTC(secs1900);
    Serial.println("...Done");
  }
  else
  {
    Serial.println("[NTP] No Packet recevied");
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
  Serial.println("[System] ");
  Serial.print(cUTC.year);  Serial.print("/");  Serial.print(cUTC.month); Serial.print("/");  Serial.print(cUTC.day);
  Serial.print(" ");
  Serial.print(padding(cUTC.hour) + ":" + padding(cUTC.minute) + ":" + padding(cUTC.second));

  Serial.print("\t");
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
    Serial.println("[NTP] 重設系統時間");
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

/***** << Atlas Scientific Module function >> *****/
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

void atlas_sleep(uint8_t _addr)
{
  Wire.beginTransmission(_addr);
  Wire.write("Sleep");
  Wire.endTransmission();
}

float get_DO(float _temp = 20.0)
{
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

void sysCMD(String _cmd)
{
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
  // 因為板子的電路會進行分壓，所以ADS1115量出來的電壓要再乘上2才會是電錶的電壓
  float _ads_Volate = 2 * _getVoltage(pH_pin);
  float pH_Value = 3.5 * _ads_Volate * 0.001;                                   // 分壓(5V->2.5V)後，用電壓換算成pH (斜率3.5的出處待確認)
  pH_Value = pH_Value * slope + intercept; // 校正用
  return pH_Value;
}

float getECmV(float _mV, float _temp = 22.0)
{
  // 溫度補償係數
  float _value;

  // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  // 先用22度做溫度補償
  float _temp_coefficient = 1.0 + 0.0185 * (_temp - 25.0);
  float _coefficient_volatge = _mV / _temp_coefficient;  // 電壓係數

  // 三個不同區間的導電度換算
  if (_coefficient_volatge < 150)
  {
    _value = 6.84 * _coefficient_volatge - 200; // EC <= 1ms/cm (暫定)
    Serial.print("[EC ] Below range!(");
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
  //  _value = 0.94 * _value + 2300;
  return _value;
}

float getEC(float slope = 1.0, float intercept = 0.0)
{
  // 因為板子的電路會進行分壓，所以ADS1115量出來的電壓要再乘上2才會是電錶的電壓
  float _ads_Volate = 2 * _getVoltage(EC_pin) ;
  float _temp = getTemp();
  EC_value = getECmV(_ads_Volate, _temp) * slope + intercept;
  if (EC_value < 0) EC_value = 0;
  return EC_value;
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


/***** << LinkIt 7697 LASS upload function >> *****
   LinkIt 7697 WIFI 上傳LASS資料用
***********************************/
String addLASS_msgTime(void)
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

  sslclient.setRootCA(rootCA, sizeof(rootCA));

  if (sslclient.connect(Host.c_str(), 443))
  {
    Serial.println("Update to LASS (GET)");
    Serial.println(getStr);
    // Make a HTTP request:
    sslclient.println(getStr);
    sslclient.println("Host : " + Host);
    sslclient.println("Accept : */*");
    sslclient.println("Connection: close");
    sslclient.println();
    delay(500);
  }
  else Serial.println("WIFI Serverconnect error");

  while (sslclient.available()) {
    char c = sslclient.read();
    Serial.write(c);
  }
  sslclient.stop();
  Serial.println();
}


void mqttLASS(String _msgTime, String _msgValue)
{
  String _topic = "LASS/Test/WaterBox_TW/" + WifiMac();

  String _DeviceID = "Field_D01_" + WifiMac();
  String _DeviceInfo = "device=Linkit7697|device_id=" + _DeviceID + "|ver_app=" + _fw + "|app=WaterBox_TW";
  String _location = "FAKE_GPS=1|gps_lat=25.029387|gps_lon=121.579060";
  String _msg = "|" + _DeviceInfo + "|" + _location + "|" + _msgTime + "|" + _msgValue + "|";


  // 切斷先前的連線
  if (mqtt.connected()) {
    mqtt_disconnect();
  }

  // 設定LASS MQTT Broker
  mqtt.setServer("gpssensor.ddns.net", 1883);
  // 重新連線
  mqtt_reconnect();

  if (mqtt.connected()) {
    Serial.println("[LASS] MQTT Connect Success");
    mqtt.publish(_topic.c_str(), _msg.c_str());
  }
  else {
    Serial.println("[LASS] MQTT Connect Fail");
  }
  Serial.println("[LASS] Publish done");
  mqtt_disconnect();

  // 更改回原本的 MQTT Broker 設定
  mqtt.setServer(sysConfig.mqtt_broker.c_str(), sysConfig.mqtt_port);
}


/***** << LinkIt 7697 IBP upload function >> *****
   LinkIt 7697 WIFI 上傳MQTT資料用
***********************************/
void mqtt_reconnect(void)
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

void mqtt_disconnect(void)
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

  OLED_content("Loading", "SD Config", 1.0, false);
  loadConfig(CONFIG_PATH);
  simpleLinearRegression(cal_ph);
  simpleLinearRegression(cal_ec);
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

  if (ssid != "") {
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
      mqtt_publish("Hi WaterBox Ver2.0!");    // MQTT 測試用
    }
    WiFi.disconnect();
  }
  LWatchDog.feed();
  atlas_sleep(97); // 關閉DO 模組的電源
  digitalWrite(modulePower, LOW);         // 關閉電源
  OLED_content("Setup", "Done", 1, false);
  Serial.println("End of Setup");
}


/***** << Main function: Loop >> *****/
void loop(void)
{
  LWatchDog.feed();
  digitalWrite(modulePower, HIGH);    // 開啟模組電源
  /*****<< 取得時間資料 >>*****/
  DateTime now = rtc.now();          // 取得目前時間
  _year = now.year();
  _month = now.month();
  //  if (now.hour() + 8 > 23) {
  //    _hour = now.hour() + 8 - 23;
  //    _day = now.day() + 1;
  //  }
  //  else {
  //    _hour = now.hour() + 8;
  //    _day = now.day();
  //  }
  _hour = now.hour();
  _day = now.day();

  _minute = now.minute();
  str_Time = convert_2digits(_hour) + ":" + convert_2digits(_minute);

  bool _mode = digitalRead(modeSwitch);

  if (_mode)
    /*****<< 進入分析運作模式 >>*****/
  {
    if (_modeStatus != _mode) {
      Serial.println("**********<< 進入分析模式 >>**********");
      _modeStatus = _mode;
      loadConfig(CONFIG_PATH);
      simpleLinearRegression(cal_ph);
      simpleLinearRegression(cal_ec);
      showCalConfig("pH", cal_ph);
      showCalConfig("EC", cal_ec);
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
      pH_value = getPH(cal_ph.SLOP, cal_ph.INTERCEPT);
      OLED_content_title(str_Time, "pH: " + String(pH_value), "Temp: " + String(Temp_value), "Analysis Mode", 1.5, false);

      digitalWrite(sensorSwitch, LOW);     // 切換到 EC
      delay(1000); // 讓Sensor 完成積分
      LWatchDog.feed();
      EC_value = getEC(cal_ec.SLOP, cal_ec.INTERCEPT);
      //      DO_value = get_DO(Temp_value);
      //      atlas_sleep(addr_DO); // 關閉DO 模組的電源

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
      Serial.print(" UTC+0");
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

      // LASS package (https)
      SensorValue[0] = Temp_value;
      SensorValue[1] = pH_value;
      SensorValue[2] = EC_value;
      SensorValue[3] = 0.0;
      SensorValue[4] = 0.0;
      SensorValue[5] = 0.0;
      SensorValue[7] = 0.0;

      String MQTT_Time = addLASS_msgTime();
      String MQTT_Value = addLASS_msgValue(SensorValue, false);

      //      updateLASS(MQTT_Time, MQTT_Value);
      //      mqttLASS(MQTT_Time, MQTT_Value);

      // Thingspeak (http)
      updateThingSpeak(sysConfig.thingspeak_key, String(Temp_value), String(pH_value), String(EC_value));

      /* MQTT 發資料*/
      String _str_Time = String(_year) + "-" + convert_2digits(_month) + "-" + convert_2digits(_day) + " " +
                         convert_2digits(_hour) + ":" + convert_2digits(_minute) + ":" + convert_2digits(now.second());

      if (sysConfig.mqtt_broker.length() > 0) {
        _mqtt_msg = "{\"dataTime\":\"" + _str_Time +
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
        //        mqtt_publish(_mqtt_msg);
        //        Serial.println("[MQTT] Publish done");
        //        mqtt_disconnect();
      }

      WiFi.disconnect();
      OLED_content_title(str_Time, "Upload", "Finish", "Analysis Mode", 1.0, false);
      Serial.println("[SYSTEM] Upload Finish");
    }

    /*****<< 更新網路時間NTP Time >>*****/
    if (_updateNTP) {
      LWatchDog.feed();
      Serial.println("[System] Update NTP");
      connectWifi();
      OLED_content_title(str_Time, "NTP", "Check", "Analysis Mode", 1.0, false);

      LWatchDog.feed();
      checkNTP(true);
      //      showNTP();
      LRTC.get();
      initRTC(true, LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());
      Serial.println("[System] NTP has updated");
      OLED_content_title(str_Time, "NTP", "Done", "Analysis Mode", 1.0, false);
      WiFi.disconnect();
    }
    
    OLED_content_title(str_Time, "SYSTEM", "STANDBY", "Analysis Mode", 1.0, true);

  } // end of if (_mode)

  /*****<< 進入系統設定模式 >>*****/
  else {
    if (_modeStatus != _mode) {
      Serial.println("**********<< 進入設定模式 >>**********");
      _modeStatus = _mode;
      showCalConfig("pH", cal_ph);
      showCalConfig("EC", cal_ec);
    }

    String newCMD = getSerial();
    if (newCMD.length() > 0) {
      sysCMD(newCMD);
    }

    /*****<< 取得ADS1115狀態 >>*****/
    pinMode(sensorSwitch, OUTPUT);        // 用USR pin 控制 pH & EC 模組電源切換
    digitalWrite(sensorSwitch, HIGH);     // 切換到 pH
    delay(1000);
    float raw_pH = getPH();
    pH_value = getPH(cal_ph.SLOP, cal_ph.INTERCEPT);

    digitalWrite(sensorSwitch, LOW);     // 切換到 pH
    delay(1000);

    float raw_EC = getEC();
    EC_value = getEC(cal_ec.SLOP, cal_ec.INTERCEPT);

    //    adc0 = ads.readADC_SingleEnded(EC_pin);
    //    delay(100);
    //    adc1 = ads.readADC_SingleEnded(pH_pin);
    //    delay(100);

    adc2 = ads.readADC_SingleEnded(Rotary_Pin_1);
    delay(450);
    adc3 = ads.readADC_SingleEnded(Rotary_Pin_2);
    delay(450);


    Serial.print("pH: "); Serial.print(raw_pH); Serial.print("/"); Serial.println(pH_value);
    Serial.print("EC: "); Serial.print(raw_EC); Serial.print("/"); Serial.println(EC_value);
    Serial.print("VR1: "); Serial.println(adc2);
    Serial.print("VR2: "); Serial.println(adc3);
    Serial.println();

    Temp_value = getTemp();

    String _state_OLDE_1 = "pH:" + String(raw_pH);
    String _state_OLDE_2 = "EC:" + String(raw_EC);
    OLED_smallContent(_state_OLDE_1, _state_OLDE_2, String(Temp_value) + " C, @ " + str_Time, 0.1, false);
  } //end of 設定模式
}
