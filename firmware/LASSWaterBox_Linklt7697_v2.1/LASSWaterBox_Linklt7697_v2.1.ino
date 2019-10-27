/**** << WaterBox_V2.1: 說明 >> *****
     TODO：操作介面
     TODO：充放電電流量測
     TODO：續航力測試
***********************************************************/
#include <Wire.h>             // I2C library
#include <Adafruit_ADS1015.h> // ADS1115 library

#include <RTClib.h>           // DS3231 library
#include <OneWire.h>          // DS18B20 library

#include <SPI.h>              // for SD Card
#include <SD.h>               // SD Card library

#include <LoRa.h>              // LoRa 函式庫
#include <EEPROM.h>            // EEPROM in 7697
//#include <ArduinoJson.h>     // JSON 函式庫


/**** << WaterBox_V2.1: pin state test with ADS1115 >> *****
     ADC 數據及 switch 切換
     使用Library Adafruit_ADS1X15 (Ver. 1.0.1)
***********************************************************/

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

//#include "LASSWaterBox.h" 還沒定義好
// 腳位設定
#define power_pin   14      // Linklt 7969 以外的所有模組電源開關
#define switch_pin  5
#define USR_pin     6
#define DS18B20_Pin 7

// CS pin for SD
#define SD_CS       4

// CS pin for LoRa  VDD:3.3V
#define LoRa_DIO0   2 // only pin 1,2,3,
#define LoRa_Reset  3
#define LoRa_CS    10

// 全域變數設定
#define ADC_CAL 3350


/**** << WaterBox_V2.1:LinkIt 7697 睡眠用library >> *****
   說明：LinkIt 7697 睡眠功能設定
   Library： "LSleep.h"
***********************************************************/
// 省電用 library
#include "LSleep.h"
LSleepClass Sleep;

// 重新定義Serial
HardwareSerial & ToSerial = Serial;


/**** << WaterBox_V2.1:系統說明文字 >> *****
   韌體版本：
   說明文字：
***********************************************************/
const char* _firwareVersion = "Ver 2.1.a";


/**** << WaterBox_V2.1:變數設定(全大寫) >> *****
   水質參數
   量測間隔
   睡眠間隔
***********************************************************/
float pH_slop, pH_intercept, EC_slop, EC_intercept, pH_alarm, EC_alarm;
int print_interval = 1000;


/**** << WaterBox_V2.1:DS3231 時間模組 >> *****
    功能：時間模組+EEPROM
    library：RTClib.h
    版本：1.2.0
***********************************************************/
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


/**** << WaterBox_V2.1:DS18B20 溫度模組 >> *****
    library：OneWire.h
    版本：2.3.4
***********************************************************/
OneWire ds(DS18B20_Pin);


/**** << WaterBox_V2.1:SSD1306 OLED顯示模組 >> *****
    library：OneWire.h
    版本：2.3.4
***********************************************************/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

uint8_t draw_color = 1;


/**** << WaterBox_V2.1:主程式Loop設定 >> *****
    版本：2.1.a
***********************************************************/
int chapter = 0;                               //  大項目：0~1(校正設定,時間設定)
int item = 0;                                  //  子項目：0~3(準備,參數1設定,參數2設定,參數3設定)
bool config_state = true;                      //  true時只能顯示目前設定，flase時可以改設定
int _YY, _year_1, _year_2, _MM, _DD, _HH, _mm; //  時間日期調整用


/**** << WaterBox_V2.1:SSD1306 OLED顯示用functin >> *****

********************************************************/
void OLED_result_u8g2(String _s_d, String _s_1, String _s_2 = "", float _sec = 0.5)
{
  u8g2.begin();

  const char* _date = _s_d.c_str();
  const char* _str_1 = _s_1.c_str();
  const char* _str_2 = _s_2.c_str();

  int _delay = (int) (_sec * 1000);

  u8g2.clearBuffer();
  // 10 Pixel Height
  u8g2.setFont(u8g2_font_8x13B_tf);          // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(4, 12, ">> WATER BOX <<");    // 16-15 = 1, 1 * 8 = 8 -> 4 開始

  u8g2.setFont(u8g2_font_timR08_tr);         // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(0, 24, _date);                // 靠左開始

  // 14 Pixel Height
  u8g2.setFont(u8g2_font_helvB14_tr);        // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(0, 40, _str_1);              // 16-13 = 3, 3*8=24 -> 12 開始
  u8g2.drawStr(0, 56, _str_2);
  u8g2.sendBuffer();
  delay(_delay);
}

