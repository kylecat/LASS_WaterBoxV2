/*
   7697 省電功能測試
   1.休眠5分鐘(300秒)
   2.清醒20秒
   2.1  掃描可用WIFI
   3.Loop

*/
// 省電用 library
#include "LSleep.h"
LSleepClass Sleep;
int count = 0;

// Wifi library
#include <LWiFi.h>

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}
// 印出wifi清單
void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }
}

// 印出wifi加密格式
void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    default :
      Serial.println("WPA-PSK/WPA2-PSK");
      break;
  }
}

void setup() {
  Serial.begin(9600);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // 檢查wifi版本
  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }
  // 印出MAC address
  printMacAddress();
  // 列出wifi清單
  Serial.println("Scanning networks...");   // 掃描wifi
  listNetworks();

  // Sleep功能初始化設定
  Sleep.init();
  Sleep.setTime(300000);  // 設定時間
  Sleep.setMode(3);       // 睡眠模式：0 不睡；1 待機狀態；2 wifi睡眠狀態；3 傳統睡眠狀態
  Serial.println(F("睡覺測試"));
  Sleep.sleep();
  Serial.println(F("起床"));
}

void loop() {
  Serial.print(F("睡覺Loop:("));
  Serial.print(count);
  Serial.println(")");
  Sleep.sleep();

  Serial.println(F("起床！！(20 sec)"));

  Serial.println("Scanning available networks...");   // 掃描wifi
  listNetworks();

  delay(20000);

  count++;
}
