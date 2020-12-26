#include "devDriver_solarSysManager.h"

#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"

#include "os.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "dataTrans_localHandler.h"

#include "bussiness_timerSoft.h"

extern EventGroupHandle_t xEventGp_devAppSupplemet_A;
extern EventGroupHandle_t xEventGp_devApplication;

extern stt_nodeObj_listManageDevCtrlBase *listHead_nodeCtrlObjBlockBaseManage;

extern void usrApp_fullScreenRefresh_self(uint16_t freshTime, lv_coord_t y);
extern void nvsDataOpreation_solarSysManagerDevList_generateToList(stt_listAttrSolarSysManager *listHaed, uint8_t stageBithold);
extern void nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(stt_listAttrSolarSysManager *srcListHaed, 
																								   stt_listAttrSolarSysManager *TarListHead,
																								   uint8_t stageBithold);
extern void nvsDataOpreation_solarSysManagerDevList_set(stt_solarSysManagerDevList_nvsOpreat *param, enum_solarSysManager_ctrlStage devListStage);

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

 extern stt_listAttrSolarSysManager *listHead_SSMRdevCtrlUnitManage;

 static stt_listAttrSolarSysManager *listHead_SSMRdevCtrlUnitManageTemp;

 static const char *TAG = "lanbon_L8 - devDriverSolarSysManager";
 static const stt_solarSysManager_operationParam devParam_solarSysManager_defInit = {

	.temperature_alarm = 50,
	.voltage_ctrlMax = 60,
	.voltage_ctrlMin = 40,
	.voltageAlarm_En = 0,
 };