void OLED_status_u8g2(String _m_1, String _m_2 = "", float _sec = 0.5)
{

  u8g2.begin();

  const char* _msg_1 = _m_1.c_str();
  const char* _msg_2 = _m_2.c_str();
  int _fw_length = 128 - 8 * strlen(_firwareVersion);
  int _delay = (int) (_sec * 1000);

  u8g2.clearBuffer();
  // 10 Pixel Height
  u8g2.setFont(u8g2_font_8x13B_tf);       // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(24, 12, ">> LASS <<");    // 16-10 = 6, 6*8=48 -> 24 開始

  u8g2.setFont(u8g2_font_timR08_tr);         // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(_fw_length, 24, _firwareVersion); // 靠左開始

  // 14 Pixel Height
  u8g2.setFont(u8g2_font_helvB14_tr);      // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(0, 40, _msg_1);            // 16-13 = 3, 3*8=24 -> 12 開始
  u8g2.drawStr(0, 56, _msg_2);
  u8g2.sendBuffer();
  delay(_delay);
}

void OLED_setting_u8g2(String _m_1, String _m_2, String _m_3, float _sec = 0.5)
{

  u8g2.begin();

  const char* _msg_1 = _m_1.c_str();
  const char* _msg_2 = _m_2.c_str();
  const char* _msg_3 = _m_3.c_str();
  int _fw_length = 128 - 8 * strlen(_firwareVersion);
  int _delay = (int) (_sec * 1000);

  u8g2.clearBuffer();
  // 10 Pixel Height
  u8g2.setFont(u8g2_font_8x13B_tf);                               // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(24, 12, ">> LASS <<");                             // 16-10 = 6, 6 * 8=48 -> 24 開始

  u8g2.setFont(u8g2_font_timR08_tr);         // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(0, 24, _msg_1);                                   // 一行12個字母
  u8g2.drawStr(_fw_length, 36, _firwareVersion);                   // 靠左開始

  u8g2.setFont(u8g2_font_helvB14_tr);      // 一個字母8 Pixel 寬，行高12
  u8g2.drawStr(0, 48, _msg_2);
  u8g2.drawStr(0, 60, _msg_3);
  u8g2.sendBuffer();
  delay(_delay);
}



/**** << WaterBox_V2.1:數據處理用funtion >> *****

********************************************************/
double averge_array(int* arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;

  if (number <= 0)
  {
    ToSerial.println("Error number for the array to avraging!/n");
    return 0;
  }

  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else {            // 大於5個資料時的處理方式

    if (arr[0] < arr[1]) {        // 確認前一筆的資料比較小
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    } // end of if

    // 過濾最大級最小的兩筆資料，不納入計算
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {   // 低於下限值以下限值計算，並更新下限值
        amount += min;      // arr < min
        min = arr[i];
      } else {
        if (arr[i] > max) { // 高於上限值以上現值計算，並更新上限值
          amount += max;    // arr > max
          max = arr[i];
        } else {            // 未達上下現值，直接納入計算
          amount += arr[i]; // min<=arr<=max
        }
      } //if
    } //for

    avg = (double)amount / (number - 2);

  }//if

  return avg;
}


/**** << WaterBox_V2.1:ADS1115轉換數值用funtion >> *****
  把特定的ADC腳位電壓，換算成0-val的數值後返回float格式
  rotary_button(指定的ADC pin, 要轉換的數值)
********************************************************/
float _analog_convert(uint8_t pin, int _val)
{
  //  int  _read = analogRead(pin);
  int  _read = ads.readADC_SingleEnded(pin);
  float _readVolt = _read * 0.125;

  float _value = map(_readVolt, 0, ADC_CAL, 0, _val);            // _val ~ 0
  _value = _val - _value;

  if (_readVolt > 4000) _value = -1;

  return _value;
}


/**** << WaterBox_V2.1:ADS1115轉換數值用funtion >> *****
  濁度(Tubidity)換算：讀取ADS1115某個pin上的電壓(A0)
********************************************************/
int getVoltage(int _ch)
{
  int adc = ads.readADC_SingleEnded(_ch);
  int mV = adc * 0.125;
  return mV;
}

