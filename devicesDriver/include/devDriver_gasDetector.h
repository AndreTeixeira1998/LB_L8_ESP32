/**
 * @file devDriver_gasDetector.h
 *
 */

#ifndef DEVDRIVER_GASDETECTOR_H
#define DEVDRIVER_GASDETECTOR_H

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
#define DEVDRIVER_GASDETECTOR_GPIO_GASBEEP_EFFEC_LEVEL	ESP_EXT1_WAKEUP_ANY_HIGH

#define DEVDRIVER_GASDETECTOR_ANA_ALARM_LEVEL_MAX		9
#define DEVDRIVER_GASDETECTOR_ANA_ALARM_LEVEL_ALARM		2

#define DEVDRIVER_GASDETECTOR_DIG_ALARM_LEVEL_MAX		3

#define DEVDRIVER_GASDETECTOR_GPIO_INPUT_GASBEEP		(32)
#define DEVDRIVER_GASDETECTOR_GPIO_INPUT_USRKEY			(4)
#define DEVDRIVER_GASDETECTOR_GPIO_OUTPUT_BEEP			(15)

#define DEVDRIVER_GASDETECTOR_SLEEP_TIME_DEF			43200ULL

#define DEVDRIVER_GASDETECTOR_AWAKE_TIMETG_USRKEY		180
#define DEVDRIVER_GASDETECTOR_AWAKE_TIMETG_ALARM		30

#define DEVDRIVER_GASDETECTOR_DEEP_SLEEP_CD_CFM			30

/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	uint8_t dType;
	uint8_t dPoint;
}stt_devGasDetectAssistTkMsg;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_gasDetect_moudleInit(void);
void devDriverBussiness_gasDetect_moudleDeinit(void);
void devDriverBussiness_gasDetect_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_gasDetect_assistantLooper1sDetector(void);
void devDriverBussiness_gasDetect_alarmParamGet(stt_devDetectingDataDef *param);

void devDriverBussinessGasDetect_wakeUp_handle(void);

void devDriverBussinessGasDetect_flgSycnUpdated_set(bool val);
void devDriverBussinessGasDetect_flgHbeatUpdated_set(bool val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_GASDETECTOR_H*/


