static const stt_solarSysManagerCtrlOperateParam devSolarSysManagerCtrlOperateParam_defInit = {

	.temperature_alarm = 46.0F,
	.voltageTurnOn = 47.0F,
	.voltageTurnOnDelay = 0.0F,
	.voltageTurnOffStage_A = 45.0F,
	.voltageTurnOffStage_B = 43.0F,
	.voltageTurnOffStage_C = 41.0F,
	.voltageTurnOffStageA_en = 0,
	.voltageTurnOffStageB_en = 0,
	.voltageTurnOffStageC_en = 0,
};

 static bool devDriver_moudleInitialize_Flg = false;

 static esp_adc_cal_characteristics_t *adc_chars;
 static const adc1_channel_t channel = ADC1_CHANNEL_7;   
 static const adc_atten_t atten = ADC_ATTEN_DB_0;
 static const adc_unit_t unit = ADC_UNIT_1;

 static float voltageCur_solarCell = 45.0F;

 static enum_solarSysManager_ctrlStage ssmrStage_cur = ssmrCtrlStage_normal,
 									   ssmrStage_rcd = ssmrCtrlStage_normal;
 static bool volatgeDescendingSatge_flg = false;

 static bool SSMR_voltageNormal_FLG = true,
			SSMR_temperatureNormal_FLG = true;

 static stt_solarSysManager_operationParam *devParam_solarSysManager = NULL;
 static stt_solarSysManagerCtrlOperateParam *devSolarSysManagerCtrlOperateParam = NULL;
 static stt_solarSysManagerDevList_nvsOpreat *devCtrlList_solarSysManager = NULL;

 static bool devParamCfgChgSync_flg = false;

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

 static void devSolarSysManager_voltageDetect_ADC_init(void){

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
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEVDRIVER_SOLARSYSMANAGER_DEFAULT_ADC_VREF, adc_chars);
	print_char_val_type(val_type);
 }

 static void devDriverBussiness_solarSysManager_paramMemoryInit(void){

	if(devParam_solarSysManager == NULL){
		devParam_solarSysManager = (stt_solarSysManager_operationParam *)os_zalloc(sizeof(stt_solarSysManager_operationParam));
		if(NULL != devParam_solarSysManager){

			memcpy(devParam_solarSysManager, &devParam_solarSysManager_defInit, sizeof(stt_solarSysManager_operationParam));
		}
	}

	if(devSolarSysManagerCtrlOperateParam == NULL){
		devSolarSysManagerCtrlOperateParam = (stt_solarSysManagerCtrlOperateParam *)os_zalloc(sizeof(stt_solarSysManagerCtrlOperateParam));
		if(NULL != devSolarSysManagerCtrlOperateParam){
	
			memcpy(devSolarSysManagerCtrlOperateParam, &devSolarSysManagerCtrlOperateParam_defInit, sizeof(stt_solarSysManagerCtrlOperateParam));
		}
	}
	
	if(devCtrlList_solarSysManager == NULL)
		devCtrlList_solarSysManager = (stt_solarSysManagerDevList_nvsOpreat *)os_zalloc(sizeof(stt_solarSysManagerDevList_nvsOpreat));

	if(NULL == listHead_SSMRdevCtrlUnitManage)
		listHead_SSMRdevCtrlUnitManage = (stt_listAttrSolarSysManager *)os_zalloc(sizeof(stt_listAttrSolarSysManager));	
	if(NULL == listHead_SSMRdevCtrlUnitManageTemp)
		listHead_SSMRdevCtrlUnitManageTemp = (stt_listAttrSolarSysManager *)os_zalloc(sizeof(stt_listAttrSolarSysManager)); 
 }

 static void devDriverBussiness_solarSysManager_periphInit(void){

	devSolarSysManager_voltageDetect_ADC_init();
 }

 static void devDriverBussiness_solarSysManager_periphDeinit(void){

	
 }

 static void devDriverBussiness_solarSysVoltage_reales(void){

	const uint16_t calcuPeriod = 1000;
	const float weightPowerCalcu = 0.35F;
	uint16_t calcuLoop = 0;
	int adcCalcu_temp = 0;
	uint32_t rawRd_temp = 0;
	float voltageTemp = 0.0F;
	static float voltageRcd = 0.0F;

	for(calcuLoop = 0; calcuLoop < calcuPeriod; calcuLoop ++)
		adcCalcu_temp += adc1_get_raw((adc1_channel_t)channel);
	adcCalcu_temp /= calcuPeriod;
	rawRd_temp = esp_adc_cal_raw_to_voltage(adcCalcu_temp, adc_chars);

	voltageTemp = (float)(rawRd_temp * 69) / 1000.0F;
	if(0.0F != voltageRcd){ //权值滤波：weightPowerCalcu
		voltageRcd = (voltageRcd * (1.0F - weightPowerCalcu)) + voltageTemp * weightPowerCalcu;
		voltageCur_solarCell = voltageRcd;
	}
	else
	{
		voltageRcd = voltageTemp;
	}
 }

 static void devDriverBussiness_solarSysManager_bussinessDataReq(uint8_t dstMac[MWIFI_ADDR_LEN], 
 																				  bool normalOpt, 
 																				  bool flgFirstTimeTrig,
 																				  bool lightDeviceRecoverOnly){

	stt_nodeObj_listManageDevCtrlBase *devNode = NULL;

	if(false == mwifi_is_connected())return;

	devNode = lvglUsrApp_devCtrlBlockBaseManageList_nodeGet(listHead_nodeCtrlObjBlockBaseManage, dstMac, true);
	if(NULL != devNode){

		mdf_err_t ret = MDF_OK;

		const uint8_t devMacDest[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
		const mlink_httpd_type_t type_L8mesh_cst = {
		
			.format = MLINK_HTTPD_FORMAT_HEX,
		};
		mwifi_data_type_t data_type = {
			
			.compression = true,
			.communicate = MWIFI_COMMUNICATE_UNICAST,
		};

		stt_devDataPonitTypedef dataPoint_temp = {0};
		bool dataTX_enable = false;

		memcpy(&(data_type.custom), &type_L8mesh_cst, sizeof(uint32_t));

		memcpy(&dataPoint_temp, &devNode->nodeData.devStatusVal, sizeof(uint8_t));
		if(false == normalOpt){ //异常

			switch(devNode->nodeData.ctrlObj_devType){ 

				case devTypeDef_mulitSwOneBit:
				case devTypeDef_mulitSwTwoBit:
				case devTypeDef_mulitSwThreeBit:
				case devTypeDef_rgbLampBelt:
				case devTypeDef_rgbLampBulb:
				case devTypeDef_moudleSwOneBit:
				case devTypeDef_moudleSwTwoBit:
				case devTypeDef_moudleSwThreeBit:
				case devTypeDef_relayBox_1bit:
				case devTypeDef_relayBox_2bit:
					memset(&dataPoint_temp, 0, sizeof(stt_devDataPonitTypedef));
					dataTX_enable = true; //一直有效
					break;
				case devTypeDef_dimmer:
					dataPoint_temp.devType_dimmer.devDimmer_brightnessVal = 0;
					dataTX_enable = true; //一直有效
					break;
				case devTypeDef_thermostat:
				case devTypeDef_thermostatExtension:
					dataPoint_temp.devType_thermostat.devThermostat_running_en = 0; 
					// if(flgFirstTimeTrig)dataTX_enable = true; //仅首次触发有效
					dataTX_enable = true; //一直有效
					break;
				case devTypeDef_socket:
					dataPoint_temp.devType_socket.devSocket_opSw = 0;
					// if(flgFirstTimeTrig)dataTX_enable = true; //仅首次触发有效
					dataTX_enable = true; //一直有效
					break;
				case devTypeDef_heater:
					dataPoint_temp.devType_heater.devHeater_swEnumVal = heaterOpreatAct_close;
					// if(flgFirstTimeTrig)dataTX_enable = true; //仅首次触发有效
					dataTX_enable = true; //一直有效
					break;
				case devTypeDef_largeCurOneBit:
					dataPoint_temp.devType_mulitSwitch_oneBit.swVal_bit1 = 0;
					// if(flgFirstTimeTrig)dataTX_enable = true; //仅首次触发有效
					dataTX_enable = true; //一直有效
					break;

				default:break;
			}
		}
		else //回归正常
		{
			if(flgFirstTimeTrig){ //仅首次触发有效

				stt_listAttrSolarSysManager *listUnitNode = listOperationNodeGet_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManage, devNode->nodeData.ctrlObj_devMac, true);
				stt_solarSysManager_ctrlUnit *nodeCtrl = NULL;
				if(NULL != listUnitNode){

					bool devClose_flg = false;
					bool devStateRcd_flg = false;

					nodeCtrl = &listUnitNode->paramCtrlUnit;
					memcpy(&dataPoint_temp, &devNode->nodeData.devStatusVal, sizeof(uint8_t)); //当前值加载

					if(false == lightDeviceRecoverOnly){

						switch(devNode->nodeData.ctrlObj_devType){

							case devTypeDef_thermostat:
							case devTypeDef_thermostatExtension:{

								if(0 == dataPoint_temp.devType_thermostat.devThermostat_running_en)
									devClose_flg = true;
								else
									devStateRcd_flg = true;
							}break;
							
							case devTypeDef_socket:{

								if(0 == dataPoint_temp.devType_socket.devSocket_opSw)
									devClose_flg = true;
								else
									devStateRcd_flg = true;
							}break;
							
							case devTypeDef_heater:{

								if(heaterOpreatAct_close == dataPoint_temp.devType_heater.devHeater_swEnumVal)
									devClose_flg = true;
								else
									devStateRcd_flg = true;
							}break;

							case devTypeDef_largeCurOneBit:{

								if(0 == dataPoint_temp.devType_mulitSwitch_oneBit.swVal_bit1)
									devClose_flg = true;
								else
									devStateRcd_flg = true;
							}break;

							default:break;
						}
					}
					else
					{
						if(ssmrCtrlStage_normal == ssmrStage_cur){
							switch(devNode->nodeData.ctrlObj_devType){
								case devTypeDef_mulitSwOneBit:
								case devTypeDef_mulitSwTwoBit:
								case devTypeDef_mulitSwThreeBit:
								case devTypeDef_rgbLampBelt:
								case devTypeDef_rgbLampBulb:
								case devTypeDef_moudleSwOneBit:
								case devTypeDef_moudleSwTwoBit:
								case devTypeDef_moudleSwThreeBit:
								case devTypeDef_relayBox_1bit:
								case devTypeDef_relayBox_2bit:{
									if(0 == (devNode->nodeData.devStatusVal & 0x0f))
										devClose_flg = true;										
									else
										devStateRcd_flg = true;
								}break;

								case devTypeDef_dimmer:{
									if(0 == dataPoint_temp.devType_dimmer.devDimmer_brightnessVal)
										devClose_flg = true;	
									else
										devStateRcd_flg = true;
								}break;

								default:break;
							}
						}
					}

					if(true == devClose_flg){ //关闭情况下，若记录开则恢复记录

						memcpy(&dataPoint_temp, &nodeCtrl->unitDevStateRcd, sizeof(uint8_t));
						dataTX_enable = true;
					}
					else //非关闭情况下，刷新记录值
					{
						if(true == devStateRcd_flg)
							memcpy(&nodeCtrl->unitDevStateRcd, &dataPoint_temp, sizeof(uint8_t));
					}
				}
			}
		}

		if(true == dataTX_enable){

			uint8_t *msehDataReq_temp = (uint8_t *)os_zalloc(1 + sizeof(stt_superCtrl_dtTransParam));

			if(NULL != msehDataReq_temp){

				stt_superCtrl_dtTransParam *superCtrl_hld = (stt_superCtrl_dtTransParam *)&msehDataReq_temp[1];

				msehDataReq_temp[0] = L8DEV_MESH_CMD_SUPER_CTRL;
				memcpy(superCtrl_hld->targetDevMac, dstMac, sizeof(uint8_t) * MWIFI_ADDR_LEN);
//				memcpy(superCtrl_hld->targetDevMac, devMacDest, sizeof(uint8_t) * MWIFI_ADDR_LEN);
				memcpy(&superCtrl_hld->devStateSet, &dataPoint_temp, sizeof(uint8_t));

				if(esp_mesh_get_layer() == MESH_ROOT){
				
					ret = mwifi_root_write((const uint8_t *)dstMac, 1, &data_type, msehDataReq_temp, 1 + sizeof(stt_superCtrl_dtTransParam), true);	//数据内容填充 -头命令长度1 + 操作值数据长度
					MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 
				}
				else
				{
					ret = mwifi_write((const uint8_t *)dstMac, &data_type, msehDataReq_temp, 1 + sizeof(stt_superCtrl_dtTransParam), true);	//数据内容填充 -头命令长度1 + 操作值数据长度
					MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> heartbeat mwifi_root_write", mdf_err_to_name(ret));
				}

				devNode->nodeData.devStatusVal = superCtrl_hld->devStateSet; //缓存刷新

				os_free(msehDataReq_temp);
			}
		}
	}
	else{

//		printf("takeOverCtrl node[%02X%02X%02X%02X%02X%02X] not found.\n", MAC2STR(dstMac));
	}
 }

 bool devDriverBussiness_solarSysManager_scrWakeByVol(void){

	bool res = false;
	float volDiffVal = 0.0F;
	static float volRcd = 0.0F;

	volDiffVal = voltageCur_solarCell - volRcd;
	if((volDiffVal > 0.2F) || (volDiffVal < -0.2F)){ //差值限定
		volRcd = voltageCur_solarCell;
		res = true;
	}
	
	return res;
 }

 void devDriverBussiness_solarSysManager_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	
	if(swCurrentDevType != devTypeDef_voltageSensor)return;
	if(devDriver_moudleInitialize_Flg)return;

	esp_log_level_set(TAG, ESP_LOG_INFO);

	devDriverBussiness_solarSysManager_paramMemoryInit();
	devDriverBussiness_solarSysManager_periphInit();

	devDriver_moudleInitialize_Flg = true;
 }

 void devDriverBussiness_solarSysManager_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_solarSysManager_periphDeinit();

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_solarSysManager_assistantLooper1sDetector(void){

	//遍历管制列表
	stt_listAttrSolarSysManager *pAbove = listHead_SSMRdevCtrlUnitManage;
	stt_listAttrSolarSysManager *pFollow = NULL;

	if(devTypeDef_voltageSensor != currentDev_typeGet()){

		return;
	}

	while(pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;

		if(pAbove->paramCtrlUnit.unitDevTFCD_counterEn){

//			printf(">>>>>>>>>%d.\n", pAbove->paramCtrlUnit.unitDevTFCD_counter);
			if(pAbove->paramCtrlUnit.unitDevTFCD_counter)pAbove->paramCtrlUnit.unitDevTFCD_counter --;
			else{

				pAbove->paramCtrlUnit.unitDevTFCD_counterEn = 0;
				devDriverBussiness_solarSysManager_bussinessDataReq(pAbove->paramCtrlUnit.unitDevMac, SSMR_voltageNormal_FLG, false, false);
			}
		}
		
//		devDriverBussiness_solarSysManager_bussinessDataReq(pAbove->paramCtrlUnit.unitDevMac, SSMR_voltageNormal_FLG, differAct_firstTrigFlg, false);
	}	
 }

 void devDriverBussiness_solarSysManager_runningDetectLoop(void){

	float tempratureVal_cur = devDriverBussiness_temperatureMeasure_get();
	uint8_t temperatureCurSolarCell_u8 = (uint8_t)tempratureVal_cur;
	uint8_t voltageCurSolarCell_u8 = (uint8_t)voltageCur_solarCell;
	uint16_t voltageCurSolarCell_u16 = (uint16_t)(voltageCur_solarCell * 100.0F);
	uint16_t temperatureCurSolarCell_u16 = (uint16_t)(tempratureVal_cur * 100.0F);
	bool hostTakeOverCtrl_trig = false;
	stt_devDataPonitTypedef devDp_temp = {0};
	static uint16_t voltageCurSolarCell_u16_rcd = 0;
	static uint16_t temperatureCurSolarCell_u16_rcd = 0;
	static bool voltageNormalRcd = true,
				temperatureNormalRcd = true;
	const uint8_t period = (uint8_t)(90.0F / SOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME);
	static uint8_t counter = 0;
	stt_timerLoop tmrScrRefresh = {(const uint16_t)(120.0F / SOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME), 0};
	static stt_timerLoop ctrlActionExcuteLooper = {2, 0}; //动作周期：xSOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME
	static uint8_t ctrlActionLoop = 0;

//	printf("counter listGet:%d.\n", counter);
	if(counter < period)counter ++;
	else{ //每90s获取一次列表
		counter = 0;
		xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_DEVINFO_LIST_REQ);			
	}

	if(tmrScrRefresh.loopCounter < tmrScrRefresh.loopPeriod)tmrScrRefresh.loopCounter ++;
	else{
		tmrScrRefresh.loopCounter = 0;
//		usrApp_fullScreenRefresh_self(1000, 0); //针对电压管理器进行动态刷屏，进行花屏补救，4s冷却
	}

	devDriverBussiness_solarSysVoltage_reales(); //电压检测刷新

