/*****<< SD卡測試程式_儲存資料 >>*****
  1.用指定ID資料夾儲存資料(8字元)
  2.建立log資料夾
***********************************/

// include the SD library:
#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

File myFile;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 4;

#define modulePower 14


/***** << SD library >> *****/
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
    else              Serial.println("failed");
  }
  return _stateCheck;
}

bool SD_sData(String _flieName, String _data)
{
  bool _stateCheck = SD_checkDir(_flieName);
  if (_stateCheck) {
    ;
  }

  return _stateCheck;
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(modulePower, OUTPUT);     //
  digitalWrite(modulePower, HIGH);  // 開啟模組電源

  pinMode(10, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(10, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa

  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  SD_checkDir("20190928");
  Serial.println(SD.exists("20190928"));

  SD_checkDir("abdc");
  Serial.println(SD.exists("abdc"));

  myFile = SD.open("abdc/example.txt", FILE_WRITE);
  myFile.close();

  card.init(SPI_HALF_SPEED, chipSelect);
  
  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void loop(void)
{
}
