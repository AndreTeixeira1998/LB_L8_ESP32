/**
 * @file devDriver_rgbLamp.h
 *
 */

#ifndef DEVDRIVER_RGBLAMP_H
#define DEVDRIVER_RGBLAMP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDataManage.h"

/*********************
 *      DEFINES
 *********************/
#define DEV_RGBLAMP_SCENE_MAX_NUM		8
#define DEV_RGBLAMP_RMT_COLOR_NUM		15

#define DEV_RGBLAMP_RMT_ADDR			0xef00

#define DEV_RGBLAMP_RMT_CMD_ON			0xfc03
#define DEV_RGBLAMP_RMT_CMD_OFF			0xfd02
#define DEV_RGBLAMP_RMT_CMD_BRTS_UP		0xff00
#define DEV_RGBLAMP_RMT_CMD_BRTS_DN		0xfe01
#define DEV_RGBLAMP_RMT_CMD_WHITE		0xf807
#define DEV_RGBLAMP_RMT_CMD_FLASH		0xf40b
#define DEV_RGBLAMP_RMT_CMD_STROBE		0xf00f
#define DEV_RGBLAMP_RMT_CMD_FADE		0xec13
#define DEV_RGBLAMP_RMT_CMD_SMOOTH		0xe817

#define DEV_RGBLAMP_TRICK_TYPE_WHITE	0
#define DEV_RGBLAMP_TRICK_TYPE_COLOR	1
#define DEV_RGBLAMP_TRICK_TYPE_SCENE 	2
#define DEV_RGBLAMP_TRICK_TYPE_MUSIC	3
#define DEV_RGBLAMP_TRICK_TYPE_BTCHG	0xF1
#define DEV_RGBLAMP_TRICK_TYPE_FLASH	0xF2
#define DEV_RGBLAMP_TRICK_TYPE_STROBE	0xF3
#define DEV_RGBLAMP_TRICK_TYPE_FADE		0xF4
#define DEV_RGBLAMP_TRICK_TYPE_SMOOTH	0xF5

#define DEV_RGBLAMP_TRICK_TYPE_REFRESH	0xFF

#define DEV_RGBLAMP_TRICK_JMPDISP_UNITPARAM_DEF	{\
	.pd_sceIndex = 0,\
	.pd_method = 1,\
	.pd_freqLevel = 1,\
	.pd_tabNum = 6,\
	.paramUnitScolor = {\
		{{255, 0, 0}, 0, 0},{{0, 255, 0}, 0, 0},{{0, 0, 255}, 0, 0},\
		{{255, 0, 0}, 0, 0},{{0, 255, 0}, 0, 0},{{0, 0, 255}, 0, 0},\
	},\
}

/**********************
 *      TYPEDEFS
 **********************/
 typedef enum{

	devLamp_sceneMode_method_null = -1,
	devLamp_sceneMode_method_static = 0,
	devLamp_sceneMode_method_jump,
	devLamp_sceneMode_method_fade,
 };

 typedef enum{

	rgbLampNvsObj_null = -1,
	rgbLampNvsObj_pdExcuteParam = 0,
	rgbLampNvsObj_pdStaticWhite,
	rgbLampNvsObj_pdStaticColor,
	rgbLampNvsObj_pdSceneUint,
	rgbLampNvsObj_pdMusicDisp,
 }enumRgbLamp_nvsObj;

 typedef enum{

	rgbLampTipsType_null = -1,
	rgbLampTipsType_normal = 0,
	rgbLampTipsType_a,
	rgbLampTipsType_b,
	rgbLampTipsType_c,
 }enumRgbLamp_tipsType;
 
 typedef struct{

	uint8_t c_r;
	uint8_t c_g;
	uint8_t c_b;
 }stt_devRgbLampColorUnit;

 typedef struct{

	uint8_t c_temperature;
	uint8_t c_brightness;
 }stt_devRgbLampWhiteLightParam;

 typedef struct{

   stt_devRgbLampColorUnit color;
 }stt_devRgbLampColorLightParam;

 typedef struct{

	uint8_t pd_sceIndex;
	uint8_t pd_method;
	uint8_t pd_freqLevel;
	uint8_t pd_tabNum;
	struct stt_sColorParamUnit{

		stt_devRgbLampColorUnit pdJumpParam_c;		
		uint8_t c_temperature;
		uint8_t c_brightness;
	}paramUnitScolor[6];
 }stt_devRgbLampJumpParamTab;

 typedef struct{

	uint8_t pd_method;
   	stt_devRgbLampColorUnit color;
 }stt_devRgbLampMusicDispParam;

 typedef struct{

	uint8_t pd_rsv;
	uint8_t pd_excuteMode;
   	uint8_t pd_sceneExcuteIndex;
 }stt_devRgbLampExcuteParam;

 typedef struct{

	uint8_t dType;
	uint8_t dPoint;
 }stt_devRgbLampAssistTkMsg;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void devDriverBussiness_rgbLamp_moudleInit(void);
void devDriverBussiness_rgbLamp_moudleDeinit(void);
void devDriverBussiness_rgbLamp_periphStatusReales(stt_devDataPonitTypedef *param);

void rgbLamp_staticWhiteDisp_param_get(stt_devRgbLampWhiteLightParam *p);
void rgbLamp_staticWhiteDisp_param_set(stt_devRgbLampWhiteLightParam *p, bool nvsRecord_IF);
void rgbLamp_staticColorDisp_param_get(stt_devRgbLampColorLightParam *p);
void rgbLamp_staticColorDisp_param_set(stt_devRgbLampColorLightParam *p, bool nvsRecord_IF);
void rgbLamp_sceneDisp_param_get(stt_devRgbLampJumpParamTab *p, uint8_t sceneIndex);
void rgbLamp_sceneDisp_param_set(stt_devRgbLampJumpParamTab *p, bool nvsRecord_IF);
void rgbLamp_musicColorDisp_param_get(stt_devRgbLampMusicDispParam *p);
void rgbLamp_musicColorDisp_param_set(stt_devRgbLampMusicDispParam *p, bool nvsRecord_IF);
void rgbLamp_excuteParam_param_get(stt_devRgbLampExcuteParam *p);
void rgbLamp_excuteParam_param_set(stt_devRgbLampExcuteParam *p, bool nvsRecord_IF);

void devDriverBussiness_rgbLamp_trickOperation(uint8_t trickType);
void devDriverBussiness_rgbLamp_tipsTrig(enumRgbLamp_tipsType tType, uint16_t loopKeep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_RGBLAMP_H*/


