//	if(voltageCurSolarCell_u8 < devParam_solarSysManager->voltage_ctrlMin)SSMR_voltageNormal_FLG = false;
//	if(voltageCurSolarCell_u8 >= devParam_solarSysManager->voltage_ctrlMax)SSMR_voltageNormal_FLG = true;
	if(voltageCur_solarCell >= (devSolarSysManagerCtrlOperateParam->voltageTurnOn + devSolarSysManagerCtrlOperateParam->voltageTurnOnDelay)){ //对比电压 = 开启电压 + 延迟电压

		ssmrStage_cur = ssmrCtrlStage_normal;
		SSMR_voltageNormal_FLG = true;
	}
	else
	{
		if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageA_en){
		
			if(voltageCur_solarCell <= devSolarSysManagerCtrlOperateParam->voltageTurnOffStage_A){

				ssmrStage_cur = ssmrCtrlStage_voltageLow_A;
				SSMR_voltageNormal_FLG = false;
				if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageB_en){
			
					if(voltageCur_solarCell <= devSolarSysManagerCtrlOperateParam->voltageTurnOffStage_B){

						ssmrStage_cur = ssmrCtrlStage_voltageLow_B;
						SSMR_voltageNormal_FLG = false;
						if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageC_en){
				
							if(voltageCur_solarCell <= devSolarSysManagerCtrlOperateParam->voltageTurnOffStage_C){

								ssmrStage_cur = ssmrCtrlStage_voltageLow_C;
								SSMR_voltageNormal_FLG = false;
							}
						}
					}
				}
			}
		}
	}

