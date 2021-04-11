#include "devDriver_curtain.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "devDriver_manage.h"

#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1		  27
#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2		  14
#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3		  12

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DEVDRIVER_CURTAIN_ACTION_OPEN()			devDriverAppFromISR_statusExexuteBySlaveMcu(0x02)
 #define DEVDRIVER_CURTAIN_ACTION_STOP()			devDriverAppFromISR_statusExexuteBySlaveMcu(0x00)
 #define DEVDRIVER_CURTAIN_ACTION_CLOSE()			devDriverAppFromISR_statusExexuteBySlaveMcu(0x01)
#else

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RELAY_BOX)
  #define DEVDRIVER_CURTAIN_ACTION_OPEN() 		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)1);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0)
  #define DEVDRIVER_CURTAIN_ACTION_STOP() 		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0)
  #define DEVDRIVER_CURTAIN_ACTION_CLOSE()		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)1);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0)
 #else
  #define DEVDRIVER_CURTAIN_ACTION_OPEN() 		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)0);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)1)
  #define DEVDRIVER_CURTAIN_ACTION_STOP() 		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
  #define DEVDRIVER_CURTAIN_ACTION_CLOSE()		  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
												  	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
												 	gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)1)
 #endif
#endif

static bool devDriver_moudleInitialize_Flg = false;

static stt_Curtain_motorAttr devParam_curtain = {
	.devRunningParam.act_period = 30000UL,
};
static uint32_t curtainOrbitalPositionTimeSet = 0;

static volatile enum{
	orbitalDetectState_null = -1,
	orbitalDetectState_none,
	orbitalDetectState_standBy,
	orbitalDetectState_step_a,
	orbitalDetectState_step_b,
	orbitalDetectState_step_complete,
	orbitalCalibrateCustom_a,
	orbitalCalibrateCustom_b,
}devCurtainOrbitalDetectState = orbitalDetectState_none;
static volatile uint32_t orbitalTimeDetectCounter = 0;
static const uint32_t devCurtainMotorStartError = 650; //电机上电到启动所需时间
static uint32_t devCurtainMotorStartUpCnt = 0;

xQueueHandle msgQh_devCurtainDriver = NULL;
xQueueHandle msgQh_devCurtainCalibration = NULL;
	
void devCurtain_runningParamSet(stt_devCurtain_runningParam *param, bool nvsRecord_IF){ //轨道时间为ms，注意设置存储时进行千倍乘

	if(param->act_period < (1000UL * 5))return; //最小限制

	memcpy(&(devParam_curtain.devRunningParam), param, sizeof(stt_devCurtain_runningParam));

	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam));
	}
}

void devCurtain_runningParamGet(stt_devCurtain_runningParam *param){ //轨道时间为ms，注意设置获取时进行千倍除

	memcpy(param, &(devParam_curtain.devRunningParam), sizeof(stt_devCurtain_runningParam));
}

void devCurtain_currentOrbitalPeriodTimeSet(uint8_t timeVal){

	devParam_curtain.devRunningParam.act_period = (uint32_t)timeVal * 1000UL;
}

uint8_t devCurtain_currentPositionPercentGet(void){

	return (uint8_t)((devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period);
}

stt_devCurtain_runningStatus devCurtain_currentRunningStatusGet(void){

	return devParam_curtain.act;
}

void devCurtain_ocrbitalTimeSaveInitiative(void){

	devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam)); //位置记录执行
}

void devDriverBussiness_curtainSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if((swCurrentDevType != devTypeDef_curtain)&&\
	   (swCurrentDevType != devTypeDef_moudleSwCurtain)&&\
	   (swCurrentDevType != devTypeDef_relayBox_curtain))return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1)|\ 
						   (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2)|\ 
						   (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

void devDriverBussiness_curtainSwitch_periphDeinit(void){

	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1);
	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2);
	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3);
}

void devDriverBussiness_curtainSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType != devTypeDef_curtain)&&\
	   (swCurrentDevType != devTypeDef_moudleSwCurtain)&&\
	   (swCurrentDevType != devTypeDef_relayBox_curtain))return;
	if(true == devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
			
#else

	devDriverBussiness_curtainSwitch_periphInit();	
#endif

	msgQh_devCurtainDriver = xQueueCreate(1, sizeof(stt_msgDats_devCurtainDriver)); //队列长度为1，保证实时性
	msgQh_devCurtainCalibration = xQueueCreate(1, sizeof(stt_msgDats_devCurtainDriver)); //队列长度为3

	devDriver_moudleInitialize_Flg = true;	
}

void devDriverBussiness_curtainSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
				
#else

	devDriverBussiness_curtainSwitch_periphDeinit();
#endif

	vQueueDelete(msgQh_devCurtainDriver);
	msgQh_devCurtainDriver = NULL;
	vQueueDelete(msgQh_devCurtainCalibration);
	msgQh_devCurtainCalibration = NULL;	
	
	devDriver_moudleInitialize_Flg = false;
}

