/****<< 說明文件 >>****************************
  System Info Display
  功能：
  1.顯示版本號碼
  2.建立各狀況顯示畫面，使用String作為Parameter
**********************************************/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// End of constructor list

String _fw = "ver 2.1.a";


void OLED_msg(String _verMsg, float _delay = 0.5)
{
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
}

void OLED_content(String _msg1, String _msg2 = "", float _delay = 0.5)
{
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
}

void OLED_content_title(String _title, String _msg1, String _msg2 = "", float _delay = 0.5)
{
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
}

void OLED_smallContent(String _msg1, String _msg2 = "",String _msg3 = "", float _delay = 0.5)
{
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
}


int pinLED = 5;

void setup(void) {

  Serial.begin(9600);
  Serial.println("System Start");
  pinMode(14, OUTPUT);
  pinMode(6, OUTPUT);

  digitalWrite(14, HIGH);

  // 提示用燈號
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

  u8g2.begin();

  Serial.println("System Init Done");
}


int _c = 0;


void loop(void) {
  OLED_msg("ABCDEFGHIJ", 2);
  OLED_content("123", "456", 2);
  OLED_content_title("title","987","654",2);
  OLED_smallContent("1234567890abcd","abcd","efg",2);
}
