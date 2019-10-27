/***<< license >>*****************************************
   Author：Kyle
   Licenses：MIT
   Version：1.1 (20191001)
*********************************************************/
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
void LSleepClass::init(bool _debug)
{
  _debugInfo = _debug;
  
  _headleIndex =  hal_sleep_manager_get_lock_status();
  hal_sleep_manager_release_sleep_handle(_headleIndex);
  
  if (hal_sleep_manager_init() == HAL_SLEEP_MANAGER_ERROR)
  {
    // Check the log for error handling.
    if (_debugInfo) Serial.println("[SleepManager] init failed");
  }
  else
  {
    if (_debugInfo) Serial.println("[SleepManager] init success");
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
    if (_debugInfo) Serial.println("[SleepManager] Sleep Time was set at " + (String)_sleep_time + " ms");
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
int LSleepClass::setMode(uint8_t _mode)
{
  _sleep_mode = _mode;
  int _state;
  switch (_sleep_mode) {
    case 0:
      if (_debugInfo) Serial.println("[SleepManager] Sleep Mode: NONE");
      _state = 1;
      break;
    case 1:
      if (_debugInfo) Serial.println("[SleepManager] Sleep Mode: IDEL");
      _state = 1;
      break;
    case 2:
      if (_debugInfo) Serial.println("[SleepManager] Sleep Mode: SLEEP");
      _state = 1;
      break;
    case 3:
      if (_debugInfo) Serial.println("[SleepManager] Sleep Mode: LEGACY_SLEEP");
      _state = 1;
      break;
    case 4:
      if (_debugInfo) Serial.println("[SleepManager] Sleep Mode: NUMBER");
      _state = 1;
      break;
    default:
      _state = 0;
      break;
  }
  return _state;
}

int LSleepClass::sleep()
{
  int _state;

  _headleIndex =  hal_sleep_manager_get_lock_status();
  if (_debugInfo) Serial.println("[SleepManager] Handle Index:" + String(_headleIndex));

  hal_sleep_manager_unlock_sleep(_headleIndex);                // 解除鎖定

  if (hal_sleep_manager_is_sleep_locked())
  {
    if (_debugInfo) Serial.println("[SleepManager] Cannot enter the sleep mode, as the sleep is locked");

    _state = 0;
  } else if (hal_sleep_manager_set_sleep_time(_sleep_time) == HAL_SLEEP_MANAGER_OK)
  {
    if (_debugInfo) Serial.println("[SleepManager] Inter Sleep Mode");
    hal_sleep_manager_enter_sleep_mode((hal_sleep_mode_t)_sleep_mode);
    _state = 1;
  }
  else {
    if (_debugInfo) Serial.println("[SleepManager] Cannot enter the sleep mode, set time error");
    _state = 0;
  }
  return _state;
}
