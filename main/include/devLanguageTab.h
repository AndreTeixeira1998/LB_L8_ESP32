/**
 * @file devLanguageTab.h
 *
 */

#ifndef DEVLANGUAGETAB_H
#define DEVLANGUAGETAB_H

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"
#include "lvgl.h"

/**********************
 *      DEFINES 
 **********************/	
#define DEV_LANGUAGE_COUNTRY_NUM_MAX		3

#define DEV_LANGUAGE_DISP_COUNTRY_DEF_A		0x90
#define DEV_LANGUAGE_DISP_COUNTRY_DEF_B		0x91
#define DEV_LANGUAGE_DISP_COUNTRY_DEF_C		0x92

#define DEV_LANGUAGE_DISP_COUNTRY_DEF		DEV_LANGUAGE_DISP_COUNTRY_DEF_A

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	languageCyId_null = -1,
	languageCyId_English,
	languageCyId_Arabic,
	languageCyId_Portuguese,
}enumDevLanguageIdentify;

typedef enum{

	tls_null = 0,

	tls_sysInit,
	tls_sysRestart,
	tls_wifiScanning,
	tls_wifiConnecting,
	tls_wifiConnected,
	tls_HAserverChanged,
	tls_serverChanged,
	tls_illmiCfgChanged,
	tls_paramChanging,	
	tls_wifiReconnected,
	tls_devListGenerate,
	
	tls_Menu,
	tls_Timer,
	tls_Delayer,
	tls_LinkageCfg,
	tls_WifiCfg,
	tls_Setting,
	tls_SysInfo,

	tls_confirm,
	tls_save,
	tls_cancel,

	tls_hour,
	tls_minute,
	tls_min,
	tls_second,
	tls_sec,

	tls_mTimer_timerSetting,
	tls_mTimer_noRepeat,
	tls_mTimer_switchSetting,
	tls_mTimer_repeatSetting,
	tls_mTimer_timeSetting,
	tls_mTimer_repeatCycle,
	tls_mTimer_Monday,
	tls_mTimer_Tuesday,
	tls_mTimer_Wednesday,
	tls_mTimer_Thursday,
	tls_mTimer_Friday,
	tls_mTimer_Saturday,
	tls_mTimer_Sunday,

	tls_mDelayer_delaySetting,
	tls_mDelayer_Delay,
	tls_mDelayer_GreenMode,
	tls_mDelayer_timeSet,
	tls_mDelayer_trigSet,
	tls_mDelayer_enable,
	tls_mDelayer_statusNormal,
	tls_mDelayer_actionObj,

	tls_mLinkage_linkageCfg,
	tls_mLinkage_unsupported,
	tls_mLinkage_voltageOption,
	tls_mLinkage_tempOption,
	tls_mLinkage_alarmTemp,
	tls_mLinkage_devList,
	tls_mLinkage_volOpen,
	tls_mLinkage_volClose,
	tls_mLinkage_volAlarmEn,

	tls_mSetting_deviceDefine,
	tls_mSetting_deviceType,
	tls_mSetting_deviceType_1,
	tls_mSetting_systemLanguage,
	tls_mSetting_systemLanguage_1,
	tls_mSetting_switchType,
	tls_mSetting_stayTheStatus,
	tls_mSetting_bGroundPicSet,
	tls_mSetting_decoractiveSet,
	tls_mSetting_illminationSet,
	tls_mSetting_automatic,
	tls_mSetting_preview,
	tls_mSetting_setAllSwitches,
	tls_mSetting_breathingEffect,
	tls_mSetting_theme,
	tls_mSetting_theme1,
	tls_mSetting_theme2,
	tls_mSetting_theme3,
	tls_mSetting_factoryReset,
	tls_mSetting_devieManage,
	tls_mSetting_homassistantSupport,
	tls_mSetting_screenBrightness,
	tls_mSetting_temperatureCalibretion,
	tls_mSetting_screenLightTime,
	tls_mSetting_elecParamDisp,
	tls_mSetting_elecDispPower,
	tls_mSetting_elecDispSum,
	tls_mSetting_elecSumReset,
	tls_mSetting_screenDispAngle,
	tls_mSetting_vertical,
	tls_mSetting_horizontal,
	tls_mSetting_upgrade,
	tls_mSetting_epidemicReport,
	tls_mSetting_epidemicZone,

	tls_mWifiCfg_wifiCfg,
	tls_mWifiCfg_wifiStatus,
	tls_mWifiCfg_noCfg,
	tls_mWifiCfg_wifiSSID,
	tls_mWifiCfg_wifiPassword,
	tls_mWifiCfg_methodSelect,
	tls_mWifiCfg_QrCode,
	tls_mWifiCfg_reset,

	tls_mSysInfo_sysInfo,
	tls_mSysInfog_sysTime,
	tls_mSysInfo_sVersion,
	tls_mSysInfo_ePower,
	tls_mSysInfo_eSum,
	tls_mSysInfo_freeHeap,
	tls_mSysInfo_freeInternalHeapMini,
	tls_mSysInfo_topicBssid,
	tls_mSysInfo_DeviceMac,
	tls_mSysInfo_parentBssid,
	tls_mSysInfo_parentRSSI,
	tls_mSysInfo_meshNodeNum,
	tls_mSysInfo_meshLayer,
	tls_mSysInfo_deviceMutualGroup,
	tls_mSysInfo_deviceTemperature,
	tls_mSysInfo_machineTime,
	tls_mSysInfo_routerSSID,
	tls_mSysInfo_routerPassword,
	tls_mSysInfo_timeZone,
	tls_mSysInfo_versionNum,
	tls_mSysInfo_hardwareType,
	tls_mSysInfo_networkInfo,
	tls_mSysInfo_sunScale,
	tls_mSysInfo_sunScaleLocationInfo,
}enumLabelStr;

typedef enum{

	dialogTips_null = -1,
	dialogTips_restart,
	dialogTips_nwkReset,
	dialogTips_restart2Effect,
	dialogTips_upgrade,
	dialogTips_elecClr,
	dialogTips_opetSuccess,
}enumDialogWordsStr;

typedef enum{

	dialogSelTab_null = -1,
	dialogSelTab_ok,
	dialogSelTab_yesNo,
}enumDialogSelTab;

typedef struct{

	enumDevLanguageIdentify sysLanguage;
}stt_sysLanguageNvsAttr;

const char *labelStrGet_bySysLanguage(enumLabelStr strTag);
char *dialogWordsStrGet_bySysLanguage(enumDialogWordsStr strTag);
const char **dialogSelTabGet_bySysLanguage(enumDialogSelTab strTag);
const lv_font_t *fontLabelGet_bySysLanguage(uint8_t f_style);

enumDevLanguageIdentify devLanguageCountry_identifyGet(void);
void devLanguageCountry_identifySet(enumDevLanguageIdentify id,  bool nvsRecord_IF);

#endif /*DEVDATAMANAGE_H*/


























