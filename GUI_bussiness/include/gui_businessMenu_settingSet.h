/**
 * @file gui_businessMenu_settingSet.h
 *
 */

#ifndef GUI_BUSINESSMENU_SETTINGSET_H
#define GUI_BUSINESSMENU_SETTINGSET_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "iot_lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define LV_FNUM_SLIDER_ALCS_R				0
#define LV_FNUM_SLIDER_ALCS_G				1
#define LV_FNUM_SLIDER_ALCS_B		   		 2

#define LV_FNUM_CB_ALCS_AUTOMATIC	0
#define LV_FNUM_CB_ALCS_BRE_IF				1
#define LV_FNUM_CB_ALCS_SETALL_IF		2

#define ITEM_DEVICE_LIST_MANAGE_EN      0
#define MENU_SETTING_REFINE_EN              1

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	pageSettingRefreshTp_null = -1,
	pageSettingRefreshTp_languageChg,
}enumPageSetting_refreshType;

typedef struct{

	uint8_t data;
}stt_dpAttrPageSetting_refreshData;


typedef struct{

	enumPageSetting_refreshType rfsType;
	stt_dpAttrPageSetting_refreshData dp;
}stt_dpAttrMsgQ_uiPageSettingRefresh;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void appUiElementSet_upgradeAvailable(bool val);
bool appUiElementGet_upgradeAvailable(void);

void lvGui_businessMenu_settingSet(lv_obj_t * obj_Parent);
void lvGui_businessMenu_pageReload(void);
void guiDispTimeOut_pageSettingSet(void);

void paramSettingGet_epidCyName(char *nameStr);

void mwifiApp_atmosLightColorCfg_boardcastNotice(void);

void guiBussinessMenuSettingSubInterfaceCreat(uint8_t objNum);
void guiBussinessMenuSettingSubInterfaceRemove(void);
void lvGui_devTypeExSetElementRefresh_export(void);

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_SETTINGSET_H*/