bool IRAM_ATTR devDriverBussiness_curtainSwitch_devRunningDetect(void){ //开关设备运行监测，每ms调用

	bool devCurtainOrbitalTimeSave_IF = false;
	stt_msgDats_devCurtainDriver sptr_msgQ_devCurtainDriver = {0};
	const uint32_t motorPowerFailureVoltageMin = 3000;
	const uint32_t motorPowerFailureVoltageMax = 8200;
	const uint32_t motorCalibraTimeBuffer = 5500;
	uint32_t devCurtainMotorPower = devDriverBussiness_elecMeasure_valElecPowerMwGet();
	static stt_devCurtain_runningStatus act_localRecord = curtainRunningStatus_cTact_stop;
	static bool motorOverLoad_flg = false;

	switch(devCurtainOrbitalDetectState){

		case orbitalCalibrateCustom_a:

			if(orbitalTimeDetectCounter){
				orbitalTimeDetectCounter --;
				if(0 == (orbitalTimeDetectCounter % 1000)){
					sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraCustom_a;
					sptr_msgQ_devCurtainDriver.msgDats.data_orbitalCalibraProcess.orbitalTimeDetectSeconds = (uint8_t)(orbitalTimeDetectCounter / 1000);
					xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
				}
				DEVDRIVER_CURTAIN_ACTION_CLOSE();
			}
			else
			{
				devCurtainOrbitalDetectState = orbitalCalibrateCustom_b;
			}
			break;

		case orbitalCalibrateCustom_b:

			DEVDRIVER_CURTAIN_ACTION_STOP();
			devCurtainOrbitalDetectState = orbitalDetectState_none;

			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraCustom_b;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalCalibraProcess.orbitalTimeDetectSeconds = (uint8_t)(orbitalTimeDetectCounter / 1000);
			xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
			break;

		case orbitalDetectState_standBy:

			orbitalTimeDetectCounter = 0;

			DEVDRIVER_CURTAIN_ACTION_OPEN();
			devCurtainOrbitalDetectState = orbitalDetectState_step_a;

			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraStart;
			xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
			break;

		case orbitalDetectState_step_a:

			orbitalTimeDetectCounter ++;
			if(orbitalTimeDetectCounter > motorCalibraTimeBuffer){
				// if(devCurtainMotorPower > motorPowerFailureVoltageMax){
				// 	DEVDRIVER_CURTAIN_ACTION_CLOSE();
				// 	orbitalTimeDetectCounter = 0;
				// 	devCurtainOrbitalDetectState = orbitalDetectState_step_b;
				// 	motorOverLoad_flg = true;
				// }else
				 if(devCurtainMotorPower < motorPowerFailureVoltageMin){
					DEVDRIVER_CURTAIN_ACTION_CLOSE();
					orbitalTimeDetectCounter = 0;
					devCurtainOrbitalDetectState = orbitalDetectState_step_b;
					motorOverLoad_flg = false;
				 }
			}

			if(0 == (orbitalTimeDetectCounter % 1000)){
				sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraProcess_a;
				sptr_msgQ_devCurtainDriver.msgDats.data_orbitalCalibraProcess.orbitalTimeDetectSeconds = (uint8_t)(orbitalTimeDetectCounter / 1000);
				xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
			}
			break;

		case orbitalDetectState_step_b:

			orbitalTimeDetectCounter ++;
			if(orbitalTimeDetectCounter > motorCalibraTimeBuffer){
				// if(devCurtainMotorPower > motorPowerFailureVoltageMax){
				// 	DEVDRIVER_CURTAIN_ACTION_STOP();
				// 	devCurtainOrbitalDetectState = orbitalDetectState_step_complete;
				// 	motorOverLoad_flg = true;
				// }else
				if(devCurtainMotorPower < motorPowerFailureVoltageMin){
					DEVDRIVER_CURTAIN_ACTION_STOP();
					devCurtainOrbitalDetectState = orbitalDetectState_step_complete;
					motorOverLoad_flg = false;
				}
			}

			if(0 == (orbitalTimeDetectCounter % 1000)){
				sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraProcess_b;
				sptr_msgQ_devCurtainDriver.msgDats.data_orbitalCalibraProcess.orbitalTimeDetectSeconds = (uint8_t)(orbitalTimeDetectCounter / 1000);
				xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
			}
			break;

		case orbitalDetectState_step_complete:
		
			devCurtainOrbitalDetectState = orbitalDetectState_none;

			devParam_curtain.devRunningParam.act_counter = 0; //轨道位置复位
			if((10UL * 1000UL) < orbitalTimeDetectCounter){ //轨道时间合法则进行设定
				(true == motorOverLoad_flg)? //机动误差
					(orbitalTimeDetectCounter -= 600):
					(orbitalTimeDetectCounter -= 8500);
				devParam_curtain.devRunningParam.act_period = orbitalTimeDetectCounter; //减去机动误差
			}

			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalCalibraComplete;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalCalibraProcess.orbitalTimeDetectSeconds = (uint8_t)(orbitalTimeDetectCounter / 1000);
			xQueueOverwriteFromISR(msgQh_devCurtainCalibration, &sptr_msgQ_devCurtainDriver, NULL);
			orbitalTimeDetectCounter = 0; //最后清零
			break;

		default:break;
	}
	if(orbitalDetectState_none != devCurtainOrbitalDetectState){ //轨道时间探测阶段不执行常规业务
		return devCurtainOrbitalTimeSave_IF;
	}

	if(devParam_curtain.act == curtainRunningStatus_cTact_stop){

		if(act_localRecord != devParam_curtain.act){

			act_localRecord = devParam_curtain.act;
			 //返停触发消息发送以便进行UI更新
			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_opreatStop;
			sptr_msgQ_devCurtainDriver.msgDats.data_opreatStop.opreatStop_sig = 1;
			xQueueOverwriteFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
//			xQueueSendToFrontFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
		
	}else{

		act_localRecord = devParam_curtain.act;
	}

	if((devParam_curtain.act == curtainRunningStatus_cTact_close) ||
	   (devParam_curtain.act == curtainRunningStatus_cTact_open)){

		static uint8_t orbitalPercent_localRecord = 0;
		uint8_t orbitalPercent_temp = (devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period;

		if(orbitalPercent_localRecord != orbitalPercent_temp){

			orbitalPercent_localRecord = orbitalPercent_temp;

			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalChgingByBtn;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent = orbitalPercent_localRecord;
			xQueueSendFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
	}

	if(devParam_curtain.act == curtainRunningStatus_cTact_custom){

		static uint8_t orbitalPercent_localRecord = 0;
		uint8_t orbitalPercent_temp = (devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period;
		
		if(orbitalPercent_localRecord != orbitalPercent_temp){
		
			orbitalPercent_localRecord = orbitalPercent_temp;
		
			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalChgingBySlider;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingBySlider.orbitalPosPercent = orbitalPercent_localRecord;
			xQueueSendFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
	}

	switch(devParam_curtain.act){

		case curtainRunningStatus_cTact_stop:{

			static uint32_t curtainOrbitalPosTime_localRecord = 0;

			if(curtainOrbitalPosTime_localRecord != devParam_curtain.devRunningParam.act_counter){ //轨道位置时间不同时才进行存储操作

				curtainOrbitalPosTime_localRecord = devParam_curtain.devRunningParam.act_counter;

				devCurtainOrbitalTimeSave_IF = true;
			
//				devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam)); //位置记录执行 --中断内无法进行nvs操作，通过返回标志位devCurtainOrbitalTimeSave_IF后使用消息队列进行nvs存储信号传输
			}

			curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_counter;

			DEVDRIVER_CURTAIN_ACTION_STOP();

		}break;
		
		case curtainRunningStatus_cTact_close:{

			if(devParam_curtain.devRunningParam.act_counter != 0)devParam_curtain.devRunningParam.act_counter --;
			else
			// if((devCurtainMotorPower > motorPowerFailureVoltageMin) && (devCurtainMotorPower < motorPowerFailureVoltageMax)); //电机正在运行，不动作
			if(devCurtainMotorPower > motorPowerFailureVoltageMin);
			else{

				if(devParam_curtain.devRunningParam.act_period){ //轨道时间不为零则返停

					stt_devDataPonitTypedef devDataPoint = {0};

					currentDev_dataPointGet(&devDataPoint);

					devParam_curtain.devRunningParam.act_counter = 0;
					devParam_curtain.act = curtainRunningStatus_cTact_stop;

					devDataPoint.devType_curtain.devCurtain_actEnumVal = curtainRunningStatus_cTact_stop;
					currentDev_dataPointRecovery(&devDataPoint, false, false, false, false, false);

					deviceDatapointSynchronousReport_actionTrig(); //状态反向同步
				}
			}

			DEVDRIVER_CURTAIN_ACTION_CLOSE();

		}break;
		
		case curtainRunningStatus_cTact_open:{

			if(devParam_curtain.devRunningParam.act_counter < devParam_curtain.devRunningParam.act_period)devParam_curtain.devRunningParam.act_counter ++;
			else
			// if((devCurtainMotorPower > motorPowerFailureVoltageMin) && (devCurtainMotorPower < motorPowerFailureVoltageMax)); //电机正在运行，不动作
			if(devCurtainMotorPower > motorPowerFailureVoltageMin);
			else{

				if(devParam_curtain.devRunningParam.act_period){ //轨道时间溢出返停

					stt_devDataPonitTypedef devDataPoint = {0};
					
					currentDev_dataPointGet(&devDataPoint);
				
					devParam_curtain.devRunningParam.act_counter = devParam_curtain.devRunningParam.act_period;
					devParam_curtain.act = curtainRunningStatus_cTact_stop;

					devDataPoint.devType_curtain.devCurtain_actEnumVal = curtainRunningStatus_cTact_stop;
					currentDev_dataPointRecovery(&devDataPoint, false, false, false, false, false);

					deviceDatapointSynchronousReport_actionTrig(); //状态反向同步
				}
			}

			DEVDRIVER_CURTAIN_ACTION_OPEN();

		}break;

		case curtainRunningStatus_cTact_custom:{

			if(curtainOrbitalPositionTimeSet > devParam_curtain.devRunningParam.act_period)
				curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_period;

			if(devParam_curtain.devRunningParam.act_counter < curtainOrbitalPositionTimeSet){

				devParam_curtain.devRunningParam.act_counter ++;

				DEVDRIVER_CURTAIN_ACTION_OPEN();
			}
			else
			if(devParam_curtain.devRunningParam.act_counter > curtainOrbitalPositionTimeSet){

				devParam_curtain.devRunningParam.act_counter --;

				DEVDRIVER_CURTAIN_ACTION_CLOSE();
			}
			else{

				if(devCurtainMotorStartUpCnt)devCurtainMotorStartUpCnt --;
				else{
					devParam_curtain.devRunningParam.act_counter = curtainOrbitalPositionTimeSet;
					devParam_curtain.act = curtainRunningStatus_cTact_stop;
				}
			}
			
		}break;

		default:break;
	}

	return devCurtainOrbitalTimeSave_IF;
}

static void devDriverBussiness_curtainSwitch_periphStatusRealesByBtn(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType == devTypeDef_curtain)||\
	   (swCurrentDevType == devTypeDef_moudleSwCurtain)||\
	   (swCurrentDevType == devTypeDef_relayBox_curtain)){

		devParam_curtain.act = param->devType_curtain.devCurtain_actEnumVal;

		if((devParam_curtain.act == curtainRunningStatus_cTact_open) && //满开触底操作
		   (devParam_curtain.devRunningParam.act_counter == devParam_curtain.devRunningParam.act_period) &&
		   (devParam_curtain.devRunningParam.act_period != 0)){

			devParam_curtain.devRunningParam.act_counter -= DEVICE_CURTAIN_OPREATION_EXTRA_TIME;
			
		}else
		if((devParam_curtain.act == curtainRunningStatus_cTact_close) && //满关触底操作
		   (devParam_curtain.devRunningParam.act_counter == 0) &&
		   (devParam_curtain.devRunningParam.act_period != 0)){

			devParam_curtain.devRunningParam.act_counter += DEVICE_CURTAIN_OPREATION_EXTRA_TIME;
		}
	}
}

static void devDriverBussiness_curtainSwitch_periphStatusRealesBySlide(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType == devTypeDef_curtain)||\
	   (swCurrentDevType == devTypeDef_moudleSwCurtain)||\
	   (swCurrentDevType == devTypeDef_relayBox_curtain)){

		devParam_curtain.act = curtainRunningStatus_cTact_custom;
	
		curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_period / 
																			DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL * 
																			param->devType_curtain.devCurtain_actEnumVal;
		devCurtainMotorStartUpCnt = devCurtainMotorStartError;
//		printf("curtain orbital set:%d.\n", curtainOrbitalPositionTimeSet);
	}
}

void devDriverBussiness_curtainSwitch_orbitalRecalibration(void){

	stt_devDataPonitTypedef dp = {
		.devType_curtain.devCurtain_actMethod = 0,
		.devType_curtain.devCurtain_actEnumVal = curtainRunningStatus_cTact_close,
	};

	devParam_curtain.devRunningParam.act_counter = devParam_curtain.devRunningParam.act_period;
	devDriverBussiness_curtainSwitch_periphStatusReales(&dp);

	if(orbitalDetectState_none == devCurtainOrbitalDetectState){
		orbitalTimeDetectCounter = devParam_curtain.devRunningParam.act_period;
		devCurtainOrbitalDetectState = orbitalCalibrateCustom_a;
	}
}

void devDriverBussiness_curtainSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	(param->devType_curtain.devCurtain_actMethod)?
		(devDriverBussiness_curtainSwitch_periphStatusRealesBySlide(param)):
		(devDriverBussiness_curtainSwitch_periphStatusRealesByBtn(param));
}

void devDriverBussiness_curtainSwitch_orbitalTimeAutoDetectTrig(void){

	if(orbitalDetectState_none == devCurtainOrbitalDetectState)
		devCurtainOrbitalDetectState = orbitalDetectState_standBy;
}


