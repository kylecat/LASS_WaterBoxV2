#include <Arduino.h>

class LSleepClass
{
  public:
    LSleepClass();
    void init();
    int setTime(uint32_t _ms);
    int setMode(int32_t _mode);
    void sleep();
    
  private:
    int _sleep_mode;
    int _sleep_time;
};
