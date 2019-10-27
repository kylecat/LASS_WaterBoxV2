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


/***** << firmware Version >> *****/
String _fw = "Ver 2.1.a";
String Description_Tittle = ">> LASS IS YOURS <<";
String Description_Firware = "The DEVICE FIRWARE: " + String(_fw);

/***** << 系統參數 >> *****/
#define pinLED 7
#define modulePower 14

int _c = 0;

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


/***** << GPIO function >> *****/
void SystemTest_GPIO(int _loop)
{
  Serial.print("LOOP:\t" + String(_loop));
  Serial.print("\tOn");
  delay(1000);
  digitalWrite(6, LOW);
  Serial.print("\tEC");
  delay(1000);
  digitalWrite(6, HIGH);
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
  Sleep.init(true);
  Sleep.setTime(_ms);       // 設定時間
  Sleep.setMode(_mode);     // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  Serial.println("晚安");
  Sleep.sleep();
  Serial.println("起床");
}


/***** << Main function: Setup >> *****/
void setup(void)
{
  Serial.begin(9600);
  Serial.println("System Start");

  pinMode(modulePower, OUTPUT);     //
  digitalWrite(modulePower, HIGH);  // 開啟模組電源
  pinMode(6, OUTPUT);               // 設定EC/pH切換控制IO

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

  SystemTest_GPIO(_c);
  OLED_content("Hello", "LASS", 1);
  SystemTest_SD(_c);

  digitalWrite(modulePower, LOW);  // 關閉電源
  delay(1000);

  Serial.print("睡覺Loop:(");
  Serial.print(_c);
  Serial.println(")(10 sec)");
  digitalWrite(pinLED, LOW);
  SystemTest_Sleep(10000, 3);
  digitalWrite(pinLED, HIGH);
  delay(1000);

  _c++;
}
