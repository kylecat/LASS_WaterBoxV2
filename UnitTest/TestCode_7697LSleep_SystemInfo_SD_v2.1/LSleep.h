/***<< license >>*****************************************
   Author：Kyle
   Licenses：MIT
   Version：1.1 (20191001)
*********************************************************/
#include <Arduino.h>

class LSleepClass
{
  public:
    LSleepClass();
    void init(bool _debug = false);
    int setTime(uint32_t _ms);
    int setMode(uint8_t _mode);
    int sleep();
    
  private:
    int _sleep_mode;
    int _sleep_time;
    bool _debugInfo;
    uint32_t _headleIndex;
};