double getTubidity(int _mV)
{
  double voltage = _mV * 0.001;
  //  double tubidity = -1120.4 * voltage * voltage + 5742.3 * voltage - 4352.9;    // 官方公式
  double tubidity = 1071.2 * voltage * voltage - 6418.5 * voltage + 9952.1;     // 實驗結果 Tubidity<800 NTU

  return tubidity;
}

double Tubidity_value()
{
  int _ads_Volate = getVoltage(0);
  return getTubidity(_ads_Volate);
}


/**** << WaterBox_V2.1:ADS1115轉換數值用funtion >> *****
  導電度(Conductivity)換算：讀取ADS1115某個pin上的電壓(A1)
********************************************************/
int EC_value(float temperature = 25.0, float slope = 1.0, float intercept = 0.0)
{
  long _value_total = 0;
  long _sample_time = millis(); // 初始化採樣時間
  long _t = millis();           // 計時用

  // 在800ms內，每50ms連續取樣放到array中
  float _value = _analog_convert(1, 5000); // 還原成真正的訊號輸出電壓

  // 溫度補償係數
  float _temp_coefficient = 1.0 + 0.0185 * (temperature - 25.0);      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  float _coefficient_volatge = _value / _temp_coefficient;  // 電壓係數

  // 三個不同區間的導電度換算
  if (_coefficient_volatge < 150)       {
    ToSerial.print("No solution!(");
    ToSerial.print(_value);
    ToSerial.println(")");
    _value = 6.84 * _coefficient_volatge - 200; // EC <= 1ms/cm (暫定)
    if (_value < 0) _value = 0;
  }
  else if (_coefficient_volatge > 3300) {
    ToSerial.println("Out of the range!(");
    ToSerial.print(_value);
    ToSerial.println(")");
    _value = 5.3 * _coefficient_volatge + 2278; // 20ms/cm<EC (暫定)
  }
  else
  {
    if (_coefficient_volatge <= 448)        _value = 6.84 * _coefficient_volatge - 64.32; // 1ms/cm<EC<=3ms/cm
    else if (_coefficient_volatge <= 1457)  _value = 6.98 * _coefficient_volatge - 127;   // 3ms/cm<EC<=10ms/cm
    else                                    _value = 5.3 * _coefficient_volatge + 2278;   // 10ms/cm<EC<20ms/cm
  }

  // 手動線性校正
  _value = _value * slope + intercept;

  _t = millis() - _t;   // 結算分析時間
  delay(1000 - _t);

  ToSerial.println("EC分析時間 >> " + (String)_t + "(ms)");

  return _value;
}


/**** << WaterBox_V2.1:DS18B20轉換數值用funtion >> *****
   TempProcess(bool ch)
   ch 為 true時讀取溫度，反之檢查溫模組並進入準備狀態
********************************************************/
float TempProcess(bool ch)
{
  //returns the temperature from one DS18B20 in DEG Celsius
  static byte data[12];
  static byte addr[8];
  static float TemperatureSum;
  if (!ch) {
    if ( !ds.search(addr)) {
      ToSerial.println("no more sensors on chain, reset search!");
      ds.reset_search();
      return 0;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      ToSerial.println("CRC is not valid!");
      return 0;
    }
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      ToSerial.print("Device is not recognized!");
      return 0;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end
  }
  else {
    byte present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad
    for (int i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = ds.read();
    }
    ds.reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    // 原本OneWire函式庫中有另外進行位數的檢查，這邊忽略

    TemperatureSum = tempRead / 16; // 攝氏溫度的計算
  }
  return TemperatureSum;
}

/*
   讀取溫度資料
  ch=0,let the DS18B20 start the convert;ch=1,MCU read the current temperature from the DS18B20.
*/

float temp_value()
{
  float _temp  = TempProcess(true);  // read the current temperature from the  DS18B20
  TempProcess(false);                // after the reading,start the convert for next reading
  // 讓DS18B20進入準備狀態(檢查模組)
  return _temp;
}


/**** << WaterBox_V2.1:控制介面用function >> *****
   計算某pin上拉一次花多少秒(0.1)
   並設定上限秒數超過自動跳出回傳
********************************************************/
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


/**** << WaterBox_V2.1:SD卡相關用funton >> *****
********************************************************/
bool InitSD()
{
  bool _state = true;

  ToSerial.print("初始化 SD card...");
  if (!SD.begin(SD_CS)) {
    ToSerial.println("初始化失敗，請檢查SD卡是否插入");
    _state = false;
  }
  ToSerial.println("SD卡初始化完成");
  return _state;
}

