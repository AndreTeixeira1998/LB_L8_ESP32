/**
 * @file devDriver_pirDetector.h
 *
 */

#ifndef DEVDRIVER_PIRDETECTOR_H
#define DEVDRIVER_PIRDETECTOR_H

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
#define DEVDRIVER_PIRDETECTOR_GPIO_PIRBEEP_EFFEC_LEVEL	ESP_EXT1_WAKEUP_ANY_HIGH

#define DEVDRIVER_PIRDETECTOR_ANA_ALARM_LEVEL_MAX		9
#define DEVDRIVER_PIRDETECTOR_ANA_ALARM_LEVEL_ALARM		2

#define DEVDRIVER_PIRDETECTOR_DIG_ALARM_LEVEL_MAX		3

#define DEVDRIVER_PIRDETECTOR_GPIO_OUTPUT_BEEP			(15)

#define DEVDRIVER_PIRDETECTOR_GPIO_INPUT_LIGHT			(32)
#define DEVDRIVER_PIRDETECTOR_GPIO_INPUT_USRKEY			(4)
#define DEVDRIVER_PIRDETECTOR_GPIO_INPUT_WKUP			(33)

#define DEVDRIVER_PIRDETECTOR_SLEEP_TIME_DEF			43200ULL

#define DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_USRKEY		180
#define DEVDRIVER_PIRDETECTOR_AWAKE_TIMETG_ALARM		30

#define DEVDRIVER_PIRDETECTOR_DEEP_SLEEP_CD_CFM			30

/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	uint8_t dType;
	uint8_t dPoint;
}stt_devPirDetectAssistTkMsg;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_pirDetect_moudleInit(void);
void devDriverBussiness_pirDetect_moudleDeinit(void);
void devDriverBussiness_pirDetect_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_pirDetect_assistantLooper1sDetector(void);
void devDriverBussiness_pirDetect_alarmParamGet(stt_devDetectingDataDef *param);

void devDriverBussinessPirDetect_wakeUp_handle(void);

void devDriverBussinessPirDetect_flgSycnUpdated_set(bool val);
void devDriverBussinessPirDetect_flgHbeatUpdated_set(bool val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_PIRDETECTOR_H*/


























