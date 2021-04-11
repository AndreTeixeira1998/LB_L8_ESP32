/**
 * @file devDriver_smmokeDetector.h
 *
 */

#ifndef DEVDRIVER_SMOKEDETECTOR_H
#define DEVDRIVER_SMOKEDETECTOR_H

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
#define DEVDRIVER_SMOKEDETECTOR_GPIO_SMOKEBEEP_EFFEC_LEVEL	ESP_EXT1_WAKEUP_ANY_HIGH

#define DEVDRIVER_SMOKEDETECTOR_GPIO_OUTPUT_BEEP			(15)
#define DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_BEEP				(33)
#define DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_LIGHT			(32)

#define DEVDRIVER_SMOKEDETECTOR_GPIO_INPUT_USRKEY			(4)

#define DEVDRIVER_SMOKEDETECTOR_SLEEP_TIME_DEF				43200ULL

#define DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_USRKEY			180
#define DEVDRIVER_SMOKEDETECTOR_AWAKE_TIMETG_ALARM			30

#define DEVDRIVER_SMOKEDETECTOR_DEEP_SLEEP_CD_CFM			30


/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	uint8_t dType;
	uint8_t dPoint;
}stt_devSmokeDetectAssistTkMsg;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_smokeDetect_moudleDeinit(void);
void devDriverBussiness_smokeDetect_moudleInit(void);
void devDriverBussiness_smokeDetect_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_smokeDetect_assistantLooper1sDetector(void);
void devDriverBussiness_smokeDetect_alarmParamGet(stt_devDetectingDataDef *param);

void devDriverBussinessSmokeDetect_wakeUp_handle(void);

void devDriverBussinessSmokeDetect_gpioBeepIOcfg(bool inputIf);

void devDriverBussinessSmokeDetect_flgSycnUpdated_set(bool val);
void devDriverBussinessSmokeDetect_flgHbeatUpdated_set(bool val);
void devDriverBussinessSmokeDetect_detectInterruptEnable_set(bool val);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_SMOKEDETECTOR_H*/


























