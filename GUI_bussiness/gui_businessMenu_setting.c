/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "mdf_common.h"

#include "sdkconfig.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu_setting.h"
#include "gui_businessMenu_settingSet.h"
#include "gui_businessHome.h"

#define LABEL_SETTING_NUM 			6

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);
LV_FONT_DECLARE(lv_font_ariblk_18);

LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);
LV_IMG_DECLARE(bGroundPrev_picFigure_sel);

static const char *TAG = "lanbon_L8 - menuSetting";

static const usrGuiBussiness_type settingChildOption[LABEL_SETTING_NUM] = {

	//	bussinessType_menuPageSetting_A,
	//	bussinessType_menuPageSetting_B,
	//	bussinessType_menuPageSetting_C,
};
static const char *setting_label[LABEL_SETTING_NUM] = {

	"System Config >",
	"Electricity & Power >",
	"Theme & Style >",
	"Display effect >",
	"System Reboot >",
	"System Upgrade >",
};

static lv_style_t *styleText_menuLevel_A = NULL;
static lv_style_t *styleBtn_listBtnPre = NULL;
static lv_style_t *styleBtn_listBtnRel = NULL;
static lv_style_t *styleList_menuSetting = NULL;
static lv_style_t *styleImg_menuFun_btnFun = NULL;

static lv_obj_t *objParentTemp = NULL;
static lv_obj_t *page_funSetOption = NULL;
static lv_obj_t *text_Title = NULL;

static lv_obj_t *objPageSetting_menuList = NULL;
//static lv_obj_t * menuText_devMac;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static void currentGui_elementClear(void)
{
	if (NULL != objPageSetting_menuList){
		lv_obj_del(objPageSetting_menuList);
		objPageSetting_menuList = NULL;
	}
}

void guiDispTimeOut_pageSetting(void)
{
	guiBussinessMenuSettingSubInterfaceRemove();
	lvGui_usrSwitch_withPrefunc(bussinessType_Home, currentGui_elementClear);
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn)
{

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	usrGuiBussiness_type guiChg_temp = bussinessType_Menu;

	switch (btnFreeNum){

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME:

			guiChg_temp = bussinessType_Home;

			break;

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK:
		default:

			guiChg_temp = bussinessType_Menu;

			break;
	}

	//	lvGui_usrSwitch(guiChg_temp);

	//	currentGui_elementClear();

	lvGui_usrSwitch_withPrefunc(guiChg_temp, currentGui_elementClear);
	guiBussinessMenuSettingSubInterfaceRemove();

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack_b(lv_obj_t *btn)
{
	lvGuiBusinessMenuSetting_menuA_creat();
	lvGuiBusinessMenuSetting_lvcb_menuChgPreHandle(0);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn)
{

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static lv_res_t funCb_listBtnSettingRelease(lv_obj_t *list_btn)
{
	uint8_t loop = 0;

	for (loop = 0; loop < LABEL_SETTING_NUM; loop++)
	{
		if (!strcmp(setting_label[loop], lv_list_get_btn_text(list_btn)))
		{
			MDF_LOGI("menuSetting touch get:%d.\n", loop);

			if (settingChildOption[loop])
			{
				//	currentGui_elementClear();
				//	lvGui_usrSwitch(settingChildOption[loop]);
				// lvGui_usrSwitch_withPrefunc(settingChildOption[loop], currentGui_elementClear);
			}

			switch(loop){
				case 0: lv_label_set_text(text_Title, "    System");break;
				case 1: lv_label_set_text(text_Title, "    Electric");break;
				case 2: lv_label_set_text(text_Title, "    Style");break;
				case 3: lv_label_set_text(text_Title, "    Display");break;
				case 4: lv_label_set_text(text_Title, "    Reboot");break;
				case 5: lv_label_set_text(text_Title, "    Upgrade");break;
				default:break;}
			guiBussinessMenuSettingSubInterfaceCreat(loop);
			lvGuiBusinessMenuSetting_lvcb_menuChgPreHandle(1);
			break;
		}
	}

	if (loop >= LABEL_SETTING_NUM)
		MDF_LOGI("menuSetting touch not identify:%s.\n", lv_list_get_btn_text(list_btn));

	currentGui_elementClear();

	return LV_RES_OK;
}

static void lvGuiMenuSetting_styleMemoryInitialization(void)
{
	static bool memAlloced_flg = false;

	if (true == memAlloced_flg)
		return;
	else
		memAlloced_flg = true;

	styleText_menuLevel_A = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleBtn_listBtnPre = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleBtn_listBtnRel = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleList_menuSetting = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleImg_menuFun_btnFun = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
}

static void lvGuiSetting_objStyle_Init(void)
{
	lvGuiMenuSetting_styleMemoryInitialization();

	lv_style_copy(styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A->text.font = fontLabelGet_bySysLanguage(5);
	styleText_menuLevel_A->text.color = LV_COLOR_WHITE;

	lv_style_copy(styleList_menuSetting, &lv_style_plain);
	styleList_menuSetting->body.main_color = LV_COLOR_BLACK;
	styleList_menuSetting->body.grad_color = LV_COLOR_BLACK;
	styleList_menuSetting->body.border.part = LV_BORDER_NONE;
	styleList_menuSetting->body.radius = 0;
	styleList_menuSetting->body.opa = LV_OPA_60;
	styleList_menuSetting->body.padding.hor = 3;
	styleList_menuSetting->body.padding.inner = 8;

	lv_style_copy(styleBtn_listBtnRel, &lv_style_btn_rel);
	styleBtn_listBtnRel->body.main_color = LV_COLOR_GRAY;
	styleBtn_listBtnRel->body.grad_color = LV_COLOR_GRAY;
	styleBtn_listBtnRel->body.border.color = LV_COLOR_SILVER;
	styleBtn_listBtnRel->body.border.width = 1;
	styleBtn_listBtnRel->body.border.opa = LV_OPA_50;
	styleBtn_listBtnRel->body.radius = 0;
	styleBtn_listBtnRel->body.border.part = LV_BORDER_BOTTOM;

	lv_style_copy(styleBtn_listBtnPre, styleBtn_listBtnRel);
	styleBtn_listBtnPre->body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
	styleBtn_listBtnPre->body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
	styleBtn_listBtnPre->text.color = LV_COLOR_MAKE(0xbb, 0xd5, 0xf1);

	lv_style_copy(styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun->image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun->image.color = LV_COLOR_MAKE(200, 191, 231);
}

void lvGuiBusinessMenuSetting_menuA_creat(void)
{
	objPageSetting_menuList = lv_list_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get()) ? (lv_obj_set_size(objPageSetting_menuList, 320, 165)) : (lv_obj_set_size(objPageSetting_menuList, 240, 245));
	lv_obj_set_pos(objPageSetting_menuList, 0, 75);
	lv_list_set_sb_mode(objPageSetting_menuList, LV_SB_MODE_DRAG);

	lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BG, styleList_menuSetting);
	lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
	lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BTN_REL, styleBtn_listBtnRel);
	lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BTN_PR, styleBtn_listBtnPre);

	for (uint8_t loop = 0; loop < LABEL_SETTING_NUM; loop++){
		switch(loop){
			case 1:
#if(0 == LVAPP_DISP_ELECPARAM_HIDDEN_EN)
				lv_list_add(objPageSetting_menuList, NULL, setting_label[loop], funCb_listBtnSettingRelease);
#endif
				break;

			case 5:
				if(true == appUiElementGet_upgradeAvailable())
					lv_list_add(objPageSetting_menuList, NULL, setting_label[loop], funCb_listBtnSettingRelease);
				break;

			default:
				lv_list_add(objPageSetting_menuList, NULL, setting_label[loop], funCb_listBtnSettingRelease);
				break;
		}
	}
}

