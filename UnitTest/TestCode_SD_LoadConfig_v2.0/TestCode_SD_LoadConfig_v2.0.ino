/*****<< SD卡測試程式_儲存資料 >>*****
  1.用指定ID資料夾儲存資料(8字元)
  2.建立log資料夾
***********************************/

/**** << WaterBox_V2.1.HP:SD Card >> *****
    功能：儲存資料在Ｍicro SD卡內
    library：SD.h
    版本：LinkIt 7697內建]
***********************************************************/
#include <SPI.h>
#include <SD.h>

File myFile;
#define SC_CS 4
#define LoRa_CS 10

#define modulePower 14

/**** << WaterBox_V2.1:Arduino JSON >> *****
    功能：資料處理用，統一轉換成JSON比較好做事情
    library：ArduinoJson.h
    版本：5.13.5
***********************************************************/
#include <ArduinoJson.h>

String CONFIG_PATH = "/config";
DynamicJsonBuffer _jsonBuffer; // ArduinoJson 5

struct CONFIG {
  const char* fw_ver;  // 韌體版本名稱
  String device_id;    // 儀器序號: MQTT用

  int analysis_cycle; // 分析週期
  int saving_cycle; // 存檔週期
  int upload_cycle; // 上傳週期

  String wifi_ssid; // wifi連線用
  String wifi_pass;

  String mqtt_broker; // mqtt連線用
  int mqtt_port;
  String mqtt_user;
  String mqtt_pass;
  String mqtt_pub_topic;
  String mqtt_sub_topic;

  String thingspeak_key; // ThingSpeak上傳用的API Key

  String http_server;      // http 上傳用Server
  String https_server;     // /https 上傳
  String restful_fromat;   // RestFul用 // 放數值的保留字(Reserve Key):r_ +item, ex:r_PH,r_EC,r_DO,r_ORP,r_TEMP
  int vr1_offset;
  int vr1_score;
  int vr2_offset;
  int vr2_score;
};


struct CAL_CONFIG
{
  float y_HIGH;
  float y_MID;
  float y_LOW;
  float x_HIGH;
  float x_MID;
  float x_LOW;
  float SLOP;
  float INTERCEPT;
  float R_2;
};


CONFIG sysConfig;                     // 存放資料的stuct
CAL_CONFIG cal_ph;
CAL_CONFIG cal_ec;

/***** << SD library >> *****/
void initSD(void)
{
  pinMode(LoRa_CS, OUTPUT);      // 手動控制LoRa 的CS
  digitalWrite(LoRa_CS, HIGH);   // 上拉LoRa SPI 的CS腳位，避免抓到LoRa
  SD.begin(SC_CS);
}

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



/***** << JSON function >> *****/
void loadConfig(String _fileName)
{
  initSD();
  bool _stateCheck = SD.exists(_fileName);
  if (_stateCheck)
  {
    Serial.println("[SD] File <" + _fileName + "> exists");
  }
  else {
    Serial.println("[SD] File <" + _fileName + "> did not exist");
  }

  File _configFile = SD.open(_fileName);
  int _size = _configFile.size();

  if (_size > 2048) Serial.println("[ERROR] the size of config > " + (String)_size);
  else              Serial.println("[SYSTEM] config size: " + (String)_size);

  // Deserialize the JSON document
  JsonObject &_json = _jsonBuffer.parseObject(_configFile); // ArduinoJson 5

  if (!_json.success()) { // ArduinoJson 5
    Serial.println(F("[JSON] Failed to read file, using default configuration"));
  }
  else {
    Serial.println("[SYSTEM] Reading JSON Config File\r\n==========");
    appendConfig(_json);
    Serial.println("\r\n==========");
  }
  _configFile.close();
}

