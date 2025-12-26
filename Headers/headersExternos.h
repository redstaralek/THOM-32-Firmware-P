//==================================================================================
//=============================== Includes externos ================================
//==================================================================================
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Adafruit_BME680.h"
#include <SparkFun_AS7331.h>
#include <SparkFun_AS3935.h>
#include "Adafruit_VEML7700.h"
// #include <DFRobot_SCD4X.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
#include <WiFi.h>
#include <RunningMedian.h>
#include "driver/adc.h" 
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include "esp_wpa2.h"
extern   "C" { uint8_t temprature_sens_read(); }
#include "esp_sleep.h"
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include "time.h"
#include "driver/rtc_io.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ESPping.h>