void lvGuiBusinessMenuSetting_menuA_remove(void)
{

	if (NULL != objPageSetting_menuList)
	{
		lv_obj_del(objPageSetting_menuList);
		objPageSetting_menuList = NULL;
	}
}

void lvGuiBusinessMenuSetting_lvcb_menuChgPreHandle(uint8_t menuNum)
{
	switch (menuNum)
	{
		case 0:
		{
			guiBussinessMenuSettingSubInterfaceRemove();
			lv_label_set_text(text_Title, labelStrGet_bySysLanguage(tls_mSetting_deviceDefine));
			lv_obj_set_hidden(menuBtnChoIcon_fun_home, false); //暴露‘home’按钮
			lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
		}
		break;

		case 1:
		{
			lvGuiBusinessMenuSetting_menuA_remove();
			lv_obj_set_hidden(menuBtnChoIcon_fun_home, false);
			lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack_b);
		}
		break;

		default:
			break;
	}
}

void lvGui_businessMenu_setting(lv_obj_t *obj_Parent)
{
	uint16_t coordOffset_cLanguage_x = 0;

	lvGuiSetting_objStyle_Init();

	objParentTemp = obj_Parent;

	//--------------原父对象基础新增-----------------------//
	text_Title = lv_label_create(obj_Parent, NULL);
	//	lv_label_set_text(text_Title, "Device define");
	lv_label_set_text(text_Title, labelStrGet_bySysLanguage(tls_mSetting_deviceDefine));
	lv_label_set_align(text_Title, LV_LABEL_ALIGN_CENTER);
	switch (devLanguageCountry_identifyGet())
	{
	case languageCyId_Arabic:
		coordOffset_cLanguage_x = -35;
		break;
	default:
		coordOffset_cLanguage_x = 0;
		break;
	}
	lv_obj_align(text_Title, NULL, LV_ALIGN_IN_BOTTOM_MID, coordOffset_cLanguage_x, -7);
	lv_obj_set_style(text_Title, styleText_menuLevel_A);

	menuBtnChoIcon_fun_home = lv_imgbtn_create(obj_Parent, NULL);
	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
	(devStatusDispMethod_landscapeIf_get()) ? (lv_obj_set_pos(menuBtnChoIcon_fun_home, 220, 23)) : (lv_obj_set_pos(menuBtnChoIcon_fun_home, 140, 23));
	lv_imgbtn_set_src(menuBtnChoIcon_fun_home, LV_BTN_STATE_REL, &iconMenu_funBack_homePage);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_home, LV_BTN_STATE_PR, &iconMenu_funBack_homePage);
	lv_imgbtn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STATE_PR, styleImg_menuFun_btnFun);
	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_obj_set_free_num(menuBtnChoIcon_fun_home, LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_fun_home);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 0, 22);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack_arrowLeft);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack_arrowLeft);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_obj_set_free_num(menuBtnChoIcon_fun_back, LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK);

	//--------------当前活动界面新增----------------------//
	lvGuiBusinessMenuSetting_menuA_creat();
}
