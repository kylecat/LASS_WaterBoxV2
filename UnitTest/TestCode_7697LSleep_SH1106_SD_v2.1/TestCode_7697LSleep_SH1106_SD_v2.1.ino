/***** << WaterBox_V2.0:LSleep with OLED >> *****
  PCB borad：WaterBox V2.0
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

void drawLogo(void)
{
  uint8_t mdy = 0;
  if ( u8g2.getDisplayHeight() < 59 )
    mdy = 5;


  u8g2.setFontMode(1);  // Transparent
  u8g2.setDrawColor(1);
#ifdef MINI_LOGO

  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_inb16_mf);
  u8g2.drawStr(0, 22, "U");

  u8g2.setFontDirection(1);
  u8g2.setFont(u8g2_font_inb19_mn);
  u8g2.drawStr(14, 8, "8");

  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_inb16_mf);
  u8g2.drawStr(36, 22, "g");
  u8g2.drawStr(48, 22, "\xb2");

  u8g2.drawHLine(2, 25, 34);
  u8g2.drawHLine(3, 26, 34);
  u8g2.drawVLine(32, 22, 12);
  u8g2.drawVLine(33, 23, 12);
#else

  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_inb24_mf);
  u8g2.drawStr(0, 30 - mdy, "U");

  u8g2.setFontDirection(1);
  u8g2.setFont(u8g2_font_inb30_mn);
  u8g2.drawStr(21, 8 - mdy, "8");

  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_inb24_mf);
  u8g2.drawStr(51, 30 - mdy, "g");
  u8g2.drawStr(67, 30 - mdy, "\xb2");

  u8g2.drawHLine(2, 35 - mdy, 47);
  u8g2.drawHLine(3, 36 - mdy, 47);
  u8g2.drawVLine(45, 32 - mdy, 12);
  u8g2.drawVLine(46, 33 - mdy, 12);

#endif
}

void drawURL(void)
{
#ifndef MINI_LOGO
  u8g2.setFont(u8g2_font_4x6_tr);
  if ( u8g2.getDisplayHeight() < 59 )
  {
    u8g2.drawStr(89, 20 - 5, "github.com");
    u8g2.drawStr(73, 29 - 5, "/olikraus/u8g2");
  }
  else
  {
    u8g2.drawStr(1, 54, "github.com/olikraus/u8g2");
  }
#endif
}

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

void SystemTest_OLED(void)
{
  u8g2.begin();
  u8g2.setFlipMode(0);
  u8g2.setPowerSave(0); // 打開螢幕
  u8g2.firstPage();
  do {
    drawLogo();
    drawURL();
  } while ( u8g2.nextPage() );
  delay(3000);
  u8g2.setPowerSave(1); // 關閉螢幕
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

  SystemTest_OLED();

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



void loop(void) {

  digitalWrite(modulePower, HIGH); // 開幾電源

  SystemTest_GPIO(_c);
  SystemTest_OLED();
  SystemTest_SD(_c);

  digitalWrite(modulePower, LOW);  // 關閉電源
  delay(1000);

  Serial.print("睡覺Loop:(");
  Serial.print(_c);
  Serial.println(")(300 sec)");

  digitalWrite(pinLED, LOW);
  SystemTest_Sleep(300000, 3);
  digitalWrite(pinLED, HIGH);
  delay(1000);

  _c++;
}