void appendConfig(JsonObject &_json)
{
  //  _json.prettyPrintTo(Serial);
  JsonVariant _buffer; // 是個 reference, 而不是實際ㄉ植
  sysConfig.fw_ver = _json["fw_ver"].as<const char*>();
  sysConfig.device_id = _json["device_id"].as<String>();

  _buffer = _json.get<JsonVariant>("interval_sec");
  sysConfig.analysis_cycle = _buffer["analysis"].as<int>();
  sysConfig.saving_cycle = _buffer["saving"].as<int>();
  sysConfig.upload_cycle = _buffer["upload"].as<int>();

  _buffer = _json.get<JsonVariant>("wifi");
  sysConfig.wifi_ssid = _buffer["wifi_ssid"].as<String>();
  sysConfig.wifi_pass = _buffer["wifi_pass"].as<String>();

  _buffer = _json.get<JsonVariant>("mqtt");
  sysConfig.mqtt_broker = _buffer["mqtt_broker"].as<String>();
  sysConfig.mqtt_port = _buffer["mqtt_port"].as<int>();
  sysConfig.mqtt_user = _buffer["mqtt_user"].as<String>();
  sysConfig.mqtt_pass = _buffer["mqtt_pass"].as<String>();
  sysConfig.mqtt_pub_topic = _buffer["mqtt_pub_topic"].as<String>();
  sysConfig.mqtt_sub_topic = _buffer["mqtt_sub_topic"].as<String>();

  _buffer = _json.get<JsonVariant>("thingspeak");
  sysConfig.thingspeak_key = _buffer["api_key"].as<String>();

  _buffer = _json.get<JsonVariant>("restful_api");
  sysConfig.http_server = _buffer["http_server"].as<String>();
  sysConfig.https_server = _buffer["https_server"].as<String>();
  sysConfig.restful_fromat = _buffer["restful_fromat"].as<String>(); // 放數值的保留字(Reserve Key):r_ +item, ex:r_PH,r_EC,r_DO,r_ORP,r_TEMP

  _buffer = _json.get<JsonVariant>("cal_ph");
  cal_ph.x_HIGH = _buffer["x_HIGH"].as<float>();
  cal_ph.x_MID = _buffer["x_MID"].as<float>();
  cal_ph.x_LOW = _buffer["x_LOW"].as<float>();
  cal_ph.y_HIGH = _buffer["y_HIGH"].as<float>();
  cal_ph.y_MID = _buffer["y_MID"].as<float>();
  cal_ph.y_LOW = _buffer["y_LOW"].as<float>();

  _buffer = _json.get<JsonVariant>("cal_ec");
  cal_ec.x_HIGH = _buffer["x_HIGH"].as<float>();
  cal_ec.x_MID = _buffer["x_MID"].as<float>();
  cal_ec.x_LOW = _buffer["x_LOW"].as<float>();
  cal_ec.y_HIGH = _buffer["y_HIGH"].as<float>();
  cal_ec.y_MID = _buffer["y_MID"].as<float>();
  cal_ec.y_LOW = _buffer["y_LOW"].as<float>();

  _buffer = _json.get<JsonVariant>("vr_1");
  sysConfig.vr1_offset = _buffer["offset"].as<int>();
  sysConfig.vr1_score = _buffer["score"].as<int>();

  _buffer = _json.get<JsonVariant>("vr_2");
  sysConfig.vr2_offset = _buffer["offset"].as<int>();
  sysConfig.vr2_score = _buffer["score"].as<int>();


  // 基本檢查
  if (sysConfig.analysis_cycle == 0) sysConfig.analysis_cycle = 150;
  if (sysConfig.saving_cycle == 0) sysConfig.saving_cycle = 300;
  if (sysConfig.upload_cycle == 0) sysConfig.upload_cycle = 600;
  if (sysConfig.mqtt_broker == "") Serial.println("No MQTT Broker Config");
  if (sysConfig.http_server == "") Serial.println("No Restful API Config");
  if (sysConfig.https_server == "") Serial.println("No Restful APIs Config");

}

