#include "devDriver_gasDetector.h"

#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "devDriver_manage.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_GAS_DETECTOR)

 #define DEV_GASDETECTOR_ESP_INTR_FLAG_DEFAULT		0

 #define DEVDRIVER_GAS_DETECTOR_DEFAULT_VREF       	1100

 extern EventGroupHandle_t xEventGp_devAppSupplemet_A;

 static const char *TAG = "lanbon_L8 - Gas detector";

 static const uint16_t 
 gasConcentrationLevel_digitalTab[] = {
	500, 1000, 2000,
 },
 gasConcentrationLevel_analogTab[DEVDRIVER_GASDETECTOR_ANA_ALARM_LEVEL_MAX] = {
	200, 220, 260, 300, 340, 400, 500, 600, 700, 
 };

 static bool devDriver_moudleInitialize_Flg = false;
 
 static TaskHandle_t *tHandle_gasDetectBuss = NULL;
 static xQueueHandle msgQh_devTipsIntIstHandle = NULL;

 static uint8_t  devGasDetect_alarmDigLevel = 0,
 				 devGasDetect_alarmAnaLevel = 0;
 static uint8_t devGasDetect_alarmClamdownTime = 0;
 static volatile uint16_t freqGasBeep_detectCfm = 0,
 				 		  freqGasBeep_detectCnt = 0;

 static uint8_t devGasDetect_clamDownSleepCounter = DEVDRIVER_GASDETECTOR_DEEP_SLEEP_CD_CFM;
 static struct{
	uint8_t flg_sycn:1;
	uint8_t flg_hb:1;
 }flgClusterDataUpdata;

 static bool usrKeyTrigged_flg = false;

 static esp_adc_cal_characteristics_t *adc_chars;
 static const adc_channel_t channel = ADC_CHANNEL_5;     
 static const adc_atten_t atten = ADC_ATTEN_DB_0;
 static const adc_unit_t unit = ADC_UNIT_1;

 static void IRAM_ATTR gpioGasBeep_isr_handler(void* arg){

	uint32_t gpio_num = (uint32_t) arg;

	if(DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP == gpio_num){

		stt_devGasDetectAssistTkMsg sptr = {0}; 
		
		sptr.dType = 0;
		sptr.dPoint = 0;
		if(NULL != msgQh_devTipsIntIstHandle)
			xQueueSendFromISR(msgQh_devTipsIntIstHandle, &sptr, NULL);

		devGasDetect_clamDownSleepCounter = DEVDRIVER_GASDETECTOR_AWAKE_TIMETG_ALARM;

		freqGasBeep_detectCnt ++;
	}
 }

 static void devDriverBussinessGasDetect_deepSleep_start(void){

	ESP_LOGI(TAG, "device deep sleep start!");

	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEVDRIVER_GASDETECTOR_SLEEP_TIME_DEF * 1000ULL * 1000ULL));
	ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(((uint64_t)1 << DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP) |\
												  ((uint64_t)1 << DEVDRIVER_GASDETECTOR_GPIO_INPUT_USRKEY), 
												  DEVDRIVER_GASDETECTOR_GPIO_GASBEEP_EFFEC_LEVEL));

	rtc_gpio_isolate(DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP);
