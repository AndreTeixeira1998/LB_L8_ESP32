/**
 * @file devDriver_solarSysManager.h
 *
 */

#ifndef DEVDRIVER_SOLARSYSMANAGER_H
#define DEVDRIVER_SOLARSYSMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDriver_manage.h"

/*********************
 *      DEFINES
 *********************/
#define SOLARSYSMANAGER_RUNNINGDETECT_PERIODLOOP_TIME		1.0F

#define DEVDRIVER_SOLARSYSMANAGER_DEFAULT_ADC_VREF       	1100

#define SOLARSYSMANAGER_TAKEOVER_AUTO_CLOSR_PERIOD			57

#define SOLARSYSMANAGER_DEVLIST_REQ_PERIOD					120.0F	//单位：s， 一般不要小于SOLARSYSMANAGER_TAKEOVER_AUTO_CLOSR_PERIOD

#define SOLARSYSMANAGER_VOLTAGE_OFFSTAGE_MAX				3

#define SOLARSYSMANAGER_VOLTAGE_ADJUSTMENT_ACCURACY 		0.5F

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	ssmrCtrlStage_null = -1,
	ssmrCtrlStage_normal,
	ssmrCtrlStage_voltageLow_A,
	ssmrCtrlStage_voltageLow_B,
	ssmrCtrlStage_voltageLow_C,
}enum_solarSysManager_ctrlStage;

typedef enum{

	cellState_null = -1,
	cellState_normal = 0,
	cellState_brownout,
	cellState_overload
}enum_solarSysManager_cellState;

typedef struct{

	uint32_t temperature_alarm:7;
	uint32_t voltage_ctrlMax:7;
	uint32_t voltage_ctrlMin:7;
	uint32_t voltageAlarm_En:1;
}stt_solarSysManager_operationParam;

typedef struct{

	float temperature_alarm;
	float voltageTurnOn;
	float voltageTurnOnDelay; //-延迟触发电压，防止负载突然上电后电压骤降
	float voltageTurnOffStage_A;
	float voltageTurnOffStage_B;
	float voltageTurnOffStage_C;
	uint8_t voltageTurnOffStageA_en:1;
	uint8_t voltageTurnOffStageB_en:1;
	uint8_t voltageTurnOffStageC_en:1;
	uint8_t voltageAlarm_en:1;
}stt_solarSysManagerCtrlOperateParam;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
bool devDriverBussiness_solarSysManager_scrWakeByVol(void);
void devDriverBussiness_solarSysManager_assistantLooper1sDetector(void);
void devDriverBussiness_solarSysManager_runningDetectLoop(void);
void devDriverBussiness_solarSysManager_devList_set(stt_solarSysManagerDevList_nvsOpreat *param, bool nvsRecord_IF);
void devDriverBussiness_solarSysManager_devList_get(stt_solarSysManagerDevList_nvsOpreat *param);
void devDriverBussiness_solarSysManager_devList_devStateRcdReales(uint8_t devMac[MWIFI_ADDR_LEN], uint8_t devState, bool forceRefreshToverCounterIf);
stt_solarSysManager_ctrlUnit *devDriverBussiness_solarSysManager_devList_devStateRcdGet(uint8_t devMac[MWIFI_ADDR_LEN]);
void devDriverBussiness_solarSysManager_ctrlRunParamSet(stt_solarSysManagerCtrlOperateParam *param, bool nvsRecord_IF);
void devDriverBussiness_solarSysManager_ctrlRunParamGet(stt_solarSysManagerCtrlOperateParam *param);
void devDriverBussiness_solarSysManager_exDevParamSet(stt_solarSysManager_operationParam *param, bool nvsRecord_IF);
void devDriverBussiness_solarSysManager_exDevParamGet(stt_solarSysManager_operationParam *param);
bool devDriverBussiness_solarSysManager_voltageAlarmEn_get(void);
float devDriverBussiness_solarSysManager_voltageCur_get(void);
bool devDriverBussiness_SSMR_devVoltState_get(void);
bool devDriverBussiness_SSMR_devTempState_get(void);
enum_solarSysManager_ctrlStage devDriverBussiness_SSMR_devVoltageCurStage_get(void);

void devDriverBussiness_solarSysManager_actionTrig(void);

void devDriverBussiness_solarSysManager_moudleInit(void);
void devDriverBussiness_solarSysManager_moudleDeinit(void);
void devDriverBussiness_solarSysManager_StatusReales(stt_devDataPonitTypedef *param);

bool devDriverBussiness_solarSysManager_paramCfgSyncValid(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_SOLARSYSMANAGER_H*/