void save2SD(int Y, int M, int D, int h, int m, float t, float ph, float ec, float pH_mV)
{

  String file = String(Y) + convert_2digits(M) + convert_2digits(D) + ".csv";
  String Header = "Date,Time,Tempture(C),pH,EC,pH_mV";
  String data = String(Y) + "/" + convert_2digits(M) + "/" + convert_2digits(D) + "," + \
                convert_2digits(h) + ":" + convert_2digits(m) + "," + \
                String(t) + "," + String(ph) + "," + String(ec)  + String(pH_mV);

  ToSerial.println("寫入SD卡中");

  if (SD.exists(file)) {
    File myFile = SD.open(file, FILE_WRITE);   // open file
    if (myFile) {
      myFile.println(data);
      myFile.close();
    }
    else {
      ToSerial.println("SD卡檔案開啟錯誤 " + file);
    }
  } else {
    File myFile = SD.open(file, FILE_WRITE);   // open a new file and add the table header:
    if (myFile) {
      myFile.println(Header);
      myFile.println(data);
      myFile.close();
    }
    else {
      ToSerial.println("SD卡檔案建立錯誤 " + file);
    }
  }
}


/**** << WaterBox_V2.1:LoRa相關用funton >> *****
********************************************************/
bool InitLoRa()
{
  bool _state = true;
  LoRa.setPins(LoRa_CS, LoRa_Reset, LoRa_DIO0);
  ToSerial.println(F("LoRa Sender init"));
  if (!LoRa.begin(915E6)) {
    ToSerial.println(F("Starting LoRa failed!"));
    _state = false;
  }
  return _state;
}


/**** << WaterBox_V2.1:睡眠模式設定用funtion >> *****
********************************************************/
void enterSleep(int _ms, int _mode)
{
  float _sleepTime = _ms * 0.001;
  Sleep.init(true);
  Sleep.setTime(_ms);       // 設定時間
  Sleep.setMode(_mode);     // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  Serial.print("晚安(");
  Serial.print(_sleepTime, 3);
  Serial.println(" s)");
  Sleep.sleep();
  Serial.println("起床");
}


/**** << WaterBox_V2.1:系統設定相關funtion >> *****
********************************************************/
bool EEPROM_write(char* _str, int _page, int _length) // 寫入資料，1頁 32 bytes
{
  int _address = _page * 32;
  if (_length > 31) {                // 超出頁面
    ToSerial.println("Out Of Pages");
    return false;
  }
  else {
    ToSerial.print("Writing data：");
    for ( int _i = 0; _i < _length; _i++ ) {
      EEPROM.update(_address + _i, _str[_i]);
      ToSerial.print(_str[_i]);
    }
    ToSerial.println();
    return true;
  } // end of if
} // end of EEPROM_write()


String EEPROM_read(int _page, int _length) // 讀取資料，1頁 30 bytes
{
  int _address = _page * 32;
  char _str;
  String read_buffer = "";

  if (_length > 31) {                         // 超出頁面
    ToSerial.println("Out Of Pages");
  }
  else {
    for ( int _i = 0; _i < _length; _i++ ) {
      _str = EEPROM.read(_address + _i);
      read_buffer += (String)_str;
    }
  }
  return read_buffer;
} // end of EEPROM_read()


void _config(bool _state = true)
{
  String _buffer;
  char _char_buffer[30];

  if (_state)  ToSerial.println("讀取設定....");
  else        ToSerial.println("開始更新設定");

  for (int _i = 0; _i < 6; _i++) {

    if (_state) {
      _buffer = EEPROM_read(_i, 10);    // Linklt 7697 內部的EEPROM
      ToSerial.print(_buffer);
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

      ToSerial.print("寫入資料");
      ToSerial.println(_char_buffer);
      EEPROM_write(_char_buffer, _i, _buffer.length() + 1);
      delay(50);
    }
    delay(100);
  }
  ToSerial.print("設定完成");
}

bool alarm_check(float pH, int EC)
{
  bool _pH_alarm = false, _EC_alarm = false;
  if (pH < 7 - pH_alarm || pH > 7 + pH_alarm)  _pH_alarm = true;
  if (EC > EC_alarm)  _EC_alarm = true;
  return _pH_alarm || _EC_alarm;
}


