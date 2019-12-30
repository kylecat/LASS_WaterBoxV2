/***<< license >>*****************************************
   Author：Kyle
   Licenses：MIT
   Version：1.0 (2018507)
*********************************************************/
/***<< Revision >>*****************************************
   Update：
   Ver 2.0
   1.PCB 版第二次修改V2.0
   1.1 電路變更：
      a.用ADS1115 取代原本的A0~A3(A0->Rotary 1, A1->Rotary 2, A2->pH, A3->EC)
      b.用P14 控制所有模組的電源供應，下拉時關閉
   1.2 函數變更
      void OLED_state(String, String, String, float) -> 增加版本顯示(第二行靠右)
      資料封包：未定
      資料上傳方式：未定


   2.外部函式庫
   2.1  U8g2.h(ver 7697)                     SH1106 (螢幕顯示)
   2.2  OneWire.h(Ver 2.3.4)                 DS18B20 (溫度)
   2.3  SPI.h SD.h (ver 7697)                SD卡
   2.4  RTClib.h(ver 1.2.0)                  DS3231  (時間模組)
   2.5  LoRa.h (Ver 0.5.0，修改library)      LoRa(通訊模組)
   2.6  ArduinoJson(Ver 6.11.1)              
   2.7  LSleep(自幹)                         Linklt 7697 睡眠功能

   3. 腳位設定
   3.1  A0->P14 Module Power Switch
   3.2  A1：no used
   3.3  A2：no used
   3.4  A3：no used
   3.5  P7：DS18B20
   3.6  P5：Mode Switch Pin
   3.7  P6：sensor Switch
   3.5  P4：SD_SPI_CS
   3.6  P2, P3：LoRa
   3.7  P10：LoRa_SPI_CS
   3.8  P14：Module Power Swtich

   4. EEPROM頁面設定
   4.1  P0: pH_slop
   4.2  P1: pH_intercept
   4.3  P2: EC_slop
   4.4  P3: EC_intercept
   4.5  P4: pH_alarm 
   4.6  P5: EC_alarm

*********************************************************/
/***<< Revision history Ver 1.1>>************************
   Update：
   V 1.1
   1.PCB 版第一次修改V1.1
   2.外部函式庫
   2.1  U8g2.h        SSD1306 (螢幕顯示)
   2.2  OneWire.h     DS18B20 (溫度)
   2.3  SPI.h SD.h    SD卡
   2.4  RTClib.h      DS3231  (時間模組)
   2.5  LoRa.h        LoRa(通訊模組)
   3. 腳位設定
   3.1  A0：Rotary button 1
   3.2  A1：Rotary button 2
   3.3  A2：pH meter
   3.4  A3：EC meter
   3.5  P7：DS18B20
   3.6  P5：SwitchPin
   3.7  P6：Relay control(Reserved) sensor電源控制
   3.5  P4：SD SPI CS
   3.6  P2, P3, P10：LoRa
   3.6  其他：SPI, I2C
   4. EEPROM頁面設定
   4.1  P0: pH_slop
   4.2  P1: pH_intercept
   4.3  P2: EC_slop
   4.4  P3: EC_intercept
   4.5  P4: pH_alarm 
   4.6  P5: EC_alarm
*********************************************************/
/***<< Revision history Ver 1.0>>************************
   Update：
   V 1.0
   1.專案開始
   2.外部函式庫
   2.1  U8g2.h        SSD1306 (螢幕顯示)
   2.2  OneWire.h     DS18B20 (溫度)
   2.3  SPI.h SD.h    SD卡
   2.4  RTClib.h      DS3231  (時間模組)
   3. 腳位設定
   3.1  A0：pH meter
   3.2  A1：EC meter
   3.3  A2：Rotary button 1
   3.4  A3：Rotary button 2
   3.5  P2：DS18B20
   3.6  P3：SwitchPin
   3.7  P4：Relay control(Reserved) sensor電源控制
   3.5  P5：(Reserved)
   3.6  其他：SPI, I2C
   4. EEPROM頁面設定
   4.1  P0: pH_slop
   4.2  P1: pH_intercept
   4.3  P2: EC_slop
   4.4  P3: EC_intercept
   4.5  P4: pH_alarm 
   4.6  P5: EC_alarm
*********************************************************/
