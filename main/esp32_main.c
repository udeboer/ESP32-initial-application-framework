// This main file only starts the various tasks and runs initialization routines.
// Then it goes in a wait loop with some debug output.
#include <sys/param.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sdkconfig.h"

// Own header files
#include "defaults_globals.h"
#include "eventhandler.h"
#include "filesystem.h"
#include "networkstartstop.h"
#include "nvramfunctions.h"
#include "time_task.h"
#include "i2c_functions.h"
#ifdef SNTP_MAX_TIME_NOSYNC
#include "ds3231.h"
#endif
#include "app_queue.h"

//Set logging tag per module
static const char* TAG =  DEFAULT_ESPNAME ;

// DebuggetTasks needs the FreeRTOS trace facilty and runtime stats enabled in
// menuconfig. Disable these when not using this function.
//#include "debugtasks.h"

//***** Start of main and global vars
// We need global setup parameters for Wifi. Network config through http
// kills networking. So we can not use a function within the http handler
// to bring the network up again. 
setupparams_t setupparams = {
    .name = DEFAULT_ESPNAME,
    .apssid = DEFAULT_APSSID,
    .appwd = DEFAULT_APPWD,
    .stssid = DEFAULT_WIFISSID,
    .stpwd = DEFAULT_WIFIPWD,
    .wifimode = DEFAULT_WIFIMODE,
    .apchannel = DEFAULT_APCHANNEL,
    .ipmode = 0,
    .ipbyte1 = 0,
    .ipbyte2 = 0,
    .ipbyte3 = 0,
    .ipbyte4 = 0,
    .maskbyte1 = 255,
    .maskbyte2 = 255,
    .maskbyte3 = 255,
    .maskbyte4 = 0,
    .gwbyte1 = 0,
    .gwbyte2 = 0,
    .gwbyte3 = 0,
    .gwbyte4 = 0,
    .dns1byte1 = 0,
    .dns1byte2 = 0,
    .dns1byte3 = 0,
    .dns1byte4 = 0,
    .dns2byte1 = 0,
    .dns2byte2 = 0,
    .dns2byte3 = 0,
    .dns2byte4 = 0,
    .ntpserver = NTPSERVER,
    .timezone = TIMEZONE_TZ,
};

// Network runs as seperate FreeRTOS task. Changing settings is done
// through FreeRTOS event. This is defined in networkstartstop.c

void app_main(void) {
#ifdef SNTP_MAX_TIME_NOSYNC
  float temp_ds3231;
#endif

  ESP_LOGI(TAG, "Start of program");
  //we need 64bit time_t to function after 2038. 
  ESP_LOGI(TAG, "Size of time_t %d", sizeof(time_t)); 

  // Initialize NVS
  // Read setup parameters on startup or use defaults. In the clock menu
  // the ESP can be reset to an open AP.
  init_nvs();
  if (read_nvram(&setupparams, sizeof(setupparams_t), NVFLASH_SETUPBLOB) != ESP_OK) {
    ESP_LOGE(TAG, "No setup parameters found in NVRAM. Using default.");
  }
  //
  // Starting filesystem
  spiffs_start();
  // Init the I2C port
  i2c_init_port();
  // Starting network and network apps when network is available.
  // Tasks are started in seperate FreeRTOS tasks.
  start_network_task();
  start_time_task();

  //Create a queue to receive events. This could also be used for displays and menus.
  //And be running in a seperate task
  static QueueHandle_t prog_event_queue;
  prog_event_queue = program_task_queue();
  // create var for receiving queue signals
  static prog_task_queue_item_t queue_item;


  while (1) {
    // Below is all example code.   
    // start waiting on queue events
    while (xQueueReceive(prog_event_queue, &queue_item, 70000 / portTICK_PERIOD_MS) !=
 pdTRUE) {
      // we have waited for 70 seconds on items in queue. Should get 1 every 30secs
      ESP_LOGE(TAG, "Error: No display events received for 70 seconds");
    }
    //We now should have a valid event type in queue_item. In this example an event of the time task
    if (queue_item.prog_task_signal != minute_passed ){
        ESP_LOGI(TAG, "received an event not from time task");
    }

    // We get an event from time task every 0 second or 30 second mark.

    // Debug loop. Comment out the header file when not needed and disable
    // settings in idf.py menuconfig. See debugtasks.h and above.
#ifdef DEBUGTASKS_H_
    DebugGetTasks();
#endif
    // Not much to do after all tasks are started
    // ESP_LOGI(TAG, "Main loop doing nothing for another 30 seconds");
    // vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
    // below is to debug possible memory leaks
    ESP_LOGI(TAG, "Esp free heap size: %d. Minimal free %d", esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "Last SNTP sync is %d minutes ago.", last_sntp_sync_min());
    ESP_LOGI(TAG, "Time is %d:%d:%d", current_timeinfo.tm_hour, current_timeinfo.tm_min, current_timeinfo.tm_sec);
    //Lots of CPU time available here

    //Just for testing 
#ifdef SNTP_MAX_TIME_NOSYNC
    ds3231_get_temp_float(&temp_ds3231);
    ESP_LOGI(TAG,"Temperature inside ds3231 is %f ", temp_ds3231);
#endif
  }
}