void showConfig()
{
  Serial.println("----- System Config -----");

  Serial.println("[ Device ]");
  Serial.print("\t fw_ver:\t"); Serial.println(sysConfig.fw_ver);
  Serial.print("\t device_id:\t"); Serial.println(sysConfig.device_id);

  Serial.println("[ Interval(sec) ]");
  Serial.print("\t analysis:\t"); Serial.println(sysConfig.analysis_cycle);
  Serial.print("\t saving:\t"); Serial.println(sysConfig.saving_cycle);
  Serial.print("\t upload:\t"); Serial.println(sysConfig.upload_cycle);

  Serial.println("[ WIFI ]");
  Serial.print("\t wifi_ssid:\t"); Serial.println(sysConfig.wifi_ssid);
  Serial.print("\t wifi_pass:\t"); Serial.println(sysConfig.wifi_pass);

  Serial.println("[ MQTT ]");
  Serial.print("\t mqtt_broker:\t"); Serial.println(sysConfig.mqtt_broker);
  Serial.print("\t mqtt_port:\t"); Serial.println(sysConfig.mqtt_port);
  Serial.print("\t mqtt_user:\t"); Serial.println(sysConfig.mqtt_user);
  Serial.print("\t mqtt_pass:\t"); Serial.println(sysConfig.mqtt_pass);
  Serial.print("\t pub_topic:\t"); Serial.println(sysConfig.mqtt_pub_topic);
  Serial.print("\t sub_topic:\t"); Serial.println(sysConfig.mqtt_sub_topic);

  Serial.println("[ ThingSpeak ]");
  Serial.print("\t thingspeak_key:\t"); Serial.println(sysConfig.thingspeak_key);

  Serial.println("[ HTTP ]");
  Serial.print("\t http_server:\t"); Serial.println(sysConfig.http_server);
  Serial.print("\t https_server:\t"); Serial.println(sysConfig.https_server);

  Serial.println("[ RestFul API Item ]");
  Serial.print("\t RestFul Fromat:\t"); Serial.println(sysConfig.restful_fromat);

  
  Serial.println("[ Knob Setting: VR1 ]");
  Serial.print("\t offset:\t"); Serial.println(sysConfig.vr1_offset);
  Serial.print("\t score: \t"); Serial.println(sysConfig.vr1_score);
  
  Serial.println("[ Knob Setting: VR2 ]");
  Serial.print("\t offset:\t"); Serial.println(sysConfig.vr2_offset);
  Serial.print("\t score: \t"); Serial.println(sysConfig.vr2_score);
  
  Serial.println("-------------------------\r\n");
}

void showCalConfig(String _name, CAL_CONFIG &_config) {
  Serial.print("****** ");
  Serial.print(_name);
  Serial.println(" Calibration Config ******");

  Serial.println("[ Data ]");
  Serial.print("\t [High]  X/Y: "); Serial.print(_config.x_HIGH); Serial.print("/"); Serial.println(_config.y_HIGH);
  Serial.print("\t [Mid ]  X/Y: "); Serial.print(_config.x_MID); Serial.print("/"); Serial.println(_config.y_MID);
  Serial.print("\t [Low ]  X/Y: "); Serial.print(_config.x_LOW); Serial.print("/"); Serial.println(_config.y_LOW);

  Serial.println("[ Calibration ]");
  Serial.print("\t SLOP:"); Serial.println(_config.SLOP, 4);
  Serial.print("\t INTERCEPT:"); Serial.println(_config.INTERCEPT, 4);
  Serial.print("\t R_2:"); Serial.println(_config.R_2, 8);
  Serial.println("************************");
}

// simple Linear Regression
// reference: https://www.statisticshowto.com/probability-and-statistics/regression-analysis/find-a-linear-regression-equation/
void simpleLinearRegression(CAL_CONFIG &_config) {

  int _n = 3;
  float _sum_y = _config.y_HIGH + _config.y_MID + _config.y_LOW;
  float _sum_x = _config.x_HIGH + _config.x_MID + _config.x_LOW;
  float _sum_xy = _config.x_HIGH * _config.y_HIGH +
                  _config.x_MID * _config.y_MID +
                  _config.x_LOW * _config.y_LOW;

  float _sum_y2 = pow(_config.y_HIGH, 2) + pow(_config.y_MID, 2) + pow(_config.y_LOW, 2);
  float _sum_x2 = pow(_config.x_HIGH, 2) + pow(_config.x_MID, 2) + pow(_config.x_LOW, 2);

  _config.SLOP =  (_n * _sum_xy - _sum_x * _sum_y) /
                  (_n * _sum_x2 - pow(_sum_x, 2));           // beta_1

  _config.INTERCEPT = (_sum_y * _sum_x2 - _sum_x * _sum_xy) /
                      (_n * _sum_x2 - pow(_sum_x, 2));       // beta_0

  double _r2_denominator = (_n * _sum_x2 - pow(_sum_x, 2)) * (_n * _sum_y2 - pow(_sum_y, 2));
  _config.R_2 = (_n * _sum_xy - _sum_x * _sum_y) / sqrt(_r2_denominator); // Pearson correlation coefficient
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  pinMode(modulePower, OUTPUT);     //
  digitalWrite(modulePower, HIGH);  // 開啟模組電源

  loadConfig(CONFIG_PATH);
  showConfig();

  simpleLinearRegression(cal_ph);
  showCalConfig("pH", cal_ph);

  simpleLinearRegression(cal_ec);
  showCalConfig("EC", cal_ec);
}


void loop(void)
{
}
