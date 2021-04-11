#include "devDriver_pirDetector.h"

#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
 
#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"
 
#include "devDriver_manage.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_PIR_DETECTOR)

 #define DEV_PIRDETECTOR_ESP_INTR_FLAG_DEFAULT	   0

 extern EventGroupHandle_t xEventGp_devAppSupplemet_A;

 static const char *TAG = "lanbon_L8 - Pir detector";

 static bool devDriver_moudleInitialize_Flg = false;

 static TaskHandle_t *tHandle_pirDetectBuss = NULL;
 static xQueueHandle msgQh_devTipsIntIstHandle = NULL;

 static uint8_t devPirDetect_alarmDigLevel = 0;
 static uint8_t devPirDetect_clamDownSleepCounter = DEVDRIVER_PIRDETECTOR_DEEP_SLEEP_CD_CFM;
 static struct{
	uint8_t flg_sycn:1;
	uint8_t flg_hb:1;
 }flgClusterDataUpdata;
  static struct{
	uint8_t trigSourceFromPir_flg:1;
	uint8_t detectClamDownTime:7;
 }detectorAlarmTrigAtrr = {0};
 static uint8_t detectorAlarmKeepTime = 0;

 static uint8_t volatile detectorInterruptDisableTime = 0;

 static bool usrKeyTrigged_flg = false;

 static void IRAM_ATTR gpioPirLight_isr_handler(void* arg){

	uint32_t gpio_num = (uint32_t) arg;

	if(detectorInterruptDisableTime)return;

	if(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT == gpio_num){

		stt_devPirDetectAssistTkMsg sptr = {0}; 
		
		sptr.dType = DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT;
		sptr.dPoint = gpio_get_level(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);
		if(NULL != msgQh_devTipsIntIstHandle)
			xQueueSendFromISR(msgQh_devTipsIntIstHandle, &sptr, NULL);

		if(devPirDetect_clamDownSleepCounter < DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_ALARM)
			devPirDetect_clamDownSleepCounter = DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_ALARM;
	}
 }

 static void devDriverBussinessPirDetect_deepSleep_start(void){

	ESP_LOGI(TAG, "device deep sleep start!");

	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEVDRIVER_PIRDETECTOR_SLEEP_TIME_DEF * 1000ULL * 1000ULL));
	ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(((uint64_t)1 << DEVDRIVER_PIRDETECTOR_GPIO_INPUT_WKUP),
												  ESP_EXT1_WAKEUP_ALL_LOW));
	esp_deep_sleep_start();
 }

 static void devDriverBussinessPirDetect_gpioLightIOcfg(bool inputIf){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_pirDetect)return;

	if(true == inputIf){
		io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);
		io_conf.mode = GPIO_MODE_INPUT;
		io_conf.pull_up_en   = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_add(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT, gpioPirLight_isr_handler, (void*)DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);}
	else{
		io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);
		io_conf.mode = GPIO_MODE_OUTPUT;
		io_conf.pull_up_en	 = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_remove(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);}
	
	gpio_config(&io_conf);
 }

 static void devDriverBussiness_pirDetect_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_pirDetect)return;

    //install gpio isr service
    gpio_install_isr_service(DEV_PIRDETECTOR_ESP_INTR_FLAG_DEFAULT);

	devDriverBussinessPirDetect_gpioLightIOcfg(true);
 }

 static void devDriverBussiness_pirDetect_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT);
 }

 static void processTask_devPirDetectBussiness(void* arg){

	const uint16_t vTaskLoopIdle_time = 20;
	static bool flg_nwkConfirm = false;
	stt_timerLoop timerLoop_alarm = {10000 / vTaskLoopIdle_time, 10000 / vTaskLoopIdle_time - 2};
	stt_timerLoop timerLoop_sycn = {5000 / vTaskLoopIdle_time, 0};

	ESP_LOGI(TAG, "device assistant task start!");

	for(;;){

		if(NULL != msgQh_devTipsIntIstHandle){

			stt_devPirDetectAssistTkMsg rptr = {0};
			if(xQueueReceive(msgQh_devTipsIntIstHandle, &rptr, 5 / portTICK_RATE_MS) == pdTRUE){

				switch(rptr.dType){

					case DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT:
						if(1 == rptr.dPoint){
							detectorAlarmTrigAtrr.detectClamDownTime = 4;
							detectorAlarmTrigAtrr.trigSourceFromPir_flg = 1;
						}
						ESP_LOGI(TAG, "pir detector redLight inputPin trig.");
						break;

					default:break;
				}
			}
		}

		if(false == flg_nwkConfirm){ //只会触发一次

			if(systemDevice_startUpTime_get() > 3){

				if(true == usrKeyTrigged_flg){
					/*基本走不到这一步，因为短按触发测试警报*/
					devPirDetect_clamDownSleepCounter = DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
					usrKeyTrigged_flg = false;
				}else{
					if(meshNetwork_connectReserve_IF_get()){
						flgGet_gotRouterOrMeshConnect()?
							(devPirDetect_clamDownSleepCounter = 60):
							(devPirDetect_clamDownSleepCounter = 120);	
					}else{
						devPirDetect_clamDownSleepCounter = 3;
					}
				}

				flg_nwkConfirm = true;
			}
		}

		if(0 == devPirDetect_clamDownSleepCounter){
		
			devDriverBussinessPirDetect_deepSleep_start();
		}
		else
		{
			if(!gpio_get_level(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_USRKEY)){

				devPirDetect_clamDownSleepCounter = DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
				usrKeyTrigged_flg = false;
			}
			else
			{
				if((flgClusterDataUpdata.flg_sycn && flgClusterDataUpdata.flg_hb)){ //数据更新已完成，直接睡眠

					if((false == usrKeyTrigged_flg) &&\
					   (0 == detectorAlarmKeepTime)){

						devDriverBussinessPirDetect_deepSleep_start();
					}
				}				
			}
		}

		if(timerLoop_sycn.loopCounter < timerLoop_sycn.loopPeriod)timerLoop_sycn.loopCounter ++;
		else{

			stt_devDataPonitTypedef dp = {0};

			timerLoop_sycn.loopCounter = 0;
			if(detectorAlarmKeepTime){}

			if(detectorAlarmTrigAtrr.trigSourceFromPir_flg){
				dp.devType_pirDetector.alarmFlg = 1;
				detectorAlarmTrigAtrr.trigSourceFromPir_flg = 0;
			}
			currentDev_dataPointSet(&dp, false, false, false, true, false);
		}

		if(timerLoop_alarm.loopCounter < timerLoop_alarm.loopPeriod)timerLoop_alarm.loopCounter ++;
		else{
			
			stt_devDataPonitTypedef dp = {0};
			uint8_t *dptr = (uint8_t *)&dp;

			timerLoop_alarm.loopCounter = 0;
		
			currentDev_dataPointGet(&dp);
			
			if(dp.devType_pirDetector.alarmFlg){
				xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_DETECT_ALARM_REPORT);				
			}
			
			ESP_LOGI(TAG, "server report[%d], _s(%d).", *dptr, dp.devType_pirDetector.alarmFlg);
		}

		vTaskDelay(vTaskLoopIdle_time / portTICK_RATE_MS);
	}
 }

 void devDriverBussiness_pirDetect_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
 #else

	devDriverBussiness_pirDetect_periphDeinit();
 #endif

	if(NULL != tHandle_pirDetectBuss){

		vTaskDelete(tHandle_pirDetectBuss);

		tHandle_pirDetectBuss = NULL;
	}

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_pirDetect_moudleInit(void){

   devTypeDef_enum swCurrentDevType = currentDev_typeGet();

   if(swCurrentDevType != devTypeDef_pirDetect)return;
   if(devDriver_moudleInitialize_Flg)return;

   esp_log_level_set(TAG, ESP_LOG_INFO);

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #else
 
   devDriverBussiness_pirDetect_periphInit();
 #endif

   msgQh_devTipsIntIstHandle = xQueueCreate(2, sizeof(stt_devPirDetectAssistTkMsg));

   xTaskCreate(processTask_devPirDetectBussiness, "devPirDetectTk", 1024 * 4, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_pirDetectBuss);
   devDriverBussinessPirDetect_wakeUp_handle();

   devDriver_moudleInitialize_Flg = true;
 }

 void devDriverBussiness_pirDetect_alarmParamGet(stt_devDetectingDataDef *param){

	stt_devDetectingDataDef dp = {0};
	
	esp_wifi_get_mac(ESP_IF_WIFI_STA, dp.devAddr); //设备MAC
	dp.devType = currentDev_typeGet(); //设备类型
	deviceParamGet_timeZone((stt_timeZone *)&dp.devTimeZone); //时区
	currentDev_dataPointGet((stt_devDataPonitTypedef *)&dp.alarmParam[0]); //数据点
	dp.devFirmwareVer = systemDevice_currentVersionGet(); //固件版本

	memcpy(param, &dp, sizeof(stt_devDetectingDataDef));
 }

 void devDriverBussiness_pirDetect_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_pirDetect){
		
	/*业务空置，PIR报警器不存在控制业务*/

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else

#endif
	}
 }

 void devDriverBussiness_pirDetect_assistantLooper1sDetector(void){

	if(devPirDetect_clamDownSleepCounter)devPirDetect_clamDownSleepCounter --;
	if(detectorAlarmKeepTime)detectorAlarmKeepTime --;
	if(detectorInterruptDisableTime)detectorInterruptDisableTime --;

	if(detectorAlarmTrigAtrr.detectClamDownTime)detectorAlarmTrigAtrr.detectClamDownTime --;
	else{
		detectorAlarmTrigAtrr.detectClamDownTime = 2;
		if(1 == gpio_get_level(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT)){
			detectorAlarmTrigAtrr.trigSourceFromPir_flg = 1;
		}
	}

	ESP_LOGI(TAG, "idle time:%d, gpio_usrKey:%d, gpio_detLight:%d", 
				  devPirDetect_clamDownSleepCounter,  
				  gpio_get_level(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_USRKEY),
				  gpio_get_level(DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT)
				  );
 }

 void devDriverBussinessPirDetect_wakeUp_handle(void){

   esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

   switch(cause){

   	   case ESP_SLEEP_WAKEUP_UNDEFINED:{
		   ESP_LOGI(TAG, "normal reset.");	
		   usrKeyTrigged_flg = true; //普通reset也算作人为usrKeyTrig
	   }break;

	   case ESP_SLEEP_WAKEUP_EXT1:{
		   uint64_t gpioMask = esp_sleep_get_ext1_wakeup_status();
		   ESP_LOGI(TAG, "wake up cause ext1, mask:0x%016llX.", gpioMask);
		   if(gpioMask & ((uint64_t)1 << DEVDRIVER_PIRDETECTOR_GPIO_INPUT_WKUP)){
				usrKeyTrigged_flg = true;
		   }
	   }break;

	   case ESP_SLEEP_WAKEUP_TIMER:
		   ESP_LOGI(TAG, "wake up cause tmr.");
		   break;

	   default:
		   ESP_LOGI(TAG, "wake up reason:%d.", cause);
		   break;
   }
 }

 void devDriverBussinessPirDetect_flgSycnUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_sycn = 1):
		(flgClusterDataUpdata.flg_sycn = 0);
	if(true == val){
		ESP_LOGI(TAG, "dp up sycn complete.");
	}
 }

 void devDriverBussinessPirDetect_flgHbeatUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_hb = 1):
		(flgClusterDataUpdata.flg_hb = 0);
	 if(true == val){
		 ESP_LOGI(TAG, "dp up hbt complete.");
	 }
 }

 void devDriverBussinessPirDetect_detectInterruptEnable_set(bool val){
 	(true == val)?
		(detectorInterruptDisableTime = 0):
		(detectorInterruptDisableTime = 10);
	if(false == val){ //长按才延长时间
		devPirDetect_clamDownSleepCounter = DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
		usrKeyTrigged_flg = false;
	}
 }
#endif









