//	if(temperatureCurSolarCell_u8 < devParam_solarSysManager->temperature_alarm)SSMR_temperatureNormal_FLG = true;
//	if(temperatureCurSolarCell_u8 >= devParam_solarSysManager->temperature_alarm)SSMR_temperatureNormal_FLG = false;
	if(temperatureCurSolarCell_u8 < (uint8_t)devSolarSysManagerCtrlOperateParam->temperature_alarm)SSMR_temperatureNormal_FLG = true;
	if(temperatureCurSolarCell_u8 >= (uint8_t)devSolarSysManagerCtrlOperateParam->temperature_alarm)SSMR_temperatureNormal_FLG = false;

//	if(((false == SSMR_voltageNormal_FLG) && (1 == devParam_solarSysManager->voltageAlarm_En))
//	   (false == SSMR_temperatureNormal_FLG)){ //声光tips
	if(((false == SSMR_voltageNormal_FLG) && (1 == devSolarSysManagerCtrlOperateParam->voltageAlarm_en)) ||\
	   (false == SSMR_temperatureNormal_FLG)){ //声光tips

		devBeepTips_trig(3, 10, 150, 0, 1);
		tipsOpreatSet_sysWarning(3); //氛围灯警告
	}
	   
	if(voltageCurSolarCell_u16_rcd != voltageCurSolarCell_u16){ //电压异变，及时上报

		voltageCurSolarCell_u16_rcd = voltageCurSolarCell_u16;

		currentDev_dataPointGet(&devDp_temp);
		(SSMR_temperatureNormal_FLG)?
			(devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 0):
			(devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 1);
		devDp_temp.devType_solarSysManager.solarSysManager_voltageCur = voltageCurSolarCell_u8;
		currentDev_dataPointSet(&devDp_temp, false, false, false, true, false);
	}

	if(temperatureCurSolarCell_u16_rcd != temperatureCurSolarCell_u16){ //温度异变，及时上报

		temperatureCurSolarCell_u16_rcd = temperatureCurSolarCell_u16;

		currentDev_dataPointGet(&devDp_temp);
		(SSMR_temperatureNormal_FLG)?
			(devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 0):
			(devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 1);
		devDp_temp.devType_solarSysManager.solarSysManager_voltageCur = voltageCurSolarCell_u8;
		currentDev_dataPointSet(&devDp_temp, false, false, false, true, false);
	}

	if(false == SSMR_temperatureNormal_FLG){ //温度异常，持续周期推送

		const uint8_t trigCount_Period = 15; //动作周期：xSOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME
		static uint8_t trigLoop_count = 0;

		temperatureNormalRcd = false;

		if(trigLoop_count < trigCount_Period)trigLoop_count ++;
		else{
			
			trigLoop_count = 0;
			
			currentDev_dataPointGet(&devDp_temp);
			devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 1;
			currentDev_dataPointSet(&devDp_temp, false, false, false, true, false);
		}
	}
	else
	{
		if(false == temperatureNormalRcd){ //异常恢复，关闭推送

			temperatureNormalRcd = true;

			currentDev_dataPointGet(&devDp_temp);
			devDp_temp.devType_solarSysManager.solarSysManager_tempAlarmFlg = 0;
			currentDev_dataPointSet(&devDp_temp, false, false, false, false, false);
		}
	}

	if(ssmrStage_rcd != ssmrStage_cur){ //阶段变化

		//过程降序判断
		(ssmrStage_cur > ssmrStage_rcd)?
			(volatgeDescendingSatge_flg = true):
			(volatgeDescendingSatge_flg = false);

		ESP_LOGI(TAG, "stage change happen, stgCur:%d, stgRcd:%d, %s",
					  ssmrStage_cur,
					  ssmrStage_rcd,
					  (volatgeDescendingSatge_flg)?("Descending!"):("Ascending!"));

		ssmrStage_rcd = ssmrStage_cur;

		if(volatgeDescendingSatge_flg){ //降序
		
			ctrlActionLoop = 1; //为达稳定性
		}
		else
		{
			ctrlActionLoop = 2; //为达稳定性，多控一次
		}
	}

	if(ctrlActionLoop){
		if(ctrlActionExcuteLooper.loopCounter < ctrlActionExcuteLooper.loopPeriod)ctrlActionExcuteLooper.loopCounter ++;
		else{
			ctrlActionExcuteLooper.loopCounter = 0;
			ctrlActionLoop --;

			hostTakeOverCtrl_trig = true;
		}
	}

