#include "devDriver_smokeDetector.h"

#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
 
#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"
 
#include "devDriver_manage.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR)

 #define DEV_SMOKEDETECTOR_ESP_INTR_FLAG_DEFAULT	   0

 extern EventGroupHandle_t xEventGp_devAppSupplemet_A;

 static const char *TAG = "lanbon_L8 - Smoke detector";
 static const uint16_t 
 smokeConcentrationLevel_digitalTab[] = {
	500, 1000, 2000,
 };

 static bool devDriver_moudleInitialize_Flg = false;

 static TaskHandle_t *tHandle_smokeDetectBuss = NULL;
 static xQueueHandle msgQh_devTipsIntIstHandle = NULL;

 static uint8_t devSmokeDetect_alarmDigLevel = 0;
 static volatile uint16_t freqSmokeBeep_detectCfm = 0,
 				 		  freqSmokeBeep_detectCnt = 0;

 static uint8_t devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_DEEP_SLEEP_CD_CFM;
 static struct{
	uint8_t flg_sycn:1;
	uint8_t flg_hb:1;
 }flgClusterDataUpdata;
 static struct{
	uint8_t trigSourceFromSmoke_flg:1;
	uint8_t detectClamDownTime:7;
 }detectorAlarmTrigAtrr = {0};
 static uint8_t detectorAlarmKeepTime = 0;

 static uint8_t volatile detectorInterruptDisableTime = 0;

 static bool usrKeyTrigged_flg = false;

 static void IRAM_ATTR gpioSmokeBeep_isr_handler(void* arg){

	uint32_t gpio_num = (uint32_t) arg;

	if(detectorInterruptDisableTime)return;

	if(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP == gpio_num){

		stt_devSmokeDetectAssistTkMsg sptr = {0}; 
		
		sptr.dType = DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP;
		sptr.dPoint = gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);
		if(NULL != msgQh_devTipsIntIstHandle)
			xQueueSendFromISR(msgQh_devTipsIntIstHandle, &sptr, NULL);

		devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_ALARM;

		freqSmokeBeep_detectCnt ++;
	}else
	if(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT == gpio_num){

		stt_devSmokeDetectAssistTkMsg sptr = {0}; 
		
		sptr.dType = DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT;
		sptr.dPoint = gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT);
		if(NULL != msgQh_devTipsIntIstHandle)
			xQueueSendFromISR(msgQh_devTipsIntIstHandle, &sptr, NULL);
		
		devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_ALARM;
	}
 }

 static void devDriverBussinessSmokeDetect_deepSleep_start(void){

	ESP_LOGI(TAG, "device deep sleep start!");

//	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEVDRIVER_SMOKEDETECTOR_SLEEP_TIME_DEF * 1000ULL * 1000ULL));
	ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP)|\
												  ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_USRKEY), 
												  DEVDRIVER_SMOKEDETECTOR_GPIO_SMOKEBEEP_EFFEC_LEVEL));

	gpio_reset_pin(DEVDRIVER_SMOKEDETECTOR_GPIO_OUTPUT_BEEP);
	gpio_set_direction(DEVDRIVER_SMOKEDETECTOR_GPIO_OUTPUT_BEEP, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(DEVDRIVER_SMOKEDETECTOR_GPIO_OUTPUT_BEEP, GPIO_PULLDOWN_ONLY);
	gpio_set_level(DEVDRIVER_SMOKEDETECTOR_GPIO_OUTPUT_BEEP, 0);
	gpio_deep_sleep_hold_en();
	
	rtc_gpio_isolate(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);

	esp_deep_sleep_start();
 }

 static void devDriverBussinessSmokeDetect_gpioLightIOcfg(bool inputIf){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_smokeDetect)return;

	if(true == inputIf){
		io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT);
		io_conf.mode = GPIO_MODE_INPUT;
		io_conf.pull_up_en   = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_add(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT, gpioSmokeBeep_isr_handler, (void*)DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT);}
	else{
		io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT);
		io_conf.mode = GPIO_MODE_OUTPUT;
		io_conf.pull_up_en	 = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_remove(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT);}
	
	gpio_config(&io_conf);
 }

 static void devDriverBussiness_smokeDetect_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_smokeDetect)return;

    //install gpio isr service
    gpio_install_isr_service(DEV_SMOKEDETECTOR_ESP_INTR_FLAG_DEFAULT);

	devDriverBussinessSmokeDetect_gpioLightIOcfg(true);
	devDriverBussinessSmokeDetect_gpioBeepIOcfg(true);
