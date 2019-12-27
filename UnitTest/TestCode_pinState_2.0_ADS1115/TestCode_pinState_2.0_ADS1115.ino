/***** << WaterBox_V2.0:pin state test with ADS1115 >> *****
   ADC 數據及 switch切換
   使用Library Adafruit_ADS1X15 (Ver. 1.0.1)
***********************************************************/
#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

/****<< 這是舊的pin設定 >>*****************
  #define VR_1  A0
  #define VR_2  A1
  #define EC_pin        A2
  #define pH_pin        A3
*****<< 這是舊的pin設定 >>*****************/

#define power_pin   14      // Linklt 7969 以外的所有模組電源開關
#define switch_pin  5       // 模式開關 SW1
#define USR_pin     6       // 功能鍵 SW2  下拉時會關閉EC/pH的電源
#define pinLED      7       // Linklt 7969 上的USR LED || DS18B20 用腳位

/****** << 打開系統電源時要帶入的函數 >> *****/
void systemPower(bool _state = false) {
  digitalWrite(power_pin, _state);
  if (_state) {
    ads.begin();
  }
}

/*
  把特定的ADC腳位電壓，換算成0-val的數值後返回float格式
  rotary_button(指定的ADC pin, 要轉換的數值)
*/
float _analog_convert(uint8_t pin, int _val)
{
  //  int  _read = analogRead(pin);
  int  _read = ads.readADC_SingleEnded(pin);
  float _readVolt = _read *0.125;

  //  float _value = _val * (_read * 0.125) / 3350.0;     // 0 ~ _val
  float _value = map(_readVolt, 0, 3350, 0, _val);            // _val ~ 0
  _value = _val - _value;
  
  if (_readVolt > 4000) _value = -1;
  
  return _value;
}

void setup()
{
  Serial.begin(9600);

  pinMode(switch_pin, INPUT);
  pinMode(USR_pin, INPUT);
  pinMode(power_pin, OUTPUT);

  systemPower(true);

  /****<< 這是舊的pin設定 >>*****************
    pinMode(EC_pin, INPUT);
    pinMode(pH_pin, INPUT);
    pinMode(VR_1, INPUT);
    pinMode(VR_2, INPUT);
  *****<< 這是舊的pin設定 >>*****************/
    ads.begin(); //放到systemPower()裡面


  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  //  Serial.println("SW_5\t USR_6\t R1_A0\t\t R2_A1\t\t pH_A2\t\t EC_A3");
  Serial.println("SW_5\t USR_6\t pH_A2\t\t EC_A3");
}

void loop() {
  systemPower(true);

  bool _SW = digitalRead(switch_pin);
  bool _USR = digitalRead(USR_pin);

  /****<< 這是舊的pin設定 >>*****************
    int _R1 = analogRead(VR_1);
    int _R2 = analogRead(VR_2);
    int _pH = analogRead(pH_pin);
    int _EC = analogRead(EC_pin);
    *****<< 這是舊的pin設定 >>*****************/


  int _pH = ads.readADC_SingleEnded(0);
  int _EC = ads.readADC_SingleEnded(1);
  int _R1 = ads.readADC_SingleEnded(2);
  int _R2 = ads.readADC_SingleEnded(3);
  //  float _PH = _pH *0.125;
  //  float _EC = _ec *0.125;

  Serial.print(_SW);
  Serial.print("\t");
  Serial.print(_USR);
  Serial.print("\t");
  //  Serial.print(_R1); Serial.print("(" + String(_R1 * 0.125, 2) + "mV)");
  //  Serial.print("\t");
  //  Serial.print(_R2); Serial.print("(" + String(_R2 * 0.125, 2) + "mV)");
  //  Serial.print("\t");
  Serial.print(_pH); Serial.print("(" + String(_pH * 0.125, 2) + "mV)");
  Serial.print("\t");
  Serial.print(_EC); Serial.print("(" + String(_EC * 0.125, 2) + "mV)");
  Serial.println();

  float test_VR1 = _analog_convert(2, 50);
  float test_VR2 = _analog_convert(3, 200);
  Serial.print("VR1:\t");
  Serial.print(test_VR1);
  Serial.print("\tVR2:\t");
  Serial.println(test_VR2);

  delay(500);  // 時間間隔太少會導致錯位

}
