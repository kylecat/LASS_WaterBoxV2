/*  Linklt 7697 使用時須修該LoRa.cpp 第331行

  #ifndef ARDUINO_SAMD_MKRWAN1300
  void LoRaClass::onReceive(void(*callback)(int))
  {
  _onReceive = callback;

  if (callback) {
    pinMode(_dio0, INPUT);

    writeRegister(REG_DIO_MAPPING_1, 0x00);
  #ifdef SPI_HAS_NOTUSINGINTERRUPT
    SPI.usingInterrupt(digitalPinToInterrupt(_dio0));
  #endif
  attachInterrupt(_dio0, LoRaClass::onDio0Rise, RISING);                                // 將digitalPinToInterrupt() 去除
    //attachInterrupt(digitalPinToInterrupt(_dio0), LoRaClass::onDio0Rise, RISING);
  } else {
  detachInterrupt(_dio0);
    //detachInterrupt(digitalPinToInterrupt(_dio0));                                    // 將digitalPinToInterrupt() 去除
  #ifdef SPI_HAS_NOTUSINGINTERRUPT
    SPI.notUsingInterrupt(digitalPinToInterrupt(_dio0));
  #endif
  }
  }
*/
#include <SPI.h>
#include <LoRa.h>

// CS pin for LoRa  VDD:3.3V
#define LoRa_DIO0   2 // only pin 1,2,3,
#define LoRa_Reset  3
#define LoRa_CS    10

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);

  Serial.println("LoRa Sender");


  LoRa.setPins(LoRa_CS, LoRa_Reset, LoRa_DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
