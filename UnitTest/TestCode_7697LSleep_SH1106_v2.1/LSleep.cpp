/*
   reference:http://labs.mediatek.com/api/mt7697/group___s_l_e_e_p___m_a_n_a_g_e_r.html
*/
#include "LSleep.h"

#include <hal_sleep_manager.h>


LSleepClass LSleep;

LSleepClass::LSleepClass()
{
}

/*
   初始化
   Sleep Manager狀態有兩個
   HAL_SLEEP_MANAGER_ERROR  An undefined error occurred.
   HAL_SLEEP_MANAGER_OK     The operation completed successfully.
*/
void LSleepClass::init()
{
  if (hal_sleep_manager_init() == HAL_SLEEP_MANAGER_ERROR)
  {
    // Check the log for error handling.
    Serial.println("Sleep Manager init failed");
    return;
  }
}

/*
   設定休眠時間(ms)
*/
int LSleepClass::setTime(uint32_t _ms)
{
  if (hal_sleep_manager_set_sleep_time(_ms) == HAL_SLEEP_MANAGER_OK)
  {
    _sleep_time = _ms;
    Serial.println("Sleep Time was set at " + (String)_sleep_time + " ms");
    return 1;
  }
  else return 0;
}

/*
   睡眠模式設定
   0 HAL_SLEEP_MODE_NONE           no sleep
   1 HAL_SLEEP_MODE_IDLE           idle state
   2 HAL_SLEEP_MODE_SLEEP          wifi sleep state
   3 HAL_SLEEP_MODE_LEGACY_SLEEP   legacy sleep state
   4 HAL_SLEEP_MODE_NUMBER         for support range detection
*/
int LSleepClass::setMode(int32_t _mode)
{
  _sleep_mode = _mode;
  switch (_sleep_mode) {
    case 0:
      Serial.println("Sleep Mode: NONE");
      break;
    case 1:
      Serial.println("Sleep Mode: IDEL");
      break;
    case 2:
      Serial.println("Sleep Mode: SLEEP");
      break;
    case 3:
      Serial.println("Sleep Mode: LEGACY_SLEEP");
      break;
    case 4:
      Serial.println("Sleep Mode: NUMBER");
      break;
  }

}

void LSleepClass::sleep()
{
  if(hal_sleep_manager_is_sleep_locked())
    {
        Serial.println("Cannot enter the sleep mode, as the sleep is locked");
    }else
    {
        if (hal_sleep_manager_set_sleep_time(_sleep_time) == HAL_SLEEP_MANAGER_OK)
        {
            hal_sleep_manager_enter_sleep_mode((hal_sleep_mode_t)_sleep_mode);
        }
    }
}
