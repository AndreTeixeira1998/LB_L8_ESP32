/**
 * @file gui_businessMenu_setting.h
 *
 */

#ifndef GUI_BUSINESSMENU_SETTING_H
#define GUI_BUSINESSMENU_SETTING_H

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

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lvGui_businessMenu_setting(lv_obj_t * obj_Parent);
void guiDispTimeOut_pageSetting(void);
void lvGuiBusinessMenuSetting_menuA_creat(void);
void lvGuiBusinessMenuSetting_menuA_remove(void);
void lvGuiBusinessMenuSetting_lvcb_menuChgPreHandle(uint8_t menuNum);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_SETTING_H*/