//	gpio_set_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT, 0);
    //hook isr handler for specific gpio pin
 }

 static void devDriverBussiness_smokeDetect_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);
 }

 static void processTask_devSmokeDetectBussiness(void* arg){

	const uint16_t vTaskLoopIdle_time = 20;
	static bool flg_nwkConfirm = false;
	stt_timerLoop timerLoop_alarm = {10000 / vTaskLoopIdle_time, 10000 / vTaskLoopIdle_time - 2};
	stt_timerLoop timerLoop_sycn = {5000 / vTaskLoopIdle_time, 0};

	ESP_LOGI(TAG, "device assistant task start!");

	for(;;){

		if(NULL != msgQh_devTipsIntIstHandle){

			stt_devSmokeDetectAssistTkMsg rptr = {0};
			if(xQueueReceive(msgQh_devTipsIntIstHandle, &rptr, 5 / portTICK_RATE_MS) == pdTRUE){

				switch(rptr.dType){
		
					case DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP:
						detectorAlarmKeepTime = 6;
						(detectorAlarmTrigAtrr.trigSourceFromSmoke_flg)?
							(devBeepTips_trig(4, 8, 1000, 200, 2)):
							(devBeepTips_trig(4, 8, 100, 50, 8));
//						ESP_LOGI(TAG, "smoke detector beep inputPin trig.");
						break;

					case DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT:
						if(0 == rptr.dPoint){
							detectorAlarmTrigAtrr.trigSourceFromSmoke_flg = 0;
							detectorAlarmTrigAtrr.detectClamDownTime = 4;
						}
						ESP_LOGI(TAG, "smoke detector redLight inputPin trig.");
						break;

					default:break;
				}
			}
		}

		if(false == flg_nwkConfirm){

			if(systemDevice_startUpTime_get() > 3){

				if(true == usrKeyTrigged_flg){
					/*基本走不到这一步，因为短按触发测试警报*/
					devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
					usrKeyTrigged_flg = false;
				}else{
					if(meshNetwork_connectReserve_IF_get()){
						flgGet_gotRouterOrMeshConnect()?
							(devSmokeDetect_clamDownSleepCounter = 60):
							(devSmokeDetect_clamDownSleepCounter = 120);	
					}else{
						devSmokeDetect_clamDownSleepCounter = 3;
					}
				}

				flg_nwkConfirm = true;
			}
		}

		if(0 == devSmokeDetect_clamDownSleepCounter){
		
			devDriverBussinessSmokeDetect_deepSleep_start();
		}
		else
		{
			if(gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_USRKEY)){

				devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
				usrKeyTrigged_flg = false;
			}
			else
			{
				if((flgClusterDataUpdata.flg_sycn && flgClusterDataUpdata.flg_hb)){ //数据更新已完成，直接睡眠

					if((false == usrKeyTrigged_flg) &&\
					   (0 == detectorAlarmKeepTime)){

						devDriverBussinessSmokeDetect_deepSleep_start();
					}
				}				
			}
		}

		if(timerLoop_sycn.loopCounter < timerLoop_sycn.loopPeriod)timerLoop_sycn.loopCounter ++;
		else{

			stt_devDataPonitTypedef dp = {0};

			timerLoop_sycn.loopCounter = 0;
			if(detectorAlarmKeepTime){
				(detectorAlarmTrigAtrr.trigSourceFromSmoke_flg)?
					(dp.devType_smokeDetector.alarmFlg_smoke = 1):
					(dp.devType_smokeDetector.alarmFlg_co = 1);
			}
			
			currentDev_dataPointSet(&dp, false, false, false, true, false);
		}

		if(timerLoop_alarm.loopCounter < timerLoop_alarm.loopPeriod)timerLoop_alarm.loopCounter ++;
		else{
			
			stt_devDataPonitTypedef dp = {0};
			uint8_t *dptr = (uint8_t *)&dp;

			timerLoop_alarm.loopCounter = 0;
		
			currentDev_dataPointGet(&dp);
			
			if(dp.devType_smokeDetector.alarmFlg_smoke || dp.devType_smokeDetector.alarmFlg_co){
				xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_DETECT_ALARM_REPORT);				
			}
			
			ESP_LOGI(TAG, "server report[%d], _s(%d), _c(%d).", *dptr, dp.devType_smokeDetector.alarmFlg_smoke, dp.devType_smokeDetector.alarmFlg_co);
		}

		vTaskDelay(vTaskLoopIdle_time / portTICK_RATE_MS);
	}
 }

 static uint8_t smokeDeteting_digLevel_generate(uint16_t cnec){

	uint8_t res = 0;
	
	if(cnec < smokeConcentrationLevel_digitalTab[0])res = 0;
	else
	if(cnec >= smokeConcentrationLevel_digitalTab[0] && cnec < smokeConcentrationLevel_digitalTab[1])res = 1;
	else
	if(cnec >= smokeConcentrationLevel_digitalTab[1] && cnec < smokeConcentrationLevel_digitalTab[2])res = 2;
	else
	if(cnec >= smokeConcentrationLevel_digitalTab[2])res = 3;

	return res;
 }

 void devDriverBussinessSmokeDetect_gpioBeepIOcfg(bool inputIf){
 
	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_smokeDetect)return;

	if(true == inputIf){
		io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);
		io_conf.mode = GPIO_MODE_INPUT;
		io_conf.pull_up_en   = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_add(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP, gpioSmokeBeep_isr_handler, (void*)DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);}
	else{
		io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
		io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);
		io_conf.mode = GPIO_MODE_OUTPUT;
		io_conf.pull_up_en	 = 0;
		io_conf.pull_down_en = 0;
		gpio_isr_handler_remove(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP);}
	
	gpio_config(&io_conf);
 }

 void devDriverBussiness_smokeDetect_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
 #else

	devDriverBussiness_smokeDetect_periphDeinit();
 #endif

	if(NULL != tHandle_smokeDetectBuss){

		vTaskDelete(tHandle_smokeDetectBuss);

		tHandle_smokeDetectBuss = NULL;
	}

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_smokeDetect_moudleInit(void){

   devTypeDef_enum swCurrentDevType = currentDev_typeGet();

   if(swCurrentDevType != devTypeDef_smokeDetect)return;
   if(devDriver_moudleInitialize_Flg)return;

   esp_log_level_set(TAG, ESP_LOG_INFO);

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #else
 
   devDriverBussiness_smokeDetect_periphInit();
 #endif

   msgQh_devTipsIntIstHandle = xQueueCreate(2, sizeof(stt_devSmokeDetectAssistTkMsg));

   xTaskCreate(processTask_devSmokeDetectBussiness, "devSmokeDetectTk", 1024 * 4, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_smokeDetectBuss);
   devDriverBussinessSmokeDetect_wakeUp_handle();

   devDriver_moudleInitialize_Flg = true;
 }

 void devDriverBussiness_smokeDetect_alarmParamGet(stt_devDetectingDataDef *param){

	stt_devDetectingDataDef dp = {0};
	
	esp_wifi_get_mac(ESP_IF_WIFI_STA, dp.devAddr); //设备MAC
	dp.devType = currentDev_typeGet(); //设备类型
	deviceParamGet_timeZone((stt_timeZone *)&dp.devTimeZone); //时区
	currentDev_dataPointGet((stt_devDataPonitTypedef *)&dp.alarmParam[0]); //数据点
	dp.devFirmwareVer = systemDevice_currentVersionGet(); //固件版本

	memcpy(param, &dp, sizeof(stt_devDetectingDataDef));
 }

 void devDriverBussiness_smokeDetect_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_smokeDetect){
		
	/*业务空置，燃气报警器不存在控制业务*/

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else

#endif
	}
 }

 void devDriverBussiness_smokeDetect_assistantLooper1sDetector(void){

	if(devSmokeDetect_clamDownSleepCounter)devSmokeDetect_clamDownSleepCounter --;
	if(detectorAlarmKeepTime)detectorAlarmKeepTime --;
	if(detectorInterruptDisableTime)detectorInterruptDisableTime --;

	if(detectorAlarmTrigAtrr.detectClamDownTime)detectorAlarmTrigAtrr.detectClamDownTime --;
	else{
		detectorAlarmTrigAtrr.detectClamDownTime = 2;
		(0 == gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT))?
			(detectorAlarmTrigAtrr.trigSourceFromSmoke_flg = 0):
			(detectorAlarmTrigAtrr.trigSourceFromSmoke_flg = 1);
	}

	freqSmokeBeep_detectCfm = freqSmokeBeep_detectCnt;
	freqSmokeBeep_detectCnt = 0;
	devSmokeDetect_alarmDigLevel = smokeDeteting_digLevel_generate(freqSmokeBeep_detectCfm);

	ESP_LOGI(TAG, "smokeBeepFreq:(%d)%d, idle time:%d, gpio_detectBeep:%d, gpio_usrKey:%d, gpio_detLight:%d", 
				  smokeDeteting_digLevel_generate(freqSmokeBeep_detectCfm),
				  freqSmokeBeep_detectCfm,
				  devSmokeDetect_clamDownSleepCounter, 
				  gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP), 
				  gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_USRKEY),
				  gpio_get_level(DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT)
				  );
 }

 void devDriverBussinessSmokeDetect_wakeUp_handle(void){

   esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
   switch(cause){

	   case ESP_SLEEP_WAKEUP_EXT1:{
		   uint64_t gpioMask = esp_sleep_get_ext1_wakeup_status();
		   ESP_LOGI(TAG, "wake up cause ext1, mask:0x%016llX.", gpioMask);
		   if(gpioMask & ((uint64_t)1 << DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_USRKEY))
			   usrKeyTrigged_flg = true;
	   }break;

	   case ESP_SLEEP_WAKEUP_TIMER:
		   ESP_LOGI(TAG, "wake up cause tmr.");
		   break;

	   default:
		   ESP_LOGI(TAG, "wake up reason:%d.", cause);
		   break;
   }
 }

 void devDriverBussinessSmokeDetect_flgSycnUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_sycn = 1):
		(flgClusterDataUpdata.flg_sycn = 0);
	if(true == val){
		ESP_LOGI(TAG, "dp up sycn complete.");
	}
 }

 void devDriverBussinessSmokeDetect_flgHbeatUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_hb = 1):
		(flgClusterDataUpdata.flg_hb = 0);
	 if(true == val){
		 ESP_LOGI(TAG, "dp up hbt complete.");
	 }
 }

 void devDriverBussinessSmokeDetect_detectInterruptEnable_set(bool val){
 	(true == val)?
		(detectorInterruptDisableTime = 0):
		(detectorInterruptDisableTime = 10);
	if(false == val){ //长按才延长时间
		devSmokeDetect_clamDownSleepCounter = DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
		usrKeyTrigged_flg = false;
	}
 }
#endif








































