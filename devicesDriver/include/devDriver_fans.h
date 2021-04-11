/**
 * @file devDriver_devFans.h
 *
 */

#ifndef DEVDRIVER_DEVFANS_H
#define DEVDRIVER_DEVFANS_H

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
#define DEVICE_FANS_OPREAT_ACTION_NUM	4	//风扇总共有几个档

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{
	
	fansOpreatAct_stop = 0,
	fansOpreatAct_firstGear,
	fansOpreatAct_secondGear,
	fansOpreatAct_thirdGear,
}stt_devFans_opratAct;

 typedef struct{

	stt_devFans_opratAct opreatAct;
 }stt_Fans_actAttr;

 typedef struct{

	 uint8_t subdivDetailGear_1;
	 uint8_t subdivDetailGear_2;
	 uint8_t subdivDetailGear_3;
 }stt_devFan_paramCfg;
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_fansSwitch_moudleInit(void);
void devDriverBussiness_fansSwitch_moudleDeinit(void);
void devDriverBussiness_fansSwitch_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_fanSwitch_cfgParamGet(stt_devFan_paramCfg *param);
void devDriverBussiness_fanSwitch_cfgParamSet(stt_devFan_paramCfg *param, bool nvsRecord_IF);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVFANS_H*/