//	if(false == SSMR_voltageNormal_FLG){ //电压异常（过低）
//	
//		const uint8_t trigCount_Period = 2; //动作周期：xSOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME
//		static uint8_t trigLoop_count = 0;
//	
//		if(true == voltageNormalRcd){ //首次触发
//	
//			trigLoop_count = trigCount_Period;
//		}
//	
//		voltageNormalRcd = false;
//	
//		if(trigLoop_count < trigCount_Period)trigLoop_count ++;
//		else{ //循环执行
//			
//			trigLoop_count = 0;
//			hostTakeOverCtrl_trig = true;
//		}
//		
//	}
//	else
//	{ //电压正常
//	
//		if(false == voltageNormalRcd){ //首次触发
//	
//			voltageNormalRcd = true;
//	
//			hostTakeOverCtrl_trig = true; //仅执行一次
//		}
//	}
	
	if(true == hostTakeOverCtrl_trig){

		xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_BITHOLD_SOLARSYS_CTRL);
	}
 }

 void devDriverBussiness_solarSysManager_StatusReales(stt_devDataPonitTypedef *param){

	/*业务空置，太阳能电池管理器不存在控制业务*/
 }

 void devDriverBussiness_solarSysManager_devList_set(stt_solarSysManagerDevList_nvsOpreat *param, bool nvsRecord_IF){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_voltageSensor){

		if(NULL == devCtrlList_solarSysManager)devDriverBussiness_solarSysManager_paramMemoryInit();
		memcpy(devCtrlList_solarSysManager, param, sizeof(stt_solarSysManagerDevList_nvsOpreat));

		if(nvsRecord_IF){

			devSystemInfoLocalRecord_save(saveObj_solarSysManager_devList, param);
		}
	}
 }

 void devDriverBussiness_solarSysManager_devList_get(stt_solarSysManagerDevList_nvsOpreat *param){

	memcpy(param, devCtrlList_solarSysManager, sizeof(stt_solarSysManagerDevList_nvsOpreat));
 }

 void devDriverBussiness_solarSysManager_devList_devStateRcdReales(uint8_t devMac[MWIFI_ADDR_LEN], uint8_t devState, bool forceRefreshToverCounterIf){

	uint8_t loop = 0;
	stt_solarSysManager_ctrlUnit *dataHandlePtr = NULL;
	uint8_t dataHandleIst = 0;
	bool devStateRcd_enable = false;

	if(NULL == devCtrlList_solarSysManager)return; //数据缓存为空，退出

	stt_listAttrSolarSysManager *devListNodePtr = NULL;
	switch(ssmrStage_cur){

		case ssmrCtrlStage_voltageLow_A:
			nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																				listHead_SSMRdevCtrlUnitManage,
																				(1 << 0));
			break;
		case ssmrCtrlStage_voltageLow_B:
			nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																				listHead_SSMRdevCtrlUnitManage,
																				(1 << 0) | (1 << 1));
			break;
		case ssmrCtrlStage_voltageLow_C:
			nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																				listHead_SSMRdevCtrlUnitManage,
																				(1 << 0) | (1 << 1) | (1 << 2));
			break;

		default:
			listOperationDestory_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp);
			break;
	}
	devListNodePtr = listOperationNodeGet_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp, devMac, true);
	if(NULL != devListNodePtr){

		stt_nodeObj_listManageDevCtrlBase *devNode = NULL;

		//ESP_LOGI(TAG, "nodeDevTakeOver node("MACSTR") find start, targetList length = %d.", MAC2STR(devMac), listOperationNodeNumDetect_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp));

		devNode = lvglUsrApp_devCtrlBlockBaseManageList_nodeGet(listHead_nodeCtrlObjBlockBaseManage, devMac, false);
		if(NULL == devNode)return;
		else{

			stt_devDataPonitTypedef dataPoint_temp = {0};
			bool takeOverCtrl_flg = false;

			memcpy(&dataPoint_temp, &devNode->nodeData.devStatusVal, sizeof(stt_devDataPonitTypedef));
			//ESP_LOGI(TAG, "nodeDevTakeOver state sycn found.");

			switch(devNode->nodeData.ctrlObj_devType){

				case devTypeDef_thermostat:{
					if(1 == dataPoint_temp.devType_thermostat.devThermostat_running_en){
						takeOverCtrl_flg = true;
					}
				}break;

				case devTypeDef_thermostatExtension:{
					if(1 == dataPoint_temp.devType_thermostat.devThermostat_running_en){
						takeOverCtrl_flg = true;
					}
				}break;

				case devTypeDef_socket:{
					if(1 == dataPoint_temp.devType_socket.devSocket_opSw){
						takeOverCtrl_flg = true;
					}
				}break;

				case devTypeDef_heater:{
					if(0 != dataPoint_temp.devType_heater.devHeater_swEnumVal){
						takeOverCtrl_flg = true;
					}
				}break;

				// case devTypeDef_thermostat:
				// case devTypeDef_thermostatExtension:
				// case devTypeDef_socket:
				// case devTypeDef_heater:{

				// 	//暂时什么都不做

				// }break;

				case devTypeDef_mulitSwOneBit:
				case devTypeDef_mulitSwTwoBit:
				case devTypeDef_mulitSwThreeBit:
				case devTypeDef_rgbLampBelt:
				case devTypeDef_rgbLampBulb:
				case devTypeDef_moudleSwOneBit:
				case devTypeDef_moudleSwTwoBit:
				case devTypeDef_moudleSwThreeBit:
				case devTypeDef_relayBox_1bit:
				case devTypeDef_relayBox_2bit:{

					if(0 != (devNode->nodeData.devStatusVal & 0x0F)){ //当前状态为开，则进行管控

						takeOverCtrl_flg = true;
//						devStateRcd_enable = true; //接管同时做记录
					}

				}break;
				
				case devTypeDef_dimmer:{

					if(dataPoint_temp.devType_dimmer.devDimmer_brightnessVal){ //当前状态为开，则进行管控

						takeOverCtrl_flg = true;
//						devStateRcd_enable = true; //接管同时做记录
					}

				}break;

				default:break;
			}

			if(true == takeOverCtrl_flg){

				stt_solarSysManager_ctrlUnit *nodeCtrl = devDriverBussiness_solarSysManager_devList_devStateRcdGet(devMac);
				if(NULL != nodeCtrl){

//					ESP_LOGI(TAG, "nodeDevTakeOver ctrl start.");

					if(true == forceRefreshToverCounterIf){ //强制

						nodeCtrl->unitDevTFCD_counterEn = 1;
						nodeCtrl->unitDevTFCD_counter = SOLARSYSMANAGER_TAKEOVER_AUTO_CLOSR_PERIOD; //额定时间倒计时后关闭
					}
					else //非强制
					{
						if(0 == nodeCtrl->unitDevTFCD_counterEn){

							nodeCtrl->unitDevTFCD_counterEn = 1;
							nodeCtrl->unitDevTFCD_counter = SOLARSYSMANAGER_TAKEOVER_AUTO_CLOSR_PERIOD; //额定时间倒计时后关闭
						}
					}
				}
				//devDriverBussiness_solarSysManager_bussinessDataReq(devMac, SSMR_voltageNormal_FLG, false, flase); //直接关闭
			}

			os_free(devNode);
		}
	}
	else
	{
		devStateRcd_enable = true;
	}

	if(true == devStateRcd_enable){ //记录

		stt_solarSysManager_ctrlUnit *nodeCtrl = devDriverBussiness_solarSysManager_devList_devStateRcdGet(devMac);

		if(NULL != nodeCtrl){

			nodeCtrl->unitDevStateRcd = devState;
//			ESP_LOGI(TAG, "nodeDev state rcd:%d.", nodeCtrl->unitDevStateRcd);	
		}
	}
 }

 stt_solarSysManager_ctrlUnit *devDriverBussiness_solarSysManager_devList_devStateRcdGet(uint8_t devMac[MWIFI_ADDR_LEN]){
	
	stt_solarSysManager_ctrlUnit *nodeRes = NULL;
	stt_listAttrSolarSysManager *pAbove = listHead_SSMRdevCtrlUnitManage;
	stt_listAttrSolarSysManager *pFollow = NULL;

	//遍历管制列表
	while(pAbove->next != NULL){
	 
		pFollow = pAbove;
		pAbove  = pFollow->next;

		if(!memcmp(pAbove->paramCtrlUnit.unitDevMac, devMac, sizeof(uint8_t) * MWIFI_ADDR_LEN)){

			nodeRes = &pAbove->paramCtrlUnit;
//			printf("SSMR devList mac[%02X%02X%02X%02X%02X%02X] state reales.\n", MAC2STR(dataHandlePtr.unitDevMac));
			break;
		}
	}

	return nodeRes;
 }

 void devDriverBussiness_solarSysManager_ctrlRunParamGet(stt_solarSysManagerCtrlOperateParam *param){

	memcpy(param, devSolarSysManagerCtrlOperateParam, sizeof(stt_solarSysManagerCtrlOperateParam));
 }

 void devDriverBussiness_solarSysManager_ctrlRunParamSet(stt_solarSysManagerCtrlOperateParam *param, bool nvsRecord_IF){

	uint8_t devListBitHoldTemp = 0;
	stt_devDataPonitTypedef devDp_temp = {0};

	if(NULL == devSolarSysManagerCtrlOperateParam)devDriverBussiness_solarSysManager_paramMemoryInit();
	memcpy(devSolarSysManagerCtrlOperateParam, param, sizeof(stt_solarSysManagerCtrlOperateParam));
	if(nvsRecord_IF){
		devSystemInfoLocalRecord_save(saveObj_solarSysManager_devCtrlRunningParam, devSolarSysManagerCtrlOperateParam);
	}

	if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageA_en)devListBitHoldTemp |= (1 << 0);
	if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageB_en)devListBitHoldTemp |= (1 << 1);
	if(devSolarSysManagerCtrlOperateParam->voltageTurnOffStageC_en)devListBitHoldTemp |= (1 << 2);
	
	nvsDataOpreation_solarSysManagerDevList_generateToList(listHead_SSMRdevCtrlUnitManage, devListBitHoldTemp); //根据阶段业务使能刷新管制链表

	if(NULL != xEventGp_devAppSupplemet_A)
		xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_CFGPARAM_SYNCHRO);

	//借助刷新数据点来刷新UI，若在主界面，则进行设置数据显示刷新
	currentDev_dataPointGet(&devDp_temp);
	currentDev_dataPointSet(&devDp_temp, false, false, false, false, false);
 }
 
 void devDriverBussiness_solarSysManager_exDevParamSet(stt_solarSysManager_operationParam *param, bool nvsRecord_IF){

	if(NULL == devParam_solarSysManager)devDriverBussiness_solarSysManager_paramMemoryInit();
	memcpy(devParam_solarSysManager, param, sizeof(stt_solarSysManager_operationParam));
	if(nvsRecord_IF){
		devSystemInfoLocalRecord_save(saveObj_solarSysManager_devOptParam, devParam_solarSysManager);
	}

	devParamCfgChgSync_flg = true;
 }

 bool devDriverBussiness_solarSysManager_paramCfgSyncValid(void){

	bool res = false;

	if(true == devParamCfgChgSync_flg){

		res = true;
		devParamCfgChgSync_flg = false;
	}

	return res;
 }

 void devDriverBussiness_solarSysManager_exDevParamGet(stt_solarSysManager_operationParam *param){

	memcpy(param, devParam_solarSysManager, sizeof(stt_solarSysManager_operationParam));
 }

 float devDriverBussiness_solarSysManager_voltageCur_get(void){

	return voltageCur_solarCell;
 }

 bool devDriverBussiness_solarSysManager_voltageAlarmEn_get(void){

	bool res = false;

//	if(devParam_solarSysManager->voltageAlarm_En)res = true;
	if(devSolarSysManagerCtrlOperateParam->voltageAlarm_en)res = true;
	
	return res;
 }

 bool devDriverBussiness_SSMR_devVoltState_get(void){

	return SSMR_voltageNormal_FLG;
 }

 bool devDriverBussiness_SSMR_devTempState_get(void){

	return SSMR_temperatureNormal_FLG;
 }

 enum_solarSysManager_ctrlStage devDriverBussiness_SSMR_devVoltageCurStage_get(void){

	return ssmrStage_cur;
 }

 void devDriverBussiness_solarSysManager_actionTrig(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(false == mwifi_is_connected())return;
	
	if(swCurrentDevType == devTypeDef_voltageSensor){

		const uint8_t actTrigLoop_max = 2;
		bool differAct_firstTrigFlg = false;
		bool lightDeviceRecoverFlg = false;
//		static bool flgRcd_devRunningStateNormal = true;
		static enum_solarSysManager_ctrlStage stageRcd = ssmrCtrlStage_normal;
		static uint8_t keepTrigCounter = 0;

//		if(flgRcd_devRunningStateNormal != SSMR_voltageNormal_FLG){

//			flgRcd_devRunningStateNormal = SSMR_voltageNormal_FLG;
//			differAct_firstTrigFlg = true;
//			keepTrigCounter = actTrigLoop_max;
//		}

		if(stageRcd != ssmrStage_cur){

			stageRcd = ssmrStage_cur;
			differAct_firstTrigFlg = true;
			keepTrigCounter = actTrigLoop_max;
		}

		if(keepTrigCounter){ //相同状态动作触发不超过额定次数

			keepTrigCounter --;

			if(volatgeDescendingSatge_flg){ //电压降序阶段，接管设备关闭

				switch(ssmrStage_cur){
					
					case ssmrCtrlStage_voltageLow_A:{

						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																							listHead_SSMRdevCtrlUnitManage,
																							(1 << 0));
					}break;
					
					case ssmrCtrlStage_voltageLow_B:{

						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																							listHead_SSMRdevCtrlUnitManage,
																							(1 << 0) | (1 << 1));
					}break;

					case ssmrCtrlStage_voltageLow_C:{

						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																							listHead_SSMRdevCtrlUnitManage,
																							(1 << 0) | (1 << 1) | (1 << 2));
					}break;

					default:
						listOperationDestory_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp);
						break;
				}
			}
			else //电压升序阶段，接管设备状态恢复
			{
				switch(ssmrStage_cur){

					case ssmrCtrlStage_normal:{ //回到正常阶段时一起恢复状态

						lightDeviceRecoverFlg = true; //恢复正常阶段时，补充恢复所有灯光设备
						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
																							listHead_SSMRdevCtrlUnitManage,
																							(1 << 0) | (1 << 1) | (1 << 2));
					}break;

//					case ssmrCtrlStage_normal:{

//						lightDeviceRecoverFlg = true; //恢复正常阶段时，补充恢复所有灯光设备
//						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
//																							listHead_SSMRdevCtrlUnitManage,
//																							(1 << 0));
//					}break;
					
//					case ssmrCtrlStage_voltageLow_A:{
//						
//						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
//																							listHead_SSMRdevCtrlUnitManage,
//																							(1 << 1));
//					}break;
//					
//					case ssmrCtrlStage_voltageLow_B:{

//						nvsDataOpreation_solarSysManagerDevList_generateToListFromTagetList(listHead_SSMRdevCtrlUnitManageTemp,
//																							listHead_SSMRdevCtrlUnitManage,
//																							(1 << 2));
//					}break;

					default:
						listOperationDestory_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp);
						break;
				}
			}

			//遍历管制列表
			stt_listAttrSolarSysManager *pAbove = listHead_SSMRdevCtrlUnitManageTemp;
			stt_listAttrSolarSysManager *pFollow = NULL;

//			ESP_LOGI(TAG, "ssmr ctrl excute, devNum:%d.", listOperationNodeNumDetect_SSMRctrlUnit(listHead_SSMRdevCtrlUnitManageTemp));
			while(pAbove->next != NULL){
				pFollow = pAbove;
				pAbove	= pFollow->next;
				devDriverBussiness_solarSysManager_bussinessDataReq(pAbove->paramCtrlUnit.unitDevMac, !volatgeDescendingSatge_flg, differAct_firstTrigFlg, false);
			}

			if(true == lightDeviceRecoverFlg){ //灯光类设备恢复，回归正常时触发
				pAbove = listHead_SSMRdevCtrlUnitManage;
				while(pAbove->next != NULL){
					pFollow = pAbove;
					pAbove	= pFollow->next;
					devDriverBussiness_solarSysManager_bussinessDataReq(pAbove->paramCtrlUnit.unitDevMac, !volatgeDescendingSatge_flg, differAct_firstTrigFlg, true);
				}
			}
		}
	}
 }

#endif