//	rtc_gpio_isolate(DEVDRIVER_GASDETECTOR_GPIO_OUTPUT_BEEP);

	esp_deep_sleep_start();
 }

 static void processTask_devGasDetectBussiness(void* arg){

	const uint16_t vTaskLoopIdle_time = 20;

	stt_timerLoop timerLoop_tips = {1000 / vTaskLoopIdle_time, 0};
	stt_timerLoop timerLoop_sycn = {5000 / vTaskLoopIdle_time, 0};
	stt_timerLoop timerLoop_alarm = {10000 / vTaskLoopIdle_time, 10000 / vTaskLoopIdle_time - 2};

	static bool flg_nwkConfirm = false;
	
	ESP_LOGI(TAG, "device assistant task start!");

	for(;;){

		if(NULL != msgQh_devTipsIntIstHandle){

			stt_devGasDetectAssistTkMsg rptr = {0};
			
			if(xQueueReceive(msgQh_devTipsIntIstHandle, &rptr, 5 / portTICK_RATE_MS) == pdTRUE){
			
				if(0 == rptr.dType){
			
					switch(rptr.dPoint){
			
						case 0:
//							ESP_LOGI(TAG, "gas detector inputPin trig.");
							break;
			
						default:break;
					}
				}
			}
		}

		if(false == flg_nwkConfirm){

			if(systemDevice_startUpTime_get() > 3){

				if(usrKeyTrigged_flg){
					devGasDetect_clamDownSleepCounter = DEVDRIVER_GASDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
					usrKeyTrigged_flg = false;
				}else{
					if(meshNetwork_connectReserve_IF_get()){
						flgGet_gotRouterOrMeshConnect()?
							(devGasDetect_clamDownSleepCounter = 60):
							(devGasDetect_clamDownSleepCounter = 120);	
					}else{
						devGasDetect_clamDownSleepCounter = 3;
					}
				}

				flg_nwkConfirm = true;
			}
		}

		if(0 == devGasDetect_clamDownSleepCounter){
		
			devDriverBussinessGasDetect_deepSleep_start();
		}
		else
		{
			if(gpio_get_level(DEVDRIVER_GASDETECTOR_GPIO_INPUT_USRKEY)){

				devGasDetect_clamDownSleepCounter = DEVDRIVER_GASDETECTOR_AWAKE_TIMETG_USRKEY; //按键动作，延长运行时间
				usrKeyTrigged_flg = false;
			}
			else
			{
				if((flgClusterDataUpdata.flg_sycn && flgClusterDataUpdata.flg_hb)){ //数据更新已完成，直接睡眠

					if((devGasDetect_alarmAnaLevel == 0) &&
					   (false == usrKeyTrigged_flg)){

						devDriverBussinessGasDetect_deepSleep_start();
					}
				}				
			}
		}

		if(timerLoop_tips.loopCounter < timerLoop_tips.loopPeriod)timerLoop_tips.loopCounter ++;
		else{

			uint16_t bTimeKeep_temp = 3 - devGasDetect_alarmAnaLevel / 3;

			timerLoop_tips.loopCounter = 0;
			if((devGasDetect_alarmAnaLevel >= 2) && devGasDetect_alarmClamdownTime)
				devBeepTips_trig(3, 6,  bTimeKeep_temp * 100, bTimeKeep_temp * 100, 10);
		}

		if(timerLoop_sycn.loopCounter < timerLoop_sycn.loopPeriod)timerLoop_sycn.loopCounter ++;
		else{

			stt_devDataPonitTypedef dp = {0};

			timerLoop_sycn.loopCounter = 0;
		
			dp.devType_gasDetector.gasConcentration = devGasDetect_alarmAnaLevel;
			(devGasDetect_alarmAnaLevel > 2)?
				(dp.devType_gasDetector.alarmFlg = 1):
				(dp.devType_gasDetector.alarmFlg = 0);
			currentDev_dataPointSet(&dp, false, false, false, true, false);
		}

		if(timerLoop_alarm.loopCounter < timerLoop_alarm.loopPeriod)timerLoop_alarm.loopCounter ++;
		else{
			
			stt_devDataPonitTypedef dp = {0};

			timerLoop_alarm.loopCounter = 0;
		
			currentDev_dataPointGet(&dp);
			if(dp.devType_gasDetector.alarmFlg){
				xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_DETECT_ALARM_REPORT);				
			}
		}

		vTaskDelay(vTaskLoopIdle_time / portTICK_RATE_MS);
	}
 }

 static void check_efuse(void){
	 
	 //Check TP is burned into eFuse
	 if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
		 printf("eFuse Two Point: Supported\n");
	 } else {
		 printf("eFuse Two Point: NOT supported\n");
	 }
 
	 //Check Vref is burned into eFuse
	 if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
		 printf("eFuse Vref: Supported\n");
	 } else {
		 printf("eFuse Vref: NOT supported\n");
	 }
 }
 
 static void print_char_val_type(esp_adc_cal_value_t val_type){
 
	 if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
		 printf("Characterized using Two Point Value\n");
	 } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
		 printf("Characterized using eFuse Vref\n");
	 } else {
		 printf("Characterized using Default Vref\n");
	 }
 }

 static void devDriverBussiness_gasDetect_adcInit(void){
 
	 //Check if Two Point or Vref are burned into eFuse
	 check_efuse();
 
	 //Configure ADC
	 if (unit == ADC_UNIT_1) {
		 adc1_config_width(ADC_WIDTH_BIT_12);
		 adc1_config_channel_atten(channel, atten);
	 } else {
		 adc2_config_channel_atten((adc2_channel_t)channel, atten);
	 }
 
	 //Characterize ADC
	 adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	 esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEVDRIVER_GAS_DETECTOR_DEFAULT_VREF, adc_chars);
	 print_char_val_type(val_type);
 }

 static void devDriverBussiness_gasDetect_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_gasDetect)return;
	
	io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
	io_conf.pin_bit_mask = ((uint64_t)1 << DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en 	 = 0;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(DEV_GASDETECTOR_ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP, gpioGasBeep_isr_handler, (void*) DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP);

	devDriverBussiness_gasDetect_adcInit();
 }

 static void devDriverBussiness_gasDetect_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP);
 }

 static uint8_t gasDeteting_anaLevel_generate(uint16_t cnec){

	uint8_t res = 0;

	if(cnec < gasConcentrationLevel_analogTab[0])res = 0;
	else
	if(cnec >= gasConcentrationLevel_analogTab[0] && cnec < gasConcentrationLevel_analogTab[1])res = 1;
	else
	if(cnec >= gasConcentrationLevel_analogTab[1] && cnec < gasConcentrationLevel_analogTab[2])res = 2;
	else
	if(cnec >= gasConcentrationLevel_analogTab[2] && cnec < gasConcentrationLevel_analogTab[3])res = 3;
	else
	if(cnec >= gasConcentrationLevel_analogTab[3] && cnec < gasConcentrationLevel_analogTab[4])res = 4;
	else
	if(cnec >= gasConcentrationLevel_analogTab[4] && cnec < gasConcentrationLevel_analogTab[5])res = 5;
	else
	if(cnec >= gasConcentrationLevel_analogTab[5] && cnec < gasConcentrationLevel_analogTab[6])res = 6;
	else
	if(cnec >= gasConcentrationLevel_analogTab[6] && cnec < gasConcentrationLevel_analogTab[7])res = 7;
	else
	if(cnec >= gasConcentrationLevel_analogTab[7] && cnec < gasConcentrationLevel_analogTab[8])res = 8;
	else
	if(cnec > gasConcentrationLevel_analogTab[8])res = 9;

	return res;
 }

 static uint8_t gasDeteting_digLevel_generate(uint16_t cnec){

	uint8_t res = 0;
	
	if(cnec < gasConcentrationLevel_digitalTab[0])res = 0;
	else
	if(cnec >= gasConcentrationLevel_digitalTab[0] && cnec < gasConcentrationLevel_digitalTab[1])res = 1;
	else
	if(cnec >= gasConcentrationLevel_digitalTab[1] && cnec < gasConcentrationLevel_digitalTab[2])res = 2;
	else
	if(cnec >= gasConcentrationLevel_digitalTab[2])res = 3;

	return res;
 }

 void devDriverBussiness_gasDetect_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_gasDetect)return;
	if(devDriver_moudleInitialize_Flg)return;

	esp_log_level_set(TAG, ESP_LOG_INFO);

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #else

	devDriverBussiness_gasDetect_periphInit();
 #endif

 	msgQh_devTipsIntIstHandle = xQueueCreate(2, sizeof(stt_devGasDetectAssistTkMsg));

	xTaskCreate(processTask_devGasDetectBussiness, "devGasDetectTk", 1024 * 4, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_gasDetectBuss);
	devDriverBussinessGasDetect_wakeUp_handle();

	devDriver_moudleInitialize_Flg = true;
 }

 void devDriverBussiness_gasDetect_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
 #else

	devDriverBussiness_gasDetect_periphDeinit();
 #endif

	if(NULL != tHandle_gasDetectBuss){

		vTaskDelete(tHandle_gasDetectBuss);

		tHandle_gasDetectBuss = NULL;
	}

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_gasDetect_alarmParamGet(stt_devDetectingDataDef *param){

	stt_devDetectingDataDef dp = {0};
	
	esp_wifi_get_mac(ESP_IF_WIFI_STA, dp.devAddr); //设备MAC
	dp.devType = currentDev_typeGet(); //设备类型
	deviceParamGet_timeZone((stt_timeZone *)&dp.devTimeZone); //时区
	currentDev_dataPointGet((stt_devDataPonitTypedef *)&dp.alarmParam[0]); //数据点
	dp.devFirmwareVer = systemDevice_currentVersionGet(); //固件版本

	memcpy(param, &dp, sizeof(stt_devDetectingDataDef));
 }

 void devDriverBussiness_gasDetect_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_gasDetect){
		
	/*业务空置，燃气报警器不存在控制业务*/

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #else

 #endif
	}
 }

 void devDriverBussiness_gasDetect_assistantLooper1sDetector(void){

	if(devGasDetect_clamDownSleepCounter)devGasDetect_clamDownSleepCounter --;

	freqGasBeep_detectCfm = freqGasBeep_detectCnt;
	freqGasBeep_detectCnt = 0;
	devGasDetect_alarmDigLevel = gasDeteting_digLevel_generate(freqGasBeep_detectCfm);
	if(devGasDetect_alarmDigLevel)devGasDetect_alarmClamdownTime = 3;
	if(devGasDetect_alarmClamdownTime)devGasDetect_alarmClamdownTime --;
	
	uint32_t valAdc = adc1_get_raw((adc1_channel_t)channel);
	uint32_t voltage = esp_adc_cal_raw_to_voltage(valAdc, adc_chars);
	devGasDetect_alarmAnaLevel = gasDeteting_anaLevel_generate(voltage);

	ESP_LOGI(TAG, "gasAnalog val:(%d)%dmv, gasBeepFreq:(%d)%d, idle time:%d, gpio_detectBeep:%d, gpio_usrKey:%d", 
				  devGasDetect_alarmAnaLevel,
				  voltage,
				  devGasDetect_alarmDigLevel,
				  freqGasBeep_detectCfm, 
				  devGasDetect_clamDownSleepCounter, 
				  gpio_get_level(DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP), 
				  gpio_get_level(DEVDRIVER_GASDETECTOR_GPIO_INPUT_USRKEY));
 }

 void devDriverBussinessGasDetect_wakeUp_handle(void){

	esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

	switch(cause){

		case ESP_SLEEP_WAKEUP_EXT1:{
			uint64_t gpioMask = esp_sleep_get_ext1_wakeup_status();
			ESP_LOGI(TAG, "wake up cause ext1, mask:0x%016llX.", gpioMask);
			if(gpioMask & ((uint64_t)1 << DEVDRIVER_GASDETECTOR_GPIO_INPUT_USRKEY))
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

 void devDriverBussinessGasDetect_flgSycnUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_sycn = 1):
		(flgClusterDataUpdata.flg_sycn = 0);
 }

 void devDriverBussinessGasDetect_flgHbeatUpdated_set(bool val){
	(true == val)?
		(flgClusterDataUpdata.flg_hb = 1):
		(flgClusterDataUpdata.flg_hb = 0);
 }

#endif







