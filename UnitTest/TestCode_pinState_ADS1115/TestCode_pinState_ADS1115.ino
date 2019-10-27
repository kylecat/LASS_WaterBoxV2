/*
   ADC 數據及 switch切換
   使用Library Adafruit_ADS1X15
*/

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

#define switch_pin  5       // 模式開關 SW2
#define USR_pin     6       // 功能鍵 SW1  下拉時會關閉EC的電源

void setup() {
  Serial.begin(9600);
  while (!Serial)
  {
    delay(1);
  }

  pinMode(switch_pin, INPUT);
  pinMode(USR_pin, INPUT);

/****<< 這是舊的pin設定 >>*****************
  pinMode(EC_pin, INPUT);
  pinMode(pH_pin, INPUT);
  pinMode(VR_1, INPUT);
  pinMode(VR_2, INPUT);
*****<< 這是舊的pin設定 >>*****************/
  ads.begin();

  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  Serial.println("SW_5\t USR_6\t R1_A0\t R2_A1\t pH_A2\t EC_A3");
}

void loop() {
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
  

  Serial.print(_SW);
  Serial.print("\t");
  Serial.print(_USR);
  Serial.print("\t");
  Serial.print(_R1);
  Serial.print("\t");
  Serial.print(_R2);
  Serial.print("\t");
  Serial.print(_pH);
  Serial.print("\t");
  Serial.print(_EC);
  Serial.println();

  delay(500);

}