/**** << WaterBox_V2.1:主程式setup設定 >> *****
 *  主程式啟動設定
*******************************************/
void setup()
{
  int _count = 0;

  //Switch pin 初始化
  pinMode(switch_pin, INPUT);
  delay(100);

  //pH EC 切換用的控制腳位
  pinMode(USR_pin, OUTPUT);               // 設定EC/pH切換控制IO

  pinMode(LoRa_CS, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(LoRa_CS, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  // OLED 初始化
  u8g2.begin();
  OLED_status_u8g2("System", "Initialize", 1.0);

  // ToSerial 初始化
  ToSerial.begin(9600);
  while (!ToSerial) {
    _count++;
    if (_count > 1000) break;
  }
  OLED_status_u8g2("Serial Port", "CHECKED", 1.0);

  // RTC 初始化
  if (rtc.lostPower()) {
    ToSerial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // 確認DS18B20模組並設定為準備狀態
  TempProcess(false);

  // SD card 初始化
  if (InitSD())  OLED_status_u8g2("SD Module", "CHECKED", 1.0);
  else           OLED_status_u8g2("SD Module", "FAILED", 1.0);


  // LoRa 初始化
  if (InitLoRa())  OLED_status_u8g2("LoRa Module", "CHECKED", 1.0);
  else            OLED_status_u8g2("LoRa Module", "FAILED", 1.0);

  OLED_status_u8g2("Loading", "Config", 1.0);
  _config(true); // 讀取目前的設定

  ToSerial.println("系統設定初始化完成");
  OLED_status_u8g2("System", "DONE" , 1.0);


  // 睡眠測試
  ToSerial.println(F("睡覺測試"));
  delay(10);
  OLED_status_u8g2("Sleep Test", "1 sec" , 1.0);
  u8g2.setPowerSave(1);
  enterSleep(1000, 3);
  u8g2.setPowerSave(0);
  OLED_status_u8g2("Sleep Test", "wake up" , 1.0);
}


/**** << WaterBox_V2.1:主程式Loop設定 >> *****
 *  主程式Loo[設定
*******************************************/
bool test_code = true;
void loop() {
  bool _switch_mode = digitalRead(switch_pin);

  DateTime now = rtc.now();          // 取得目前時間
  int _year = now.year();
  int _month = now.month();
  int _day = now.day();
  String _days = daysOfTheWeek[now.dayOfTheWeek()];
  int _hour = now.hour();
  int _minute = now.minute();
  bool _SD_save = now.unixtime() % 300 == 0;  //  5分鐘存檔一次
  bool _upload = now.unixtime() % 1 == 0;   // 10分鐘上傳一次
  String _str_time = (String)_year + "/" + convert_2digits(_month) + "/" + convert_2digits(_day) + " " + convert_2digits(_hour) + ":" + convert_2digits(_minute);

  //  if (_switch_mode) // 分析模式
  if (true)
  {
    ToSerial.println("分析模式");

    pinMode(USR_pin, OUTPUT);     // 用USR pin 控制 pH & EC 模組電源切換

    float temperature = temp_value();
    digitalWrite(USR_pin, HIGH);     // 用USR pin 控制 pH & EC 模組電源切換
    delay(1000);

    int EC = EC_value(temperature, EC_slop, EC_intercept);
    digitalWrite(USR_pin, LOW);     // 用USR pin 控制 pH & EC
    delay(1000);

    String _str_temp = "temp:" + (String)temperature;
    OLED_result_u8g2(_str_time, _str_temp, "", 1.5);

    String _str_pH;
    String _str_EC = "EC:" + (String)EC;
    OLED_result_u8g2(_str_time, _str_pH, _str_EC);

    String _Msg = "";

    if (_SD_save)  {
      OLED_result_u8g2(_str_time, "Save Data");
      //      save2SD(_year, _month, _day, _hour, _minute, temperature, pH_Value, EC, pH_mV);
      delay(500);

    }
    else delay(1000);

    /*
      DynamicJsonDocument jsonBuffer;
      JsonObject root = jsonBuffer.to<JsonObject>();

      root["ID"] = "irrigation_gate_001";                 // 閘門的ID

      //    if (alarm_check(pH_Value, EC)) root["code"] = "U";   // 警報時拉起
      if (test_code) {                              // 測試用 code
      root["code"] = "U";
      test_code = false;
      }
      else {
      root["code"] = "D";                          // 平時拉下
      test_code = true;
      }
      root["time"] = 30;                                // 運轉30秒

      serializeJson(root, _Msg);      // 轉換成String
      delay(500);
      /*
        LoRa.beginPacket();         // send packet
        LoRa.print(_Msg);
        LoRa.endPacket();
    */
    delay(500);

    // 把測值輸出到序列埠上

    // 進入睡眠狀態


  } // end of if (_switch_mode)

  else {
    pinMode(USR_pin, INPUT);                                      // 用USR pin 作為按鈕輸入
    unsigned long printTime = millis();                            // Serial印出時間計時器
    float _time = pull_time(USR_pin, 3);                           // 設定按鈕計時器
    String _state_str, _config_str;                                // Serial輸出用
    String _state_OLDE_1, _state_OLDE_2, _state_str_OLED;             // OLED 顯示用

    if (_time > 0 && _time < 1) {       // 切換子項目
      if (item == 3) item = 0;          // 到最後一項時跳回第一項
      else item += 1;                   // 跳到下一項
    }
    else if (_time > 1 && _time < 2) {  // 切換大項目
      if (chapter == 1) chapter = 0;    // 到最後一項時跳回第一項
      else chapter += 1;                // 跳到下一項
    }
    else if (_time >= 3) {              // 切換設定顯示/調整
      if (config_state) config_state = false;                          // 切換到設定調整狀態
      else  {                                                          // 切回設定顯示狀態
        config_state = true;
        _config(false);                                                // 儲存目前的設定
        if (_YY != 0 && _MM != 0 && _DD != 0 && _HH != 0 && _mm != 0)  rtc.adjust(DateTime(_YY, _MM, _DD, _HH, _mm, 0)); // 設定時間
      }
    }

    if (printTime % print_interval <= print_interval * 0.05) {
      if (config_state) {
        ToSerial.println("設定模式：顯示目前設定");
      }
      else {
        ToSerial.println("設定模式：調整設定");
      }
    }
    // 顯示目前的項目
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
            //            int _year_1 = (int)_analog_convert(Rotary_Pin_1, 9);
            //            int _year_2 = (int)_analog_convert(Rotary_Pin_2, 9);
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
            //            pH_slop = 0.8 + _analog_convert(Rotary_Pin_1, 400) * 0.001;
            //            pH_intercept = -0.5 + _analog_convert(Rotary_Pin_2, 100) * 0.01;
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
            //            _MM = (int)_analog_convert(Rotary_Pin_1, 11) + 1;
            //            _DD = (int)_analog_convert(Rotary_Pin_2, 30) + 1;
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
            //            EC_slop = 0.8 + _analog_convert(Rotary_Pin_1, 400) * 0.001;
            //            EC_intercept = _analog_convert(Rotary_Pin_2, 1000) - 500;
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
            //            _HH = (int)_analog_convert(Rotary_Pin_1, 23);
            //            _mm = (int)_analog_convert(Rotary_Pin_2, 59) + 1;
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
            _state_OLDE_1 = "pH's limit: " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's limit: " + (String)EC_alarm;
          }
          else {
            _state_str = "上下限設定";
            //            pH_alarm = _analog_convert(Rotary_Pin_1, 20) * 0.1;
            //            EC_alarm = _analog_convert(Rotary_Pin_2, 1000);
            _config_str = "pH上下限:7.00±" + (String)pH_alarm + "\t EC上限：" + (String)EC_alarm;
            _state_str_OLED = "Setting limit";
            _state_OLDE_1 = "pH's limit: " + (String)(7 - pH_alarm) + "-" + (String)(7 + pH_alarm);
            _state_OLDE_2 = "EC's limit: " + (String)EC_alarm;
          }
        }
        break;
    }

    if (printTime % print_interval <= print_interval * 0.05 ) {
      ToSerial.println("目前頁面:" + (String)chapter + "-" + (String)item);
      ToSerial.println("\t " + _state_str);
      ToSerial.println("\t 目前設定：" + _config_str);
      ToSerial.println("*******************************\n\r\n\r ");
    }

    OLED_setting_u8g2(_state_str_OLED, _state_OLDE_1, _state_OLDE_2, 0.0);
  } //end of 設定模式
}
