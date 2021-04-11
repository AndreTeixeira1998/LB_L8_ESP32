
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "mdf_common.h"

#include "sdkconfig.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "devDriver_manage.h"

#include "dataTrans_remoteServer.h"
#include "dataTrans_localHandler.h"
#include "dataTrans_meshUpgrade.h"

#include "gui_businessMenu_settingSet.h"

#include "gui_businessHome.h"
#include "gui_businessReuse_reactionObjPage.h"
	
#define OBJ_DDLIST_DEVTYPE_FREENUM			  1
#define OBJ_DDLIST_HPTHEME_FREENUM			2
#define OBJ_DDLIST_HUIINDEX_FREENUM			3

#define FUNCTION_NUM_DEF_SCREENLIGHT_TIME	13

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);
LV_FONT_DECLARE(lv_font_ariblk_18);

LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);
LV_IMG_DECLARE(bGroundPrev_picFigure_sel);
LV_IMG_DECLARE(iconTrackTime_ref);

extern EventGroupHandle_t xEventGp_devAppSupplemet_A;

xQueueHandle msgQh_uiPageSetting_refresh = NULL;

static const char *TAG = "lanbon_L8 - uiSetting";

static lv_obj_t *objParentTemp = NULL;

static const char *deviceType_listTab = {

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE) 

	"Switch-1bit\n"
	"Switch-2bit\n"
	"Switch-3bit\n"
	"Dimmer\n"
	"Fans\n"
	"Scenario\n"
	"Curtain\n" 
	"Heater\n"
	"Thermostat\n"
	"ThermostatEx"
		
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	"Switch-1bit\n"
	"Switch-2bit\n"
	"Switch-3bit\n"
	"Curtain\n" 
	"Scenario"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)
	
	"Fans"
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
	
	"Dimmer"
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)

	"Heater\n"
	"Switch-1bit"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_LC_1GANG)

	"LC switch"
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	"infrared"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	"socket"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	"moudle 3 bit"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RELAY_BOX)

	"relay 3 bit"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_MULIT_THERMO)

 #if(DEVICE_THERMOSTAT_PEDESTAL_HEATER_RESERVE == 1)

	"Thermostat"
 #else
 
	"Thermostat\n"
	"ThermostatEx"
 #endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_THERMO_INDP_A)

	"Thermostat"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

	"Voltage sensor"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BELT)

	"Lamp belt"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BULB)

	"Lamp bulb"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_GAS_DETECTOR)

	"Gas detector"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR)

	"Smoke detector"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_PIR_DETECTOR)

	"Pir detector"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_UART_MOUDLE)

	"uart Components"

#endif
};

static const char *homepageThemeStyle_listTab = {
	"theme-A\n"
	"theme-B\n"
	"theme-C"
};

static const char *homepageUiKindIndex_listTab = {
	"Kind-A\n"
	"Kind-B"
};

static const char **mbox_btnm_textTab = NULL; /*Button description. '\221' lv_btnm like control char*/

static const struct stt_deviceTypeTab_disp{

	devTypeDef_enum devType;
	uint8_t devInst_disp;
	
}deviceTypeTab_disp[DEVICE_TYPE_LIST_NUM] = {

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE) 
	
	{devTypeDef_mulitSwOneBit,			0},
	{devTypeDef_mulitSwTwoBit,			1},
	{devTypeDef_mulitSwThreeBit,		2},
	{devTypeDef_dimmer, 				3},
	{devTypeDef_fans,					4},
	{devTypeDef_scenario,				5},
	{devTypeDef_curtain,				6},
	{devTypeDef_heater, 				7}, 
	{devTypeDef_thermostat, 			8}, 
	{devTypeDef_thermostatExtension, 	9}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	{devTypeDef_mulitSwOneBit, 	 		0},
	{devTypeDef_mulitSwTwoBit, 	 		1},
	{devTypeDef_mulitSwThreeBit,		2},
	{devTypeDef_curtain,				3}, 
	{devTypeDef_scenario,				4},
	
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)
		
	{devTypeDef_fans,					0},
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
		
	{devTypeDef_dimmer, 				0},

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_LC_1GANG)

 	{devTypeDef_largeCurOneBit, 		0},
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)
	
	{devTypeDef_heater, 				0}, 
	{devTypeDef_mulitSwOneBit, 			0}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_MULIT_THERMO)

	{devTypeDef_thermostat,		 		0}, 
	{devTypeDef_thermostatExtension,	1}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_THERMO_INDP_A)

	{devTypeDef_thermostat,		 		0}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

	{devTypeDef_voltageSensor,		0},  
 
 #endif
};

static const struct stt_deviceTypeStr_disp{

	devTypeDef_enum devType;
	char *devName;
	
}deviceTypeStr_disp[DEVICE_TYPE_LIST_NUM + 1] = {

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE) 
	
	{devTypeDef_mulitSwOneBit,			"1 gang switch"},
	{devTypeDef_mulitSwTwoBit,			"2 gang switch"},
	{devTypeDef_mulitSwThreeBit,		"3 gang switch"},
	{devTypeDef_dimmer, 				"Dimmer"},
	{devTypeDef_fans,					"Fans"},
	{devTypeDef_scenario,				"Scene"},
	{devTypeDef_curtain,				"Curtain"},
	{devTypeDef_heater, 				"Heater"}, 
	{devTypeDef_thermostat, 			"Thermostat"}, 
	{devTypeDef_thermostatExtension, 	"ThermostatEx"}, 
	{devTypeDef_voltageSensor, 			"Voltage sensor"}, 
	{devTypeDef_largeCurOneBit,			"LC switch"},
	{devTypeDef_null, 					NULL}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	{devTypeDef_mulitSwOneBit,			"1 gang switch"},
	{devTypeDef_mulitSwTwoBit,			"2 gang switch"},
	{devTypeDef_mulitSwThreeBit,		"3 gang switch"},
	{devTypeDef_curtain,				"Curtain"},
	{devTypeDef_scenario,				"Scene"},
	{devTypeDef_null, 					NULL}, 
	
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)
		
	{devTypeDef_fans,					"Fans"},
	{devTypeDef_null, 					NULL}, 
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
		
	{devTypeDef_dimmer, 				"Dimmer"},
	{devTypeDef_null, 					NULL}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_LC_1GANG)

 	{devTypeDef_largeCurOneBit,			"LC switch"},
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)
	
	{devTypeDef_heater, 				"Heater"}, 
	{devTypeDef_mulitSwOneBit, 			"Switch-1bit"}, 
	{devTypeDef_null, 					NULL}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_MULIT_THERMO)

   #if(DEVICE_THERMOSTAT_PEDESTAL_HEATER_RESERVE == 1)

	{devTypeDef_thermostat, 			"Thermostat"}, 
	{devTypeDef_null, 					NULL}, 
   #else
   
	{devTypeDef_thermostat, 			"Thermostat"}, 
	{devTypeDef_thermostatExtension, 	"ThermostatEx"}, 
	{devTypeDef_null, 					NULL}, 
   #endif

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_THERMO_INDP_A)

	{devTypeDef_thermostat, 			"Thermostat"}, 
	{devTypeDef_null, 					NULL}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

	{devTypeDef_voltageSensor, 			"Voltage sensor"}, 
	{devTypeDef_null, 					NULL}, 

 #endif
};

static const struct stt_systemLanguageStr_disp{

	enumDevLanguageIdentify devType;
	char *devName;
	
}systemLanguageStr_disp[DEV_LANGUAGE_COUNTRY_NUM_MAX + 1] = {

	{languageCyId_English, 		"English"}, 
	{languageCyId_Arabic, 		"Arabic"}, 
	{languageCyId_Portuguese, 	"Portuguese"}, 

	{languageCyId_null, 		NULL}, //占坑
};

static const struct stt_gearScreenLightTime{

	uint16_t screenLightTime2Sec;
	char strDisp_ref[60];
	
}screenLightTimeGear_refTab[FUNCTION_NUM_DEF_SCREENLIGHT_TIME] = {

	{60 * 00 + 1 * 10, "#FFFF00 00##C0C0FF min##FFFF00 10##C0C0FF sec#"},
	{60 * 00 + 1 * 20, "#FFFF00 00##C0C0FF min##FFFF00 20##C0C0FF sec#"},
	{60 * 00 + 1 * 30, "#FFFF00 00##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 01 + 1 * 00, "#FFFF00 01##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 01 + 1 * 30, "#FFFF00 01##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 02 + 1 * 00, "#FFFF00 02##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 02 + 1 * 30, "#FFFF00 02##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 03 + 1 * 00, "#FFFF00 03##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 05 + 1 * 00, "#FFFF00 05##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 10 + 1 * 00, "#FFFF00 10##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 20 + 1 * 00, "#FFFF00 20##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 30 + 1 * 00, "#FFFF00 30##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{COUNTER_DISENABLE_MASK_SPECIALVAL_U16, 	    "#FFFF00   ALWAYS#"}
};

 #define EPID_STATISTICAL_COUNTRY_SUM		226

 static const char epidCyAbbre[EPID_STATISTICAL_COUNTRY_SUM][3] = {

	 "AF","AX","AL","DZ","AS","AD","AO","AI","AG","AR","AM","AW","AU",
	 "AT","AZ","BD","BH","BS","BB","BY","BE","BZ","BJ","BM","BT","BO",
	 "BA","BW","BV","BR","BN","BG","BF","BI","KH","CM","CA","CV","CF",
	 "TD","CL","CX","CC","CO","KM","CD","CG","CK","CR","CI","CN","HR",
	 "CU","CZ","CY","DK","DJ","DM","DW","EC","EG","GQ","ER","EE","ET",
	 "FO","FJ","FI","FR","FX","GF","PF","GA","GM","GE","DE","GH","GI",
	 "GR","GD","GP","GU","GT","GG","GW","GN","GY","HT","HN","HU","IS",
	 "IN","ID","IR","IQ","IE","IM","IL","IT","JM","JP","JE","JO","KZ",
	 "KE","KI","KR","KD","KW","KG","LO","LV","LB","LS","LR","LY","LI",
	 "LT","LU","MK","MW","MY","MG","MV","ML","MT","MH","MQ","MR","MU",
	 "YT","MX","MF","MD","MC","MN","ME","MS","MA","MZ","MM","NA","NR",
	 "NP","NL","NC","NZ","NI","NE","NG","NU","NF","NO","OM","PK","PW",
	 "PS","PA","PG","PE","PH","PN","PL","PT","PR","QA","RE","RO","RW",
	 "RU","SH","KN","LC","VG","SV","WS","SM","ST","SA","SN","SC","SL",
	 "SG","RS","SK","SI","SB","SO","ZA","ES","LK","SD","SR","SZ","SE",
	 "CH","SY","TJ","TZ","TH","TT","TL","TG","TK","TO","TN","TR","TM",
	 "TV","UG","UA","AE","UK","US","UY","UZ","VN","VA","VE","VN","WF",
	 "EH","YE","YU","ZM","ZW"
 };

 static const char epidCyName[] = {

	 "Afghanistan\n""Aland Islands\n""Albania\n""Algeria\n""American Samoa\n""Andorra\n"
	 "Angola\n""Anguilla\n""Antigua and Barbuda\n""Argentina\n""Armenia\n""Aruba\n"
	 "Australia\n""Austria\n""Azerbaijan\n""Bangladesh\n""Bahrain\n""Bahamas\n""Barbados\n"
	 "Belarus\n""Belgium\n""Belize\n""Benin\n""Bermuda\n""Bhutan\n""Bolivia\n"
	 "Bosnia and Herzegovina\n""Botswana\n""Bouvet Island\n""Brazil\n""Brunei\n"
	 "Bulgaria\n""Burkina Faso\n""Burundi\n""Cambodia\n""Cameroon\n""Canada\n"
	 "Cape Verde\n""Central African Republic\n""Chad\n""Chile\n""Christmas Islands\n"
	 "Cocos (keeling) Islands\n""Colombia\n""Comoros\n""Congo (Congo-Kinshasa)\n"
	 "Congo\n""Cook Islands\n""Costa Rica\n""Cote D'Ivoire\n""China\n""Croatia\n"
	 "Cuba\n""Czech\n""Cyprus\n""Denmark\n""Djibouti\n""Dominica\n""East Timor\n"
	 "Ecuador\n""Egypt\n""Equatorial Guinea\n""Eritrea\n""Estonia\n""Ethiopia\n"
	 "Faroe Islands\n""Fiji\n""Finland\n""France\n""MetropolitanFrance\n""French Guiana\n"
	 "French Polynesia\n""Gabon\n""Gambia\n""Georgia\n""Germany\n""Ghana\n"
	 "Gibraltar\n""Greece\n""Grenada\n""Guadeloupe\n""Guam\n""Guatemala\n""Guernsey\n"
	 "Guinea-Bissau\n""Guinea\n""Guyana\n""Haiti\n""Honduras\n""Hungary\n""Iceland\n"
	 "India\n""Indonesia\n""Iran\n""Iraq\n""Ireland\n""Isle of Man\n""Israel\n""Italy\n"
	 "Jamaica\n""Japan\n""Jersey\n""Jordan\n""Kazakhstan\n""Kenya\n""Kiribati\n"
	 "Korea (South)\n""Korea (North)\n""Kuwait\n""Kyrgyzstan\n""Laos\n""Latvia\n"
	 "Lebanon\n""Lesotho\n""Liberia\n""Libya\n""Liechtenstein\n""Lithuania\n""Luxembourg\n"
	 "Macedonia\n""Malawi\n""Malaysia\n""Madagascar\n""Maldives\n""Mali\n""Malta\n"
	 "Marshall Islands\n""Martinique\n""Mauritania\n""Mauritius\n""Mayotte\n""Mexico\n"
	 "Micronesia\n""Moldova\n""Monaco\n""Mongolia\n""Montenegro\n""Montserrat\n"
	 "Morocco\n""Mozambique\n""Myanmar\n""Namibia\n""Nauru\n""Nepal\n""Netherlands\n"
	 "New Caledonia\n""New Zealand\n""Nicaragua\n""Niger\n""Nigeria\n""Niue\n"
	 "Norfolk Island\n""Norway\n""Oman\n""Pakistan\n""Palau\n""Palestine\n""Panama\n"
	 "Papua New Guinea\n""Peru\n""Philippines\n""Pitcairn Islands\n""Poland\n""Portugal\n"
	 "Puerto Rico\n""Qatar\n""Reunion\n""Romania\n""Rwanda\n""Russian Federation\n"
	 "Saint Helena\n""Saint Kitts-Nevis\n""Saint Lucia\n""Saint Vincent and the Grenadines\n"
	 "El Salvador\n""Samoa\n""San Marino\n""Sao Tome and Principe\n""Saudi Arabia\n"
	 "Senegal\n""Seychelles\n""Sierra Leone\n""Singapore\n""Serbia\n""Slovakia\n"
	 "Slovenia\n""Solomon Islands\n""Somalia\n""South Africa\n""Spain\n""Sri Lanka\n"
	 "Sudan\n""Suriname\n""Swaziland\n""Sweden\n""Switzerland\n""Syria\n""Tajikistan\n"
	 "Tanzania\n""Thailand\n""Trinidad and Tobago\n""Timor-Leste\n""Togo\n""Tokelau\n"
	 "Tonga\n""Tunisia\n""Turkey\n""Turkmenistan\n""Tuvalu\n""Uganda\n""Ukraine\n"
	 "United Arab Emirates\n""United Kingdom\n""United States\n""Uruguay\n""Uzbekistan\n"
	 "Vanuatu\n""Vatican City\n""Venezuela\n""Vietnam\n""Wallis and Futuna\n"
	 "Western Sahara\n""Yemen\n""Yugoslavia\n""Zambia\n""Zimbabwe"
 };
 
// static const struct stt_epidCyName{

//	char cyName[36];
//	char cyAbbre[2];
//	
// }epidCountryList[EPID_STATISTICAL_COUNTRY_SUM] = {

//	 {"Afghanistan","AF"},	{"Aland Islands","AX"},		{"Albania","AL"},
//	 {"Algeria","DZ"},		{"American Samoa","AS"},	{"Andorra","AD"},
//	 {"Angola","AO"},		{"Anguilla","AI"},			{"Antigua and Barbuda","AG"},
//	 {"Argentina","AR"},	{"Armenia","AM"},			{"Aruba","AW"},
//	 {"Australia","AU"},	{"Austria","AT"},			{"Azerbaijan","AZ"},	
//	 {"Bangladesh","BD"},	{"Bahrain","BH"},			{"Bahamas","BS"},		
//	 {"Barbados","BB"},		{"Belarus","BY"},			{"Belgium","BE"},
//	 {"Belize","BZ"},		{"Benin","BJ"},				{"Bermuda","BM"},
//	 {"Bhutan","BT"},		{"Bolivia","BO"},			{"Bosnia and Herzegovina","BA"},
//	 {"Botswana","BW"},		{"Bouvet Island","BV"},		{"Brazil","BR"},
//	 {"Brunei","BN"},		{"Bulgaria","BG"},			{"Burkina Faso","BF"},
//	 {"Burundi","BI"},		{"Cambodia","KH"},			{"Cameroon","CM"},
//	 {"Canada","CA"},		{"Cape Verde","CV"},		{"Central African Republic","CF"},
//	 {"Chad","TD"},			{"Chile","CL"},				{"Christmas Islands","CX"},
//	 {"Cocos (keeling) Islands","CC"}, {"Colombia","CO"},{"Comoros","KM"},
//	 {"Congo (Congo-Kinshasa)","CD"},  {"Congo","CG"},	{"Cook Islands","CK"},
//	 {"Costa Rica","CR"},	{"Cote D'Ivoire","CI"},		{"China","CN"},
//	 {"Croatia","HR"},		{"Cuba","CU"},				{"Czech","CZ"},
//	 {"Cyprus","CY"},		{"Denmark","DK"},			{"Djibouti","DJ"},
//	 {"Dominica","DM"},		{"East Timor",""},			{"Ecuador","EC"},
//	 {"Egypt","EG"},		{"Equatorial Guinea","GQ"},	{"Eritrea","ER"},
//	 {"Estonia","EE"},		{"Ethiopia","ET"},			{"Faroe Islands","FO"},
//	 {"Fiji","FJ"},			{"Finland","FI"},			{"France","FR"},
//	 {"MetropolitanFrance","FX"},{"French Guiana","GF"},{"French Polynesia","PF"},
//	 {"Gabon","GA"},		{"Gambia","GM"},			{"Georgia","GE"},
//	 {"Germany","DE"},		{"Ghana","GH"},				{"Gibraltar","GI"},
//	 {"Greece","GR"},		{"Grenada","GD"},			{"Guadeloupe","GP"},
//	 {"Guam","GU"},			{"Guatemala","GT"},			{"Guernsey","GG"},
//	 {"Guinea-Bissau","GW"},{"Guinea","GN"},			{"Guyana","GY"},
//	 {"Haiti","HT"},		{"Honduras","HN"},			{"Hungary","HU"},
//	 {"Iceland","IS"},		{"India","IN"},				{"Indonesia","ID"},
//	 {"Iran","IR"},			{"Iraq","IQ"},				{"Ireland","IE"},
//	 {"Isle of Man","IM"},	{"Israel","IL"},			{"Italy","IT"},
//	 {"Jamaica","JM"},		{"Japan","JP"},				{"Jersey","JE"},
//	 {"Jordan","JO"},		{"Kazakhstan","KZ"},		{"Kenya","KE"},		
//	 {"Kiribati","KI"},		{"Korea (South)","KR"},		{"Korea (North)","KD"},
//	 {"Kuwait","KW"},		{"Kyrgyzstan","KG"},		{"Laos","LO"},
//	 {"Latvia","LV"},		{"Lebanon","LB"},			{"Lesotho","LS"},
//	 {"Liberia","LR"},		{"Libya","LY"},				{"Liechtenstein","LI"},
//	 {"Lithuania","LT"},	{"Luxembourg","LU"},		{"Macedonia","MK"},
//	 {"Malawi","MW"},		{"Malaysia","MY"},			{"Madagascar","MG"},
//	 {"Maldives","MV"},		{"Mali","ML"},				{"Malta","MT"},
//	 {"Marshall Islands","MH"},{"Martinique","MQ"},		{"Mauritania","MR"},
//	 {"Mauritius","MU"},	{"Mayotte","YT"},			{"Mexico","MX"},
//	 {"Micronesia","MF"},	{"Moldova","MD"},			{"Monaco","MC"},
//	 {"Mongolia","MN"},		{"Montenegro","ME"},		{"Montserrat","MS"},
//	 {"Morocco","MA"},		{"Mozambique","MZ"},		{"Myanmar","MM"},
//	 {"Namibia","NA"},		{"Nauru","NR"},				{"Nepal","NP"},
//	 {"Netherlands","NL"},	{"New Caledonia","NC"},		{"New Zealand","NZ"},
//	 {"Nicaragua","NI"},	{"Niger","NE"},				{"Nigeria","NG"},
//	 {"Niue","NU"},			{"Norfolk Island","NF"},	{"Norway","NO"},
//	 {"Oman","OM"},			{"Pakistan","PK"},			{"Palau","PW"},
//	 {"Palestine","PS"},	{"Panama","PA"},			{"Papua New Guinea","PG"},
//	 {"Peru","PE"},			{"Philippines","PH"},		{"Pitcairn Islands","PN"},
//	 {"Poland","PL"},		{"Portugal","PT"},			{"Puerto Rico","PR"},
//	 {"Qatar","QA"},		{"Reunion","RE"},			{"Romania","RO"},
//	 {"Rwanda","RW"},		{"Russian Federation","RU"},{"Saint Helena","SH"},
//	 {"Saint Kitts-Nevis","KN"},{"Saint Lucia","LC"},	{"Saint Vincent and the Grenadines","VG"},
//	 {"El Salvador","SV"},	{"Samoa","WS"},				{"San Marino","SM"},
//	 {"Sao Tome and Principe","ST"},{"Saudi Arabia","SA"},{"Senegal","SN"},
//	 {"Seychelles","SC"},	{"Sierra Leone","SL"},		{"Singapore","SG"},
//	 {"Serbia","RS"},		{"Slovakia","SK"},			{"Slovenia","SI"},
//	 {"Solomon Islands","SB"},{"Somalia","SO"},			{"South Africa","ZA"},
//	 {"Spain","ES"},		{"Sri Lanka","LK"},			{"Sudan","SD"},
//	 {"Suriname","SR"},		{"Swaziland","SZ"},			{"Sweden","SE"},
//	 {"Switzerland","CH"},	{"Syria","SY"},				{"Tajikistan","TJ"},
//	 {"Tanzania","TZ"},		{"Thailand","TH"},			{"Trinidad and Tobago","TT"},
//	 {"Timor-Leste","TL"},	{"Togo","TG"},				{"Tokelau","TK"},
//	 {"Tonga","TO"},		{"Tunisia","TN"},			{"Turkey","TR"},
//	 {"Turkmenistan","TM"},	{"Tuvalu","TV"},			{"Uganda","UG"},
//	 {"Ukraine","UA"},		{"United Arab Emirates","AE"},{"United Kingdom","UK"},
//	 {"United States","US"},{"Uruguay","UY"},			{"Uzbekistan","UZ"},
//	 {"Vanuatu","VN"},		{"Vatican City","VA"},		{"Venezuela","VE"},
//	 {"Vietnam","VN"},		{"Wallis and Futuna","WF"},	{"Western Sahara","EH"},
//	 {"Yemen","YE"},		{"Yugoslavia","YU"},		{"Zambia","ZM"},
//	 {"Zimbabwe","ZW"}
// };

static const uint8_t screenBrightness_sliderAdj_div = 20;

static uint8_t devTypeDdist_istRecord = 0;

static lv_style_t *stylePage_funSetOption = NULL;
static lv_style_t *styleText_menuLevel_A = NULL;
static lv_style_t *styleText_menuLevel_B = NULL;
static lv_style_t *styleText_menuLevel_C = NULL;
static lv_style_t *styleBk_objBground = NULL;
static lv_style_t *styleDdlistSettingA_devType = NULL;
static lv_style_t *styleLabel_devTypeSelected = NULL;
static lv_style_t *bg_styleDevStatusRecordIF = NULL;
static lv_style_t *indic_styleDevStatusRecordIF = NULL;
static lv_style_t *knob_on_styleDevStatusRecordIF = NULL;
static lv_style_t *knob_off_styleDevStatusRecordIF = NULL;
static lv_style_t *styleMbox_bg = NULL;
static lv_style_t *styleMbox_btn_pr = NULL;
static lv_style_t *styleMbox_btn_rel = NULL;
static lv_style_t *styleBtn_specialTransparent = NULL;
static lv_style_t *styleImg_menuFun_btnFun = NULL;
static lv_style_t *stylePageCb_tempDispMethodSet = NULL;
static lv_style_t *stylePageCb_elecParamDispSet = NULL;
static lv_style_t *stylePageCb_screenLandscape = NULL;
static lv_style_t *styleRoller_epidemicCyLocation_bg = NULL;
static lv_style_t *styleRoller_epidemicCyLocation_sel = NULL;
static lv_style_t *styleBtn_devAtmosLightColorSet = NULL;

static lv_style_t *stylePage_devTypeSet = NULL;
static lv_style_t *styleTextPageTitle_devTypeSet = NULL;
static lv_style_t *styleTextPageRef_devTypeExSet = NULL;
static lv_style_t *stylePageCb_devTypeSet = NULL;
static lv_style_t *styleLabelPageBtn_devTypeSet = NULL;
static lv_style_t *styleRoller_devTypeExSet_bg = NULL;
static lv_style_t *styleRoller_devTypeExSet_sel = NULL;
static lv_style_t *styleLabel_devTypeExSet_rollerIns = NULL;

static lv_style_t *stylePage_devAtmosLightColorSet = NULL;
static lv_style_t *styleTextPageTitle_devAtmosLightColorSet = NULL;
static lv_style_t *styleLabelPageBtn_devAtmosLightColorSet = NULL;
static lv_style_t *styleLabelPageCb_devALSC_breathIf = NULL;
static lv_style_t *styleLabelPageCb_devALSC_setAllSw = NULL;
static lv_style_t *styleLabelPageCb_devALSC_autoSet = NULL;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static lv_obj_t *page_funSetOption = NULL;
static lv_obj_t *text_Title = NULL;
static lv_obj_t *textSettingA_deviceType = NULL;
static lv_obj_t *textSettingA_systemLanguage = NULL;
static lv_obj_t	*textSettingA_devStatusRecordIF = NULL;
static lv_obj_t	*textSettingA_devForceAsMeshChild = NULL;
static lv_obj_t	*textSettingA_devListManagerEN = NULL;
static lv_obj_t	*textSettingA_devHomeAssistantEN = NULL;
static lv_obj_t	*textSettingA_bGroundPicSelect = NULL;
static lv_obj_t	*textSettingA_devAtmosLightColorSet = NULL;
static lv_obj_t *textSettingA_homepageThemestyle = NULL;
static lv_obj_t *textSettingA_homepageUiKindIndexCho = NULL;
static lv_obj_t *textSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *textSettingA_restartAllDevices = NULL;
static lv_obj_t *textSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textSettingA_screenBrightness = NULL;
static lv_obj_t *textSettingA_tempParamDispSelect = NULL;
static lv_obj_t *textSettingA_tempCalibration = NULL;
static lv_obj_t *textSettingA_screenLightTime = NULL;
static lv_obj_t *textSettingA_elecParamDispSelect = NULL;
static lv_obj_t *textSettingA_elecsumResetClear = NULL;
static lv_obj_t *textSettingA_screenLandscape = NULL;
static lv_obj_t *btnSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *btnSettingA_rebootAllDevices = NULL;
static lv_obj_t *textBtnRef_factoryRecoveryIf = NULL;
static lv_obj_t *textBtnRef_rebootAllDevices = NULL;
static lv_obj_t *btnSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textBtnRef_touchRecalibrationIf = NULL;
static lv_obj_t *btnSettingA_elecsumReset = NULL;
static lv_obj_t *textBtnRef_elecsumReset = NULL;
static lv_obj_t	*ddlistSettingA_deviceType = NULL;
static lv_obj_t	*btnSettingA_deviceType = NULL;
static lv_obj_t	*btnLabelSettingA_deviceType = NULL;
static lv_obj_t	*btnSettingA_typeDevExSet = NULL;
static lv_obj_t	*btnLabelSettingA_typeDevExSet = NULL;
static lv_obj_t	*btnSettingA_systemLanguage = NULL;
static lv_obj_t	*btnLabelSettingA_systemLanguage = NULL;
static lv_obj_t	*pageSettingA_deviceTypeSelect = NULL;
static lv_obj_t	*pageSettingA_devTypeExSet = NULL;
static lv_obj_t	*pageSettingA_devExParamPreCho = NULL;

static lv_obj_t *objpageDevTypeSelBtn_confirm = NULL;
static lv_obj_t *objpageDevTypeSelBtn_cancel = NULL;
static lv_obj_t *objpageDevTypeExSetBtn_confirm = NULL;
static lv_obj_t *objpageDevTypeExSetBtn_cancel = NULL;
static lv_obj_t *objpageDevExParamCfgPreChoBtn_confirm = NULL;
static lv_obj_t *objpageDevExParamCfgPreChoBtn_cancel = NULL;
static lv_obj_t *objpageDevTypeSelLabel_title = NULL;
static lv_obj_t *objpageDevTypeExSetLabel_title = NULL;
static lv_obj_t *objpageDevExParamCgfPreChoLabel_title = NULL;
static lv_obj_t *objpageDevTypeExSetLabel_ref = NULL;
static lv_obj_t *objpageDevExParamCfgPreCho_ref = NULL;
static lv_obj_t *objCb_devTypeSel[DEVICE_TYPE_LIST_NUM] = {NULL};
static lv_obj_t *objpageDevTypeSelBtnLabel_confirm = NULL;
static lv_obj_t *objpageDevTypeSelBtnLabel_cancel = NULL;
static lv_obj_t *objpageDevTypeExSetBtnLabel_confirm = NULL;
static lv_obj_t *objpageDevTypeExSetBtnLabel_cancel = NULL;
static lv_obj_t *objpageDevExParamCfgPreChoBtnLabel_confirm = NULL;
static lv_obj_t *objpageDevExParamCfgPreChoBtnLabel_cancel = NULL;
static lv_obj_t	*pageSettingA_sysLanguageSelect = NULL;
static lv_obj_t *objpageSysLanguageSelBtn_confirm = NULL;
static lv_obj_t *objpageSysLanguageSelBtn_cancel = NULL;
static lv_obj_t *objpageSysLanguageSelLabel_title = NULL;
static lv_obj_t *objCb_sysLanguageSel[DEV_LANGUAGE_COUNTRY_NUM_MAX] = {NULL};
static lv_obj_t *objpageSysLanguageSelBtnLabel_confirm = NULL;
static lv_obj_t *objpageSysLanguageSelBtnLabel_cancel = NULL;
static lv_obj_t	*pageSettingA_devAtmosLightColorSet = NULL;
static lv_obj_t *objpageDevAtmosLightColorSet_title = NULL;
static lv_obj_t	*btnSettingA_devAtmosLightColorSet = NULL;
static lv_obj_t *objpageDevAtmosLightColorSetBtn_confirm = NULL;
static lv_obj_t *objpageDevAtmosLightColorSetBtn_cancel = NULL;
static lv_obj_t *objpageDevAtmosLightColorSet_confirm = NULL;
static lv_obj_t *objpageDevAtmosLightColorSet_cancel = NULL;
static lv_obj_t *objpageDevALCSetLabel_colorPreview = NULL;
static lv_obj_t *objpageDevALCSetBlock_colorPreview = NULL;
static lv_obj_t *objpageDevALCSetLabel_ref = NULL;
static lv_obj_t *objpageDevALCSetSlider_color_r = NULL;
static lv_obj_t *objpageDevALCSetSlider_color_g = NULL;
static lv_obj_t *objpageDevALCSetSlider_color_b = NULL;
static lv_obj_t *objpageDevALCSetCb_automaticSet = NULL;
static lv_obj_t *objpageDevALCSetCb_breathIf = NULL;
static lv_obj_t *objpageDevALCSetCb_setAllSwitch = NULL;
static lv_obj_t	*ddlistSettingA_homepageThemestyle = NULL;
static lv_obj_t	*ddlistSettingA_homepageUiKindIndex = NULL;
static lv_obj_t	*swSettingA_devStatusRecordIF = NULL;
static lv_obj_t	*swSettingA_devForceAsMeshChild = NULL;
static lv_obj_t	*swSettingA_devListManagerEN = NULL;
static lv_obj_t	*swSettingA_devHomeAssistantEN = NULL;
static lv_obj_t	*pageBgroundPic_select = NULL;
static lv_obj_t	*imgBgroundPicSel_reserve = NULL;
static lv_obj_t	*btnBgroundPicSel_previewObjBase[BROUND_PIC_SELECT_MAX_NUM] = {NULL};
static lv_obj_t	*btnBgroundPicSel_preview[BROUND_PIC_SELECT_MAX_NUM] = {NULL};
static lv_obj_t *mboxFactoryRecovery_comfirm = NULL;
static lv_obj_t *mboxDevicesReboot_comfirm = NULL;
static lv_obj_t *mboxTouchRecalibration_comfirm = NULL;
static lv_obj_t *mboxTouchElecsumReset_comfirm = NULL;
static lv_obj_t *mboxTouchFwareUpgrade_comfirm = NULL;
static lv_obj_t *mboxScreenLandscape_comfirm = NULL;
static lv_obj_t *mboxDevForceAsChild_comfirm = NULL;
static lv_obj_t *sliderSettingA_screenBrightness = NULL;
static lv_obj_t *sliderSettingA_screenBrightnessSleep = NULL;
static lv_obj_t *sliderSettingA_tempratureCal = NULL;
static lv_obj_t *textSliderRef_tempratureCal = NULL;
static lv_obj_t *btnValAdd_tempratureCal = NULL;
static lv_obj_t *textBtnValAdd_tempratureCal = NULL;
static lv_obj_t *btnValCut_tempratureCal = NULL;
static lv_obj_t *textBtnValCut_tempratureCal = NULL;
static lv_obj_t	*swSettingA_epidemicReportIF = NULL;
static lv_obj_t	*rollerEpidemic_cyLocation = NULL;
static lv_obj_t	*iconDevTypeExSetLabel_ref = NULL;
static lv_obj_t	*iconDevExParamCfgPreChoLabel_ref = NULL;
static lv_obj_t	*objRoller_devTypeExSet_alpha = NULL;
static lv_obj_t	*objRoller_devTypeExSet_beta = NULL;
static lv_obj_t	*objLabel_devTypeExSet_alpha = NULL;
static lv_obj_t	*objLabel_devTypeExSet_beta = NULL;
static lv_obj_t	*objCb_devExParamCfgPreCho_a = NULL;
static lv_obj_t	*objCb_devExParamCfgPreCho_b = NULL;
static lv_obj_t	*objCb_devTypeExSet_a = NULL;
static lv_obj_t	*objCb_devTypeExSet_b = NULL;
static lv_obj_t	*objCb_devTypeExSet_c = NULL;
static lv_obj_t *textBtnTimeRef_screenLightTime = NULL;
static lv_obj_t *btnTimeAdd_screenLightTime = NULL;
static lv_obj_t *textBtnTimeAdd_screenLightTime = NULL;
static lv_obj_t *btnTimeCut_screenLightTime = NULL;
static lv_obj_t *textBtnTimeCut_screenLightTime = NULL;
static lv_obj_t *cbDispSelect_tempMethod_fahrenheit  = NULL;
static lv_obj_t *cbDispSelect_tempMethod_celsius  = NULL;
static lv_obj_t *cbDispSelect_elecParam_power = NULL;
static lv_obj_t *cbDispSelect_elecParam_elecsum = NULL;
static lv_obj_t *cbDispSelect_screenVer = NULL;
static lv_obj_t *cbDispSelect_screenHor = NULL;
static lv_obj_t *textSettingA_fwareUpgrade = NULL;
static lv_obj_t *btnSettingA_fwareUpgrade = NULL;
static lv_obj_t *textBtnRef_fwareUpgrade = NULL;
static lv_obj_t *textSettingA_epidemicReport = NULL;
static lv_obj_t *textSettingA_epidemicCountry = NULL;

static bool 	upgradeAvailable_flg = false;
static bool 	settingItem_ALCS_affiObjs_availableFlg = false;

static uint8_t  functionGearScreenTime_ref = 0;
static bool 	opreatTemp_screenLandscapeIf = false;

static bool 	opreatTemp_devForceAsMeshChild = false;

static int16_t 	screenLandscapeCoordinate_objOffset = 0,
				screenLandscapeCoordinateObjOffset_temp = 0;	

static char 	textlabel_dispTemp[32] = {0};

static uint8_t  devTypeSelectOpreation_dataTemp = 0;

static uint8_t  sysLanguageSelectOpreation_dataTemp = 0;

stt_devAtmosLightRunningParam atmosLightRunParam_dataTemp = {0};
static bool		atmosLightCfgOptAvailableAllSw_if = false;

static uint8_t devCurtainExParamCfgPreChoCbCheckIndex = 0;
static struct{
	uint16_t vTmp_minute;
	uint16_t vTmp_second;
}devCurtainTrackTimeParamTmp = {0};
static uint8_t devDimmerBrightnessBaseTmp = 0;
static uint8_t devFanGearSubdivTmp = 0;
static uint8_t devFanGearSubdivCbCheckIndex = 0;
static stt_devFan_paramCfg fanGearSubdivDetailParamTmp = {0};

static void lvGui_settingSet_epidemicCtrlObj_opt(bool creatIf);
static void lvApp_objCb_devTypeSel_batchDel(void);
static void lvGui_pageAtmosLightColorSetAffiAttend(bool creatIf);
static void lvGuiMenuSettingSet_styleMemoryDeinit(void);
static void lvGui_pageDevCurtainTrackTimeCfgMethod_creat(void);
static void lvGui_pageDevCurtainTrackTimeSet_creat(void);
static void lvGui_pageDevDimmerBrightnessBaseSet_creat(void);
static void lvGui_pageDevFanGearScaleSet_creat(void);
static void lvGui_devTypeExSetElementRefresh(void);

static void currentGui_elementClear(void){

	if(atmosphereLightType_idleCfgPreview == devAtmosphere_statusTips_trigGet())
		devAtmosphere_statusTips_trigSet(atmosphereLightType_none);

	lvApp_objCb_devTypeSel_batchDel();
	if(NULL != pageSettingA_deviceTypeSelect){
		lv_obj_del(pageSettingA_deviceTypeSelect);
		pageSettingA_deviceTypeSelect = NULL;}

	if(NULL != pageSettingA_devExParamPreCho){
		lv_obj_del(pageSettingA_devExParamPreCho);
		pageSettingA_devExParamPreCho = NULL;}	

	if(NULL != pageSettingA_devTypeExSet){
		lv_obj_del(pageSettingA_devTypeExSet);
		pageSettingA_devTypeExSet = NULL;}

	if(NULL != pageSettingA_sysLanguageSelect){
		lv_obj_del(pageSettingA_sysLanguageSelect);
		pageSettingA_sysLanguageSelect = NULL;}

	if(NULL != pageSettingA_devAtmosLightColorSet){
		lv_obj_del(pageSettingA_devAtmosLightColorSet);
		pageSettingA_devAtmosLightColorSet = NULL;
		settingItem_ALCS_affiObjs_availableFlg = false;}

	if(NULL != page_funSetOption){
		lv_obj_del(page_funSetOption);
		page_funSetOption = NULL;
	}
//	lvGuiMenuSettingSet_styleMemoryDeinit();

	textSettingA_epidemicCountry = NULL;
	rollerEpidemic_cyLocation = NULL;
}

void guiDispTimeOut_pageSettingSet(void){

//	lvGui_usrSwitch(bussinessType_Home);
//	currentGui_elementClear();
	lvGui_usrSwitch_withPrefunc(bussinessType_Home, currentGui_elementClear);
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	usrGuiBussiness_type guiChg_temp = bussinessType_Menu;

	switch(btnFreeNum){

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

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	lv_obj_t *objImg_colorChg = NULL;

	switch(btnFreeNum){

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME:

			objImg_colorChg = imgMenuBtnChoIcon_fun_home;

		break;

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK	:
		default:

			objImg_colorChg = imgMenuBtnChoIcon_fun_back;

		break;
	}

	lv_img_set_style(objImg_colorChg, styleImg_menuFun_btnFun);
	lv_obj_refresh_style(objImg_colorChg);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_bGroundPicOrgSelcet(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	uint8_t loop = 0;

	printf("bGroundOrg sel:%d.\n", btnFreeNum);

	lv_obj_del(imgBgroundPicSel_reserve);

	for(loop = 0; loop < BROUND_PIC_SELECT_MAX_NUM; loop ++){

		(btnFreeNum == loop)?
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL)):
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR));
	}

	imgBgroundPicSel_reserve = lv_img_create(btnBgroundPicSel_preview[btnFreeNum], NULL);
	lv_img_set_src(imgBgroundPicSel_reserve, &bGroundPrev_picFigure_sel);

	usrAppHomepageBgroundPicOrg_Set(btnFreeNum, true, false);

	return LV_RES_OK;
}

void appUiElementSet_upgradeAvailable(bool val){

	upgradeAvailable_flg = val;
}

bool appUiElementGet_upgradeAvailable(void){

	return upgradeAvailable_flg;
}

static lv_res_t funCb_mboxBtnActionClick_factoryRecovery(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("factoryRecovery mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					mdf_info_erase("ESP-MDF");
					devSystemInfoLocalRecord_allErase();
					
					usrApplication_systemRestartTrig(5);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxFactoryRecovery_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_allDevicesReboot(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("allDevicesReboot mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){
			switch(loop){

				case 0:{
					usrApplication_systemRestartTrig(5);
				}break;

				case 1:{
					xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_DEVICES_REBOOT);
					usrApplication_systemRestartTrig(5);
				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxDevicesReboot_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_touchRecalibration(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("touchRecalibration mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					extern void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if);

					usrApp_devIptdrv_paramRecalibration_set(true);

					usrApplication_systemRestartTrig(5);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchRecalibration_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_elecsumReset(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("elecsumReset mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					devDriverBussiness_elecMeasure_elecSumResetClear();

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchElecsumReset_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_firewareUpgrade(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("firewareUpgrade mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{				

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

					//同开关类型强制批量升级
					usrApp_firewareUpgrade_trig(true, (uint8_t)currentDev_typeGet());

//					//根节点设备被授权后，有权发起所有设备一起升级 --六合一有效
//					(esp_mesh_get_layer() == MESH_ROOT)?
//						(usrApp_firewareUpgrade_trig(true)):
//						(usrApp_firewareUpgrade_trig(false));
#else

//					usrApp_firewareUpgrade_trig(false);

					usrApp_firewareUpgrade_trig(true, (uint8_t)currentDev_typeGet());
#endif

					lvGui_tipsFullScreen_generate("system upgrading...", 245);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchFwareUpgrade_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_screenLandscapeConfirm(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("screen landscape opreation mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					stt_devStatusRecord devStatusRecordFlg_temp = {0};

					devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

					if(opreatTemp_screenLandscapeIf){

						devStatusRecordFlg_temp.devScreenLandscape_IF = 1;
						lv_cb_set_checked(cbDispSelect_screenVer, false);
						lv_cb_set_checked(cbDispSelect_screenHor, true);
					}
					else
					{
						devStatusRecordFlg_temp.devScreenLandscape_IF = 0;
						lv_cb_set_checked(cbDispSelect_screenVer, true);
						lv_cb_set_checked(cbDispSelect_screenHor, false);
					}

//					usrApp_devIptdrv_paramRecalibration_set(true);
					devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
					usrApplication_systemRestartTrig(3);

				}break;

				case 1:{

					if(devStatusDispMethod_landscapeIf_get()){

						lv_cb_set_checked(cbDispSelect_screenVer, false);
						lv_cb_set_checked(cbDispSelect_screenHor, true);
					}
					else
					{
						lv_cb_set_checked(cbDispSelect_screenVer, true);
						lv_cb_set_checked(cbDispSelect_screenHor, false);
					}

				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxScreenLandscape_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_devForceAsMeshChildConfirm(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;
	stt_devSystemKeyParamRecord devSystemKeyParam_temp = {0};

	printf("screen dev ForceAsMeshChild opreation mbox btn: %s released\n", txt);

	devSystemKeyAttr_paramGet(&devSystemKeyParam_temp);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					mwifi_config_t ap_config			= {0x0};
					mwifi_init_config_t init_config 	= {0x0};

					devSystemKeyParam_temp.sysParam_meshRoleForceAsChild =\
						opreatTemp_devForceAsMeshChild;
					devSystemKeyAttr_paramSet(&devSystemKeyParam_temp, true);
					usrApplication_systemRestartTrig(3);
					
					if (mdf_info_load("init_config", &init_config, sizeof(mwifi_init_config_t)) == MDF_OK && 
						mdf_info_load("ap_config", &ap_config, sizeof(mwifi_config_t)) == MDF_OK){
					
						(devSystemKeyParam_temp.sysParam_meshRoleForceAsChild)?
							(ap_config.mesh_type = MESH_NODE):
							(ap_config.mesh_type = MESH_IDLE);
					
						ap_config.router_switch_disable = 1; //禁止切换相同SSID的路由器
						init_config.root_conflicts_enable = false; //只允许一个ROOT设备
					
						mdf_info_save("init_config", &init_config, sizeof(mwifi_init_config_t));
						mdf_info_save("ap_config", &ap_config, sizeof(mwifi_config_t));
					}

				}break;

				case 1:{

					(devSystemKeyParam_temp.sysParam_meshRoleForceAsChild)?
						(lv_sw_on(swSettingA_devForceAsMeshChild)):
						(lv_sw_off(swSettingA_devForceAsMeshChild));

				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxDevForceAsChild_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_factoryRecovery(lv_obj_t *btn){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_restart);

	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxFactoryRecovery_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxFactoryRecovery_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxFactoryRecovery_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxFactoryRecovery_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_restart));
	lv_mbox_set_text(mboxFactoryRecovery_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxFactoryRecovery_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxFactoryRecovery_comfirm, funCb_mboxBtnActionClick_factoryRecovery);
	lv_mbox_set_anim_time(mboxFactoryRecovery_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_rebootAllDevices(lv_obj_t *btn){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_reboot2Effect);
		
	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_ObjSelMesh);
	mboxDevicesReboot_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxDevicesReboot_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxDevicesReboot_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxDevicesReboot_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect));
	lv_mbox_set_text(mboxDevicesReboot_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxDevicesReboot_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxDevicesReboot_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxDevicesReboot_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxDevicesReboot_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxDevicesReboot_comfirm, funCb_mboxBtnActionClick_allDevicesReboot);
	lv_mbox_set_anim_time(mboxDevicesReboot_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_touchRecalibration(lv_obj_t *btn){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect);
		
	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxTouchRecalibration_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxTouchRecalibration_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxTouchRecalibration_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxTouchRecalibration_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect));
	lv_mbox_set_text(mboxTouchRecalibration_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxTouchRecalibration_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchRecalibration_comfirm, funCb_mboxBtnActionClick_touchRecalibration);
	lv_mbox_set_anim_time(mboxTouchRecalibration_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_elecsumResetClear(lv_obj_t *btn){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_elecClr);

	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxTouchElecsumReset_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxTouchElecsumReset_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxTouchElecsumReset_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxTouchElecsumReset_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_elecClr));
	lv_mbox_set_text(mboxTouchElecsumReset_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxTouchElecsumReset_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchElecsumReset_comfirm, funCb_mboxBtnActionClick_elecsumReset);
	lv_mbox_set_anim_time(mboxTouchElecsumReset_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_fwareUpgrade(lv_obj_t *btn){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_upgrade);

	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxTouchFwareUpgrade_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxTouchFwareUpgrade_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxTouchFwareUpgrade_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxTouchFwareUpgrade_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_upgrade));
	lv_mbox_set_text(mboxTouchFwareUpgrade_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxTouchFwareUpgrade_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchFwareUpgrade_comfirm, funCb_mboxBtnActionClick_firewareUpgrade);
	lv_mbox_set_anim_time(mboxTouchFwareUpgrade_comfirm, 200);

	return LV_RES_OK;
}

void screenLandscape_opreatConfirm(void){

	char *wordsStr = dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect);

	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxScreenLandscape_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxScreenLandscape_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxScreenLandscape_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxScreenLandscape_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect));
	lv_mbox_set_text(mboxScreenLandscape_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxScreenLandscape_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxScreenLandscape_comfirm, funCb_mboxBtnActionClick_screenLandscapeConfirm);
	lv_mbox_set_anim_time(mboxScreenLandscape_comfirm, 200);

	return LV_RES_OK;
}

void devForceAsMeshChild_opreatConfirm(void){

	char *wordsStr = (char *)LV_MEM_CUSTOM_ALLOC(sizeof(char) * 256);

	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);
	mboxDevForceAsChild_comfirm = lv_mbox_create(page_funSetOption, NULL);
	lv_obj_set_protect(mboxDevForceAsChild_comfirm, LV_PROTECT_POS);
	lv_obj_set_width(mboxDevForceAsChild_comfirm, (devStatusDispMethod_landscapeIf_get())?280:200);
	lv_obj_align(mboxDevForceAsChild_comfirm, lv_scr_act(), LV_ALIGN_CENTER, (devStatusDispMethod_landscapeIf_get())?3:0,  (devStatusDispMethod_landscapeIf_get())?-20:-55);
	sprintf(wordsStr, "%s", dialogWordsStrGet_bySysLanguage(dialogTips_restart2Effect));
	lv_mbox_set_text(mboxDevForceAsChild_comfirm, (const char *)wordsStr);
	os_free(wordsStr);
	lv_mbox_add_btns(mboxDevForceAsChild_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxDevForceAsChild_comfirm, LV_MBOX_STYLE_BG, styleMbox_bg);
	lv_mbox_set_style(mboxDevForceAsChild_comfirm, LV_MBOX_STYLE_BTN_REL, styleMbox_btn_rel);
	lv_mbox_set_style(mboxDevForceAsChild_comfirm, LV_MBOX_STYLE_BTN_PR, styleMbox_btn_pr);
	lv_mbox_set_action(mboxDevForceAsChild_comfirm, funCb_mboxBtnActionClick_devForceAsMeshChildConfirm);
	lv_mbox_set_anim_time(mboxDevForceAsChild_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_tempParamDispSelect_celsius(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, false);
		devStatusRecordFlg_temp.devTempDispMethod = 0;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_tempMethod_celsius, true);
	}

	memset(textlabel_dispTemp, 0, sizeof(textlabel_dispTemp));
	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())));
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_tempParamDispSelect_fahrenheit(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_tempMethod_celsius, false);
		devStatusRecordFlg_temp.devTempDispMethod = 1;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, true);
	}

	memset(textlabel_dispTemp, 0, sizeof(textlabel_dispTemp));
	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())));
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_elecParamDispSelect_power(lv_obj_t *cb){

	extern void lvGui_usrAppElecParam_dispRefresh(void);

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_elecParam_elecsum, false);
		devStatusRecordFlg_temp.devElecsumDisp_IF = 0;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

		lvGui_usrAppElecParam_dispRefresh();
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_elecParam_power, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_elecParamDispSelect_elecsum(lv_obj_t *cb){

	extern void lvGui_usrAppElecParam_dispRefresh(void);

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_elecParam_power, false);
		devStatusRecordFlg_temp.devElecsumDisp_IF = 1;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

		lvGui_usrAppElecParam_dispRefresh();
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_elecParam_elecsum, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_scrrenDispMethod_ver(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		if(devStatusRecordFlg_temp.devScreenLandscape_IF){

			opreatTemp_screenLandscapeIf = false;
			
			lv_cb_set_checked(cbDispSelect_screenHor, false);
		
			screenLandscape_opreatConfirm();
		}
	}	
	else
	{
		lv_cb_set_checked(cb, true);
		lv_cb_set_checked(cbDispSelect_screenHor, false);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_scrrenDispMethod_hor(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		if(!devStatusRecordFlg_temp.devScreenLandscape_IF){

			opreatTemp_screenLandscapeIf = true;

			lv_cb_set_checked(cbDispSelect_screenVer, false);

			screenLandscape_opreatConfirm();
		}
	}	
	else
	{
		lv_cb_set_checked(cb, true);
		lv_cb_set_checked(cbDispSelect_screenVer, false);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_functionSet_screenBrightnessAdj(lv_obj_t *slider){

	LV_OBJ_FREE_NUM_TYPE sliderFreeNum = lv_obj_get_free_num(slider);

	uint8_t brightnessTemp = lv_slider_get_value(slider) * (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div);
	uint8_t brightnessA = devScreenDriver_configParam_brightness_get();
	uint8_t brightnessB = devScreenDriver_configParam_brightnessSleep_get();

	switch(sliderFreeNum){

		case 0:{

			if(brightnessTemp == 0){
			
				lv_slider_set_value(sliderSettingA_screenBrightness, 1);
				brightnessTemp = lv_slider_get_value(slider) * (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div);
			}

			devScreenDriver_configParam_brightness_set(brightnessTemp, true);
			if(brightnessTemp < brightnessB){

				devScreenDriver_configParam_brightnessSleep_set(brightnessTemp, true);
				lv_slider_set_value(sliderSettingA_screenBrightnessSleep, brightnessTemp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			}

		}break;

		case 1:{

			devScreenDriver_configParam_brightnessSleep_set(brightnessTemp, true);
			if(brightnessTemp > brightnessA){

				devScreenDriver_configParam_brightness_set(brightnessTemp, true);
				lv_slider_set_value(sliderSettingA_screenBrightness, brightnessTemp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			}

		}break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_functionSet_tempratureCal(lv_obj_t *slider){

	uint8_t tempratureCal = lv_slider_get_value(slider);

	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(tempratureCal)) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(tempratureCal)));
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);
	
	devTempratureSensor_dataCal_set(tempratureCal, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_tempratureCalAdd(lv_obj_t *btn){

	uint8_t dataCal = devTempratureSensor_dataCal_get();

	if(dataCal < DEV_TEMPRATURE_DATA_CAL_MAX_RANGE)dataCal += 1;
	devTempratureSensor_dataCal_set(dataCal, true);

	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(dataCal)) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(dataCal)));
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);
	lv_slider_set_value(sliderSettingA_tempratureCal, dataCal);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_tempratureCalCut(lv_obj_t *btn){

	uint8_t dataCal = devTempratureSensor_dataCal_get();

	if(dataCal > 0)dataCal -= 1;
	devTempratureSensor_dataCal_set(dataCal, true);

	
	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(dataCal)) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(dataCal)));
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);
	lv_slider_set_value(sliderSettingA_tempratureCal, dataCal);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_screenLightTimeAdd(lv_obj_t *btn){

	if(functionGearScreenTime_ref < (FUNCTION_NUM_DEF_SCREENLIGHT_TIME - 1))
		functionGearScreenTime_ref ++;

	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_obj_refresh_style(textBtnTimeRef_screenLightTime);

	devScreenDriver_configParam_screenLightTime_set(screenLightTimeGear_refTab[functionGearScreenTime_ref].screenLightTime2Sec, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_screenLightTimeCut(lv_obj_t *btn){

	if(functionGearScreenTime_ref)
		functionGearScreenTime_ref --;

	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_obj_refresh_style(textBtnTimeRef_screenLightTime);

	devScreenDriver_configParam_screenLightTime_set(screenLightTimeGear_refTab[functionGearScreenTime_ref].screenLightTime2Sec, true);

	return LV_RES_OK;
}

static void lvApp_objCb_devTypeSel_batchDel(void){

	uint8_t loop = 0;

	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(NULL != objCb_devTypeSel[loop]){

			lv_obj_del(objCb_devTypeSel[loop]);
			objCb_devTypeSel[loop] = NULL;
		}
	}
}

static void lvApp_objCb_devSysLanguage_batchDel(void){

	uint8_t loop = 0;

	for(loop = 0; loop < DEV_LANGUAGE_COUNTRY_NUM_MAX; loop ++){

		if(NULL != objCb_sysLanguageSel[loop]){

			lv_obj_del(objCb_sysLanguageSel[loop]);
			objCb_sysLanguageSel[loop] = NULL;
		}
	}
}

static lv_res_t funCb_btnActionPress_btnPage_devType_confirm(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint_temp = {0};

	lv_label_set_text(btnLabelSettingA_deviceType, deviceTypeStr_disp[devTypeSelectOpreation_dataTemp].devName);
	
	currentDev_typeSet(deviceTypeStr_disp[devTypeSelectOpreation_dataTemp].devType, true);
	devDriverManageBussiness_deviceChangeRefresh(); //驱动更新
	switch(currentDev_typeGet()){
	
		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:{
	
			devDataPoint_temp.devType_thermostat.devThermostat_tempratureTarget = DEVICE_THERMOSTAT_TEMPTARGET_DEFAULT;
			currentDev_dataPointSet(&devDataPoint_temp, true, false, false, false, false); //数据点加载默认值
	
		}break;
	
		case devTypeDef_scenario:{
	
			/*什么都不做*/
	
		}break;
	
		case devTypeDef_curtain:{
	
			stt_devStatusRecord devStatusRecordFlg_temp = {0};
			
			devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);
	
			devDataPoint_temp.devType_curtain.devCurtain_actMethod = 1;
			devDataPoint_temp.devType_curtain.devCurtain_actEnumVal = 0;
			currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false);
	
			devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
			devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

			// lvGui_pageDevCurtainTrackTimeSet_creat();
			lvGui_pageDevCurtainTrackTimeCfgMethod_creat();

		}break;

		case devTypeDef_dimmer:{

			currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false); //数据点清零
			lvGui_pageDevDimmerBrightnessBaseSet_creat();
		
		}break;

		case devTypeDef_fans:{

			currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false); //数据点清零
			lvGui_pageDevFanGearScaleSet_creat();

		}break;
	
		default:{
	
			currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false); //数据点清零

		}break;
	}

	lvGui_devTypeExSetElementRefresh();	

	devSystemInfoLocalRecord_normalClear();
	
	usrAppHomepageBtnTextDisp_defaultLoad(deviceTypeStr_disp[devTypeSelectOpreation_dataTemp].devType, true); //图文显示恢复默认
	usrAppHomepageBtnIconNumDisp_defaultLoad(deviceTypeStr_disp[devTypeSelectOpreation_dataTemp].devType, true);

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lvApp_objCb_devTypeSel_batchDel();
	lv_obj_del(pageSettingA_deviceTypeSelect);
	pageSettingA_deviceTypeSelect = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devType_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lvApp_objCb_devTypeSel_batchDel();
	lv_obj_del(pageSettingA_deviceTypeSelect);
	pageSettingA_deviceTypeSelect = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_curtainOrbitalCfgMethod_select(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE objFreeNum = lv_obj_get_free_num(cb);

	switch(objFreeNum){

		case 0:
			lv_cb_set_checked(objCb_devExParamCfgPreCho_a, true);
			lv_cb_set_checked(objCb_devExParamCfgPreCho_b, false);
			devCurtainExParamCfgPreChoCbCheckIndex = objFreeNum;
			break;

		case 1:
			lv_cb_set_checked(objCb_devExParamCfgPreCho_a, false);
			lv_cb_set_checked(objCb_devExParamCfgPreCho_b, true);
			devCurtainExParamCfgPreChoCbCheckIndex = objFreeNum;
			break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_curtainTrackTimSet_UnitSetMinute(lv_obj_t *rol){

	devCurtainTrackTimeParamTmp.vTmp_minute = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_curtainTrackTimSet_UnitSetSecond(lv_obj_t *rol){

	devCurtainTrackTimeParamTmp.vTmp_second = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_dimmerBaseBtSet_percentage(lv_obj_t *rol){

	devDimmerBrightnessBaseTmp = lv_roller_get_selected(rol) * 5;

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_fanGearSubdiv_percentage(lv_obj_t *rol){

	devFanGearSubdivTmp = (lv_roller_get_selected(rol) + 1) * 5;
	switch(devFanGearSubdivCbCheckIndex){
		case 0:
			fanGearSubdivDetailParamTmp.subdivDetailGear_1 = devFanGearSubdivTmp;
			break;
		case 1:
			fanGearSubdivDetailParamTmp.subdivDetailGear_2 = devFanGearSubdivTmp;
			break;
		case 2:
			fanGearSubdivDetailParamTmp.subdivDetailGear_3 = devFanGearSubdivTmp;
			break;
		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_fanGearSubdiv_select(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE objFreeNum = lv_obj_get_free_num(cb);

	switch(objFreeNum){
		case 0:
			lv_cb_set_checked(objCb_devTypeExSet_a, true);
			lv_cb_set_checked(objCb_devTypeExSet_b, false);
			lv_cb_set_checked(objCb_devTypeExSet_c, false);
			devFanGearSubdivTmp =\
				fanGearSubdivDetailParamTmp.subdivDetailGear_1;
			devFanGearSubdivCbCheckIndex = 0;
			break;
		case 1:
			lv_cb_set_checked(objCb_devTypeExSet_a, false);
			lv_cb_set_checked(objCb_devTypeExSet_b, true);
			lv_cb_set_checked(objCb_devTypeExSet_c, false);
			devFanGearSubdivTmp =\
				fanGearSubdivDetailParamTmp.subdivDetailGear_2;
			devFanGearSubdivCbCheckIndex = 1;
			break;
		case 2:
			lv_cb_set_checked(objCb_devTypeExSet_a, false);
			lv_cb_set_checked(objCb_devTypeExSet_b, false);
			lv_cb_set_checked(objCb_devTypeExSet_c, true);
			devFanGearSubdivTmp =\
				fanGearSubdivDetailParamTmp.subdivDetailGear_3;
			devFanGearSubdivCbCheckIndex = 2;
			break;
		default:break;
	}

	lv_roller_set_selected(objRoller_devTypeExSet_alpha, (devFanGearSubdivTmp / 5) - 1, false);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devCurtainTrackTimeCfgPreCho_confirm(lv_obj_t *btn){

	switch(devCurtainExParamCfgPreChoCbCheckIndex){
		case 0:
			lvGui_pageDevCurtainTrackTimeSet_creat();
			break;
		case 1:
			devDriverBussiness_curtainSwitch_orbitalTimeAutoDetectTrig();
			break;
		default:break;
	}

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devExParamPreCho);
	pageSettingA_devExParamPreCho = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devCurtainTrackTimeCfgPreCho_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devExParamPreCho);
	pageSettingA_devExParamPreCho = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devCurtainTrackTimeSet_confirm(lv_obj_t *btn){

	stt_devCurtain_runningParam curtainTrackTimeTmp = {0};
	uint16_t paramTrackTimeTmp = devCurtainTrackTimeParamTmp.vTmp_minute * 60 +\
																	   devCurtainTrackTimeParamTmp.vTmp_second;
	char strLabelTmp[32] = {0};

	if(10 > paramTrackTimeTmp){ //设置限制
		paramTrackTimeTmp = 10;
		devCurtainTrackTimeParamTmp.vTmp_second = 10;
	}

	curtainTrackTimeTmp.act_period = (unsigned long)paramTrackTimeTmp * 1000UL;
	devCurtain_runningParamSet(&curtainTrackTimeTmp, true);

	memset(strLabelTmp, 0, sizeof(strLabelTmp));
	sprintf(strLabelTmp, "#E8E820 T:%ds#", devCurtainTrackTimeParamTmp.vTmp_minute * 60 + devCurtainTrackTimeParamTmp.vTmp_second);
	lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);

	devDriverBussiness_curtainSwitch_orbitalRecalibration(); //全关一次，使校准

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devCurtainTrackTimeSet_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devDimmerBaseBtSet_confirm(lv_obj_t *btn){

	stt_devDimmer_paramCfg dimmercfgParamTmp = {0};
	stt_devDataPonitTypedef devDataPoint_temp = {0};
	char strLabelTmp[32] = {0};

	devDriverBussiness_dimmerSwitch_cfgParamGet(&dimmercfgParamTmp);

	dimmercfgParamTmp.brightnessBaseVar = devDimmerBrightnessBaseTmp;	
	devDriverBussiness_dimmerSwitch_cfgParamSet(&dimmercfgParamTmp, true);

	currentDev_dataPointGet(&devDataPoint_temp); //设备亮度刷新
	currentDev_dataPointSet(&devDataPoint_temp, true, false, false, false, false);

	memset(strLabelTmp, 0, sizeof(strLabelTmp));
	sprintf(strLabelTmp, "#E8E820 S:%d%%#", devDimmerBrightnessBaseTmp);
	lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devDimmerBaseBtSet_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devFanGearScaleSet_confirm(lv_obj_t *btn){

	char strLabelTmp[32] = {0};

	devDriverBussiness_fanSwitch_cfgParamSet(&fanGearSubdivDetailParamTmp, true);

	memset(strLabelTmp, 0, sizeof(strLabelTmp));
	sprintf(strLabelTmp, "#E8E820 G1:%d%%#", fanGearSubdivDetailParamTmp.subdivDetailGear_1);
	lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devFanGearScaleSet_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lv_obj_del(pageSettingA_devTypeExSet);
	pageSettingA_devTypeExSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_sysLanguage_confirm(lv_obj_t *btn){

	if(devLanguageCountry_identifyGet() != systemLanguageStr_disp[sysLanguageSelectOpreation_dataTemp].devType){

		stt_dpAttrMsgQ_uiPageSettingRefresh sdp = {0};
		
		sdp.rfsType = pageSettingRefreshTp_languageChg;
		sdp.dp.data = 0;
		xQueueSend(msgQh_uiPageSetting_refresh, &sdp, 5 / portTICK_RATE_MS);
		
		devLanguageCountry_identifySet(systemLanguageStr_disp[sysLanguageSelectOpreation_dataTemp].devType, true);
	}

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lvApp_objCb_devSysLanguage_batchDel();
	lv_obj_del(pageSettingA_sysLanguageSelect);
	pageSettingA_sysLanguageSelect = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_sysLanguage_cancel(lv_obj_t *btn){

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, true);

	lvApp_objCb_devSysLanguage_batchDel();
	lv_obj_del(pageSettingA_sysLanguageSelect);
	pageSettingA_sysLanguageSelect = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devAtmosLightColor_confirm(lv_obj_t *btn){

	settingItem_ALCS_affiObjs_availableFlg = false;

	if(atmosphereLightType_idleCfgPreview == devAtmosphere_statusTips_trigGet())
		devAtmosphere_statusTips_trigSet(atmosphereLightType_none);

	devAtmosLight_runningParam_set(&atmosLightRunParam_dataTemp, true);
	if(true == atmosLightCfgOptAvailableAllSw_if){

		atmosLightCfgOptAvailableAllSw_if = false;

		xEventGroupSetBits(xEventGp_devAppSupplemet_A, DEVAPPSUPPLEMENT_A_FLG_DEVATMOS_LIGHT_CFG);
	}

	lv_obj_del(pageSettingA_devAtmosLightColorSet);
	pageSettingA_devAtmosLightColorSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_devAtmosLightColor_cancel(lv_obj_t *btn){

	settingItem_ALCS_affiObjs_availableFlg = false;

	if(atmosphereLightType_idleCfgPreview == devAtmosphere_statusTips_trigGet())
		devAtmosphere_statusTips_trigSet(atmosphereLightType_none);

	lv_obj_del(pageSettingA_devAtmosLightColorSet);
	pageSettingA_devAtmosLightColorSet = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_devTypeSelOpreation(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(cb);
	uint8_t loop = 0;

	lv_cb_set_checked(cb, true);

	devTypeSelectOpreation_dataTemp = id;
	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){
		if(NULL != objCb_devTypeSel[loop]){
			if(loop != id){
				lv_cb_set_checked(objCb_devTypeSel[loop], false);
			}
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_sysLanguageSelOpreation(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(cb);
	uint8_t loop = 0;

	lv_cb_set_checked(cb, true);

	sysLanguageSelectOpreation_dataTemp = id;
	for(loop = 0; loop < DEV_LANGUAGE_COUNTRY_NUM_MAX; loop ++){

		if(NULL != objCb_sysLanguageSel[loop]){

			if(loop != id){

				lv_cb_set_checked(objCb_sysLanguageSel[loop], false);
			}
		}
	}

	return LV_RES_OK;
}

static void lvGui_pageDevTypeSeclect_creat(void){

	uint8_t loop = 0;
	int16_t coordOffset_cLanguage_x = 0;

	if(pageSettingA_deviceTypeSelect == NULL)
		pageSettingA_deviceTypeSelect = lv_page_create(lv_scr_act(), NULL);

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_deviceTypeSelect, 300, 210)):
		(lv_obj_set_size(pageSettingA_deviceTypeSelect, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_deviceTypeSelect, 10, 20)):
		(lv_obj_set_pos(pageSettingA_deviceTypeSelect, 20, 45));		
	lv_page_set_style(pageSettingA_deviceTypeSelect, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_deviceTypeSelect, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_deviceTypeSelect, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_deviceTypeSelect, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_deviceTypeSelect, LV_LAYOUT_PRETTY);

	objpageDevTypeSelLabel_title = lv_label_create(pageSettingA_deviceTypeSelect, NULL);
	lv_obj_set_style(objpageDevTypeSelLabel_title, styleTextPageTitle_devTypeSet);
//	lv_label_set_text(objpageDevTypeSelLabel_title, "Switch type");
	lv_label_set_text(objpageDevTypeSelLabel_title, labelStrGet_bySysLanguage(tls_mSetting_deviceType_1));
	lv_obj_set_protect(objpageDevTypeSelLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeSelLabel_title, pageSettingA_deviceTypeSelect, LV_ALIGN_IN_TOP_MID, 0, -5);

	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Portuguese:coordOffset_cLanguage_x = 25;break;
		case languageCyId_Arabic:coordOffset_cLanguage_x = -10;break;
		default:coordOffset_cLanguage_x = 0;break;}

	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(NULL != deviceTypeStr_disp[loop].devName){

			objCb_devTypeSel[loop] = lv_cb_create(pageSettingA_deviceTypeSelect, NULL);
			lv_obj_set_size(objCb_devTypeSel[loop], 160, 20);
			lv_cb_set_text(objCb_devTypeSel[loop], deviceTypeStr_disp[loop].devName);
			lv_obj_set_protect(objCb_devTypeSel[loop], LV_PROTECT_POS);
			lv_obj_set_protect(objCb_devTypeSel[loop], LV_PROTECT_FOLLOW);
			lv_cb_set_style(objCb_devTypeSel[loop], LV_CB_STYLE_BG, stylePageCb_devTypeSet);

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE)

			(devStatusDispMethod_landscapeIf_get())?
				(lv_obj_align(objCb_devTypeSel[loop], objpageDevTypeSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, (loop % 2) * 120 - 75, loop / 2 * 25 + 5)):
				(lv_obj_align(objCb_devTypeSel[loop], objpageDevTypeSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -30, loop * 20));
#else

			(devStatusDispMethod_landscapeIf_get())?
				(lv_obj_align(objCb_devTypeSel[loop], objpageDevTypeSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -25 + coordOffset_cLanguage_x, loop * 25)):
				(lv_obj_align(objCb_devTypeSel[loop], objpageDevTypeSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -20 + coordOffset_cLanguage_x, loop * 30 + 10));
#endif
			lv_obj_set_free_num(objCb_devTypeSel[loop], loop);
			lv_cb_set_action(objCb_devTypeSel[loop], funCb_cbOpreat_devTypeSelOpreation);
			if(currentDev_typeGet() == deviceTypeStr_disp[loop].devType){
				lv_cb_set_checked(objCb_devTypeSel[loop], true);
				devTypeSelectOpreation_dataTemp = loop;
			}
		}
		else{

			break;
		}
	}

	objpageDevTypeSelBtn_confirm = lv_btn_create(pageSettingA_deviceTypeSelect, NULL);
    lv_btn_set_style(objpageDevTypeSelBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeSelBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeSelBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeSelBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevTypeSelBtn_confirm, false, false);
	lv_obj_set_size(objpageDevTypeSelBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageDevTypeSelBtn_confirm, true);
	lv_obj_set_protect(objpageDevTypeSelBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevTypeSelBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeSelBtn_confirm, pageSettingA_deviceTypeSelect, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevTypeSelBtn_confirm, pageSettingA_deviceTypeSelect, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevTypeSelBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devType_confirm);
	objpageDevTypeSelBtn_cancel = lv_btn_create(pageSettingA_deviceTypeSelect, objpageDevTypeSelBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeSelBtn_cancel, pageSettingA_deviceTypeSelect, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevTypeSelBtn_cancel, objpageDevTypeSelBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevTypeSelBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devType_cancel);

	objpageDevTypeSelBtnLabel_confirm = lv_label_create(objpageDevTypeSelBtn_confirm, NULL);
	lv_obj_set_style(objpageDevTypeSelBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeSelBtnLabel_confirm, "Confirm");
	objpageDevTypeSelBtnLabel_cancel = lv_label_create(objpageDevTypeSelBtn_cancel, objpageDevTypeSelBtnLabel_confirm);
	lv_obj_set_style(objpageDevTypeSelBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeSelBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(pageSettingA_deviceTypeSelect);
	//usrApp_fullScreenRefresh_self(20, 80);
}

static void paramRefreshDevCurtainTrackTime(void){

	stt_devCurtain_runningParam curtainTrackTimeTmp = {0};

	devCurtain_runningParamGet(&curtainTrackTimeTmp);
	devCurtainTrackTimeParamTmp.vTmp_minute =\
		 curtainTrackTimeTmp.act_period / 1000UL / 60UL;
	devCurtainTrackTimeParamTmp.vTmp_second =\
		 curtainTrackTimeTmp.act_period / 1000UL % 60UL;	
}

static void paramRefreshDevDimmerBrightnessBase(void){

	stt_devDimmer_paramCfg dimmercfgParamTmp = {0};

	devDriverBussiness_dimmerSwitch_cfgParamGet(&dimmercfgParamTmp);

	devDimmerBrightnessBaseTmp = dimmercfgParamTmp.brightnessBaseVar;
}

static void paramRefreshDevFanGearScale(void){

	devDriverBussiness_fanSwitch_cfgParamGet(&fanGearSubdivDetailParamTmp);
}

static void lvGui_pageDevCurtainTrackTimeCfgMethod_creat(void){

	if(pageSettingA_devExParamPreCho == NULL)
		pageSettingA_devExParamPreCho = lv_page_create(lv_scr_act(), NULL);

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_devExParamPreCho, 300, 210)):
		(lv_obj_set_size(pageSettingA_devExParamPreCho, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_devExParamPreCho, 10, 20)):
		(lv_obj_set_pos(pageSettingA_devExParamPreCho, 20, 45));		
	lv_page_set_style(pageSettingA_devExParamPreCho, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_devExParamPreCho, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_devExParamPreCho, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_devExParamPreCho, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_devExParamPreCho, LV_LAYOUT_PRETTY);

	objpageDevExParamCgfPreChoLabel_title = lv_label_create(pageSettingA_devExParamPreCho, NULL);
	lv_obj_set_style(objpageDevExParamCgfPreChoLabel_title, styleTextPageTitle_devTypeSet);
	lv_label_set_recolor(objpageDevExParamCgfPreChoLabel_title, true);
	lv_label_set_text(objpageDevExParamCgfPreChoLabel_title, "#000000 Track settings#");
	lv_obj_set_protect(objpageDevExParamCgfPreChoLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevExParamCgfPreChoLabel_title, pageSettingA_devExParamPreCho, LV_ALIGN_IN_TOP_MID, 0, -5);

	objCb_devExParamCfgPreCho_a = lv_cb_create(pageSettingA_devExParamPreCho, NULL);
	lv_obj_set_size(objCb_devExParamCfgPreCho_a, 180, 20);
	lv_cb_set_text(objCb_devExParamCfgPreCho_a, "Manual setting");
	lv_obj_set_protect(objCb_devExParamCfgPreCho_a, LV_PROTECT_POS);
	lv_obj_set_protect(objCb_devExParamCfgPreCho_a, LV_PROTECT_FOLLOW);
	lv_obj_align(objCb_devExParamCfgPreCho_a, objpageDevExParamCgfPreChoLabel_title, LV_ALIGN_OUT_BOTTOM_RIGHT, -15, 40);
	lv_cb_set_style(objCb_devExParamCfgPreCho_a, LV_CB_STYLE_BG, stylePageCb_devTypeSet);
	lv_obj_set_free_num(objCb_devExParamCfgPreCho_a, 0);
	lv_cb_set_action(objCb_devExParamCfgPreCho_a, funCb_cbOpreat_curtainOrbitalCfgMethod_select);
	objCb_devExParamCfgPreCho_b = lv_cb_create(pageSettingA_devExParamPreCho, objCb_devExParamCfgPreCho_a);
	lv_cb_set_text(objCb_devExParamCfgPreCho_b, "Auto configuration");
	lv_obj_align(objCb_devExParamCfgPreCho_b, objCb_devExParamCfgPreCho_a, LV_ALIGN_OUT_BOTTOM_MID, 17, 0);
	lv_obj_set_free_num(objCb_devExParamCfgPreCho_b, 1);
	lv_cb_set_action(objCb_devExParamCfgPreCho_b, funCb_cbOpreat_curtainOrbitalCfgMethod_select);
	lv_cb_set_checked(objCb_devExParamCfgPreCho_a, true);
	lv_cb_set_checked(objCb_devExParamCfgPreCho_b, false);
	devCurtainExParamCfgPreChoCbCheckIndex = 0;

	objpageDevExParamCfgPreCho_ref = lv_label_create(pageSettingA_devExParamPreCho, NULL);
	lv_label_set_long_mode(objpageDevExParamCfgPreCho_ref, LV_LABEL_LONG_BREAK);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objpageDevExParamCfgPreCho_ref, 260, 80)):
		(lv_obj_set_size(objpageDevExParamCfgPreCho_ref, 180, 120));
	lv_obj_set_style(objpageDevExParamCfgPreCho_ref, styleTextPageRef_devTypeExSet);
	lv_label_set_recolor(objpageDevExParamCfgPreCho_ref, true);
	lv_label_set_static_text(objpageDevExParamCfgPreCho_ref, "    Please choose a way to set the curtain track.");
	lv_obj_set_protect(objpageDevExParamCfgPreCho_ref, LV_PROTECT_POS);
	lv_obj_align(objpageDevExParamCfgPreCho_ref, pageSettingA_devExParamPreCho, LV_ALIGN_IN_BOTTOM_MID, 5, -30);

	iconDevExParamCfgPreChoLabel_ref = lv_img_create(pageSettingA_devExParamPreCho, NULL);
	lv_img_set_src(iconDevExParamCfgPreChoLabel_ref, &iconTrackTime_ref);
	lv_obj_set_protect(iconDevExParamCfgPreChoLabel_ref, LV_PROTECT_POS);
	lv_obj_align(iconDevExParamCfgPreChoLabel_ref, objpageDevExParamCfgPreCho_ref, LV_ALIGN_IN_TOP_LEFT, 0, -6);

	objpageDevExParamCfgPreChoBtn_confirm = lv_btn_create(pageSettingA_devExParamPreCho, NULL);
    lv_btn_set_style(objpageDevExParamCfgPreChoBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevExParamCfgPreChoBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevExParamCfgPreChoBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevExParamCfgPreChoBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevExParamCfgPreChoBtn_confirm, false, false);
	lv_obj_set_size(objpageDevExParamCfgPreChoBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageDevExParamCfgPreChoBtn_confirm, true);
	lv_obj_set_protect(objpageDevExParamCfgPreChoBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevExParamCfgPreChoBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevExParamCfgPreChoBtn_confirm, pageSettingA_devExParamPreCho, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevExParamCfgPreChoBtn_confirm, pageSettingA_devExParamPreCho, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevExParamCfgPreChoBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devCurtainTrackTimeCfgPreCho_confirm);
	objpageDevExParamCfgPreChoBtn_cancel = lv_btn_create(pageSettingA_devExParamPreCho, objpageDevExParamCfgPreChoBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevExParamCfgPreChoBtn_cancel, pageSettingA_devExParamPreCho, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevExParamCfgPreChoBtn_cancel, objpageDevExParamCfgPreChoBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevExParamCfgPreChoBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devCurtainTrackTimeCfgPreCho_cancel);

	objpageDevExParamCfgPreChoBtnLabel_confirm = lv_label_create(objpageDevExParamCfgPreChoBtn_confirm, NULL);
	lv_obj_set_style(objpageDevExParamCfgPreChoBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevExParamCfgPreChoBtnLabel_confirm, "Confirm");
	objpageDevExParamCfgPreChoBtnLabel_cancel = lv_label_create(objpageDevExParamCfgPreChoBtn_cancel, objpageDevExParamCfgPreChoBtnLabel_confirm);
	lv_obj_set_style(objpageDevExParamCfgPreChoBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevExParamCfgPreChoBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);
}

static void lvGui_pageDevCurtainTrackTimeSet_creat(void){

	uint8_t loop = 0;
	int16_t coordOffset_cLanguage_x = 0;
	uint8_t visibleRow_temp = 0;

	if(pageSettingA_devTypeExSet == NULL)
		pageSettingA_devTypeExSet = lv_page_create(lv_scr_act(), NULL);

	paramRefreshDevCurtainTrackTime();

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_devTypeExSet, 300, 210)):
		(lv_obj_set_size(pageSettingA_devTypeExSet, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 10, 20)):
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 20, 45));		
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_devTypeExSet, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_devTypeExSet, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_devTypeExSet, LV_LAYOUT_PRETTY);

	objpageDevTypeExSetLabel_title = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_obj_set_style(objpageDevTypeExSetLabel_title, styleTextPageTitle_devTypeSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_title, true);
	lv_label_set_text(objpageDevTypeExSetLabel_title, "#000000 Track Time#");
	// lv_label_set_text(objpageDevTypeExSetLabel_title, labelStrGet_bySysLanguage(tls_mSetting_deviceType_1));
	lv_obj_set_protect(objpageDevTypeExSetLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_title, pageSettingA_devTypeExSet, LV_ALIGN_IN_TOP_MID, 0, -5);

	(devStatusDispMethod_landscapeIf_get())?
		(visibleRow_temp = 2):
		(visibleRow_temp = 2);

	objRoller_devTypeExSet_alpha = lv_roller_create(pageSettingA_devTypeExSet, NULL);
	lv_roller_set_action(objRoller_devTypeExSet_alpha, funCb_rollerOpreat_curtainTrackTimSet_UnitSetMinute);
	lv_roller_set_visible_row_count(objRoller_devTypeExSet_alpha, visibleRow_temp);
	lv_roller_set_options(objRoller_devTypeExSet_alpha, "00\n""01\n""02\n""03\n""04\n""05\n"
																													"06\n""07\n""08\n""09\n""10\n""11\n"
																													"12\n""13\n""14\n""15\n""16\n""17\n"
																													"18\n""19\n""20\n""21\n""22\n""23\n"
																													"24\n""25\n""26\n""27\n""28\n""29\n"
																													"30");
	lv_obj_set_protect(objRoller_devTypeExSet_alpha, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, -40, 25)):
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, -40, 35));
	lv_roller_set_hor_fit(objRoller_devTypeExSet_alpha, false);
	lv_obj_set_width(objRoller_devTypeExSet_alpha, 40);
	lv_roller_set_selected(objRoller_devTypeExSet_alpha, devCurtainTrackTimeParamTmp.vTmp_minute, false);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_BG, styleRoller_devTypeExSet_bg);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_SEL, styleRoller_devTypeExSet_sel);

	objRoller_devTypeExSet_beta = lv_roller_create(pageSettingA_devTypeExSet, NULL);
	lv_roller_set_action(objRoller_devTypeExSet_beta, funCb_rollerOpreat_curtainTrackTimSet_UnitSetSecond);
	lv_roller_set_visible_row_count(objRoller_devTypeExSet_beta, visibleRow_temp);
	lv_roller_set_options(objRoller_devTypeExSet_beta, "00\n""01\n""02\n""03\n""04\n""05\n"
																												"06\n""07\n""08\n""09\n""10\n""11\n"
																												"12\n""13\n""14\n""15\n""16\n""17\n"
																												"18\n""19\n""20\n""21\n""22\n""23\n"
																												"24\n""25\n""26\n""27\n""28\n""29\n"
																												"30\n""31\n""32\n""33\n""34\n""35\n"
																												"36\n""37\n""38\n""39\n""40\n""41\n"
																												"42\n""43\n""44\n""45\n""46\n""47\n"
																												"48\n""49\n""50\n""51\n""52\n""53\n"
																												"54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(objRoller_devTypeExSet_beta, LV_PROTECT_POS);
	lv_obj_align(objRoller_devTypeExSet_beta, objRoller_devTypeExSet_alpha, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_roller_set_hor_fit(objRoller_devTypeExSet_beta, false);
	lv_obj_set_width(objRoller_devTypeExSet_beta, 40);
	lv_roller_set_selected(objRoller_devTypeExSet_beta, devCurtainTrackTimeParamTmp.vTmp_second, false);
	lv_roller_set_style(objRoller_devTypeExSet_beta, LV_ROLLER_STYLE_BG, styleRoller_devTypeExSet_bg);
	lv_roller_set_style(objRoller_devTypeExSet_beta, LV_ROLLER_STYLE_SEL, styleRoller_devTypeExSet_sel);

	objLabel_devTypeExSet_alpha = lv_label_create(pageSettingA_devTypeExSet, NULL);
//	lv_label_set_text(objLabel_devTypeExSet_alpha, "min");
	lv_label_set_text(objLabel_devTypeExSet_alpha, labelStrGet_bySysLanguage(tls_min));
	lv_obj_set_protect(objLabel_devTypeExSet_alpha, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:coordOffset_cLanguage_x = 23;break;
		default:coordOffset_cLanguage_x = 18;break;}
	lv_obj_align(objLabel_devTypeExSet_alpha, objRoller_devTypeExSet_alpha, LV_ALIGN_IN_RIGHT_MID, coordOffset_cLanguage_x + 8, 1);
	lv_obj_set_style(objLabel_devTypeExSet_alpha, styleLabel_devTypeExSet_rollerIns);

	objLabel_devTypeExSet_beta = lv_label_create(pageSettingA_devTypeExSet, NULL);
//	lv_label_set_text(objLabel_devTypeExSet_beta, "sec");
	lv_label_set_text(objLabel_devTypeExSet_beta, labelStrGet_bySysLanguage(tls_sec));
	lv_obj_set_protect(objLabel_devTypeExSet_beta, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:coordOffset_cLanguage_x = 23;break;
		default:coordOffset_cLanguage_x = 15;break;}
	lv_obj_align(objLabel_devTypeExSet_beta, objRoller_devTypeExSet_beta, LV_ALIGN_IN_RIGHT_MID, coordOffset_cLanguage_x + 10, 1);
	lv_obj_set_style(objLabel_devTypeExSet_beta, styleLabel_devTypeExSet_rollerIns);

	objpageDevTypeExSetLabel_ref = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_label_set_long_mode(objpageDevTypeExSetLabel_ref, LV_LABEL_LONG_BREAK);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 260, 80)):
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 180, 120));
	lv_obj_set_style(objpageDevTypeExSetLabel_ref, styleTextPageRef_devTypeExSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_ref, true);
	lv_label_set_static_text(objpageDevTypeExSetLabel_ref, "    #80FF00 Track time# means the time of curtain 100% open or 100% close, if track time not set or set wrongly, it will effect the curtain open #80FF00 percentage function#.");
	lv_obj_set_protect(objpageDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_ref, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_MID, 5, -30);

	iconDevTypeExSetLabel_ref = lv_img_create(pageSettingA_devTypeExSet, NULL);
	lv_img_set_src(iconDevTypeExSetLabel_ref, &iconTrackTime_ref);
	lv_obj_set_protect(iconDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(iconDevTypeExSetLabel_ref, objpageDevTypeExSetLabel_ref, LV_ALIGN_IN_TOP_LEFT, 0, -6);

	objpageDevTypeExSetBtn_confirm = lv_btn_create(pageSettingA_devTypeExSet, NULL);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevTypeExSetBtn_confirm, false, false);
	lv_obj_set_size(objpageDevTypeExSetBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageDevTypeExSetBtn_confirm, true);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevTypeExSetBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devCurtainTrackTimeSet_confirm);
	objpageDevTypeExSetBtn_cancel = lv_btn_create(pageSettingA_devTypeExSet, objpageDevTypeExSetBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevTypeExSetBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devCurtainTrackTimeSet_cancel);

	objpageDevTypeExSetBtnLabel_confirm = lv_label_create(objpageDevTypeExSetBtn_confirm, NULL);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_confirm, "Confirm");
	objpageDevTypeExSetBtnLabel_cancel = lv_label_create(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtnLabel_confirm);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(pageSettingA_devTypeExSet);
	// usrApp_fullScreenRefresh_self(20, 80);
}

static void lvGui_pageDevDimmerBrightnessBaseSet_creat(void){

	uint8_t loop = 0;
	int16_t coordOffset_cLanguage_x = 0;
	uint8_t visibleRow_temp = 0;

	if(pageSettingA_devTypeExSet == NULL)
		pageSettingA_devTypeExSet = lv_page_create(lv_scr_act(), NULL);

	paramRefreshDevDimmerBrightnessBase();

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_devTypeExSet, 300, 210)):
		(lv_obj_set_size(pageSettingA_devTypeExSet, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 10, 20)):
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 20, 45));		
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_devTypeExSet, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_devTypeExSet, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_devTypeExSet, LV_LAYOUT_PRETTY);

	objpageDevTypeExSetLabel_title = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_obj_set_style(objpageDevTypeExSetLabel_title, styleTextPageTitle_devTypeSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_title, true);
	lv_label_set_text(objpageDevTypeExSetLabel_title, "#000000 Basic brightness#");
	// lv_label_set_text(objpageDevTypeExSetLabel_title, labelStrGet_bySysLanguage(tls_mSetting_deviceType_1));
	lv_obj_set_protect(objpageDevTypeExSetLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_title, pageSettingA_devTypeExSet, LV_ALIGN_IN_TOP_MID, 0, -5);

	(devStatusDispMethod_landscapeIf_get())?
		(visibleRow_temp = 2):
		(visibleRow_temp = 2);

	objRoller_devTypeExSet_alpha = lv_roller_create(pageSettingA_devTypeExSet, NULL);
	lv_roller_set_action(objRoller_devTypeExSet_alpha, funCb_rollerOpreat_dimmerBaseBtSet_percentage);
	lv_roller_set_visible_row_count(objRoller_devTypeExSet_alpha, visibleRow_temp);
	lv_roller_set_options(objRoller_devTypeExSet_alpha, "0\n""5\n""10\n""15\n""20\n""25\n"
																													"30\n""35\n""40\n""45\n""50\n""55\n""60");
	lv_obj_set_protect(objRoller_devTypeExSet_alpha, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, 0, 25)):
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, 0, 35));
	lv_roller_set_hor_fit(objRoller_devTypeExSet_alpha, false);
	lv_obj_set_width(objRoller_devTypeExSet_alpha, 40);
	lv_roller_set_selected(objRoller_devTypeExSet_alpha, devDimmerBrightnessBaseTmp / 5, false);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_BG, styleRoller_devTypeExSet_bg);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_SEL, styleRoller_devTypeExSet_sel);

	objLabel_devTypeExSet_alpha = lv_label_create(pageSettingA_devTypeExSet, NULL);
//	lv_label_set_text(objLabel_devTypeExSet_alpha, "min");
	lv_label_set_text(objLabel_devTypeExSet_alpha, "%");
	lv_obj_set_protect(objLabel_devTypeExSet_alpha, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:coordOffset_cLanguage_x = 23;break;
		default:coordOffset_cLanguage_x = 18;break;}
	lv_obj_align(objLabel_devTypeExSet_alpha, objRoller_devTypeExSet_alpha, LV_ALIGN_IN_RIGHT_MID, coordOffset_cLanguage_x + 8, 1);
	lv_obj_set_style(objLabel_devTypeExSet_alpha, styleLabel_devTypeExSet_rollerIns);

	objpageDevTypeExSetLabel_ref = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_label_set_long_mode(objpageDevTypeExSetLabel_ref, LV_LABEL_LONG_BREAK);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 260, 80)):
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 180, 120));
	lv_obj_set_style(objpageDevTypeExSetLabel_ref, styleTextPageRef_devTypeExSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_ref, true);
	lv_label_set_static_text(objpageDevTypeExSetLabel_ref, "    #80FF00 Track time# means the lowest brightness that the lamp can be adjusted to, below which the lamp will malfunction, such as flickering.");
	lv_obj_set_protect(objpageDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_ref, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_MID, 5, -30);

	iconDevTypeExSetLabel_ref = lv_img_create(pageSettingA_devTypeExSet, NULL);
	lv_img_set_src(iconDevTypeExSetLabel_ref, &iconTrackTime_ref);
	lv_obj_set_protect(iconDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(iconDevTypeExSetLabel_ref, objpageDevTypeExSetLabel_ref, LV_ALIGN_IN_TOP_LEFT, 0, -6);

	objpageDevTypeExSetBtn_confirm = lv_btn_create(pageSettingA_devTypeExSet, NULL);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevTypeExSetBtn_confirm, false, false);
	lv_obj_set_size(objpageDevTypeExSetBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageDevTypeExSetBtn_confirm, true);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevTypeExSetBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devDimmerBaseBtSet_confirm);
	objpageDevTypeExSetBtn_cancel = lv_btn_create(pageSettingA_devTypeExSet, objpageDevTypeExSetBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevTypeExSetBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devDimmerBaseBtSet_cancel);

	objpageDevTypeExSetBtnLabel_confirm = lv_label_create(objpageDevTypeExSetBtn_confirm, NULL);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_confirm, "Confirm");
	objpageDevTypeExSetBtnLabel_cancel = lv_label_create(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtnLabel_confirm);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);

	lv_obj_refresh_style(pageSettingA_devTypeExSet);
}

static void lvGui_pageDevFanGearScaleSet_creat(void){

	uint8_t loop = 0;
	int16_t coordOffset_cLanguage_x = 0;
	uint8_t visibleRow_temp = 2;

	if(pageSettingA_devTypeExSet == NULL)
		pageSettingA_devTypeExSet = lv_page_create(lv_scr_act(), NULL);

	paramRefreshDevFanGearScale();
	devFanGearSubdivTmp =\
		fanGearSubdivDetailParamTmp.subdivDetailGear_1;
	devFanGearSubdivCbCheckIndex = 0;

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_devTypeExSet, 300, 210)):
		(lv_obj_set_size(pageSettingA_devTypeExSet, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 10, 20)):
		(lv_obj_set_pos(pageSettingA_devTypeExSet, 20, 45));		
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_devTypeExSet, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_devTypeExSet, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_devTypeExSet, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_devTypeExSet, LV_LAYOUT_PRETTY);

	objpageDevTypeExSetLabel_title = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_obj_set_style(objpageDevTypeExSetLabel_title, styleTextPageTitle_devTypeSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_title, true);
	lv_label_set_text(objpageDevTypeExSetLabel_title, "#000000 Gear Scale#");
	// lv_label_set_text(objpageDevTypeExSetLabel_title, labelStrGet_bySysLanguage(tls_mSetting_deviceType_1));
	lv_obj_set_protect(objpageDevTypeExSetLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_title, pageSettingA_devTypeExSet, LV_ALIGN_IN_TOP_MID, 0, -5);

	objRoller_devTypeExSet_alpha = lv_roller_create(pageSettingA_devTypeExSet, NULL);
	lv_roller_set_action(objRoller_devTypeExSet_alpha, funCb_rollerOpreat_fanGearSubdiv_percentage);
	lv_roller_set_visible_row_count(objRoller_devTypeExSet_alpha, visibleRow_temp);
	lv_roller_set_options(objRoller_devTypeExSet_alpha, "5\n""10\n""15\n""20\n""25\n"
																													"30\n""35\n""40\n""45\n""50\n"
																													"55\n""60\n""65\n""70\n""75\n"
																													"80\n""85\n""90\n""95\n""100");
	lv_obj_set_protect(objRoller_devTypeExSet_alpha, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, 0, 25)):
		(lv_obj_align(objRoller_devTypeExSet_alpha, NULL, LV_ALIGN_IN_TOP_MID, 0, 35));
	lv_roller_set_hor_fit(objRoller_devTypeExSet_alpha, false);
	lv_obj_set_width(objRoller_devTypeExSet_alpha, 40);
	lv_roller_set_selected(objRoller_devTypeExSet_alpha, (devFanGearSubdivTmp / 5) - 1, false);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_BG, styleRoller_devTypeExSet_bg);
	lv_roller_set_style(objRoller_devTypeExSet_alpha, LV_ROLLER_STYLE_SEL, styleRoller_devTypeExSet_sel);

	objLabel_devTypeExSet_alpha = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_label_set_text(objLabel_devTypeExSet_alpha, "%");
	lv_obj_set_protect(objLabel_devTypeExSet_alpha, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:coordOffset_cLanguage_x = 23;break;
		default:coordOffset_cLanguage_x = 18;break;}
	lv_obj_align(objLabel_devTypeExSet_alpha, objRoller_devTypeExSet_alpha, LV_ALIGN_IN_RIGHT_MID, coordOffset_cLanguage_x + 8, 1);
	lv_obj_set_style(objLabel_devTypeExSet_alpha, styleLabel_devTypeExSet_rollerIns);

	objCb_devTypeExSet_a = lv_cb_create(pageSettingA_devTypeExSet, NULL);
	lv_obj_set_size(objCb_devTypeExSet_a, 160, 20);
	lv_cb_set_text(objCb_devTypeExSet_a, "G1");
	lv_obj_set_protect(objCb_devTypeExSet_a, LV_PROTECT_POS);
	lv_obj_set_protect(objCb_devTypeExSet_a, LV_PROTECT_FOLLOW);
	lv_obj_align(objCb_devTypeExSet_a, objRoller_devTypeExSet_alpha, LV_ALIGN_OUT_BOTTOM_RIGHT, -50, 20);
	lv_cb_set_style(objCb_devTypeExSet_a, LV_CB_STYLE_BG, stylePageCb_devTypeSet);
	lv_obj_set_free_num(objCb_devTypeExSet_a, 0);
	lv_cb_set_action(objCb_devTypeExSet_a, funCb_cbOpreat_fanGearSubdiv_select);
	objCb_devTypeExSet_b = lv_cb_create(pageSettingA_devTypeExSet, objCb_devTypeExSet_a);
	lv_cb_set_text(objCb_devTypeExSet_b, "G2");
	lv_obj_align(objCb_devTypeExSet_b, objCb_devTypeExSet_a, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_obj_set_free_num(objCb_devTypeExSet_b, 1);
	lv_cb_set_action(objCb_devTypeExSet_b, funCb_cbOpreat_fanGearSubdiv_select);
	objCb_devTypeExSet_c = lv_cb_create(pageSettingA_devTypeExSet, objCb_devTypeExSet_a);
	lv_cb_set_text(objCb_devTypeExSet_c, "G3");
	lv_obj_align(objCb_devTypeExSet_c, objCb_devTypeExSet_b, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_obj_set_free_num(objCb_devTypeExSet_c, 2);
	lv_cb_set_action(objCb_devTypeExSet_c, funCb_cbOpreat_fanGearSubdiv_select);
	lv_cb_set_checked(objCb_devTypeExSet_a, true);
	lv_cb_set_checked(objCb_devTypeExSet_b, false);
	lv_cb_set_checked(objCb_devTypeExSet_c, false);

	objpageDevTypeExSetLabel_ref = lv_label_create(pageSettingA_devTypeExSet, NULL);
	lv_label_set_long_mode(objpageDevTypeExSetLabel_ref, LV_LABEL_LONG_BREAK);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 260, 80)):
		(lv_obj_set_size(objpageDevTypeExSetLabel_ref, 180, 120));
	lv_obj_set_style(objpageDevTypeExSetLabel_ref, styleTextPageRef_devTypeExSet);
	lv_label_set_recolor(objpageDevTypeExSetLabel_ref, true);
	lv_label_set_static_text(objpageDevTypeExSetLabel_ref, "    #80FF00 Gear Scale# means the value of the air volume of each gear.");
	lv_obj_set_protect(objpageDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(objpageDevTypeExSetLabel_ref, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_MID, 5, -30);

	iconDevTypeExSetLabel_ref = lv_img_create(pageSettingA_devTypeExSet, NULL);
	lv_img_set_src(iconDevTypeExSetLabel_ref, &iconTrackTime_ref);
	lv_obj_set_protect(iconDevTypeExSetLabel_ref, LV_PROTECT_POS);
	lv_obj_align(iconDevTypeExSetLabel_ref, objpageDevTypeExSetLabel_ref, LV_ALIGN_IN_TOP_LEFT, 0, -6);

	objpageDevTypeExSetBtn_confirm = lv_btn_create(pageSettingA_devTypeExSet, NULL);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevTypeExSetBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevTypeExSetBtn_confirm, false, false);
	lv_obj_set_size(objpageDevTypeExSetBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageDevTypeExSetBtn_confirm, true);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevTypeExSetBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_confirm, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevTypeExSetBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devFanGearScaleSet_confirm);
	objpageDevTypeExSetBtn_cancel = lv_btn_create(pageSettingA_devTypeExSet, objpageDevTypeExSetBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, pageSettingA_devTypeExSet, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevTypeExSetBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devFanGearScaleSet_cancel);

	objpageDevTypeExSetBtnLabel_confirm = lv_label_create(objpageDevTypeExSetBtn_confirm, NULL);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_confirm, "Confirm");
	objpageDevTypeExSetBtnLabel_cancel = lv_label_create(objpageDevTypeExSetBtn_cancel, objpageDevTypeExSetBtnLabel_confirm);
	lv_obj_set_style(objpageDevTypeExSetBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageDevTypeExSetBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(pageSettingA_devTypeExSet);
	// usrApp_fullScreenRefresh_self(20, 80);
}

static void lvGui_pageSysLanguageSeclect_creat(void){

	uint8_t loop = 0;
	int16_t coordOffset_cLanguage_x = 0;

	if(pageSettingA_sysLanguageSelect == NULL)
		pageSettingA_sysLanguageSelect = lv_page_create(lv_scr_act(), NULL);

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_sysLanguageSelect, 300, 210)):
		(lv_obj_set_size(pageSettingA_sysLanguageSelect, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_sysLanguageSelect, 10, 20)):
		(lv_obj_set_pos(pageSettingA_sysLanguageSelect, 20, 45));		
	lv_page_set_style(pageSettingA_sysLanguageSelect, LV_PAGE_STYLE_SB, stylePage_devTypeSet);
	lv_page_set_style(pageSettingA_sysLanguageSelect, LV_PAGE_STYLE_BG, stylePage_devTypeSet);
	lv_page_set_sb_mode(pageSettingA_sysLanguageSelect, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_sysLanguageSelect, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_sysLanguageSelect, LV_LAYOUT_PRETTY);

	objpageSysLanguageSelLabel_title = lv_label_create(pageSettingA_sysLanguageSelect, NULL);
	lv_obj_set_style(objpageSysLanguageSelLabel_title, styleTextPageTitle_devTypeSet);
//	lv_label_set_text(objpageSysLanguageSelLabel_title, "System language");
	lv_label_set_text(objpageSysLanguageSelLabel_title, labelStrGet_bySysLanguage(tls_mSetting_systemLanguage_1));
	lv_obj_set_protect(objpageSysLanguageSelLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageSysLanguageSelLabel_title, pageSettingA_sysLanguageSelect, LV_ALIGN_IN_TOP_MID, 0, -5);

	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Portuguese:coordOffset_cLanguage_x = 20;break;
		case languageCyId_Arabic:coordOffset_cLanguage_x = -20;break;
		default:coordOffset_cLanguage_x = 0;break;}

	for(loop = 0; loop < DEV_LANGUAGE_COUNTRY_NUM_MAX; loop ++){

		if(NULL != systemLanguageStr_disp[loop].devName){

			objCb_sysLanguageSel[loop] = lv_cb_create(pageSettingA_sysLanguageSelect, NULL);
			lv_obj_set_size(objCb_sysLanguageSel[loop], 160, 20);
			lv_cb_set_text(objCb_sysLanguageSel[loop], systemLanguageStr_disp[loop].devName);
			lv_obj_set_protect(objCb_sysLanguageSel[loop], LV_PROTECT_POS);
			lv_obj_set_protect(objCb_sysLanguageSel[loop], LV_PROTECT_FOLLOW);
			lv_cb_set_style(objCb_sysLanguageSel[loop], LV_CB_STYLE_BG, stylePageCb_devTypeSet);

//			(devStatusDispMethod_landscapeIf_get())?
//				(lv_obj_align(objCb_sysLanguageSel[loop], objpageSysLanguageSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -25, loop * 25)):
//				(lv_obj_align(objCb_sysLanguageSel[loop], objpageSysLanguageSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -20, loop * 30 + 10));

			(devStatusDispMethod_landscapeIf_get())?
				(lv_obj_align(objCb_sysLanguageSel[loop], objpageSysLanguageSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + (loop % 2) * 120 - 45, loop / 2 * 25 + 10)):
				(lv_obj_align(objCb_sysLanguageSel[loop], objpageSysLanguageSelLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x, loop * 24 + 15));

			lv_obj_set_free_num(objCb_sysLanguageSel[loop], loop);
			lv_cb_set_action(objCb_sysLanguageSel[loop], funCb_cbOpreat_sysLanguageSelOpreation);
			if(devLanguageCountry_identifyGet() == systemLanguageStr_disp[loop].devType)lv_cb_set_checked(objCb_sysLanguageSel[loop], true);
		}
		else{

			break;
		}
	}

	objpageSysLanguageSelBtn_confirm = lv_btn_create(pageSettingA_sysLanguageSelect, NULL);
    lv_btn_set_style(objpageSysLanguageSelBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageSysLanguageSelBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageSysLanguageSelBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageSysLanguageSelBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageSysLanguageSelBtn_confirm, false, false);
	lv_obj_set_size(objpageSysLanguageSelBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageSysLanguageSelBtn_confirm, true);
	lv_obj_set_protect(objpageSysLanguageSelBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageSysLanguageSelBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?

		(lv_obj_align(objpageSysLanguageSelBtn_confirm, pageSettingA_sysLanguageSelect, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageSysLanguageSelBtn_confirm, pageSettingA_sysLanguageSelect, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageSysLanguageSelBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_sysLanguage_confirm);
	objpageSysLanguageSelBtn_cancel = lv_btn_create(pageSettingA_sysLanguageSelect, objpageSysLanguageSelBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?

		(lv_obj_align(objpageSysLanguageSelBtn_cancel, pageSettingA_sysLanguageSelect, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageSysLanguageSelBtn_cancel, objpageSysLanguageSelBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageSysLanguageSelBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_sysLanguage_cancel);

	objpageSysLanguageSelBtnLabel_confirm = lv_label_create(objpageSysLanguageSelBtn_confirm, NULL);
	lv_obj_set_style(objpageSysLanguageSelBtnLabel_confirm, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageSysLanguageSelBtnLabel_confirm, "Confirm");
	objpageSysLanguageSelBtnLabel_cancel = lv_label_create(objpageSysLanguageSelBtn_cancel, objpageSysLanguageSelBtnLabel_confirm);
	lv_obj_set_style(objpageSysLanguageSelBtnLabel_cancel, styleLabelPageBtn_devTypeSet);
	lv_label_set_text(objpageSysLanguageSelBtnLabel_cancel, "Cancel");

	if(NULL != btnSettingA_systemLanguage)
		lv_obj_set_click(btnSettingA_systemLanguage, false);

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(pageSettingA_sysLanguageSelect);
	// usrApp_fullScreenRefresh_self(20, 80);
}

static lv_res_t funCb_slidAction_ALCS_set_color(lv_obj_t *slider){

	LV_OBJ_FREE_NUM_TYPE objFnum = lv_obj_get_free_num(slider);

	switch(objFnum){

		case LV_FNUM_SLIDER_ALCS_R:{

			atmosLightRunParam_dataTemp.lightColorCustomParamcfg.red = lv_slider_get_value(slider);
	
		}break;
		
		case LV_FNUM_SLIDER_ALCS_G:{

			atmosLightRunParam_dataTemp.lightColorCustomParamcfg.green = lv_slider_get_value(slider);

		}break;
		
		case LV_FNUM_SLIDER_ALCS_B:{

			atmosLightRunParam_dataTemp.lightColorCustomParamcfg.blue = lv_slider_get_value(slider);

		}break;

		default:break;
	}

	styleBtn_devAtmosLightColorSet->body.main_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
	styleBtn_devAtmosLightColorSet->body.grad_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;	
	lv_obj_set_style(objpageDevALCSetBlock_colorPreview, styleBtn_devAtmosLightColorSet);
	lv_obj_refresh_style(objpageDevALCSetBlock_colorPreview);
	stylePage_devAtmosLightColorSet->body.border.color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
	lv_obj_refresh_style(pageSettingA_devAtmosLightColorSet);

	return LV_RES_OK;
}

static lv_res_t funCb_cbAction_ALCS_exParamSet(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE objFnum = lv_obj_get_free_num(cb);

	switch(objFnum){

		case LV_FNUM_CB_ALCS_AUTOMATIC:{

			atmosLightRunParam_dataTemp.runingAsCustomCfg_flg = !lv_cb_is_checked(cb);
			lvGui_pageAtmosLightColorSetAffiAttend(!lv_cb_is_checked(cb));

		}break;

		case LV_FNUM_CB_ALCS_BRE_IF:{

			(true == lv_cb_is_checked(cb))?
				(atmosLightRunParam_dataTemp.customCfg_breathIf = 1):
				(atmosLightRunParam_dataTemp.customCfg_breathIf = 0);

		}break;

		case LV_FNUM_CB_ALCS_SETALL_IF:{

			atmosLightCfgOptAvailableAllSw_if = lv_cb_is_checked(cb);

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static void lvGui_pageAtmosLightColorSetAffiAttend(bool creatIf){

	if(creatIf){

		devAtmosphere_statusTips_trigSet(atmosphereLightType_idleCfgPreview);

		if(false == settingItem_ALCS_affiObjs_availableFlg){

			objpageDevALCSetLabel_colorPreview = lv_label_create(pageSettingA_devAtmosLightColorSet, NULL);
			lv_obj_set_style(objpageDevALCSetLabel_colorPreview, styleTextPageTitle_devAtmosLightColorSet);
//			lv_label_set_text(objpageDevALCSetLabel_colorPreview, "Preview:");
			lv_label_set_text(objpageDevALCSetLabel_colorPreview, labelStrGet_bySysLanguage(tls_mSetting_preview));
			lv_obj_set_protect(objpageDevALCSetLabel_colorPreview, LV_PROTECT_POS);
			if(true == devStatusDispMethod_landscapeIf_get()){
				lv_obj_align(objpageDevALCSetLabel_colorPreview, objpageDevAtmosLightColorSet_title, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 20);
			}else{
				lv_obj_align(objpageDevALCSetLabel_colorPreview, objpageDevAtmosLightColorSet_title, LV_ALIGN_OUT_BOTTOM_RIGHT, 15, 20);}
			objpageDevALCSetBlock_colorPreview = lv_obj_create(pageSettingA_devAtmosLightColorSet, NULL);
			lv_obj_set_size(objpageDevALCSetBlock_colorPreview, 35, 35);
			lv_obj_set_protect(objpageDevALCSetBlock_colorPreview, LV_PROTECT_POS);
			styleBtn_devAtmosLightColorSet->body.main_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
			styleBtn_devAtmosLightColorSet->body.grad_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
			lv_obj_set_style(objpageDevALCSetBlock_colorPreview, styleBtn_devAtmosLightColorSet);
			if(true == devStatusDispMethod_landscapeIf_get()){
				lv_obj_align(objpageDevALCSetBlock_colorPreview, objpageDevALCSetLabel_colorPreview, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
			}else{
				lv_obj_align(objpageDevALCSetBlock_colorPreview, objpageDevALCSetLabel_colorPreview, LV_ALIGN_OUT_BOTTOM_MID, 3, 14);
			}
			
			objpageDevALCSetLabel_ref = lv_label_create(pageSettingA_devAtmosLightColorSet, NULL);
			lv_label_set_style(objpageDevALCSetLabel_ref, styleTextPageTitle_devAtmosLightColorSet);
			lv_label_set_recolor(objpageDevALCSetLabel_ref, true);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:
					lv_label_set_text(objpageDevALCSetLabel_ref, "#FF2020 R  # #20FF20        G  # #2020FF       B#");
					break;
				default:
					lv_label_set_text(objpageDevALCSetLabel_ref, "#FF2020 R  # #20FF20  G  # #2020FF B#");
					break;}
			lv_obj_set_protect(objpageDevALCSetLabel_ref, LV_PROTECT_POS);
			if(true == devStatusDispMethod_landscapeIf_get()){
				lv_obj_align(objpageDevALCSetLabel_ref, objpageDevALCSetLabel_colorPreview, LV_ALIGN_OUT_LEFT_BOTTOM, -35, 100);
			}else{
				lv_obj_align(objpageDevALCSetLabel_ref, objpageDevALCSetLabel_colorPreview, LV_ALIGN_OUT_LEFT_BOTTOM, -12, 100);
			}
			
			objpageDevALCSetSlider_color_r = lv_slider_create(pageSettingA_devAtmosLightColorSet, NULL);
			lv_slider_set_range(objpageDevALCSetSlider_color_r, 0, 32 - 1);
			lv_obj_set_size(objpageDevALCSetSlider_color_r, 13, 110);
			lv_obj_set_protect(objpageDevALCSetSlider_color_r, LV_PROTECT_POS);
			lv_obj_align(objpageDevALCSetSlider_color_r, objpageDevALCSetLabel_ref, LV_ALIGN_OUT_TOP_LEFT, -2, -5);
			lv_slider_set_action(objpageDevALCSetSlider_color_r, funCb_slidAction_ALCS_set_color);
			lv_obj_set_free_num(objpageDevALCSetSlider_color_r, LV_FNUM_SLIDER_ALCS_R);
			lv_slider_set_value(objpageDevALCSetSlider_color_r, atmosLightRunParam_dataTemp.lightColorCustomParamcfg.red);
			objpageDevALCSetSlider_color_g = lv_slider_create(pageSettingA_devAtmosLightColorSet, objpageDevALCSetSlider_color_r);
			lv_slider_set_range(objpageDevALCSetSlider_color_g, 0, 64 - 1);
			lv_obj_align(objpageDevALCSetSlider_color_g, objpageDevALCSetSlider_color_r, LV_ALIGN_OUT_RIGHT_MID, 18, 0);
			lv_slider_set_action(objpageDevALCSetSlider_color_g, funCb_slidAction_ALCS_set_color);
			lv_obj_set_free_num(objpageDevALCSetSlider_color_g, LV_FNUM_SLIDER_ALCS_G);
			lv_slider_set_value(objpageDevALCSetSlider_color_g, atmosLightRunParam_dataTemp.lightColorCustomParamcfg.green);
			objpageDevALCSetSlider_color_b = lv_slider_create(pageSettingA_devAtmosLightColorSet, objpageDevALCSetSlider_color_r);
			lv_slider_set_range(objpageDevALCSetSlider_color_b, 0, 32 - 1);
			lv_obj_align(objpageDevALCSetSlider_color_b, objpageDevALCSetSlider_color_g, LV_ALIGN_OUT_RIGHT_MID, 19, 0);
			lv_slider_set_action(objpageDevALCSetSlider_color_b, funCb_slidAction_ALCS_set_color);
			lv_obj_set_free_num(objpageDevALCSetSlider_color_b, LV_FNUM_SLIDER_ALCS_B);
			lv_slider_set_value(objpageDevALCSetSlider_color_b, atmosLightRunParam_dataTemp.lightColorCustomParamcfg.blue);
			
			objpageDevALCSetCb_breathIf = lv_cb_create(pageSettingA_devAtmosLightColorSet, NULL);
			lv_label_set_recolor(objpageDevALCSetCb_breathIf, true);
			lv_obj_set_size(objpageDevALCSetCb_breathIf, 15, 80);
			lv_obj_set_protect(objpageDevALCSetCb_breathIf, LV_PROTECT_POS);
			if(true == devStatusDispMethod_landscapeIf_get()){
				lv_obj_align(objpageDevALCSetCb_breathIf, objpageDevALCSetLabel_colorPreview, LV_ALIGN_OUT_BOTTOM_LEFT, -27, 15);
			}else{
				lv_obj_align(objpageDevALCSetCb_breathIf, objpageDevALCSetLabel_ref, LV_ALIGN_OUT_BOTTOM_LEFT, -5, 5);
			}
			lv_cb_set_style(objpageDevALCSetCb_breathIf, LV_CB_STYLE_BG, styleLabelPageCb_devALSC_breathIf);
			//lv_cb_set_text(objpageDevALCSetCb_breathIf, "Breathing effect");
			lv_cb_set_text(objpageDevALCSetCb_breathIf, labelStrGet_bySysLanguage(tls_mSetting_breathingEffect));
			lv_obj_set_free_num(objpageDevALCSetCb_breathIf, LV_FNUM_CB_ALCS_BRE_IF);
			lv_cb_set_action(objpageDevALCSetCb_breathIf, funCb_cbAction_ALCS_exParamSet);
			(atmosLightRunParam_dataTemp.customCfg_breathIf)?
				(lv_cb_set_checked(objpageDevALCSetCb_breathIf, true)):
				(lv_cb_set_checked(objpageDevALCSetCb_breathIf, false));

			settingItem_ALCS_affiObjs_availableFlg = true;
		}

		switch(devLanguageCountry_identifyGet()){
			case languageCyId_Arabic:
				lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_breathIf, LV_ALIGN_OUT_BOTTOM_MID, -8, -10);
				break;
			case languageCyId_Portuguese:
				lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_breathIf, LV_ALIGN_OUT_BOTTOM_MID, -17, -10);
				break;
			default:
				lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_breathIf, LV_ALIGN_OUT_BOTTOM_MID, -4, -10);
				break;}

		if(true == devStatusDispMethod_landscapeIf_get()){
			lv_obj_align(objpageDevALCSetCb_automaticSet, objpageDevALCSetCb_setAllSwitch, LV_ALIGN_OUT_BOTTOM_MID, 0, -5);
		}else{
			lv_obj_align(objpageDevALCSetCb_automaticSet, pageSettingA_devAtmosLightColorSet, LV_ALIGN_IN_BOTTOM_MID, 0, -22);
		}
	}
	else{

		if(true == settingItem_ALCS_affiObjs_availableFlg){

			lv_obj_del(objpageDevALCSetLabel_colorPreview);
			lv_obj_del(objpageDevALCSetBlock_colorPreview);
			lv_obj_del(objpageDevALCSetLabel_ref);
			lv_obj_del(objpageDevALCSetSlider_color_r);
			lv_obj_del(objpageDevALCSetSlider_color_g);
			lv_obj_del(objpageDevALCSetSlider_color_b);
			lv_obj_del(objpageDevALCSetCb_breathIf);

			settingItem_ALCS_affiObjs_availableFlg = false;
		}
		
		lv_obj_align(objpageDevALCSetCb_automaticSet, pageSettingA_devAtmosLightColorSet, LV_ALIGN_CENTER, 0, 0);
		if(true == devStatusDispMethod_landscapeIf_get()){
			lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_automaticSet, LV_ALIGN_OUT_BOTTOM_MID, 14, 5);		
		}else{
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Portuguese:lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_automaticSet, LV_ALIGN_OUT_BOTTOM_MID, 7, 5);break;
				default:lv_obj_align(objpageDevALCSetCb_setAllSwitch, objpageDevALCSetCb_automaticSet, LV_ALIGN_OUT_BOTTOM_MID, 14, 5);break;
			}
		}
	}
}

static void lvGui_pageAtmosLightColorSet_creat(void){

	devAtmosLight_runningParam_get(&atmosLightRunParam_dataTemp);

	atmosLightCfgOptAvailableAllSw_if = false;

	if(pageSettingA_devAtmosLightColorSet == NULL)
		pageSettingA_devAtmosLightColorSet = lv_page_create(lv_scr_act(), NULL);

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageSettingA_devAtmosLightColorSet, 300, 210)):
		(lv_obj_set_size(pageSettingA_devAtmosLightColorSet, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(pageSettingA_devAtmosLightColorSet, 10, 20)):
		(lv_obj_set_pos(pageSettingA_devAtmosLightColorSet, 20, 45));	
	stylePage_devAtmosLightColorSet->body.border.color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
	lv_page_set_style(pageSettingA_devAtmosLightColorSet, LV_PAGE_STYLE_SB, stylePage_devAtmosLightColorSet);
	lv_page_set_style(pageSettingA_devAtmosLightColorSet, LV_PAGE_STYLE_BG, stylePage_devAtmosLightColorSet);
	lv_page_set_sb_mode(pageSettingA_devAtmosLightColorSet, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(pageSettingA_devAtmosLightColorSet, false, true); //key opration
	lv_page_set_scrl_layout(pageSettingA_devAtmosLightColorSet, LV_LAYOUT_PRETTY);

	objpageDevAtmosLightColorSet_title = lv_label_create(pageSettingA_devAtmosLightColorSet, NULL);
	lv_obj_set_style(objpageDevAtmosLightColorSet_title, styleTextPageTitle_devAtmosLightColorSet);
	lv_label_set_text(objpageDevAtmosLightColorSet_title, labelStrGet_bySysLanguage(tls_mSetting_illminationSet));
	lv_obj_set_protect(objpageDevAtmosLightColorSet_title, LV_PROTECT_POS);
	lv_obj_align(objpageDevAtmosLightColorSet_title, pageSettingA_deviceTypeSelect, LV_ALIGN_IN_TOP_MID, 0, -5);

	objpageDevALCSetCb_automaticSet = lv_cb_create(pageSettingA_devAtmosLightColorSet, NULL);
	lv_label_set_recolor(objpageDevALCSetCb_automaticSet, true);
	lv_obj_set_size(objpageDevALCSetCb_automaticSet, 15, 80);
	lv_obj_set_protect(objpageDevALCSetCb_automaticSet, LV_PROTECT_POS);
	lv_obj_align(objpageDevALCSetCb_automaticSet, pageSettingA_devAtmosLightColorSet, LV_ALIGN_CENTER, 0, 0);
	lv_cb_set_style(objpageDevALCSetCb_automaticSet, LV_CB_STYLE_BG, styleLabelPageCb_devALSC_autoSet);
//	lv_cb_set_text(objpageDevALCSetCb_automaticSet, "Automatic");
	lv_cb_set_text(objpageDevALCSetCb_automaticSet, labelStrGet_bySysLanguage(tls_mSetting_automatic));
	lv_obj_set_free_num(objpageDevALCSetCb_automaticSet, LV_FNUM_CB_ALCS_AUTOMATIC);
	lv_cb_set_action(objpageDevALCSetCb_automaticSet, funCb_cbAction_ALCS_exParamSet);
	(atmosLightRunParam_dataTemp.runingAsCustomCfg_flg)?
		(lv_cb_set_checked(objpageDevALCSetCb_automaticSet, false)):
		(lv_cb_set_checked(objpageDevALCSetCb_automaticSet, true));
	objpageDevALCSetCb_setAllSwitch = lv_cb_create(pageSettingA_devAtmosLightColorSet, objpageDevALCSetCb_automaticSet);
	lv_cb_set_style(objpageDevALCSetCb_setAllSwitch, LV_CB_STYLE_BG, styleLabelPageCb_devALSC_setAllSw);
//	lv_cb_set_text(objpageDevALCSetCb_setAllSwitch, "Set all switches");
	lv_cb_set_text(objpageDevALCSetCb_setAllSwitch, labelStrGet_bySysLanguage(tls_mSetting_setAllSwitches));
	lv_obj_set_free_num(objpageDevALCSetCb_setAllSwitch, LV_FNUM_CB_ALCS_SETALL_IF);
	lv_cb_set_checked(objpageDevALCSetCb_setAllSwitch, false);
	lvGui_pageAtmosLightColorSetAffiAttend(!lv_cb_is_checked(objpageDevALCSetCb_automaticSet));

	objpageDevAtmosLightColorSetBtn_confirm = lv_btn_create(pageSettingA_devAtmosLightColorSet, NULL);
    lv_btn_set_style(objpageDevAtmosLightColorSetBtn_confirm, LV_BTN_STYLE_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevAtmosLightColorSetBtn_confirm, LV_BTN_STYLE_PR, &lv_style_transp);
    lv_btn_set_style(objpageDevAtmosLightColorSetBtn_confirm, LV_BTN_STYLE_TGL_REL, &lv_style_transp);
    lv_btn_set_style(objpageDevAtmosLightColorSetBtn_confirm, LV_BTN_STYLE_TGL_PR, &lv_style_transp);
	lv_btn_set_fit(objpageDevAtmosLightColorSetBtn_confirm, false, false);
	lv_obj_set_size(objpageDevAtmosLightColorSetBtn_confirm, 80, 25);
	lv_page_glue_obj(objpageDevAtmosLightColorSetBtn_confirm, true);
	lv_obj_set_protect(objpageDevAtmosLightColorSetBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageDevAtmosLightColorSetBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?

		(lv_obj_align(objpageDevAtmosLightColorSetBtn_confirm, pageSettingA_devAtmosLightColorSet, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageDevAtmosLightColorSetBtn_confirm, pageSettingA_devAtmosLightColorSet, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageDevAtmosLightColorSetBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devAtmosLightColor_confirm);
	objpageDevAtmosLightColorSetBtn_cancel = lv_btn_create(pageSettingA_devAtmosLightColorSet, objpageDevAtmosLightColorSetBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?

		(lv_obj_align(objpageDevAtmosLightColorSetBtn_cancel, pageSettingA_devAtmosLightColorSet, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageDevAtmosLightColorSetBtn_cancel, objpageDevAtmosLightColorSetBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageDevAtmosLightColorSetBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_devAtmosLightColor_cancel);
	
	objpageDevAtmosLightColorSet_confirm = lv_label_create(objpageDevAtmosLightColorSetBtn_confirm, NULL);
	lv_obj_set_style(objpageDevAtmosLightColorSet_confirm, styleLabelPageBtn_devAtmosLightColorSet);
//	lv_label_set_text(objpageDevAtmosLightColorSet_confirm, "Confirm");
	lv_label_set_text(objpageDevAtmosLightColorSet_confirm, labelStrGet_bySysLanguage(tls_confirm));
	objpageDevAtmosLightColorSet_cancel = lv_label_create(objpageDevAtmosLightColorSetBtn_cancel, objpageDevAtmosLightColorSet_confirm);
	lv_obj_set_style(objpageDevAtmosLightColorSet_cancel, styleLabelPageBtn_devAtmosLightColorSet);
//	lv_label_set_text(objpageDevAtmosLightColorSet_cancel, "Cancel");
	lv_label_set_text(objpageDevAtmosLightColorSet_cancel, labelStrGet_bySysLanguage(tls_cancel));

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(pageSettingA_devAtmosLightColorSet);
//	usrApp_fullScreenRefresh_self(20, 80);
}

static lv_res_t funCb_btnActionClick_btnDevType_funBack(lv_obj_t *btn){

	lvGui_pageDevTypeSeclect_creat();

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_curtainTrackTime_funBack(lv_obj_t *btn){

	// lvGui_pageDevCurtainTrackTimeSet_creat();
	lvGui_pageDevCurtainTrackTimeCfgMethod_creat();

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionLongPreReapt_curtainTrackTime_funBack(lv_obj_t *btn){

	static uint8_t reaptRcd = 0;

	printf(">>>long press happen, %d.\n", reaptRcd);
	if(reaptRcd < 20)reaptRcd ++;
	else{
		reaptRcd = 0;
		// lv_obj_set_click(btn, false);
		devDriverBussiness_curtainSwitch_orbitalTimeAutoDetectTrig();
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_fanGearScale_funBack(lv_obj_t *btn){

	lvGui_pageDevFanGearScaleSet_creat();

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_dimmerBrightnessBase_funBack(lv_obj_t *btn){

	lvGui_pageDevDimmerBrightnessBaseSet_creat();

	return LV_RES_OK;
}

static void lvGui_devTypeExSetElementRefresh(void){

		char strLabelTmp[32] = {0};

		switch(currentDev_typeGet()){
			case devTypeDef_curtain:
				paramRefreshDevCurtainTrackTime();
				memset(strLabelTmp, 0, sizeof(strLabelTmp));
				sprintf(strLabelTmp, "#E8E820 T:%ds#", devCurtainTrackTimeParamTmp.vTmp_minute * 60 + devCurtainTrackTimeParamTmp.vTmp_second);
				lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);
				lv_btn_set_action(btnSettingA_typeDevExSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_curtainTrackTime_funBack);
				lv_btn_set_action(btnSettingA_typeDevExSet, LV_BTN_ACTION_LONG_PR_REPEAT, funCb_btnActionLongPreReapt_curtainTrackTime_funBack);
				lv_obj_set_hidden(btnSettingA_typeDevExSet, false);
				lv_obj_set_width(btnSettingA_deviceType, 100);
				lv_obj_set_width(btnLabelSettingA_deviceType, 80);
				lv_obj_align(btnLabelSettingA_deviceType, btnSettingA_deviceType, LV_ALIGN_CENTER, 18, 0);
				lv_obj_align(btnSettingA_typeDevExSet, btnSettingA_deviceType, LV_ALIGN_OUT_RIGHT_MID, 5,  -5);
				break;
			case devTypeDef_dimmer:
				paramRefreshDevDimmerBrightnessBase();
				memset(strLabelTmp, 0, sizeof(strLabelTmp));
				sprintf(strLabelTmp, "#E8E820 S:%d%%#", devDimmerBrightnessBaseTmp);
				lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);
				lv_btn_set_action(btnSettingA_typeDevExSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_dimmerBrightnessBase_funBack);
				lv_obj_set_hidden(btnSettingA_typeDevExSet, false);
				lv_obj_set_width(btnSettingA_deviceType, 100);
				lv_obj_set_width(btnLabelSettingA_deviceType, 80);
				lv_obj_align(btnLabelSettingA_deviceType, btnSettingA_deviceType, LV_ALIGN_CENTER, 18, 0);
				lv_obj_align(btnSettingA_typeDevExSet, btnSettingA_deviceType, LV_ALIGN_OUT_RIGHT_MID, 5,  -5);
				break;
			case devTypeDef_fans:
				paramRefreshDevFanGearScale();
				memset(strLabelTmp, 0, sizeof(strLabelTmp));
				sprintf(strLabelTmp, "#E8E820 G1:%d%%#", fanGearSubdivDetailParamTmp.subdivDetailGear_1);
				lv_label_set_text(btnLabelSettingA_typeDevExSet, strLabelTmp);
				lv_btn_set_action(btnSettingA_typeDevExSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_fanGearScale_funBack);
				lv_obj_set_hidden(btnSettingA_typeDevExSet, false);
				lv_obj_set_width(btnSettingA_deviceType, 100);
				lv_obj_set_width(btnLabelSettingA_deviceType, 80);
				lv_obj_align(btnLabelSettingA_deviceType, btnSettingA_deviceType, LV_ALIGN_CENTER, 18, 0);
				lv_obj_align(btnSettingA_typeDevExSet, btnSettingA_deviceType, LV_ALIGN_OUT_RIGHT_MID, 5,  -5);
				break;
			default:
				lv_obj_set_hidden(btnSettingA_typeDevExSet, true);
				lv_obj_set_width(btnSettingA_deviceType, 200);
				lv_obj_set_width(btnLabelSettingA_deviceType, 180);
				lv_obj_align(btnLabelSettingA_deviceType, btnSettingA_deviceType, LV_ALIGN_CENTER, 0, 0);
				lv_obj_align(btnSettingA_typeDevExSet, btnSettingA_deviceType, LV_ALIGN_OUT_RIGHT_MID, 10, 0 );
				switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, 0 - screenLandscapeCoordinate_objOffset - 55, 15);break;
					case languageCyId_Portuguese:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 25, 15);break;
					default:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 35, 15);break;}
				break;
		}
}

static lv_res_t funCb_btnActionClick_btnSysLanguage_funBack(lv_obj_t *btn){

	lvGui_pageSysLanguageSeclect_creat();

	return LV_RES_OK;
}

static lv_res_t funCb_ddlistDevTypeDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_DEVTYPE_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);

		if(devTypeDdist_istRecord != ddlist_opSelect){

			devTypeDdist_istRecord = ddlist_opSelect;
			
			for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){
			
				if(ddlist_opSelect == deviceTypeTab_disp[loop].devInst_disp){
			
					stt_devDataPonitTypedef devDataPoint_temp = {0};
			
					currentDev_typeSet(deviceTypeTab_disp[loop].devType, true);
					devDriverManageBussiness_deviceChangeRefresh(); //驱动更新
					switch(currentDev_typeGet()){

						case devTypeDef_thermostat:
						case devTypeDef_thermostatExtension:{

							devDataPoint_temp.devType_thermostat.devThermostat_tempratureTarget = DEVICE_THERMOSTAT_TEMPTARGET_DEFAULT;
							currentDev_dataPointSet(&devDataPoint_temp, true, false, false, false, false); //数据点加载默认值

						}break;

						case devTypeDef_scenario:{

							/*什么都不做*/
				
						}break;

						case devTypeDef_curtain:{

							stt_devStatusRecord devStatusRecordFlg_temp = {0};
							
							devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

							devDataPoint_temp.devType_curtain.devCurtain_actMethod = 1;
							devDataPoint_temp.devType_curtain.devCurtain_actEnumVal = 0;
							currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false);

							devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
							devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
						}break;

						default:{

							currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false, false); //数据点清零
						}break;
					}
			
					devSystemInfoLocalRecord_normalClear();
			
					usrAppHomepageBtnTextDisp_defaultLoad(deviceTypeTab_disp[loop].devType, true); //图文显示恢复默认
					usrAppHomepageBtnIconNumDisp_defaultLoad(deviceTypeTab_disp[loop].devType, true);
					
					break;
				}
			}
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_ddlistHomepageThemeDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_HPTHEME_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);

		usrAppHomepageThemeType_Set(ddlist_opSelect, true, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_ddlistHomepageUiKindIndexDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_HUIINDEX_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);

		usrAppHomepageUiKindIndex_Set(ddlist_opSelect, true, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_swDevStatusRecordIF(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	printf("swDevStatusRecordIF set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 1;
	}
	else
	{
		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
	}
	devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devAtmosLightColorSet(lv_obj_t *sw){

	lvGui_pageAtmosLightColorSet_creat();

	return LV_RES_OK;
}

static lv_res_t funCb_swDevForceAsMeshChild(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	mwifi_config_t ap_config			= {0x0};
	mwifi_init_config_t init_config 	= {0x0};

	if(sw_action){

		opreatTemp_devForceAsMeshChild = 1;
	}
	else
	{
		opreatTemp_devForceAsMeshChild = 0;
	}
	devForceAsMeshChild_opreatConfirm();

	return LV_RES_OK;
}

static lv_res_t funCb_swDevListManagerEN(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	printf("swDevListManagerEN set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.devListManager_En = 1;
	}
	else
	{
		devStatusRecordFlg_temp.devListManager_En = 0;
	}
	devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

	return LV_RES_OK;
}

static lv_res_t funCb_epidemicReportIF(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	printf("swEpidemicReportIF set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.screensaverRunning_IF = 1;
	}
	else
	{
		devStatusRecordFlg_temp.screensaverRunning_IF = 0;
	}
	
	lvGui_settingSet_epidemicCtrlObj_opt(devStatusRecordFlg_temp.screensaverRunning_IF);
	
	devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

	return LV_RES_OK;
}

static lv_res_t funCb_homeassistantEnable(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	printf("homeassistantEnable set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.homeassitant_En = 1;
	}
	else
	{
		devStatusRecordFlg_temp.homeassitant_En = 0;
	}
	
	devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

	dtRmoteServer_serverSwitchByDefault_trig(sw_action);
//	lvGui_usrAppBussinessRunning_block(0, "\nparam changing", 5); 	
	lvGui_usrAppBussinessRunning_block(0, labelStrGet_bySysLanguage(tls_paramChanging), 5);	

	return LV_RES_OK;
}

static lv_res_t funCb_rollerEpidemicCyLocation(lv_obj_t *rol){

	uint16_t valSet = lv_roller_get_selected(rol);

//	sprintf(textlabel_dispTemp, "Location[#FFFF00 %s#]:", epidCyAbbre[valSet]);
//	lv_label_set_text(textSettingA_epidemicCountry, (const char *)textlabel_dispTemp);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:sprintf(textlabel_dispTemp, ":%s[#FFFF00 %s#]", labelStrGet_bySysLanguage(tls_mSetting_epidemicZone), epidCyAbbre[valSet]);break;
		default:sprintf(textlabel_dispTemp, "%s[#FFFF00 %s#]:", labelStrGet_bySysLanguage(tls_mSetting_epidemicZone), epidCyAbbre[valSet]);break;}
	lv_label_set_text(textSettingA_epidemicCountry, (const char *)textlabel_dispTemp);

	dispApplication_epidCyLocation_set(valSet, true);
	
	return LV_RES_OK;
}

static void lvGuiMenuSettingSet_styleMemoryInitialization(void){

	static bool memAlloced_flg = false;

	if(true == memAlloced_flg)return;
	else memAlloced_flg = true;

	stylePage_funSetOption 		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleText_menuLevel_A 		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleText_menuLevel_B 		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleText_menuLevel_C 		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleBk_objBground 			= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleDdlistSettingA_devType = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabel_devTypeSelected	= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	bg_styleDevStatusRecordIF 	= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	indic_styleDevStatusRecordIF = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	knob_on_styleDevStatusRecordIF = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	knob_off_styleDevStatusRecordIF = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleMbox_bg 				= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleMbox_btn_pr 			= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleMbox_btn_rel 			= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleBtn_specialTransparent = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleImg_menuFun_btnFun 	= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePageCb_elecParamDispSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePageCb_tempDispMethodSet  = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePageCb_screenLandscape = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleRoller_epidemicCyLocation_bg = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleRoller_epidemicCyLocation_sel = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePage_devTypeSet 		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleTextPageTitle_devTypeSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleTextPageRef_devTypeExSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePageCb_devTypeSet		= (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabelPageBtn_devTypeSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleRoller_devTypeExSet_bg = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleRoller_devTypeExSet_sel = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabel_devTypeExSet_rollerIns = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleBtn_devAtmosLightColorSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	stylePage_devAtmosLightColorSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleTextPageTitle_devAtmosLightColorSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabelPageBtn_devAtmosLightColorSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabelPageCb_devALSC_breathIf = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabelPageCb_devALSC_setAllSw = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
	styleLabelPageCb_devALSC_autoSet = (lv_style_t *)LV_MEM_CUSTOM_ALLOC(sizeof(lv_style_t));
}

static void lvGuiMenuSettingSet_styleMemoryDeinit(void){

	LV_STYLE_MEM_FREE(stylePage_funSetOption);
	LV_STYLE_MEM_FREE(styleText_menuLevel_A);
	LV_STYLE_MEM_FREE(styleText_menuLevel_B);
	LV_STYLE_MEM_FREE(styleText_menuLevel_C);
	LV_STYLE_MEM_FREE(styleBk_objBground);
	LV_STYLE_MEM_FREE(styleDdlistSettingA_devType);
	LV_STYLE_MEM_FREE(styleLabel_devTypeSelected);
	LV_STYLE_MEM_FREE(bg_styleDevStatusRecordIF);
	LV_STYLE_MEM_FREE(indic_styleDevStatusRecordIF);
	LV_STYLE_MEM_FREE(knob_on_styleDevStatusRecordIF);
	LV_STYLE_MEM_FREE(knob_off_styleDevStatusRecordIF);
	LV_STYLE_MEM_FREE(styleMbox_bg);
	LV_STYLE_MEM_FREE(styleMbox_btn_pr);
	LV_STYLE_MEM_FREE(styleMbox_btn_rel);
	LV_STYLE_MEM_FREE(styleBtn_specialTransparent);
	LV_STYLE_MEM_FREE(styleImg_menuFun_btnFun);
	LV_STYLE_MEM_FREE(stylePageCb_elecParamDispSet);
	LV_STYLE_MEM_FREE(stylePageCb_tempDispMethodSet);
	LV_STYLE_MEM_FREE(stylePageCb_screenLandscape);
	LV_STYLE_MEM_FREE(styleRoller_epidemicCyLocation_bg);
	LV_STYLE_MEM_FREE(styleRoller_epidemicCyLocation_sel);
	LV_STYLE_MEM_FREE(stylePage_devTypeSet);
	LV_STYLE_MEM_FREE(styleTextPageTitle_devTypeSet);
	LV_STYLE_MEM_FREE(styleTextPageRef_devTypeExSet);
	LV_STYLE_MEM_FREE(stylePageCb_devTypeSet);
	LV_STYLE_MEM_FREE(styleLabelPageBtn_devTypeSet);
	LV_STYLE_MEM_FREE(styleRoller_devTypeExSet_bg);
	LV_STYLE_MEM_FREE(styleRoller_devTypeExSet_sel);
	LV_STYLE_MEM_FREE(styleLabel_devTypeExSet_rollerIns);
	LV_STYLE_MEM_FREE(styleBtn_devAtmosLightColorSet);
	LV_STYLE_MEM_FREE(stylePage_devAtmosLightColorSet);
	LV_STYLE_MEM_FREE(styleTextPageTitle_devAtmosLightColorSet);
	LV_STYLE_MEM_FREE(styleLabelPageBtn_devAtmosLightColorSet);
	LV_STYLE_MEM_FREE(styleLabelPageCb_devALSC_breathIf);
	LV_STYLE_MEM_FREE(styleLabelPageCb_devALSC_setAllSw);
	LV_STYLE_MEM_FREE(styleLabelPageCb_devALSC_autoSet);
}

static void lvGuiSettingSet_objStyle_Init(void){

	lvGuiMenuSettingSet_styleMemoryInitialization();
	mbox_btnm_textTab = dialogSelTabGet_bySysLanguage(dialogSelTab_yesNo);

	lv_style_copy(styleText_menuLevel_A, &lv_style_plain);
//	styleText_menuLevel_A->text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A->text.font = fontLabelGet_bySysLanguage(5);
	styleText_menuLevel_A->text.color = LV_COLOR_WHITE;
	
	lv_style_copy(styleText_menuLevel_B, &lv_style_plain);
//	styleText_menuLevel_B->text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B->text.font = fontLabelGet_bySysLanguage(1);
	styleText_menuLevel_B->text.color = LV_COLOR_WHITE;

	lv_style_copy(styleText_menuLevel_C, &lv_style_plain);
	styleText_menuLevel_C->text.font = &lv_font_consola_16;
	styleText_menuLevel_C->text.color = LV_COLOR_WHITE;

	lv_style_copy(stylePage_funSetOption, &lv_style_plain);
	stylePage_funSetOption->body.main_color = LV_COLOR_GRAY;
	stylePage_funSetOption->body.grad_color = LV_COLOR_GRAY;

	lv_style_copy(styleDdlistSettingA_devType, &lv_style_pretty);
	styleDdlistSettingA_devType->body.shadow.width = 1;
	styleDdlistSettingA_devType->text.color = LV_COLOR_MAKE(0x10, 0x20, 0x50);
	styleDdlistSettingA_devType->text.font = &lv_font_dejavu_15;
	
	lv_style_copy(styleLabel_devTypeSelected, &lv_style_plain);
	styleLabel_devTypeSelected->text.font = &lv_font_ariblk_18;
	styleLabel_devTypeSelected->text.color = LV_COLOR_MAKE(192, 255, 0);

	lv_style_copy(bg_styleDevStatusRecordIF, &lv_style_pretty);
	bg_styleDevStatusRecordIF->body.radius = LV_RADIUS_CIRCLE;
	lv_style_copy(indic_styleDevStatusRecordIF, &lv_style_pretty_color);
	indic_styleDevStatusRecordIF->body.radius = LV_RADIUS_CIRCLE;
	indic_styleDevStatusRecordIF->body.main_color = LV_COLOR_HEX(0x9fc8ef);
	indic_styleDevStatusRecordIF->body.grad_color = LV_COLOR_HEX(0x9fc8ef);
	indic_styleDevStatusRecordIF->body.padding.hor = 0;
	indic_styleDevStatusRecordIF->body.padding.ver = 0;	
	lv_style_copy(knob_off_styleDevStatusRecordIF, &lv_style_pretty);
	knob_off_styleDevStatusRecordIF->body.radius = LV_RADIUS_CIRCLE;
	knob_off_styleDevStatusRecordIF->body.shadow.width = 4;
	knob_off_styleDevStatusRecordIF->body.shadow.type = LV_SHADOW_BOTTOM;
	lv_style_copy(knob_on_styleDevStatusRecordIF, &lv_style_pretty_color);
	knob_on_styleDevStatusRecordIF->body.radius = LV_RADIUS_CIRCLE;
	knob_on_styleDevStatusRecordIF->body.shadow.width = 4;
	knob_on_styleDevStatusRecordIF->body.shadow.type = LV_SHADOW_BOTTOM;

	lv_style_copy(styleMbox_bg, &lv_style_pretty);
	styleMbox_bg->body.main_color = LV_COLOR_MAKE(0xf5, 0x45, 0x2e);
	styleMbox_bg->body.grad_color = LV_COLOR_MAKE(0xb9, 0x1d, 0x09);
	styleMbox_bg->body.border.color = LV_COLOR_MAKE(0x3f, 0x0a, 0x03);
	styleMbox_bg->text.font = fontLabelGet_bySysLanguage(3);
	styleMbox_bg->text.color = LV_COLOR_WHITE;
	styleMbox_bg->body.padding.hor = 12;
	styleMbox_bg->body.padding.ver = 8;
	styleMbox_bg->body.shadow.width = 8;
	lv_style_copy(styleMbox_btn_rel, &lv_style_btn_rel);
	styleMbox_btn_rel->text.font = fontLabelGet_bySysLanguage(4);
	styleMbox_btn_rel->body.empty = 1;					 /*Draw only the border*/
	styleMbox_btn_rel->body.border.color = LV_COLOR_WHITE;
	styleMbox_btn_rel->body.border.width = 2;
	styleMbox_btn_rel->body.border.opa = LV_OPA_70;
	styleMbox_btn_rel->body.padding.hor = 12;
	styleMbox_btn_rel->body.padding.ver = 8;
	lv_style_copy(styleMbox_btn_pr, styleMbox_btn_rel);
	styleMbox_btn_pr->body.empty = 0;
	styleMbox_btn_pr->body.main_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);
	styleMbox_btn_pr->body.grad_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);

    lv_style_copy(styleBtn_specialTransparent, &lv_style_btn_rel);
	styleBtn_specialTransparent->body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent->body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent->body.border.part = LV_BORDER_NONE;
    styleBtn_specialTransparent->body.opa = LV_OPA_TRANSP;
	styleBtn_specialTransparent->body.radius = 0;
    styleBtn_specialTransparent->body.shadow.width =  0;

	lv_style_copy(styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun->image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun->image.color = LV_COLOR_MAKE(200, 191, 231);

	lv_style_copy(stylePageCb_elecParamDispSet, &lv_style_plain);
	stylePageCb_elecParamDispSet->body.border.part = LV_BORDER_NONE;
	stylePageCb_elecParamDispSet->body.empty = 1;
	stylePageCb_elecParamDispSet->text.color = LV_COLOR_AQUA;
//	stylePageCb_elecParamDispSet->text.font = &lv_font_consola_16;
	stylePageCb_elecParamDispSet->text.font = fontLabelGet_bySysLanguage(0);

	lv_style_copy(stylePageCb_tempDispMethodSet, &lv_style_plain);
	stylePageCb_tempDispMethodSet->body.border.part = LV_BORDER_NONE;
	stylePageCb_tempDispMethodSet->body.empty = 1;
	stylePageCb_tempDispMethodSet->text.color = LV_COLOR_AQUA;
//	stylePageCb_tempDispMethodSet->text.font = &lv_font_consola_16;
	stylePageCb_tempDispMethodSet->text.font = fontLabelGet_bySysLanguage(0);

	lv_style_copy(stylePageCb_screenLandscape, &lv_style_plain);
	stylePageCb_screenLandscape->body.border.part = LV_BORDER_NONE;
	stylePageCb_screenLandscape->body.empty = 1;
	stylePageCb_screenLandscape->text.color = LV_COLOR_AQUA;
//	stylePageCb_screenLandscape->text.font = &lv_font_consola_16;
	stylePageCb_screenLandscape->text.font = fontLabelGet_bySysLanguage(0);

	lv_style_copy(styleRoller_epidemicCyLocation_bg, &lv_style_plain);
	styleRoller_epidemicCyLocation_bg->body.main_color = LV_COLOR_GRAY;
	styleRoller_epidemicCyLocation_bg->body.grad_color = LV_COLOR_GRAY;
	styleRoller_epidemicCyLocation_bg->text.color = LV_COLOR_SILVER;
	styleRoller_epidemicCyLocation_bg->text.font = &lv_font_consola_13;
	styleRoller_epidemicCyLocation_bg->text.line_space = 5;
	styleRoller_epidemicCyLocation_bg->text.opa = LV_OPA_COVER;
	lv_style_copy(styleRoller_epidemicCyLocation_sel, &lv_style_plain);
	styleRoller_epidemicCyLocation_sel->body.empty = 1;
	styleRoller_epidemicCyLocation_sel->body.radius = 30;
	styleRoller_epidemicCyLocation_sel->text.color = LV_COLOR_YELLOW;
	styleRoller_epidemicCyLocation_sel->text.font = &lv_font_consola_13;

	lv_style_copy(stylePage_devTypeSet, &lv_style_plain_color);
	stylePage_devTypeSet->body.main_color = LV_COLOR_SILVER;
	stylePage_devTypeSet->body.grad_color = LV_COLOR_SILVER;
	stylePage_devTypeSet->body.border.part = LV_BORDER_NONE;
	stylePage_devTypeSet->body.radius = 6;
	stylePage_devTypeSet->body.opa = LV_OPA_90;
	stylePage_devTypeSet->body.padding.hor = 0;
	stylePage_devTypeSet->body.padding.inner = 0;

	lv_style_copy(stylePage_devAtmosLightColorSet, &lv_style_plain_color);
	stylePage_devAtmosLightColorSet->body.main_color = LV_COLOR_MAKE(0xB0, 0xB0, 0xB0);
	stylePage_devAtmosLightColorSet->body.grad_color = LV_COLOR_MAKE(0xB0, 0xB0, 0xB0);
	stylePage_devAtmosLightColorSet->body.border.part = LV_BORDER_FULL;
	stylePage_devAtmosLightColorSet->body.border.width = 2;
	stylePage_devAtmosLightColorSet->body.border.color = LV_COLOR_YELLOW;
	stylePage_devAtmosLightColorSet->body.radius = 3;
	stylePage_devAtmosLightColorSet->body.opa = LV_OPA_COVER;
	stylePage_devAtmosLightColorSet->body.padding.hor = 0;
	stylePage_devAtmosLightColorSet->body.padding.inner = 0;

	lv_style_copy(styleTextPageTitle_devTypeSet, &lv_style_plain);
//	styleTextPageTitle_devTypeSet->text.font = &lv_font_consola_19;
	styleTextPageTitle_devTypeSet->text.font = fontLabelGet_bySysLanguage(4);
	styleTextPageTitle_devTypeSet->text.color = LV_COLOR_BLACK;

	lv_style_copy(styleTextPageRef_devTypeExSet, &lv_style_plain);
	styleTextPageRef_devTypeExSet->text.font = &lv_font_consola_13;
	styleTextPageRef_devTypeExSet->text.color = LV_COLOR_MAKE(0x40, 0x40, 0x40);

	lv_style_copy(styleRoller_devTypeExSet_bg, &lv_style_plain);
	styleRoller_devTypeExSet_bg->body.main_color = LV_COLOR_SILVER;
	styleRoller_devTypeExSet_bg->body.grad_color = LV_COLOR_SILVER;
	styleRoller_devTypeExSet_bg->body.opa =  LV_OPA_90;
	styleRoller_devTypeExSet_bg->text.color = LV_COLOR_MAKE(70, 173, 185);
	styleRoller_devTypeExSet_bg->text.font = &lv_font_consola_19;
	styleRoller_devTypeExSet_bg->text.line_space = 5;
	lv_style_copy(styleRoller_devTypeExSet_sel, &lv_style_plain);
	styleRoller_devTypeExSet_sel->body.empty = 1;
	styleRoller_devTypeExSet_sel->body.radius = 30;
	styleRoller_devTypeExSet_sel->text.color = LV_COLOR_YELLOW;
	styleRoller_devTypeExSet_sel->text.font = &lv_font_consola_19;

	lv_style_copy(styleLabel_devTypeExSet_rollerIns, &lv_style_plain);
//	styleLabel_devTypeExSet_rollerIns->text.font = &lv_font_arial_15;
	styleLabel_devTypeExSet_rollerIns->text.font = fontLabelGet_bySysLanguage(0);
	styleLabel_devTypeExSet_rollerIns->text.color = LV_COLOR_MAKE(80, 240, 80);

	lv_style_copy(stylePageCb_devTypeSet, &lv_style_plain);
	stylePageCb_devTypeSet->body.border.part = LV_BORDER_NONE;
	stylePageCb_devTypeSet->body.empty = 1;
//	stylePageCb_devTypeSet->text.color = LV_COLOR_MAKE(192, 255, 0);
	stylePageCb_devTypeSet->text.color = LV_COLOR_BLACK;
	stylePageCb_devTypeSet->text.font = &lv_font_consola_17;

	lv_style_copy(styleLabelPageBtn_devTypeSet, &lv_style_plain);
	styleLabelPageBtn_devTypeSet->text.font = &lv_font_ariblk_18;
	styleLabelPageBtn_devTypeSet->text.color = LV_COLOR_MAKE(0, 128, 255);

	lv_style_copy(styleBtn_devAtmosLightColorSet, &lv_style_plain);
	styleBtn_devAtmosLightColorSet->body.main_color = LV_COLOR_WHITE;
	styleBtn_devAtmosLightColorSet->body.grad_color = LV_COLOR_WHITE;
	styleBtn_devAtmosLightColorSet->body.opa = LV_OPA_COVER;
	styleBtn_devAtmosLightColorSet->body.border.part = LV_BORDER_NONE;
	styleBtn_devAtmosLightColorSet->body.radius = 0;

	lv_style_copy(styleTextPageTitle_devAtmosLightColorSet, &lv_style_plain);
	styleTextPageTitle_devAtmosLightColorSet->body.border.part = LV_BORDER_NONE;
	styleTextPageTitle_devAtmosLightColorSet->body.empty = 1;
//	styleTextPageTitle_devAtmosLightColorSet->text.color = LV_COLOR_MAKE(192, 255, 0);
	styleTextPageTitle_devAtmosLightColorSet->text.color = LV_COLOR_BLACK;
	styleTextPageTitle_devAtmosLightColorSet->text.font = fontLabelGet_bySysLanguage(3);

	lv_style_copy(styleLabelPageBtn_devAtmosLightColorSet, &lv_style_plain);
	styleLabelPageBtn_devAtmosLightColorSet->text.font = fontLabelGet_bySysLanguage(4);
	styleLabelPageBtn_devAtmosLightColorSet->text.color = LV_COLOR_MAKE(0, 128, 255);

	lv_style_copy(styleLabelPageCb_devALSC_breathIf, &lv_style_plain);
	styleLabelPageCb_devALSC_breathIf->body.border.part = LV_BORDER_NONE;
	styleLabelPageCb_devALSC_breathIf->body.empty = 1;
	styleLabelPageCb_devALSC_breathIf->text.color = LV_COLOR_MAKE(128, 128, 255);
	styleLabelPageCb_devALSC_breathIf->text.font = fontLabelGet_bySysLanguage(0);

	lv_style_copy(styleLabelPageCb_devALSC_setAllSw, styleLabelPageCb_devALSC_breathIf);
	styleLabelPageCb_devALSC_setAllSw->text.color = LV_COLOR_MAKE(255, 128, 64);

	lv_style_copy(styleLabelPageCb_devALSC_autoSet, styleLabelPageCb_devALSC_breathIf);
	styleLabelPageCb_devALSC_autoSet->text.color = LV_COLOR_CYAN;
	styleLabelPageCb_devALSC_autoSet->text.font = fontLabelGet_bySysLanguage(2);
}

static void lvGui_settingSet_epidemicCtrlObj_opt(bool creatIf){

	if(esp_mesh_get_layer() != MESH_ROOT)return;

	if(creatIf){

		if(textSettingA_epidemicCountry == NULL){

			uint8_t cyIst = dispApplication_epidCyLocation_get();
			char strTemp[32] = {0};
			textSettingA_epidemicCountry = lv_label_create(page_funSetOption, NULL);
			lv_label_set_recolor(textSettingA_epidemicCountry, true);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:sprintf(textlabel_dispTemp, ":%s[#FFFF00 %s#]", labelStrGet_bySysLanguage(tls_mSetting_epidemicZone), epidCyAbbre[cyIst]);break;
				default:sprintf(textlabel_dispTemp, "%s[#FFFF00 %s#]:", labelStrGet_bySysLanguage(tls_mSetting_epidemicZone), epidCyAbbre[cyIst]);break;}
			lv_label_set_text(textSettingA_epidemicCountry, (const char *)textlabel_dispTemp);
			lv_obj_set_style(textSettingA_epidemicCountry, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_epidemicCountry, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_epidemicCountry, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_epidemicCountry, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			
			rollerEpidemic_cyLocation = lv_roller_create(page_funSetOption, NULL);
			lv_roller_set_action(rollerEpidemic_cyLocation, funCb_rollerEpidemicCyLocation);
			lv_roller_set_options(rollerEpidemic_cyLocation, epidCyName);
			lv_obj_set_protect(rollerEpidemic_cyLocation, LV_PROTECT_POS);
			lv_roller_set_hor_fit(rollerEpidemic_cyLocation, false);
			lv_obj_set_width(rollerEpidemic_cyLocation, 215);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(rollerEpidemic_cyLocation, textSettingA_epidemicCountry, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset, 5);break;
				default:lv_obj_align(rollerEpidemic_cyLocation, textSettingA_epidemicCountry, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset, 5);break;}
			lv_roller_set_selected(rollerEpidemic_cyLocation, cyIst, false);
			lv_roller_set_style(rollerEpidemic_cyLocation, LV_ROLLER_STYLE_BG, styleRoller_epidemicCyLocation_bg);
			lv_roller_set_style(rollerEpidemic_cyLocation, LV_ROLLER_STYLE_SEL, styleRoller_epidemicCyLocation_sel);
			lv_roller_set_visible_row_count(rollerEpidemic_cyLocation, 4);
		}
	}
	else
	{
		if(textSettingA_epidemicCountry != NULL){

			lv_obj_del(rollerEpidemic_cyLocation);
			lv_obj_del(textSettingA_epidemicCountry);

			rollerEpidemic_cyLocation = NULL;
			textSettingA_epidemicCountry = NULL;
		}
	}
}

void paramCfgTemp_ALCScolor_get(lv_color_t *c){

	memcpy(c, &atmosLightRunParam_dataTemp.lightColorCustomParamcfg, sizeof(lv_color_t));
}

void paramSettingGet_epidCyAbbre(char param[2]){

	memcpy(param, epidCyAbbre[dispApplication_epidCyLocation_get()], sizeof(char) * 2);
}

void paramSettingGet_epidCyName(char *nameStr){

	const uint8_t ctIstMax = 226;

	uint8_t targetIst = dispApplication_epidCyLocation_get() + 1;
	uint8_t loop = 0;
	char cyNameStr_temp[32] = {0};
	uint16_t cyTempCounter = 0;
	uint16_t tabCyNameCursor = 0;

	if(targetIst > ctIstMax)return;
	else
	if(targetIst == ctIstMax){
		
		strcpy(nameStr, "Zimbabwe");
	}
	else{
		
		for(loop = 0; loop < targetIst; loop ++){
	
			memset(cyNameStr_temp, 0, sizeof(char) * 32);
	
			do{
				
				cyTempCounter ++;
				tabCyNameCursor ++;
			
			}while(epidCyName[tabCyNameCursor] != '\n');
	
			memcpy(cyNameStr_temp, &epidCyName[tabCyNameCursor - cyTempCounter], cyTempCounter);
	
			cyTempCounter = 0;
			tabCyNameCursor ++;
		}
		
		strcpy(nameStr, cyNameStr_temp);	
	}
	
	printf("epidCyName get:<%s>\n", nameStr);
}

void lvGui_businessMenu_settingSet(lv_obj_t * obj_Parent){

	const uint16_t obj_animate_time = 150;
	const uint16_t obj_animate_delay = 100;
	uint16_t obj_animate_delayBasic = 0;
	uint16_t coordOffset_cLanguage_x = 0;

	uint8_t loop = 0;
	devTypeDef_enum devType_Temp = devTypeDef_null;
	enumDevLanguageIdentify systemLanguageType_Temp = languageCyId_null;
	uint8_t homepageThemeStyle_temp = 0;
	stt_devStatusRecord devStatusRecordFlg_temp = {0};
	stt_devSystemKeyParamRecord devSysKparam = {0};
	char strLabelTmp[32] = {0};

	lv_obj_t *lvObjAlign_temp = NULL;

	objParentTemp = obj_Parent;
		
	lvGuiSettingSet_objStyle_Init();

	if(NULL == msgQh_uiPageSetting_refresh){
		msgQh_uiPageSetting_refresh = xQueueCreate(1, sizeof(stt_dpAttrMsgQ_uiPageSettingRefresh));
	}

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);
	devSystemKeyAttr_paramGet(&devSysKparam);
	devAtmosLight_runningParam_get(&atmosLightRunParam_dataTemp);

	(devStatusDispMethod_landscapeIf_get())?
		(screenLandscapeCoordinate_objOffset = 60):
		(screenLandscapeCoordinate_objOffset = 0);

	text_Title = lv_label_create(obj_Parent, NULL);
//	lv_label_set_text(text_Title, "Device define");
	lv_label_set_text(text_Title, labelStrGet_bySysLanguage(tls_mSetting_deviceDefine));
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:coordOffset_cLanguage_x = -35;break;
		default:coordOffset_cLanguage_x = 0;break;}
	lv_obj_align(text_Title, NULL, LV_ALIGN_IN_BOTTOM_MID, coordOffset_cLanguage_x, -7);
	lv_obj_set_style(text_Title, styleText_menuLevel_A);

	menuBtnChoIcon_fun_home = lv_imgbtn_create(obj_Parent, NULL);
	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 220, 23)):
		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 140, 23));
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

//	menuBtnChoIcon_fun_home = lv_btn_create(obj_Parent, NULL);
//	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
//	(devStatusDispMethod_landscapeIf_get())?
//		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 240, 25)):
//		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 160, 25));
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_home, LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_home = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_home, &iconMenu_funBack_homePage);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_home, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_home, menuBtnChoIcon_fun_home, LV_ALIGN_IN_RIGHT_MID, -25, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);

//	menuBtnChoIcon_fun_back = lv_btn_create(obj_Parent, menuBtnChoIcon_fun_home);
//	lv_obj_set_pos(menuBtnChoIcon_fun_back, 0, 25);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_back, LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_back = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_back, &iconMenu_funBack_arrowLeft);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_back, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_back, menuBtnChoIcon_fun_back, LV_ALIGN_IN_LEFT_MID, 5, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_back, true);

//	lv_obj_set_click(menuBtnChoIcon_fun_home, false);
//	lv_obj_set_click(menuBtnChoIcon_fun_back, false);

	page_funSetOption = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(page_funSetOption, 320, 180)):
		(lv_obj_set_size(page_funSetOption, 240, 260));
	lv_obj_set_pos(page_funSetOption, 0, 75);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_SB, stylePage_funSetOption);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_BG, stylePage_funSetOption);
	lv_page_set_sb_mode(page_funSetOption, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(page_funSetOption, false, false); //key opration
	if(devStatusDispMethod_landscapeIf_get()){
		lv_page_set_scrl_width(page_funSetOption, 300); 
		lv_page_set_scrl_height(page_funSetOption, 1500);}
	else{
		lv_page_set_scrl_width(page_funSetOption, 220); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		lv_page_set_scrl_height(page_funSetOption, 1400);} //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);
//	lv_page_scroll_ver(page_funSetOption, 480);

	textSettingA_deviceType = lv_label_create(page_funSetOption, NULL);
//	lv_label_set_text(textSettingA_deviceType, "Device type:");
	lv_label_set_text(textSettingA_deviceType, labelStrGet_bySysLanguage(tls_mSetting_deviceType));
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_label_set_align(textSettingA_deviceType, LV_LABEL_ALIGN_LEFT);break;
		default:lv_label_set_align(textSettingA_deviceType, LV_LABEL_ALIGN_RIGHT);break;}
	lv_obj_set_style(textSettingA_deviceType, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_deviceType, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
		default:lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
	btnSettingA_deviceType = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_deviceType, 100, 35);
    lv_btn_set_style(btnSettingA_deviceType, LV_BTN_STATE_REL, &lv_style_transp);
	lv_btn_set_style(btnSettingA_deviceType, LV_BTN_STATE_PR, &lv_style_transp);
	lv_obj_set_protect(btnSettingA_deviceType, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, 0 - screenLandscapeCoordinate_objOffset - 55, 15);break;
		case languageCyId_Portuguese:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 25, 15);break;
		default:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 35, 15);break;}
	lv_btn_set_action(btnSettingA_deviceType, LV_BTN_ACTION_CLICK, funCb_btnActionClick_btnDevType_funBack);
	btnLabelSettingA_deviceType = lv_label_create(btnSettingA_deviceType, NULL);
	lv_obj_set_protect(btnLabelSettingA_deviceType, LV_PROTECT_POS);
	lv_label_set_align(btnLabelSettingA_deviceType, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(btnLabelSettingA_deviceType, LV_LABEL_LONG_ROLL);
	lv_obj_set_size(btnLabelSettingA_deviceType, 80, 30);
	lv_label_set_recolor(btnLabelSettingA_deviceType, true);
	lv_obj_set_style(btnLabelSettingA_deviceType, styleLabel_devTypeSelected);
	devType_Temp = currentDev_typeGet();
	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){
		if(devType_Temp == deviceTypeTab_disp[loop].devType){
			lv_label_set_text(btnLabelSettingA_deviceType, deviceTypeStr_disp[loop].devName);
			devTypeDdist_istRecord = loop;
			break;}}
	lvObjAlign_temp = textSettingA_deviceType;

	btnSettingA_typeDevExSet = lv_btn_create(page_funSetOption, NULL);
	lv_btn_set_style(btnSettingA_typeDevExSet, LV_BTN_STATE_REL, &lv_style_transp);
	lv_btn_set_style(btnSettingA_typeDevExSet, LV_BTN_STATE_PR, &lv_style_transp);
	lv_obj_set_protect(btnSettingA_typeDevExSet, LV_PROTECT_POS);
	btnLabelSettingA_typeDevExSet = lv_label_create(btnSettingA_typeDevExSet, NULL);
	lv_obj_set_protect(btnLabelSettingA_typeDevExSet, LV_PROTECT_POS);
	lv_label_set_align(btnLabelSettingA_typeDevExSet, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(btnLabelSettingA_typeDevExSet, LV_LABEL_LONG_ROLL);
	lv_label_set_recolor(btnLabelSettingA_typeDevExSet, true);
	lv_obj_set_style(btnLabelSettingA_typeDevExSet, styleLabel_devTypeSelected);
	lv_obj_set_hidden(btnSettingA_typeDevExSet, true);
	lvGui_devTypeExSetElementRefresh();

	textSettingA_systemLanguage = lv_label_create(page_funSetOption, NULL);
//	lv_label_set_text(textSettingA_systemLanguage, "System Language:");
	lv_label_set_text(textSettingA_systemLanguage, labelStrGet_bySysLanguage(tls_mSetting_systemLanguage));
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_label_set_align(textSettingA_systemLanguage, LV_LABEL_ALIGN_LEFT);break;
		default:lv_label_set_align(textSettingA_systemLanguage, LV_LABEL_ALIGN_RIGHT);break;}
	lv_obj_set_style(textSettingA_systemLanguage, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_systemLanguage, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_systemLanguage, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_systemLanguage, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	btnSettingA_systemLanguage = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_systemLanguage, 200, 35);
	lv_btn_set_style(btnSettingA_systemLanguage, LV_BTN_STATE_REL, &lv_style_transp);
	lv_btn_set_style(btnSettingA_systemLanguage, LV_BTN_STATE_PR, &lv_style_transp);
	lv_obj_set_protect(btnSettingA_systemLanguage, LV_PROTECT_POS);
//	(devStatusDispMethod_landscapeIf_get())?
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_systemLanguage, textSettingA_systemLanguage, LV_ALIGN_OUT_BOTTOM_MID, 0 - screenLandscapeCoordinate_objOffset - 62, 15);break;
		default:lv_obj_align(btnSettingA_systemLanguage, textSettingA_systemLanguage, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 21, 15);break;}
	lv_btn_set_action(btnSettingA_systemLanguage, LV_BTN_ACTION_CLICK, funCb_btnActionClick_btnSysLanguage_funBack);
	btnLabelSettingA_systemLanguage = lv_label_create(btnSettingA_systemLanguage, NULL);
	lv_label_set_align(btnLabelSettingA_systemLanguage, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(btnLabelSettingA_systemLanguage, LV_LABEL_LONG_ROLL);
	lv_obj_set_size(btnLabelSettingA_systemLanguage, 180, 30);
	lv_label_set_recolor(btnLabelSettingA_systemLanguage, true);
	lv_obj_set_style(btnLabelSettingA_systemLanguage, styleLabel_devTypeSelected);
	lv_obj_set_protect(btnLabelSettingA_systemLanguage, LV_PROTECT_POS);
	systemLanguageType_Temp = devLanguageCountry_identifyGet();
	for(loop = 0; loop < DEV_LANGUAGE_COUNTRY_NUM_MAX; loop ++){
		if(systemLanguageType_Temp == systemLanguageStr_disp[loop].devType){
			lv_label_set_text(btnLabelSettingA_systemLanguage, systemLanguageStr_disp[loop].devName);
			devTypeDdist_istRecord = loop;
			break;
		}
	}
	lvObjAlign_temp = textSettingA_systemLanguage;
	
//	ddlistSettingA_deviceType = lv_ddlist_create(page_funSetOption, NULL);
//	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_SB, styleDdlistSettingA_devType);
//	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_BG, styleDdlistSettingA_devType);
//	lv_obj_set_protect(ddlistSettingA_deviceType, LV_PROTECT_POS);
//	lv_obj_align(ddlistSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 50, 25);
//	lv_ddlist_set_options(ddlistSettingA_deviceType, deviceType_listTab);
//	lv_ddlist_set_action(ddlistSettingA_deviceType, funCb_ddlistDevTypeDef);
//	lv_obj_set_top(ddlistSettingA_deviceType, true);
//	lv_obj_set_free_num(ddlistSettingA_deviceType, OBJ_DDLIST_DEVTYPE_FREENUM);
//	devType_Temp = currentDev_typeGet();
//	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

//		if(devType_Temp == deviceTypeTab_disp[loop].devType){

//			lv_ddlist_set_selected(ddlistSettingA_deviceType, deviceTypeTab_disp[loop].devInst_disp);
//			devTypeDdist_istRecord = deviceTypeTab_disp[loop].devInst_disp;
//			break;
//		}
//	}

	textSettingA_devStatusRecordIF = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_devStatusRecordIF, "Stay the status:");
	lv_label_set_text(textSettingA_devStatusRecordIF, labelStrGet_bySysLanguage(tls_mSetting_stayTheStatus));
	lv_obj_set_style(textSettingA_devStatusRecordIF, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devStatusRecordIF, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_devStatusRecordIF, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_devStatusRecordIF, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	swSettingA_devStatusRecordIF = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devStatusRecordIF, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
		default:lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
	lv_sw_set_anim_time(swSettingA_devStatusRecordIF, 100);
	if(devStatusRecordFlg_temp.devStatusOnOffRecord_IF)lv_sw_on(swSettingA_devStatusRecordIF);
	else lv_sw_off(swSettingA_devStatusRecordIF);
	lv_sw_set_action(swSettingA_devStatusRecordIF, funCb_swDevStatusRecordIF);
	lvObjAlign_temp = textSettingA_devStatusRecordIF;

	textSettingA_bGroundPicSelect = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_bGroundPicSelect, "Background pic set:");
	lv_label_set_text(textSettingA_bGroundPicSelect, labelStrGet_bySysLanguage(tls_mSetting_bGroundPicSet));
	lv_obj_set_style(textSettingA_bGroundPicSelect, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_bGroundPicSelect, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_bGroundPicSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_bGroundPicSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	pageBgroundPic_select = lv_page_create(page_funSetOption, NULL);
	lv_obj_set_size(pageBgroundPic_select, 200, 65);
	lv_obj_set_protect(pageBgroundPic_select, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(pageBgroundPic_select, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 0);break;
		default:lv_obj_align(pageBgroundPic_select, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 0);break;}
	lv_page_set_scrl_fit(pageBgroundPic_select, true, false);
//	lv_page_set_scrl_width(pageBgroundPic_select, 720); 
	lv_page_set_scrl_height(pageBgroundPic_select, 45); 
	lv_page_set_sb_mode(pageBgroundPic_select, LV_SB_MODE_HIDE);
	lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_SB, stylePage_funSetOption);
	lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_BG, stylePage_funSetOption);
	for(loop = 0; loop < BROUND_PIC_SELECT_MAX_NUM; loop ++){
		btnBgroundPicSel_preview[loop] = lv_imgbtn_create(pageBgroundPic_select, NULL);
		lv_obj_set_size(btnBgroundPicSel_preview[loop], 40, 40);
		lv_imgbtn_set_toggle(btnBgroundPicSel_preview[loop], true);
//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_PR, usrAppHomepageBkPic_prevDataGet(loop));
//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_REL, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_action(btnBgroundPicSel_preview[loop], LV_BTN_ACTION_CLICK, funCb_btnActionClick_bGroundPicOrgSelcet);
		lv_obj_set_free_num(btnBgroundPicSel_preview[loop], loop);
		lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_PR, usrAppHomepageBkPicStyle_prevDataGet(loop));
		lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_REL, usrAppHomepageBkPicStyle_prevDataGet(loop));
		lv_obj_set_protect(btnBgroundPicSel_preview[loop], LV_PROTECT_POS);
		lv_obj_set_pos(btnBgroundPicSel_preview[loop], loop * 45 + 5, 3);
		lv_page_glue_obj(btnBgroundPicSel_preview[loop], true);
		(loop == usrAppHomepageBgroundPicOrg_Get())?
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR)):
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL));
	}
	imgBgroundPicSel_reserve = lv_img_create(btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], NULL);
	lv_img_set_src(imgBgroundPicSel_reserve, &bGroundPrev_picFigure_sel);
	lv_page_focus(pageBgroundPic_select, btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], 0);
	lvObjAlign_temp = textSettingA_bGroundPicSelect;

	textSettingA_devAtmosLightColorSet = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_devAtmosLightColorSet, "Decorative lamp set:");
	lv_label_set_text(textSettingA_devAtmosLightColorSet, labelStrGet_bySysLanguage(tls_mSetting_decoractiveSet));
	lv_obj_set_style(textSettingA_devAtmosLightColorSet, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devAtmosLightColorSet, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_devAtmosLightColorSet, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_devAtmosLightColorSet, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	btnSettingA_devAtmosLightColorSet = lv_btn_create(page_funSetOption, NULL);
	styleBtn_devAtmosLightColorSet->body.main_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
	styleBtn_devAtmosLightColorSet->body.grad_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
	lv_btn_set_style(btnSettingA_devAtmosLightColorSet, LV_BTN_STYLE_REL, styleBtn_devAtmosLightColorSet);
	lv_btn_set_style(btnSettingA_devAtmosLightColorSet, LV_BTN_STYLE_PR, styleBtn_devAtmosLightColorSet);
	lv_btn_set_action(btnSettingA_devAtmosLightColorSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devAtmosLightColorSet);
	lv_obj_set_size(btnSettingA_devAtmosLightColorSet, 80, 30);
	lv_obj_set_protect(btnSettingA_devAtmosLightColorSet, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_LEFT, 0 - screenLandscapeCoordinate_objOffset + 25, 15);break;
		case languageCyId_Portuguese:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_RIGHT, screenLandscapeCoordinate_objOffset - 80, 15);break;
		default:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_RIGHT, screenLandscapeCoordinate_objOffset - 15, 15);break;}
	lvObjAlign_temp = textSettingA_devAtmosLightColorSet;

	switch(currentDev_typeGet()){
		case devTypeDef_largeCurOneBit:
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			textSettingA_homepageThemestyle = lv_label_create(page_funSetOption, NULL);
//			lv_label_set_text(textSettingA_homepageThemestyle, "Theme:");
			lv_label_set_text(textSettingA_homepageThemestyle, labelStrGet_bySysLanguage(tls_mSetting_theme));
			lv_obj_set_style(textSettingA_homepageThemestyle, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_homepageThemestyle, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_homepageThemestyle, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_homepageThemestyle, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			ddlistSettingA_homepageThemestyle = lv_ddlist_create(page_funSetOption, NULL);
			lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_SB, styleDdlistSettingA_devType);
			lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_BG, styleDdlistSettingA_devType);
			lv_obj_set_protect(ddlistSettingA_homepageThemestyle, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 50, 20);break;
				default:lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
			lv_ddlist_set_options(ddlistSettingA_homepageThemestyle, homepageThemeStyle_listTab);
			lv_ddlist_set_action(ddlistSettingA_homepageThemestyle, funCb_ddlistHomepageThemeDef);
			lv_obj_set_top(ddlistSettingA_homepageThemestyle, true);
			lv_obj_set_free_num(ddlistSettingA_homepageThemestyle, OBJ_DDLIST_HPTHEME_FREENUM);
			homepageThemeStyle_temp = usrAppHomepageThemeType_Get();
			lv_ddlist_set_selected(ddlistSettingA_homepageThemestyle, homepageThemeStyle_temp);
			lvObjAlign_temp = textSettingA_homepageThemestyle;
		}break;

		default:{}break;
	}
	
	textSettingA_factoryRecoveryIf = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_factoryRecoveryIf, "Factory reset:");
	lv_label_set_text(textSettingA_factoryRecoveryIf, labelStrGet_bySysLanguage(tls_mSetting_factoryReset));
	lv_obj_set_style(textSettingA_factoryRecoveryIf, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	switch(currentDev_typeGet()){
//		case devTypeDef_largeCurOneBit:
//		case devTypeDef_mulitSwOneBit:
//		case devTypeDef_mulitSwTwoBit:
//		case devTypeDef_mulitSwThreeBit:
//		case devTypeDef_thermostatExtension:{
//			lv_obj_align(textSettingA_factoryRecoveryIf, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
//		}break;
		default:{
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_factoryRecoveryIf, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_factoryRecoveryIf, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
		}break;}
	btnSettingA_factoryRecoveryIf = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_factoryRecoveryIf, 60, 20);
	lv_obj_set_protect(btnSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_factoryRecoveryIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_factoryRecovery);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
		default:lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnRef_factoryRecoveryIf = lv_label_create(btnSettingA_factoryRecoveryIf, NULL);
	lv_label_set_recolor(textBtnRef_factoryRecoveryIf, true);
	lv_label_set_text(textBtnRef_factoryRecoveryIf, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_factoryRecoveryIf, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_factoryRecoveryIf, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_factoryRecoveryIf, NULL, LV_ALIGN_CENTER, 0, 0);
	lvObjAlign_temp = textSettingA_factoryRecoveryIf;

	textSettingA_restartAllDevices = lv_label_create(page_funSetOption, textSettingA_deviceType);
	lv_label_set_text(textSettingA_restartAllDevices, "Reboot system:");
	lv_obj_set_style(textSettingA_restartAllDevices, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_restartAllDevices, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_restartAllDevices, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_restartAllDevices, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	btnSettingA_rebootAllDevices = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_rebootAllDevices, 60, 20);
	lv_obj_set_protect(btnSettingA_rebootAllDevices, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_rebootAllDevices, LV_BTN_ACTION_CLICK, funCb_btnActionClick_rebootAllDevices);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
		default:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
    lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnRef_rebootAllDevices = lv_label_create(btnSettingA_rebootAllDevices, NULL);
	lv_label_set_recolor(textBtnRef_rebootAllDevices, true);
	lv_label_set_text(textBtnRef_rebootAllDevices, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_rebootAllDevices, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_rebootAllDevices, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_rebootAllDevices, NULL, LV_ALIGN_CENTER, 0, 0);
	lvObjAlign_temp = textSettingA_restartAllDevices;

//	textSettingA_devForceAsMeshChild = lv_label_create(page_funSetOption, NULL);
//	lv_label_set_text(textSettingA_devForceAsMeshChild, "tend to sub-device(one-time action):");
//	lv_obj_set_style(textSettingA_devForceAsMeshChild, styleText_menuLevel_B);
//	lv_obj_set_protect(textSettingA_devForceAsMeshChild, LV_PROTECT_POS);
//	lv_obj_align(textSettingA_devForceAsMeshChild, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
//	swSettingA_devForceAsMeshChild = lv_sw_create(page_funSetOption, NULL);
//	lv_sw_set_style(swSettingA_devForceAsMeshChild, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
//	lv_sw_set_style(swSettingA_devForceAsMeshChild, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
//	lv_sw_set_style(swSettingA_devForceAsMeshChild, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
//	lv_sw_set_style(swSettingA_devForceAsMeshChild, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
//	lv_obj_set_protect(swSettingA_devForceAsMeshChild, LV_PROTECT_POS);
//	lv_obj_align(swSettingA_devForceAsMeshChild, textSettingA_devForceAsMeshChild, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);
//	lv_sw_set_anim_time(swSettingA_devForceAsMeshChild, 100);
//	if(devSysKparam.sysParam_meshRoleForceAsChild)lv_sw_on(swSettingA_devForceAsMeshChild);
//	else lv_sw_off(swSettingA_devForceAsMeshChild);
//	lv_sw_set_action(swSettingA_devForceAsMeshChild, funCb_swDevForceAsMeshChild);
//	lvObjAlign_temp = textSettingA_devForceAsMeshChild;

#if(0 != ITEM_DEVICE_LIST_MANAGE_EN)
	textSettingA_devListManagerEN = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_devListManagerEN, "Device manager:");
	lv_label_set_text(textSettingA_devListManagerEN, labelStrGet_bySysLanguage(tls_mSetting_devieManage));
	lv_obj_set_style(textSettingA_devListManagerEN, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devListManagerEN, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_devListManagerEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_devListManagerEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	swSettingA_devListManagerEN = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devListManagerEN, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(swSettingA_devListManagerEN, textSettingA_devListManagerEN, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
		default:lv_obj_align(swSettingA_devListManagerEN, textSettingA_devListManagerEN, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
	lv_sw_set_anim_time(swSettingA_devListManagerEN, 100);
	if(devStatusRecordFlg_temp.devListManager_En)lv_sw_on(swSettingA_devListManagerEN);
	else lv_sw_off(swSettingA_devListManagerEN);
	lv_sw_set_action(swSettingA_devListManagerEN, funCb_swDevListManagerEN);
	lvObjAlign_temp = textSettingA_devListManagerEN;
#endif

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER) //特殊设备不支持HA
	textSettingA_devHomeAssistantEN = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_devHomeAssistantEN, "Homeassistant support:");
	lv_label_set_text(textSettingA_devHomeAssistantEN, labelStrGet_bySysLanguage(tls_mSetting_homassistantSupport));
	lv_obj_set_style(textSettingA_devHomeAssistantEN, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devHomeAssistantEN, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_devHomeAssistantEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_devHomeAssistantEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	swSettingA_devHomeAssistantEN = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devHomeAssistantEN, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(swSettingA_devHomeAssistantEN, textSettingA_devHomeAssistantEN, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
		default:lv_obj_align(swSettingA_devHomeAssistantEN, textSettingA_devHomeAssistantEN, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
	lv_sw_set_anim_time(swSettingA_devHomeAssistantEN, 100);
	if(devStatusRecordFlg_temp.homeassitant_En)lv_sw_on(swSettingA_devHomeAssistantEN);
	else lv_sw_off(swSettingA_devHomeAssistantEN);
	lv_sw_set_action(swSettingA_devHomeAssistantEN, funCb_homeassistantEnable);
	lvObjAlign_temp = textSettingA_devHomeAssistantEN;
#endif

//	textSettingA_touchRecalibrationIf = lv_label_create(page_funSetOption, NULL);
//	lv_label_set_text(textSettingA_touchRecalibrationIf, "touch recalibration:");
//	lv_obj_set_style(textSettingA_touchRecalibrationIf, styleText_menuLevel_B);
//	lv_obj_set_protect(textSettingA_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_obj_align(textSettingA_touchRecalibrationIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
//	btnSettingA_touchRecalibrationIf = lv_btn_create(page_funSetOption, NULL);
//	lv_obj_set_size(btnSettingA_touchRecalibrationIf, 60, 20);
//	lv_obj_set_protect(btnSettingA_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_btn_set_action(btnSettingA_touchRecalibrationIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_touchRecalibration);
//	lv_obj_align(btnSettingA_touchRecalibrationIf, textSettingA_touchRecalibrationIf, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
//	textBtnRef_touchRecalibrationIf = lv_label_create(btnSettingA_touchRecalibrationIf, NULL);
//	lv_label_set_recolor(textBtnRef_touchRecalibrationIf, true);
//	lv_label_set_text(textBtnRef_touchRecalibrationIf, "#00FF40 >>>o<<<#");
//	lv_obj_set_style(textBtnRef_touchRecalibrationIf, styleText_menuLevel_B);
//	lv_obj_set_protect(textBtnRef_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_obj_align(textBtnRef_touchRecalibrationIf, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_screenBrightness = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_screenBrightness, "Screen brightness:");
	lv_label_set_text(textSettingA_screenBrightness, labelStrGet_bySysLanguage(tls_mSetting_screenBrightness));
	lv_obj_set_style(textSettingA_screenBrightness, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenBrightness, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_screenBrightness, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_screenBrightness, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	sliderSettingA_screenBrightness = lv_slider_create(page_funSetOption, NULL);
	lv_obj_set_size(sliderSettingA_screenBrightness, 180, 15);
	lv_obj_set_protect(sliderSettingA_screenBrightness, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 10);break;
		default:lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 10);break;}
	lv_obj_set_free_num(sliderSettingA_screenBrightness, 0);
	lv_slider_set_action(sliderSettingA_screenBrightness, funCb_slidAction_functionSet_screenBrightnessAdj);
	lv_slider_set_range(sliderSettingA_screenBrightness, 0, screenBrightness_sliderAdj_div);
	uint8_t brightnessSlider_valDisp = devScreenDriver_configParam_brightness_get();
	lv_slider_set_value(sliderSettingA_screenBrightness, brightnessSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
	sliderSettingA_screenBrightnessSleep = lv_slider_create(page_funSetOption, sliderSettingA_screenBrightness);
	lv_slider_set_action(sliderSettingA_screenBrightnessSleep, funCb_slidAction_functionSet_screenBrightnessAdj);
	lv_obj_set_free_num(sliderSettingA_screenBrightnessSleep, 1);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(sliderSettingA_screenBrightnessSleep, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 40);break;
		default:lv_obj_align(sliderSettingA_screenBrightnessSleep, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 40);break;}
	uint8_t brightnessSleepSlider_valDisp = devScreenDriver_configParam_brightnessSleep_get();
	lv_slider_set_value(sliderSettingA_screenBrightnessSleep, brightnessSleepSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
	lvObjAlign_temp = textSettingA_screenBrightness;

	textSettingA_tempParamDispSelect = lv_label_create(page_funSetOption, textSettingA_deviceType);
	lv_label_set_text(textSettingA_tempParamDispSelect, labelStrGet_bySysLanguage(tls_mSetting_tempParamDisp));
	lv_obj_set_style(textSettingA_tempParamDispSelect, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_tempParamDispSelect, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_tempParamDispSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_tempParamDispSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	cbDispSelect_tempMethod_celsius = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_tempMethod_celsius, 160, 15);
	lv_cb_set_text(cbDispSelect_tempMethod_celsius, labelStrGet_bySysLanguage(tls_mSetting_tempDispCels));
	lv_obj_set_protect(cbDispSelect_tempMethod_celsius, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_tempMethod_celsius, LV_PROTECT_FOLLOW);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:
			coordOffset_cLanguage_x = -20;
			screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
			break;
		default:
			coordOffset_cLanguage_x = 0;
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			break;}
	lv_obj_align(cbDispSelect_tempMethod_celsius, textSettingA_tempParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
	(devStatusRecordFlg_temp.devTempDispMethod)?
		(lv_cb_set_checked(cbDispSelect_tempMethod_celsius, false)):
		(lv_cb_set_checked(cbDispSelect_tempMethod_celsius, true));
	cbDispSelect_tempMethod_fahrenheit = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_tempMethod_fahrenheit, 160, 15);
	lv_cb_set_text(cbDispSelect_tempMethod_fahrenheit, labelStrGet_bySysLanguage(tls_mSetting_tempDispFahr));
	lv_obj_set_protect(cbDispSelect_tempMethod_fahrenheit, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_tempMethod_fahrenheit, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_tempMethod_fahrenheit, textSettingA_tempParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
	lv_cb_set_style(cbDispSelect_tempMethod_celsius, LV_CB_STYLE_BG, stylePageCb_tempDispMethodSet);
	lv_cb_set_style(cbDispSelect_tempMethod_fahrenheit, LV_CB_STYLE_BG, stylePageCb_tempDispMethodSet);
	lv_cb_set_action(cbDispSelect_tempMethod_celsius, funCb_cbOpreat_tempParamDispSelect_celsius);
	lv_cb_set_action(cbDispSelect_tempMethod_fahrenheit, funCb_cbOpreat_tempParamDispSelect_fahrenheit);
	(devStatusRecordFlg_temp.devTempDispMethod)?
		(lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, true)):
		(lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, false));
	lvObjAlign_temp = textSettingA_tempParamDispSelect;

	textSettingA_tempCalibration = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_tempCalibration, "Temperature calibration:");
	lv_label_set_text(textSettingA_tempCalibration, labelStrGet_bySysLanguage(tls_mSetting_temperatureCalibretion));
	lv_obj_set_style(textSettingA_tempCalibration, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_tempCalibration, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_tempCalibration, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_tempCalibration, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	textSliderRef_tempratureCal = lv_label_create(page_funSetOption, NULL);
	(devTempDispMethod_unit_get())?
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())) - 32.0F)):
		(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())));
	lv_label_set_recolor(textSliderRef_tempratureCal, true);
	lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);
	lv_obj_set_style(textSliderRef_tempratureCal, styleText_menuLevel_B);
	lv_obj_set_protect(textSliderRef_tempratureCal, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
			case languageCyId_Arabic:lv_obj_align(textSliderRef_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 85, 7);break;
			default:lv_obj_align(textSliderRef_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 85, 7);break;}
	sliderSettingA_tempratureCal = lv_slider_create(page_funSetOption, NULL);
	lv_obj_set_size(sliderSettingA_tempratureCal, 150, 15);
	lv_obj_set_protect(sliderSettingA_tempratureCal, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(sliderSettingA_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 12, 30);break;
		default:lv_obj_align(sliderSettingA_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 12, 30);break;}
	lv_slider_set_action(sliderSettingA_tempratureCal, funCb_slidAction_functionSet_tempratureCal);
	lv_slider_set_range(sliderSettingA_tempratureCal, 0, DEV_TEMPRATURE_DATA_CAL_MAX_RANGE);
	lv_slider_set_value(sliderSettingA_tempratureCal, devTempratureSensor_dataCal_get());
	btnValAdd_tempratureCal = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnValAdd_tempratureCal, 40, 40);
	lv_obj_set_protect(btnValAdd_tempratureCal, LV_PROTECT_POS);
	lv_btn_set_action(btnValAdd_tempratureCal, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_tempratureCalAdd);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnValAdd_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, -18);break;
		default:lv_obj_align(btnValAdd_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 155, -18);break;}
    lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnValAdd_tempratureCal = lv_label_create(btnValAdd_tempratureCal, NULL);
	lv_label_set_recolor(textBtnValAdd_tempratureCal, true);
	lv_label_set_text(textBtnValAdd_tempratureCal, "#00FF40 + #");
	lv_obj_set_style(textBtnValAdd_tempratureCal, styleText_menuLevel_A);
	lv_obj_set_protect(textBtnValAdd_tempratureCal, LV_PROTECT_POS);
	lv_obj_align(textBtnValAdd_tempratureCal, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 2);
	btnValCut_tempratureCal = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnValCut_tempratureCal, 40, 40);
	lv_obj_set_protect(btnValCut_tempratureCal, LV_PROTECT_POS);
	lv_btn_set_action(btnValCut_tempratureCal, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_tempratureCalCut);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnValCut_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 155, -18);break;
		default:lv_obj_align(btnValCut_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, -18);break;}
    lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnValCut_tempratureCal = lv_label_create(btnValCut_tempratureCal, NULL);
	lv_label_set_recolor(textBtnValCut_tempratureCal, true);
	lv_label_set_text(textBtnValCut_tempratureCal, "#00FF40 _ #");
	lv_obj_set_style(textBtnValCut_tempratureCal, styleText_menuLevel_A);
	lv_obj_set_protect(textBtnValCut_tempratureCal, LV_PROTECT_POS);
	lv_obj_align(textBtnValCut_tempratureCal, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -6);
	lvObjAlign_temp = textSettingA_tempCalibration;

	textSettingA_screenLightTime = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_screenLightTime, "Screen light time:");
	lv_label_set_text(textSettingA_screenLightTime, labelStrGet_bySysLanguage(tls_mSetting_screenLightTime));
	lv_obj_set_style(textSettingA_screenLightTime, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLightTime, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_screenLightTime, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_screenLightTime, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	textBtnTimeRef_screenLightTime = lv_label_create(page_funSetOption, NULL);
	lv_label_set_recolor(textBtnTimeRef_screenLightTime, true);
	uint32_t screenLightTime_temp = devScreenDriver_configParam_screenLightTime_get();
	for(loop = 0; loop < FUNCTION_NUM_DEF_SCREENLIGHT_TIME; loop ++){
		if(screenLightTime_temp == screenLightTimeGear_refTab[loop].screenLightTime2Sec){
			functionGearScreenTime_ref = loop;
			break;
		}
	}
	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_label_set_align(textBtnTimeRef_screenLightTime, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style(textBtnTimeRef_screenLightTime, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeRef_screenLightTime, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:
			screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
			coordOffset_cLanguage_x = -35;
			break;
		case languageCyId_Portuguese:
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			coordOffset_cLanguage_x = 27;
			break;
		default:
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			coordOffset_cLanguage_x = 0;
			break;}
	lv_obj_align(textBtnTimeRef_screenLightTime, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_MID, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 25);
	btnTimeAdd_screenLightTime = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTimeAdd_screenLightTime, 90, 60);
	lv_obj_set_protect(btnTimeAdd_screenLightTime, LV_PROTECT_POS);
	lv_btn_set_action(btnTimeAdd_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeAdd);
	lv_obj_align(btnTimeAdd_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_RIGHT_MID, -10, -2);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnTimeAdd_screenLightTime = lv_label_create(btnTimeAdd_screenLightTime, NULL);
	lv_label_set_recolor(textBtnTimeAdd_screenLightTime, true);
	lv_label_set_text(textBtnTimeAdd_screenLightTime, "#00FF40 >>>>#");
	lv_label_set_align(textBtnTimeAdd_screenLightTime, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style(textBtnTimeAdd_screenLightTime, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeAdd_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textBtnTimeAdd_screenLightTime, NULL, LV_ALIGN_CENTER, 0, 2);
	btnTimeCut_screenLightTime = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTimeCut_screenLightTime, 90, 60);
	lv_obj_set_protect(btnTimeCut_screenLightTime, LV_PROTECT_POS);
	lv_btn_set_action(btnTimeCut_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeCut);
	lv_obj_align(btnTimeCut_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_LEFT_MID, 10, -2);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnTimeCut_screenLightTime = lv_label_create(btnTimeCut_screenLightTime, NULL);
	lv_label_set_recolor(textBtnTimeCut_screenLightTime, true);
	lv_label_set_text(textBtnTimeCut_screenLightTime, "#00FF40 <<<<#");
	lv_label_set_align(textBtnTimeCut_screenLightTime, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style(textBtnTimeCut_screenLightTime, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeCut_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textBtnTimeCut_screenLightTime, NULL, LV_ALIGN_CENTER, 0, 2);

#if(0 == LVAPP_DISP_ELECPARAM_HIDDEN_EN) //热水器及恒温器电量功能暂隐
	textSettingA_elecParamDispSelect = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_elecParamDispSelect, "Electric parameter display:");
	lv_label_set_text(textSettingA_elecParamDispSelect, labelStrGet_bySysLanguage(tls_mSetting_elecParamDisp));
	lv_obj_set_style(textSettingA_elecParamDispSelect, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_elecParamDispSelect, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_elecParamDispSelect, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_elecParamDispSelect, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	cbDispSelect_elecParam_power = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_elecParam_power, 160, 15);
//	lv_cb_set_text(cbDispSelect_elecParam_power, " Power");
	lv_cb_set_text(cbDispSelect_elecParam_power, labelStrGet_bySysLanguage(tls_mSetting_elecDispPower));
	lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_FOLLOW);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:
			coordOffset_cLanguage_x = -20;
			screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
			break;
		default:
			coordOffset_cLanguage_x = 0;
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			break;}
	lv_obj_align(cbDispSelect_elecParam_power, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
	(devStatusRecordFlg_temp.devElecsumDisp_IF)?
		(lv_cb_set_checked(cbDispSelect_elecParam_power, false)):
		(lv_cb_set_checked(cbDispSelect_elecParam_power, true));
	cbDispSelect_elecParam_elecsum = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_elecParam_elecsum, 160, 15);
//	lv_cb_set_text(cbDispSelect_elecParam_elecsum, " Electric sum");
	lv_cb_set_text(cbDispSelect_elecParam_elecsum, labelStrGet_bySysLanguage(tls_mSetting_elecDispSum));
	lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_elecParam_elecsum, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
	lv_cb_set_style(cbDispSelect_elecParam_power, LV_CB_STYLE_BG, stylePageCb_elecParamDispSet);
	lv_cb_set_style(cbDispSelect_elecParam_elecsum, LV_CB_STYLE_BG, stylePageCb_elecParamDispSet);
	lv_cb_set_action(cbDispSelect_elecParam_power, funCb_cbOpreat_elecParamDispSelect_power);
	lv_cb_set_action(cbDispSelect_elecParam_elecsum, funCb_cbOpreat_elecParamDispSelect_elecsum);
	(devStatusRecordFlg_temp.devElecsumDisp_IF)?
		(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, true)):
		(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, false));

	textSettingA_elecsumResetClear = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_elecsumResetClear, "Electric sum reset:");
	lv_label_set_text(textSettingA_elecsumResetClear, labelStrGet_bySysLanguage(tls_mSetting_elecSumReset));
	lv_obj_set_style(textSettingA_elecsumResetClear, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_elecsumResetClear, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_elecsumResetClear, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_elecsumResetClear, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	btnSettingA_elecsumReset = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_elecsumReset, 60, 20);
	lv_obj_set_protect(btnSettingA_elecsumReset, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_elecsumReset, LV_BTN_ACTION_CLICK, funCb_btnActionClick_elecsumResetClear);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(btnSettingA_elecsumReset, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
		default:lv_obj_align(btnSettingA_elecsumReset, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
	textBtnRef_elecsumReset = lv_label_create(btnSettingA_elecsumReset, NULL);
	lv_label_set_recolor(textBtnRef_elecsumReset, true);
	lv_label_set_text(textBtnRef_elecsumReset, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_elecsumReset, styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_elecsumReset, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_elecsumReset, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_screenLandscape = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_screenLandscape, "Screen display angle:");
	lv_label_set_text(textSettingA_screenLandscape, labelStrGet_bySysLanguage(tls_mSetting_screenDispAngle));
	lv_obj_set_style(textSettingA_screenLandscape, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLandscape, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_screenLandscape, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_screenLandscape, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	cbDispSelect_screenVer = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenVer, 160, 15);
//	lv_cb_set_text(cbDispSelect_screenVer, " Vertical");
	lv_cb_set_text(cbDispSelect_screenVer, labelStrGet_bySysLanguage(tls_mSetting_vertical));
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_FOLLOW);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:
			coordOffset_cLanguage_x = -20;
			screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
			break;
		default:coordOffset_cLanguage_x = 0;
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			break;}
	lv_obj_align(cbDispSelect_screenVer, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenVer, false)):
		(lv_cb_set_checked(cbDispSelect_screenVer, true));
	cbDispSelect_screenHor = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenHor, 160, 15);
//	lv_cb_set_text(cbDispSelect_screenHor, " Horizontal");
	lv_cb_set_text(cbDispSelect_screenHor, labelStrGet_bySysLanguage(tls_mSetting_horizontal));
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_screenHor, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenHor, true)):
		(lv_cb_set_checked(cbDispSelect_screenHor, false));
	lv_cb_set_style(cbDispSelect_screenVer, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
	lv_cb_set_style(cbDispSelect_screenHor, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
	lv_cb_set_action(cbDispSelect_screenVer, funCb_cbOpreat_scrrenDispMethod_ver);
	lv_cb_set_action(cbDispSelect_screenHor, funCb_cbOpreat_scrrenDispMethod_hor);
	opreatTemp_screenLandscapeIf = devStatusRecordFlg_temp.devScreenLandscape_IF;
#else

	textSettingA_screenLandscape = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_screenLandscape, "Screen display angle:");
	lv_label_set_text(textSettingA_screenLandscape, labelStrGet_bySysLanguage(tls_mSetting_screenDispAngle));
	lv_obj_set_style(textSettingA_screenLandscape, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLandscape, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_screenLandscape, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_screenLandscape, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	cbDispSelect_screenVer = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenVer, 160, 15);
//	lv_cb_set_text(cbDispSelect_screenVer, " Vertical");
	lv_cb_set_text(cbDispSelect_screenVer, labelStrGet_bySysLanguage(tls_mSetting_vertical));
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:
			coordOffset_cLanguage_x = -20;
			screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
			break;
		default:coordOffset_cLanguage_x = 0;
			screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
			break;}
	lv_obj_align(cbDispSelect_screenVer, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenVer, false)):
		(lv_cb_set_checked(cbDispSelect_screenVer, true));
	cbDispSelect_screenHor = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenHor, 160, 15);
//	lv_cb_set_text(cbDispSelect_screenHor, " Horizontal");
	lv_cb_set_text(cbDispSelect_screenHor, labelStrGet_bySysLanguage(tls_mSetting_horizontal));
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_POS);
	lv_obj_align(cbDispSelect_screenHor, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenHor, true)):
		(lv_cb_set_checked(cbDispSelect_screenHor, false));
	lv_cb_set_style(cbDispSelect_screenVer, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
	lv_cb_set_style(cbDispSelect_screenHor, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
	lv_cb_set_action(cbDispSelect_screenVer, funCb_cbOpreat_scrrenDispMethod_ver);
	lv_cb_set_action(cbDispSelect_screenHor, funCb_cbOpreat_scrrenDispMethod_hor);
	opreatTemp_screenLandscapeIf = devStatusRecordFlg_temp.devScreenLandscape_IF;
#endif

	if(upgradeAvailable_flg){

		textSettingA_fwareUpgrade = lv_label_create(page_funSetOption, textSettingA_deviceType);
//		lv_label_set_text(textSettingA_fwareUpgrade, "Firmware upgrade:");
		lv_label_set_text(textSettingA_fwareUpgrade, labelStrGet_bySysLanguage(tls_mSetting_upgrade));
		lv_obj_set_style(textSettingA_fwareUpgrade, styleText_menuLevel_B);
		lv_obj_set_protect(textSettingA_fwareUpgrade, LV_PROTECT_POS);
		switch(devLanguageCountry_identifyGet()){
			case languageCyId_Arabic:lv_obj_align(textSettingA_fwareUpgrade, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
			default:lv_obj_align(textSettingA_fwareUpgrade, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
		btnSettingA_fwareUpgrade = lv_btn_create(page_funSetOption, NULL);
		lv_obj_set_size(btnSettingA_fwareUpgrade, 60, 20);
		lv_obj_set_protect(btnSettingA_fwareUpgrade, LV_PROTECT_POS);
		lv_btn_set_action(btnSettingA_fwareUpgrade, LV_BTN_ACTION_CLICK, funCb_btnActionClick_fwareUpgrade);
		switch(devLanguageCountry_identifyGet()){
			case languageCyId_Arabic:lv_obj_align(btnSettingA_fwareUpgrade, textSettingA_fwareUpgrade, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
			default:lv_obj_align(btnSettingA_fwareUpgrade, textSettingA_fwareUpgrade, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
		textBtnRef_fwareUpgrade = lv_label_create(btnSettingA_fwareUpgrade, NULL);
		lv_label_set_recolor(textBtnRef_fwareUpgrade, true);
		lv_label_set_text(textBtnRef_fwareUpgrade, "#00FF40 >>>o<<<#");
		lv_obj_set_style(textBtnRef_fwareUpgrade, styleText_menuLevel_B);
		lv_obj_set_protect(textBtnRef_fwareUpgrade, LV_PROTECT_POS);
		lv_obj_align(textBtnRef_fwareUpgrade, NULL, LV_ALIGN_CENTER, 0, 0);
	}

#if(SCREENSAVER_RUNNING_ENABLE == 1)

	textSettingA_epidemicReport = lv_label_create(page_funSetOption, textSettingA_deviceType);
//	lv_label_set_text(textSettingA_epidemicReport, "COVID19 report:");
	lv_label_set_text(textSettingA_epidemicReport, labelStrGet_bySysLanguage(tls_mSetting_epidemicReport));
	lv_obj_set_style(textSettingA_epidemicReport, styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_epidemicReport, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(textSettingA_epidemicReport, (upgradeAvailable_flg)?textSettingA_fwareUpgrade:textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
		default:lv_obj_align(textSettingA_epidemicReport, (upgradeAvailable_flg)?textSettingA_fwareUpgrade:textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
	swSettingA_epidemicReportIF = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_epidemicReportIF, LV_PROTECT_POS);
	switch(devLanguageCountry_identifyGet()){
		case languageCyId_Arabic:lv_obj_align(swSettingA_epidemicReportIF, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
		default:lv_obj_align(swSettingA_epidemicReportIF, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
	lv_sw_set_anim_time(swSettingA_epidemicReportIF, 100);
	if(devStatusRecordFlg_temp.screensaverRunning_IF)lv_sw_on(swSettingA_epidemicReportIF);
	else lv_sw_off(swSettingA_epidemicReportIF);
	lv_sw_set_action(swSettingA_epidemicReportIF, funCb_epidemicReportIF);
	
	lvGui_settingSet_epidemicCtrlObj_opt(devStatusRecordFlg_temp.screensaverRunning_IF);
#endif

	lv_page_glue_obj(textSettingA_deviceType, true);
	lv_page_glue_obj(btnSettingA_deviceType, true);
	lv_page_glue_obj(textSettingA_systemLanguage, true);
	lv_page_glue_obj(btnSettingA_systemLanguage, true);
//	lv_page_glue_obj(ddlistSettingA_deviceType, true);
	lv_page_glue_obj(textSettingA_devAtmosLightColorSet, true);
	lv_page_glue_obj(btnSettingA_devAtmosLightColorSet, true);
	switch(currentDev_typeGet()){
		case devTypeDef_largeCurOneBit:
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			lv_page_glue_obj(textSettingA_homepageThemestyle, true);
			lv_page_glue_obj(ddlistSettingA_homepageThemestyle, true);
		}break;
		default:{}break;
	}
	lv_page_glue_obj(textSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(swSettingA_devStatusRecordIF, true);
//	lv_page_glue_obj(textSettingA_devForceAsMeshChild, true);
//	lv_page_glue_obj(swSettingA_devForceAsMeshChild, true);
	lv_page_glue_obj(textSettingA_bGroundPicSelect, true);
	lv_page_glue_obj(textSettingA_factoryRecoveryIf, true);
	lv_page_glue_obj(btnSettingA_factoryRecoveryIf, true);
#if(0 != ITEM_DEVICE_LIST_MANAGE_EN)
	lv_page_glue_obj(textSettingA_devListManagerEN, true);
	lv_page_glue_obj(swSettingA_devListManagerEN, true);
#endif
	if(swSettingA_devHomeAssistantEN)lv_page_glue_obj(swSettingA_devHomeAssistantEN, true);
	if(textSettingA_devHomeAssistantEN)lv_page_glue_obj(textSettingA_devHomeAssistantEN, true);
//	lv_page_glue_obj(textSettingA_touchRecalibrationIf, true);
//	lv_page_glue_obj(btnSettingA_touchRecalibrationIf, true);
//	lv_page_glue_obj(sliderSettingA_screenBrightness, true);
//	lv_page_glue_obj(sliderSettingA_screenBrightnessSleep, true);
	lv_page_glue_obj(textBtnTimeRef_screenLightTime, true);
	lv_page_glue_obj(textSettingA_tempCalibration, true);
	lv_page_glue_obj(btnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(btnTimeCut_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeCut_screenLightTime, true);
	lv_page_glue_obj(textSliderRef_tempratureCal, true);
#if(0 == LVAPP_DISP_ELECPARAM_HIDDEN_EN) //热水器及恒温器电量功能暂隐
	lv_page_glue_obj(textSettingA_elecParamDispSelect, true);
	lv_page_glue_obj(textSettingA_elecsumResetClear, true);
#endif
	lv_page_glue_obj(textSettingA_screenLandscape, true);

	lv_obj_animate(textSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);
	lv_obj_animate(btnSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
//	lv_obj_animate(ddlistSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_systemLanguage, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);
	lv_obj_animate(btnSettingA_systemLanguage, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(swSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_bGroundPicSelect, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(pageBgroundPic_select, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_devAtmosLightColorSet, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_devAtmosLightColorSet, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	switch(currentDev_typeGet()){
		case devTypeDef_largeCurOneBit:
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			lv_obj_animate(textSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
			lv_obj_animate(ddlistSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
		}break;
		default:{}break;
	}
	
	lv_obj_animate(textSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

//	lv_obj_animate(textSettingA_devForceAsMeshChild, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
//	lv_obj_animate(swSettingA_devForceAsMeshChild, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

#if(0 != ITEM_DEVICE_LIST_MANAGE_EN)
	lv_obj_animate(textSettingA_devListManagerEN, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(swSettingA_devListManagerEN, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
#endif

	if(textSettingA_devHomeAssistantEN){
		lv_obj_animate(textSettingA_devHomeAssistantEN, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
		lv_obj_animate(swSettingA_devHomeAssistantEN, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	}

//	lv_obj_animate(textSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
//	lv_obj_animate(btnSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_screenBrightnessSleep, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(textBtnTimeRef_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnTimeAdd_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnTimeCut_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_tempCalibration, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(textSliderRef_tempratureCal, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(btnValAdd_tempratureCal, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnValCut_tempratureCal, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_tempratureCal, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);	

#if(0 == LVAPP_DISP_ELECPARAM_HIDDEN_EN) //热水器及恒温器电量功能暂隐
	lv_obj_animate(textSettingA_elecParamDispSelect, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(cbDispSelect_elecParam_power, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(cbDispSelect_elecParam_elecsum, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	
	lv_obj_animate(textSettingA_elecsumResetClear, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_elecsumReset, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
#endif

	lv_obj_animate(textSettingA_screenLandscape, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(cbDispSelect_screenVer, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(cbDispSelect_screenHor, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);

	if(upgradeAvailable_flg){

		lv_obj_animate(textSettingA_fwareUpgrade, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
		lv_obj_animate(btnSettingA_fwareUpgrade, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	}

	// vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(page_funSetOption);
//	lv_obj_refresh_style(obj_Parent);

//	vTaskDelay((obj_animate_delay * 10) / portTICK_RATE_MS);
//	lv_page_focus(page_funSetOption, btnTimeCut_screenLightTime, obj_animate_time * 5);
//	vTaskDelay((obj_animate_time * 15) / portTICK_RATE_MS);
//	lv_page_focus(page_funSetOption, textSettingA_deviceType, obj_animate_time * 3);

//	lv_obj_set_click(menuBtnChoIcon_fun_back, true);
//	lv_obj_set_click(menuBtnChoIcon_fun_home, true);
//	usrApp_fullScreenRefresh_self(50, 0);
}

// void lvGui_businessMenu_pageReload(void){

// 	lv_obj_clean(objParentTemp);
// 	lv_obj_del(page_funSetOption);
// 	page_funSetOption = NULL;
// 	vTaskDelay(20 / portTICK_PERIOD_MS);
// 	lvGui_businessMenu_settingSet(objParentTemp);
// }

void lvGui_businessMenu_pageReload(void){

#if(0 == MENU_SETTING_REFINE_EN)
	lv_obj_clean(objParentTemp);
#endif
	lv_obj_del(page_funSetOption);
	page_funSetOption = NULL;
	// vTaskDelay(20 / portTICK_PERIOD_MS);
	guiBussinessMenuSettingSubInterfaceCreat(0);
}

void mwifiApp_atmosLightColorCfg_boardcastNotice(void){

	if(false == mwifi_is_connected())return; //当前网络不可用
	else{

		const uint8_t devMacDest[MWIFI_ADDR_LEN] = MWIFI_ADDR_ANY;
		mwifi_data_type_t data_type = {
			
			.compression = true,
			.communicate = MWIFI_COMMUNICATE_UNICAST,
		};
		const mlink_httpd_type_t type_L8mesh_cst = {

			.format = MLINK_HTTPD_FORMAT_HEX,
		};

		mdf_err_t ret = MDF_OK;

		size_t	dataTX_len = 1 + sizeof(stt_devAtmosLightRunningParam);		
		uint8_t *dataTX_temp = (uint8_t *)LV_MEM_CUSTOM_ALLOC(sizeof(uint8_t) * dataTX_len);

		memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

		dataTX_temp[0] = L8DEV_MESH_CMD_ATMOS_CFG_NOTICE;
		memcpy(&dataTX_temp[1], &atmosLightRunParam_dataTemp, sizeof(stt_devAtmosLightRunningParam));
		
		if(esp_mesh_get_layer() == MESH_ROOT){
		
			ret = mwifi_root_write(devMacDest, 
								   1, 
								   &data_type, 
								   dataTX_temp, 
								   dataTX_len, 
								   true);
			MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> atmosLight cfg notice mwifi_translate", mdf_err_to_name(ret));
		}
		else
		{
			ret = mwifi_write(devMacDest, 
							  &data_type, 
							  dataTX_temp, 
							  dataTX_len,  
							  true);
			MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> atmosLight cfg notice mwifi_translate", mdf_err_to_name(ret)); 
		}

		os_free(dataTX_temp);
	}
}

void guiBussinessMenuSettingSubInterfaceCreat(uint8_t objNum){
	
	const uint16_t obj_animate_time = 150;
	const uint16_t obj_animate_delay = 100;
	uint16_t obj_animate_delayBasic = 0;
	uint16_t coordOffset_cLanguage_x = 0;
	char strLabelTmp[32] = {0};

	uint8_t loop = 0;
	devTypeDef_enum devType_Temp = devTypeDef_null;
	enumDevLanguageIdentify systemLanguageType_Temp = languageCyId_null;
	uint8_t homepageThemeStyle_temp = 0;
	uint8_t homepageUiKindIndex_temp = 0;
	stt_devStatusRecord devStatusRecordFlg_temp = {0};
	stt_devSystemKeyParamRecord devSysKparam = {0};
	lv_coord_t menuListPageHeight_tmp = 0;

	lv_obj_t *lvObjAlign_temp = NULL;

	lvGuiSettingSet_objStyle_Init();

	if(NULL == msgQh_uiPageSetting_refresh){
		msgQh_uiPageSetting_refresh = xQueueCreate(1, sizeof(stt_dpAttrMsgQ_uiPageSettingRefresh));
	}

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);
	devSystemKeyAttr_paramGet(&devSysKparam);
	devAtmosLight_runningParam_get(&atmosLightRunParam_dataTemp);

	(devStatusDispMethod_landscapeIf_get())?
		(screenLandscapeCoordinate_objOffset = 60):
		(screenLandscapeCoordinate_objOffset = 0);

	page_funSetOption = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(page_funSetOption, 320, 260)):
		(lv_obj_set_size(page_funSetOption, 240, 260));
	lv_obj_set_pos(page_funSetOption, 0, 75);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_SB, stylePage_funSetOption);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_BG, stylePage_funSetOption);
	lv_page_set_sb_mode(page_funSetOption, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(page_funSetOption, false, false); //key opration
	switch(objNum){
		case 0:menuListPageHeight_tmp = 720;break;
		case 3:menuListPageHeight_tmp = 640;break;
		case 2:menuListPageHeight_tmp =580;break;
		default:menuListPageHeight_tmp = 360;break;}
	if(devStatusDispMethod_landscapeIf_get()){
		lv_page_set_scrl_width(page_funSetOption, 300); 
		lv_page_set_scrl_height(page_funSetOption, menuListPageHeight_tmp);}
	else{
		lv_page_set_scrl_width(page_funSetOption, 220); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		lv_page_set_scrl_height(page_funSetOption, menuListPageHeight_tmp);} //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);

	switch(objNum){

		case 0:{
			textSettingA_deviceType = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_deviceType, "Device type:");
			lv_label_set_text(textSettingA_deviceType, labelStrGet_bySysLanguage(tls_mSetting_deviceType));
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_label_set_align(textSettingA_deviceType, LV_LABEL_ALIGN_LEFT);break;
				default:lv_label_set_align(textSettingA_deviceType, LV_LABEL_ALIGN_RIGHT);break;}
			lv_obj_set_style(textSettingA_deviceType, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_deviceType, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
				default:lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
			btnSettingA_deviceType = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_deviceType, 200, 35);
			lv_btn_set_style(btnSettingA_deviceType, LV_BTN_STATE_REL, &lv_style_transp);
			lv_btn_set_style(btnSettingA_deviceType, LV_BTN_STATE_PR, &lv_style_transp);
			lv_obj_set_protect(btnSettingA_deviceType, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, 0 - screenLandscapeCoordinate_objOffset - 55, 15);break;
				case languageCyId_Portuguese:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 25, 15);break;
				default:lv_obj_align(btnSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 35, 15);break;}
			lv_btn_set_action(btnSettingA_deviceType, LV_BTN_ACTION_CLICK, funCb_btnActionClick_btnDevType_funBack);
			btnLabelSettingA_deviceType = lv_label_create(btnSettingA_deviceType, NULL);
			lv_label_set_align(btnLabelSettingA_deviceType, LV_LABEL_ALIGN_CENTER);
			lv_label_set_long_mode(btnLabelSettingA_deviceType, LV_LABEL_LONG_ROLL);
			lv_obj_set_size(btnLabelSettingA_deviceType, 180, 30);
			lv_label_set_recolor(btnLabelSettingA_deviceType, true);
			lv_obj_set_style(btnLabelSettingA_deviceType, styleLabel_devTypeSelected);
			lv_obj_set_protect(btnLabelSettingA_deviceType, LV_PROTECT_POS);
			devType_Temp = currentDev_typeGet();
			for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){
				if(devType_Temp == deviceTypeTab_disp[loop].devType){
					lv_label_set_text(btnLabelSettingA_deviceType, deviceTypeStr_disp[loop].devName);
					devTypeDdist_istRecord = loop;
					break;}}
			lvObjAlign_temp = textSettingA_deviceType;

			btnSettingA_typeDevExSet = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_typeDevExSet, 100, 35);
			lv_btn_set_style(btnSettingA_typeDevExSet, LV_BTN_STATE_REL, &lv_style_transp);
			lv_btn_set_style(btnSettingA_typeDevExSet, LV_BTN_STATE_PR, &lv_style_transp);
			lv_obj_set_protect(btnSettingA_typeDevExSet, LV_PROTECT_POS);
			lv_obj_align(btnSettingA_typeDevExSet, btnSettingA_deviceType, LV_ALIGN_OUT_RIGHT_MID, 10, 0 );
			btnLabelSettingA_typeDevExSet = lv_label_create(btnSettingA_typeDevExSet, NULL);
			lv_obj_set_protect(btnLabelSettingA_typeDevExSet, LV_PROTECT_POS);
			lv_label_set_align(btnLabelSettingA_typeDevExSet, LV_LABEL_ALIGN_CENTER);
			lv_label_set_long_mode(btnLabelSettingA_typeDevExSet, LV_LABEL_LONG_ROLL);
			lv_obj_set_size(btnLabelSettingA_typeDevExSet, 80, 30);
			lv_label_set_recolor(btnLabelSettingA_typeDevExSet, true);
			lv_obj_set_style(btnLabelSettingA_typeDevExSet, styleLabel_devTypeSelected);
			lv_obj_set_hidden(btnSettingA_typeDevExSet, true);
			lvGui_devTypeExSetElementRefresh();

			textSettingA_systemLanguage = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_systemLanguage, "System Language:");
			lv_label_set_text(textSettingA_systemLanguage, labelStrGet_bySysLanguage(tls_mSetting_systemLanguage));
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_label_set_align(textSettingA_systemLanguage, LV_LABEL_ALIGN_LEFT);break;
				default:lv_label_set_align(textSettingA_systemLanguage, LV_LABEL_ALIGN_RIGHT);break;}
			lv_obj_set_style(textSettingA_systemLanguage, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_systemLanguage, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_systemLanguage, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_systemLanguage, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			btnSettingA_systemLanguage = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_systemLanguage, 200, 35);
			lv_btn_set_style(btnSettingA_systemLanguage, LV_BTN_STATE_REL, &lv_style_transp);
			lv_btn_set_style(btnSettingA_systemLanguage, LV_BTN_STATE_PR, &lv_style_transp);
			lv_obj_set_protect(btnSettingA_systemLanguage, LV_PROTECT_POS);
		//	(devStatusDispMethod_landscapeIf_get())?
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_systemLanguage, textSettingA_systemLanguage, LV_ALIGN_OUT_BOTTOM_MID, 0 - screenLandscapeCoordinate_objOffset - 62, 15);break;
				default:lv_obj_align(btnSettingA_systemLanguage, textSettingA_systemLanguage, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 21, 15);break;}
			lv_btn_set_action(btnSettingA_systemLanguage, LV_BTN_ACTION_CLICK, funCb_btnActionClick_btnSysLanguage_funBack);
			btnLabelSettingA_systemLanguage = lv_label_create(btnSettingA_systemLanguage, NULL);
			lv_label_set_align(btnLabelSettingA_systemLanguage, LV_LABEL_ALIGN_CENTER);
			lv_label_set_long_mode(btnLabelSettingA_systemLanguage, LV_LABEL_LONG_ROLL);
			lv_obj_set_size(btnLabelSettingA_systemLanguage, 180, 30);
			lv_label_set_recolor(btnLabelSettingA_systemLanguage, true);
			lv_obj_set_style(btnLabelSettingA_systemLanguage, styleLabel_devTypeSelected);
			lv_obj_set_protect(btnLabelSettingA_systemLanguage, LV_PROTECT_POS);
			systemLanguageType_Temp = devLanguageCountry_identifyGet();
			for(loop = 0; loop < DEV_LANGUAGE_COUNTRY_NUM_MAX; loop ++){
				if(systemLanguageType_Temp == systemLanguageStr_disp[loop].devType){
					lv_label_set_text(btnLabelSettingA_systemLanguage, systemLanguageStr_disp[loop].devName);
					devTypeDdist_istRecord = loop;
					break;
				}
			}
			lvObjAlign_temp = textSettingA_systemLanguage;

			textSettingA_devStatusRecordIF = lv_label_create(page_funSetOption, textSettingA_deviceType);
		//	lv_label_set_text(textSettingA_devStatusRecordIF, "Stay the status:");
			lv_label_set_text(textSettingA_devStatusRecordIF, labelStrGet_bySysLanguage(tls_mSetting_stayTheStatus));
			lv_obj_set_style(textSettingA_devStatusRecordIF, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_devStatusRecordIF, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_devStatusRecordIF, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_devStatusRecordIF, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			swSettingA_devStatusRecordIF = lv_sw_create(page_funSetOption, NULL);
			lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
			lv_obj_set_protect(swSettingA_devStatusRecordIF, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
				default:lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
			lv_sw_set_anim_time(swSettingA_devStatusRecordIF, 100);
			if(devStatusRecordFlg_temp.devStatusOnOffRecord_IF)lv_sw_on(swSettingA_devStatusRecordIF);
			else lv_sw_off(swSettingA_devStatusRecordIF);
			lv_sw_set_action(swSettingA_devStatusRecordIF, funCb_swDevStatusRecordIF);
			lvObjAlign_temp = textSettingA_devStatusRecordIF;

			textSettingA_factoryRecoveryIf = lv_label_create(page_funSetOption, textSettingA_deviceType);
			lv_label_set_text(textSettingA_factoryRecoveryIf, labelStrGet_bySysLanguage(tls_mSetting_factoryReset));
			lv_obj_set_style(textSettingA_factoryRecoveryIf, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_factoryRecoveryIf, LV_PROTECT_POS);
			switch(currentDev_typeGet()){
				default:{
					switch(devLanguageCountry_identifyGet()){
						case languageCyId_Arabic:lv_obj_align(textSettingA_factoryRecoveryIf, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
						default:lv_obj_align(textSettingA_factoryRecoveryIf, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
				}break;
			}
			btnSettingA_factoryRecoveryIf = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_factoryRecoveryIf, 60, 20);
			lv_obj_set_protect(btnSettingA_factoryRecoveryIf, LV_PROTECT_POS);
			lv_btn_set_action(btnSettingA_factoryRecoveryIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_factoryRecovery);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
				default:lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
			lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnRef_factoryRecoveryIf = lv_label_create(btnSettingA_factoryRecoveryIf, NULL);
			lv_label_set_recolor(textBtnRef_factoryRecoveryIf, true);
			lv_label_set_text(textBtnRef_factoryRecoveryIf, "#00FF40 >>>o<<<#");
			lv_obj_set_style(textBtnRef_factoryRecoveryIf, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnRef_factoryRecoveryIf, LV_PROTECT_POS);
			lv_obj_align(textBtnRef_factoryRecoveryIf, NULL, LV_ALIGN_CENTER, 0, 0);
			lvObjAlign_temp = textSettingA_factoryRecoveryIf;

			// textSettingA_restartAllDevices = lv_label_create(page_funSetOption, textSettingA_deviceType);
			// lv_label_set_text(textSettingA_restartAllDevices, "Reboot system:");
			// lv_obj_set_style(textSettingA_restartAllDevices, styleText_menuLevel_B);
			// lv_obj_set_protect(textSettingA_restartAllDevices, LV_PROTECT_POS);
			// switch(devLanguageCountry_identifyGet()){
			// 	case languageCyId_Arabic:lv_obj_align(textSettingA_restartAllDevices, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
			// 	default:lv_obj_align(textSettingA_restartAllDevices, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			// btnSettingA_rebootAllDevices = lv_btn_create(page_funSetOption, NULL);
			// lv_obj_set_size(btnSettingA_rebootAllDevices, 60, 20);
			// lv_obj_set_protect(btnSettingA_rebootAllDevices, LV_PROTECT_POS);
			// lv_btn_set_action(btnSettingA_rebootAllDevices, LV_BTN_ACTION_CLICK, funCb_btnActionClick_rebootAllDevices);
			// switch(devLanguageCountry_identifyGet()){
			// 	case languageCyId_Arabic:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
			// 	default:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
			// lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			// lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			// lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			// lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			// textBtnRef_rebootAllDevices = lv_label_create(btnSettingA_rebootAllDevices, NULL);
			// lv_label_set_recolor(textBtnRef_rebootAllDevices, true);
			// lv_label_set_text(textBtnRef_rebootAllDevices, "#00FF40 >>>o<<<#");
			// lv_obj_set_style(textBtnRef_rebootAllDevices, styleText_menuLevel_B);
			// lv_obj_set_protect(textBtnRef_rebootAllDevices, LV_PROTECT_POS);
			// lv_obj_align(textBtnRef_rebootAllDevices, NULL, LV_ALIGN_CENTER, 0, 0);
			// lvObjAlign_temp = textSettingA_restartAllDevices;

			textSettingA_tempParamDispSelect = lv_label_create(page_funSetOption, textSettingA_deviceType);
			lv_label_set_text(textSettingA_tempParamDispSelect, labelStrGet_bySysLanguage(tls_mSetting_tempParamDisp));
			lv_obj_set_style(textSettingA_tempParamDispSelect, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_tempParamDispSelect, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_tempParamDispSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_tempParamDispSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			cbDispSelect_tempMethod_celsius = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_tempMethod_celsius, 160, 15);
			lv_cb_set_text(cbDispSelect_tempMethod_celsius, labelStrGet_bySysLanguage(tls_mSetting_tempDispCels));
			lv_obj_set_protect(cbDispSelect_tempMethod_celsius, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_tempMethod_celsius, LV_PROTECT_FOLLOW);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:
					coordOffset_cLanguage_x = -20;
					screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
					break;
				default:
					coordOffset_cLanguage_x = 0;
					screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
					break;}
			lv_obj_align(cbDispSelect_tempMethod_celsius, textSettingA_tempParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
			(devStatusRecordFlg_temp.devTempDispMethod)?
				(lv_cb_set_checked(cbDispSelect_tempMethod_celsius, false)):
				(lv_cb_set_checked(cbDispSelect_tempMethod_celsius, true));
			cbDispSelect_tempMethod_fahrenheit = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_tempMethod_fahrenheit, 160, 15);
			lv_cb_set_text(cbDispSelect_tempMethod_fahrenheit, labelStrGet_bySysLanguage(tls_mSetting_tempDispFahr));
			lv_obj_set_protect(cbDispSelect_tempMethod_fahrenheit, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_tempMethod_fahrenheit, LV_PROTECT_FOLLOW);
			lv_obj_align(cbDispSelect_tempMethod_fahrenheit, textSettingA_tempParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
			lv_cb_set_style(cbDispSelect_tempMethod_celsius, LV_CB_STYLE_BG, stylePageCb_tempDispMethodSet);
			lv_cb_set_style(cbDispSelect_tempMethod_fahrenheit, LV_CB_STYLE_BG, stylePageCb_tempDispMethodSet);
			lv_cb_set_action(cbDispSelect_tempMethod_celsius, funCb_cbOpreat_tempParamDispSelect_celsius);
			lv_cb_set_action(cbDispSelect_tempMethod_fahrenheit, funCb_cbOpreat_tempParamDispSelect_fahrenheit);
			(devStatusRecordFlg_temp.devTempDispMethod)?
				(lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, true)):
				(lv_cb_set_checked(cbDispSelect_tempMethod_fahrenheit, false));
			lvObjAlign_temp = textSettingA_tempParamDispSelect;

			textSettingA_tempCalibration = lv_label_create(page_funSetOption, textSettingA_deviceType);
		//	lv_label_set_text(textSettingA_tempCalibration, "Temperature calibration:");
			lv_label_set_text(textSettingA_tempCalibration, labelStrGet_bySysLanguage(tls_mSetting_temperatureCalibretion));
			lv_obj_set_style(textSettingA_tempCalibration, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_tempCalibration, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_tempCalibration, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_tempCalibration, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			textSliderRef_tempratureCal = lv_label_create(page_funSetOption, NULL);
			(devTempDispMethod_unit_get())?
				(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", varConverionCelsius2Fahrenheit_f(devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())) - 32.0F)):
				(sprintf(textlabel_dispTemp, "#FFFF00 %.1f#", devTempratureSensor_dataCal_hanldeGet(devTempratureSensor_dataCal_get())));
			lv_label_set_recolor(textSliderRef_tempratureCal, true);
			lv_label_set_text(textSliderRef_tempratureCal, textlabel_dispTemp);
			lv_obj_set_style(textSliderRef_tempratureCal, styleText_menuLevel_B);
			lv_obj_set_protect(textSliderRef_tempratureCal, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(textSliderRef_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 85, 7);break;
					default:lv_obj_align(textSliderRef_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 85, 7);break;}
			sliderSettingA_tempratureCal = lv_slider_create(page_funSetOption, NULL);
			lv_obj_set_size(sliderSettingA_tempratureCal, 150, 15);
			lv_obj_set_protect(sliderSettingA_tempratureCal, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(sliderSettingA_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 12, 30);break;
				default:lv_obj_align(sliderSettingA_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 12, 30);break;}
			lv_slider_set_action(sliderSettingA_tempratureCal, funCb_slidAction_functionSet_tempratureCal);
			lv_slider_set_range(sliderSettingA_tempratureCal, 0, DEV_TEMPRATURE_DATA_CAL_MAX_RANGE);
			lv_slider_set_value(sliderSettingA_tempratureCal, devTempratureSensor_dataCal_get());
			btnValAdd_tempratureCal = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnValAdd_tempratureCal, 40, 40);
			lv_obj_set_protect(btnValAdd_tempratureCal, LV_PROTECT_POS);
			lv_btn_set_action(btnValAdd_tempratureCal, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_tempratureCalAdd);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnValAdd_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, -18);break;
				default:lv_obj_align(btnValAdd_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 155, -18);break;}
			lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnValAdd_tempratureCal, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnValAdd_tempratureCal = lv_label_create(btnValAdd_tempratureCal, NULL);
			lv_label_set_recolor(textBtnValAdd_tempratureCal, true);
			lv_label_set_text(textBtnValAdd_tempratureCal, "#00FF40 + #");
			lv_obj_set_style(textBtnValAdd_tempratureCal, styleText_menuLevel_A);
			lv_obj_set_protect(textBtnValAdd_tempratureCal, LV_PROTECT_POS);
			lv_obj_align(textBtnValAdd_tempratureCal, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 2);
			btnValCut_tempratureCal = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnValCut_tempratureCal, 40, 40);
			lv_obj_set_protect(btnValCut_tempratureCal, LV_PROTECT_POS);
			lv_btn_set_action(btnValCut_tempratureCal, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_tempratureCalCut);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnValCut_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 155, -18);break;
				default:lv_obj_align(btnValCut_tempratureCal, textSettingA_tempCalibration, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, -18);break;}
			lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnValCut_tempratureCal, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnValCut_tempratureCal = lv_label_create(btnValCut_tempratureCal, NULL);
			lv_label_set_recolor(textBtnValCut_tempratureCal, true);
			lv_label_set_text(textBtnValCut_tempratureCal, "#00FF40 _ #");
			lv_obj_set_style(textBtnValCut_tempratureCal, styleText_menuLevel_A);
			lv_obj_set_protect(textBtnValCut_tempratureCal, LV_PROTECT_POS);
			lv_obj_align(textBtnValCut_tempratureCal, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -6);
			lvObjAlign_temp = textSettingA_tempCalibration;

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER) //特殊设备不支持HA
			textSettingA_devHomeAssistantEN = lv_label_create(page_funSetOption, textSettingA_deviceType);
			lv_label_set_text(textSettingA_devHomeAssistantEN, labelStrGet_bySysLanguage(tls_mSetting_homassistantSupport));
			lv_obj_set_style(textSettingA_devHomeAssistantEN, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_devHomeAssistantEN, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_devHomeAssistantEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_devHomeAssistantEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			swSettingA_devHomeAssistantEN = lv_sw_create(page_funSetOption, NULL);
			lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devHomeAssistantEN, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
			lv_obj_set_protect(swSettingA_devHomeAssistantEN, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(swSettingA_devHomeAssistantEN, textSettingA_devHomeAssistantEN, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
				default:lv_obj_align(swSettingA_devHomeAssistantEN, textSettingA_devHomeAssistantEN, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
			lv_sw_set_anim_time(swSettingA_devHomeAssistantEN, 100);
			if(devStatusRecordFlg_temp.homeassitant_En)lv_sw_on(swSettingA_devHomeAssistantEN);
			else lv_sw_off(swSettingA_devHomeAssistantEN);
			lv_sw_set_action(swSettingA_devHomeAssistantEN, funCb_homeassistantEnable);
			lvObjAlign_temp = textSettingA_devHomeAssistantEN;
#endif

#if(0 != ITEM_DEVICE_LIST_MANAGE_EN)
			textSettingA_devListManagerEN = lv_label_create(page_funSetOption, textSettingA_deviceType);
			lv_label_set_text(textSettingA_devListManagerEN, labelStrGet_bySysLanguage(tls_mSetting_devieManage));
			lv_obj_set_style(textSettingA_devListManagerEN, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_devListManagerEN, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_devListManagerEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_devListManagerEN, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			swSettingA_devListManagerEN = lv_sw_create(page_funSetOption, NULL);
			lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
			lv_sw_set_style(swSettingA_devListManagerEN, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
			lv_obj_set_protect(swSettingA_devListManagerEN, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(swSettingA_devListManagerEN, textSettingA_devListManagerEN, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
				default:lv_obj_align(swSettingA_devListManagerEN, textSettingA_devListManagerEN, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
			lv_sw_set_anim_time(swSettingA_devListManagerEN, 100);
			if(devStatusRecordFlg_temp.devListManager_En)lv_sw_on(swSettingA_devListManagerEN);
			else lv_sw_off(swSettingA_devListManagerEN);
			lv_sw_set_action(swSettingA_devListManagerEN, funCb_swDevListManagerEN);
			lvObjAlign_temp = textSettingA_devListManagerEN;
#endif
		}break;

		case 1:{
			textSettingA_elecParamDispSelect = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_elecParamDispSelect, "Electric parameter display:");
			lv_label_set_text(textSettingA_elecParamDispSelect, labelStrGet_bySysLanguage(tls_mSetting_elecParamDisp));
			lv_obj_set_style(textSettingA_elecParamDispSelect, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_elecParamDispSelect, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_elecParamDispSelect, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
				default:lv_obj_align(textSettingA_elecParamDispSelect, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
			cbDispSelect_elecParam_power = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_elecParam_power, 160, 15);
		//	lv_cb_set_text(cbDispSelect_elecParam_power, " Power");
			lv_cb_set_text(cbDispSelect_elecParam_power, labelStrGet_bySysLanguage(tls_mSetting_elecDispPower));
			lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_FOLLOW);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:
					coordOffset_cLanguage_x = -20;
					screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
					break;
				default:
					coordOffset_cLanguage_x = 0;
					screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
					break;}
			lv_obj_align(cbDispSelect_elecParam_power, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
			(devStatusRecordFlg_temp.devElecsumDisp_IF)?
				(lv_cb_set_checked(cbDispSelect_elecParam_power, false)):
				(lv_cb_set_checked(cbDispSelect_elecParam_power, true));
			cbDispSelect_elecParam_elecsum = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_elecParam_elecsum, 160, 15);
		//	lv_cb_set_text(cbDispSelect_elecParam_elecsum, " Electric sum");
			lv_cb_set_text(cbDispSelect_elecParam_elecsum, labelStrGet_bySysLanguage(tls_mSetting_elecDispSum));
			lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_FOLLOW);
			lv_obj_align(cbDispSelect_elecParam_elecsum, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
			lv_cb_set_style(cbDispSelect_elecParam_power, LV_CB_STYLE_BG, stylePageCb_elecParamDispSet);
			lv_cb_set_style(cbDispSelect_elecParam_elecsum, LV_CB_STYLE_BG, stylePageCb_elecParamDispSet);
			lv_cb_set_action(cbDispSelect_elecParam_power, funCb_cbOpreat_elecParamDispSelect_power);
			lv_cb_set_action(cbDispSelect_elecParam_elecsum, funCb_cbOpreat_elecParamDispSelect_elecsum);
			(devStatusRecordFlg_temp.devElecsumDisp_IF)?
				(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, true)):
				(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, false));
			lvObjAlign_temp = textSettingA_elecParamDispSelect;

			textSettingA_elecsumResetClear = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_elecsumResetClear, "Electric sum reset:");
			lv_label_set_text(textSettingA_elecsumResetClear, labelStrGet_bySysLanguage(tls_mSetting_elecSumReset));
			lv_obj_set_style(textSettingA_elecsumResetClear, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_elecsumResetClear, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_elecsumResetClear, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_elecsumResetClear, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			btnSettingA_elecsumReset = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_elecsumReset, 60, 20);
			lv_obj_set_protect(btnSettingA_elecsumReset, LV_PROTECT_POS);
			lv_btn_set_action(btnSettingA_elecsumReset, LV_BTN_ACTION_CLICK, funCb_btnActionClick_elecsumResetClear);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_elecsumReset, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
				default:lv_obj_align(btnSettingA_elecsumReset, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
			lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnRef_elecsumReset = lv_label_create(btnSettingA_elecsumReset, NULL);
			lv_label_set_recolor(textBtnRef_elecsumReset, true);
			lv_label_set_text(textBtnRef_elecsumReset, "#00FF40 >>>o<<<#");
			lv_obj_set_style(textBtnRef_elecsumReset, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnRef_elecsumReset, LV_PROTECT_POS);
			lv_obj_align(textBtnRef_elecsumReset, NULL, LV_ALIGN_CENTER, 0, 0);
		}break;

		case 2:{
			textSettingA_bGroundPicSelect = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_bGroundPicSelect, "Background pic set:");
			lv_label_set_text(textSettingA_bGroundPicSelect, labelStrGet_bySysLanguage(tls_mSetting_bGroundPicSet));
			lv_obj_set_style(textSettingA_bGroundPicSelect, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_bGroundPicSelect, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_bGroundPicSelect, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
				default:lv_obj_align(textSettingA_bGroundPicSelect, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
				// case languageCyId_Arabic:lv_obj_align(textSettingA_bGroundPicSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				// default:lv_obj_align(textSettingA_bGroundPicSelect, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			pageBgroundPic_select = lv_page_create(page_funSetOption, NULL);
			lv_obj_set_size(pageBgroundPic_select, 200, 65);
			lv_obj_set_protect(pageBgroundPic_select, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(pageBgroundPic_select, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 0);break;
				default:lv_obj_align(pageBgroundPic_select, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 0);break;}
			lv_page_set_scrl_fit(pageBgroundPic_select, true, false);
		//	lv_page_set_scrl_width(pageBgroundPic_select, 720); 
			lv_page_set_scrl_height(pageBgroundPic_select, 45); 
			lv_page_set_sb_mode(pageBgroundPic_select, LV_SB_MODE_HIDE);
			lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_SB, stylePage_funSetOption);
			lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_BG, stylePage_funSetOption);
			for(loop = 0; loop < BROUND_PIC_SELECT_MAX_NUM; loop ++){
				btnBgroundPicSel_preview[loop] = lv_imgbtn_create(pageBgroundPic_select, NULL);
				lv_obj_set_size(btnBgroundPicSel_preview[loop], 40, 40);
				lv_imgbtn_set_toggle(btnBgroundPicSel_preview[loop], true);
		//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_PR, usrAppHomepageBkPic_prevDataGet(loop));
		//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_REL, usrAppHomepageBkPic_prevDataGet(loop));
				lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL, usrAppHomepageBkPic_prevDataGet(loop));
				lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR, usrAppHomepageBkPic_prevDataGet(loop));
				lv_imgbtn_set_action(btnBgroundPicSel_preview[loop], LV_BTN_ACTION_CLICK, funCb_btnActionClick_bGroundPicOrgSelcet);
				lv_obj_set_free_num(btnBgroundPicSel_preview[loop], loop);
				lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_PR, usrAppHomepageBkPicStyle_prevDataGet(loop));
				lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_REL, usrAppHomepageBkPicStyle_prevDataGet(loop));
				lv_obj_set_protect(btnBgroundPicSel_preview[loop], LV_PROTECT_POS);
				lv_obj_set_pos(btnBgroundPicSel_preview[loop], loop * 45 + 5, 3);
				lv_page_glue_obj(btnBgroundPicSel_preview[loop], true);
				(loop == usrAppHomepageBgroundPicOrg_Get())?
					(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR)):
					(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL));
			}
			imgBgroundPicSel_reserve = lv_img_create(btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], NULL);
			lv_img_set_src(imgBgroundPicSel_reserve, &bGroundPrev_picFigure_sel);
			lv_page_focus(pageBgroundPic_select, btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], 0);
			lvObjAlign_temp = textSettingA_bGroundPicSelect;

			switch(currentDev_typeGet()){
				case devTypeDef_scenario:
				case devTypeDef_largeCurOneBit:
				case devTypeDef_mulitSwOneBit:
				case devTypeDef_mulitSwTwoBit:
				case devTypeDef_mulitSwThreeBit:
				case devTypeDef_fans:
				case devTypeDef_thermostatExtension:{
					textSettingA_homepageThemestyle = lv_label_create(page_funSetOption, NULL);
		//			lv_label_set_text(textSettingA_homepageThemestyle, "Theme:");
					lv_label_set_text(textSettingA_homepageThemestyle, labelStrGet_bySysLanguage(tls_mSetting_theme));
					lv_obj_set_style(textSettingA_homepageThemestyle, styleText_menuLevel_B);
					lv_obj_set_protect(textSettingA_homepageThemestyle, LV_PROTECT_POS);
					switch(devLanguageCountry_identifyGet()){
						case languageCyId_Arabic:lv_obj_align(textSettingA_homepageThemestyle, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
						default:lv_obj_align(textSettingA_homepageThemestyle, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
					ddlistSettingA_homepageThemestyle = lv_ddlist_create(page_funSetOption, NULL);
					lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_SB, styleDdlistSettingA_devType);
					lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_BG, styleDdlistSettingA_devType);
					lv_obj_set_protect(ddlistSettingA_homepageThemestyle, LV_PROTECT_POS);
					switch(devLanguageCountry_identifyGet()){
						case languageCyId_Arabic:lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 50, 20);break;
						default:lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
					lv_ddlist_set_options(ddlistSettingA_homepageThemestyle, homepageThemeStyle_listTab);
					lv_ddlist_set_action(ddlistSettingA_homepageThemestyle, funCb_ddlistHomepageThemeDef);
					lv_obj_set_top(ddlistSettingA_homepageThemestyle, true);
					lv_obj_set_free_num(ddlistSettingA_homepageThemestyle, OBJ_DDLIST_HPTHEME_FREENUM);
					homepageThemeStyle_temp = usrAppHomepageThemeType_Get();
					lv_ddlist_set_selected(ddlistSettingA_homepageThemestyle, homepageThemeStyle_temp);
					lvObjAlign_temp = textSettingA_homepageThemestyle;
				}break;

				default:{}break;
			}

			switch(currentDev_typeGet()){
				case devTypeDef_fans:{
					textSettingA_homepageUiKindIndexCho = lv_label_create(page_funSetOption, NULL);
					// lv_label_set_text(textSettingA_homepageUiKindIndexCho, "Interface Type:");
					 lv_label_set_text(textSettingA_homepageUiKindIndexCho, labelStrGet_bySysLanguage(tls_mSetting_uiKindIndex));
					lv_obj_set_style(textSettingA_homepageUiKindIndexCho, styleText_menuLevel_B);
					lv_obj_set_protect(textSettingA_homepageUiKindIndexCho, LV_PROTECT_POS);
					switch(devLanguageCountry_identifyGet()){
						case languageCyId_Arabic:lv_obj_align(textSettingA_homepageUiKindIndexCho, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
						default:lv_obj_align(textSettingA_homepageUiKindIndexCho, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
					ddlistSettingA_homepageUiKindIndex = lv_ddlist_create(page_funSetOption, NULL);
					lv_ddlist_set_style(ddlistSettingA_homepageUiKindIndex, LV_DDLIST_STYLE_SB, styleDdlistSettingA_devType);
					lv_ddlist_set_style(ddlistSettingA_homepageUiKindIndex, LV_DDLIST_STYLE_BG, styleDdlistSettingA_devType);
					lv_obj_set_protect(ddlistSettingA_homepageUiKindIndex, LV_PROTECT_POS);
					switch(devLanguageCountry_identifyGet()){
						case languageCyId_Arabic:lv_obj_align(ddlistSettingA_homepageUiKindIndex, textSettingA_homepageUiKindIndexCho, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 50, 20);break;
						default:lv_obj_align(ddlistSettingA_homepageUiKindIndex, textSettingA_homepageUiKindIndexCho, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
					lv_ddlist_set_options(ddlistSettingA_homepageUiKindIndex, homepageUiKindIndex_listTab);
					lv_ddlist_set_action(ddlistSettingA_homepageUiKindIndex, funCb_ddlistHomepageUiKindIndexDef);
					lv_obj_set_top(ddlistSettingA_homepageUiKindIndex, true);
					lv_obj_set_free_num(ddlistSettingA_homepageUiKindIndex, OBJ_DDLIST_HUIINDEX_FREENUM);
					homepageUiKindIndex_temp = usrAppHomepageUiKindIndex_Get();
					lv_ddlist_set_selected(ddlistSettingA_homepageUiKindIndex, homepageUiKindIndex_temp);
					lvObjAlign_temp = textSettingA_homepageUiKindIndexCho;
				}break;
				default:break;
			}

			textSettingA_devAtmosLightColorSet = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_devAtmosLightColorSet, "Decorative lamp set:");
			lv_label_set_text(textSettingA_devAtmosLightColorSet, labelStrGet_bySysLanguage(tls_mSetting_decoractiveSet));
			lv_obj_set_style(textSettingA_devAtmosLightColorSet, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_devAtmosLightColorSet, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_devAtmosLightColorSet, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_devAtmosLightColorSet, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			btnSettingA_devAtmosLightColorSet = lv_btn_create(page_funSetOption, NULL);
			styleBtn_devAtmosLightColorSet->body.main_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
			styleBtn_devAtmosLightColorSet->body.grad_color = atmosLightRunParam_dataTemp.lightColorCustomParamcfg;
			lv_btn_set_style(btnSettingA_devAtmosLightColorSet, LV_BTN_STYLE_REL, styleBtn_devAtmosLightColorSet);
			lv_btn_set_style(btnSettingA_devAtmosLightColorSet, LV_BTN_STYLE_PR, styleBtn_devAtmosLightColorSet);
			lv_btn_set_action(btnSettingA_devAtmosLightColorSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devAtmosLightColorSet);
			lv_obj_set_size(btnSettingA_devAtmosLightColorSet, 80, 30);
			lv_obj_set_protect(btnSettingA_devAtmosLightColorSet, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_LEFT, 0 - screenLandscapeCoordinate_objOffset + 25, 15);break;
				case languageCyId_Portuguese:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_RIGHT, screenLandscapeCoordinate_objOffset - 80, 15);break;
				default:lv_obj_align(btnSettingA_devAtmosLightColorSet, textSettingA_devAtmosLightColorSet, LV_ALIGN_OUT_BOTTOM_RIGHT, screenLandscapeCoordinate_objOffset - 15, 15);break;}
			lvObjAlign_temp = textSettingA_devAtmosLightColorSet;
		}break;

		case 3:{
			textSettingA_screenBrightness = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_screenBrightness, "Screen brightness:");
			lv_label_set_text(textSettingA_screenBrightness, labelStrGet_bySysLanguage(tls_mSetting_screenBrightness));
			lv_obj_set_style(textSettingA_screenBrightness, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_screenBrightness, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_screenBrightness, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
				default:lv_obj_align(textSettingA_screenBrightness, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
			sliderSettingA_screenBrightness = lv_slider_create(page_funSetOption, NULL);
			lv_obj_set_size(sliderSettingA_screenBrightness, 150, 15);
			lv_obj_set_protect(sliderSettingA_screenBrightness, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 10);break;
				default:lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 10);break;}
			lv_obj_set_free_num(sliderSettingA_screenBrightness, 0);
			lv_slider_set_action(sliderSettingA_screenBrightness, funCb_slidAction_functionSet_screenBrightnessAdj);
			lv_slider_set_range(sliderSettingA_screenBrightness, 0, screenBrightness_sliderAdj_div);
			uint8_t brightnessSlider_valDisp = devScreenDriver_configParam_brightness_get();
			lv_slider_set_value(sliderSettingA_screenBrightness, brightnessSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			sliderSettingA_screenBrightnessSleep = lv_slider_create(page_funSetOption, sliderSettingA_screenBrightness);
			lv_slider_set_action(sliderSettingA_screenBrightnessSleep, funCb_slidAction_functionSet_screenBrightnessAdj);
			lv_obj_set_free_num(sliderSettingA_screenBrightnessSleep, 1);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(sliderSettingA_screenBrightnessSleep, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 10, 40);break;
				default:lv_obj_align(sliderSettingA_screenBrightnessSleep, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 40);break;}
			uint8_t brightnessSleepSlider_valDisp = devScreenDriver_configParam_brightnessSleep_get();
			lv_slider_set_value(sliderSettingA_screenBrightnessSleep, brightnessSleepSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			lvObjAlign_temp = textSettingA_screenBrightness;

			textSettingA_screenLightTime = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_screenLightTime, "Screen light time:");
			lv_label_set_text(textSettingA_screenLightTime, labelStrGet_bySysLanguage(tls_mSetting_screenLightTime));
			lv_obj_set_style(textSettingA_screenLightTime, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_screenLightTime, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_screenLightTime, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_screenLightTime, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			textBtnTimeRef_screenLightTime = lv_label_create(page_funSetOption, NULL);
			lv_label_set_recolor(textBtnTimeRef_screenLightTime, true);
			uint32_t screenLightTime_temp = devScreenDriver_configParam_screenLightTime_get();
			for(loop = 0; loop < FUNCTION_NUM_DEF_SCREENLIGHT_TIME; loop ++){
				if(screenLightTime_temp == screenLightTimeGear_refTab[loop].screenLightTime2Sec){
					functionGearScreenTime_ref = loop;
					break;
				}
			}
			lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
			lv_label_set_align(textBtnTimeRef_screenLightTime, LV_LABEL_ALIGN_CENTER);
			lv_obj_set_style(textBtnTimeRef_screenLightTime, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnTimeRef_screenLightTime, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:
					screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
					coordOffset_cLanguage_x = -35;
					break;
				case languageCyId_Portuguese:
					screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
					coordOffset_cLanguage_x = 27;
					break;
				default:
					screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
					coordOffset_cLanguage_x = 0;
					break;}
			lv_obj_align(textBtnTimeRef_screenLightTime, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_MID, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 25);
			btnTimeAdd_screenLightTime = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnTimeAdd_screenLightTime, 90, 60);
			lv_obj_set_protect(btnTimeAdd_screenLightTime, LV_PROTECT_POS);
			lv_btn_set_action(btnTimeAdd_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeAdd);
			lv_obj_align(btnTimeAdd_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_RIGHT_MID, -10, -2);
			lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnTimeAdd_screenLightTime = lv_label_create(btnTimeAdd_screenLightTime, NULL);
			lv_label_set_recolor(textBtnTimeAdd_screenLightTime, true);
			lv_label_set_text(textBtnTimeAdd_screenLightTime, "#00FF40 >>>>#");
			lv_label_set_align(textBtnTimeAdd_screenLightTime, LV_LABEL_ALIGN_CENTER);
			lv_obj_set_style(textBtnTimeAdd_screenLightTime, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnTimeAdd_screenLightTime, LV_PROTECT_POS);
			lv_obj_align(textBtnTimeAdd_screenLightTime, NULL, LV_ALIGN_CENTER, 0, 2);
			btnTimeCut_screenLightTime = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnTimeCut_screenLightTime, 90, 60);
			lv_obj_set_protect(btnTimeCut_screenLightTime, LV_PROTECT_POS);
			lv_btn_set_action(btnTimeCut_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeCut);
			lv_obj_align(btnTimeCut_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_LEFT_MID, 10, -2);
			lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnTimeCut_screenLightTime = lv_label_create(btnTimeCut_screenLightTime, NULL);
			lv_label_set_recolor(textBtnTimeCut_screenLightTime, true);
			lv_label_set_text(textBtnTimeCut_screenLightTime, "#00FF40 <<<<#");
			lv_label_set_align(textBtnTimeCut_screenLightTime, LV_LABEL_ALIGN_CENTER);
			lv_obj_set_style(textBtnTimeCut_screenLightTime, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnTimeCut_screenLightTime, LV_PROTECT_POS);
			lv_obj_align(textBtnTimeCut_screenLightTime, NULL, LV_ALIGN_CENTER, 0, 2);
			lvObjAlign_temp = textSettingA_screenLightTime;

			textSettingA_screenLandscape = lv_label_create(page_funSetOption, NULL);
		//	lv_label_set_text(textSettingA_screenLandscape, "Screen display angle:");
			lv_label_set_text(textSettingA_screenLandscape, labelStrGet_bySysLanguage(tls_mSetting_screenDispAngle));
			lv_obj_set_style(textSettingA_screenLandscape, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_screenLandscape, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_screenLandscape, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
				default:lv_obj_align(textSettingA_screenLandscape, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
			cbDispSelect_screenVer = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_screenVer, 160, 15);
		//	lv_cb_set_text(cbDispSelect_screenVer, " Vertical");
			lv_cb_set_text(cbDispSelect_screenVer, labelStrGet_bySysLanguage(tls_mSetting_vertical));
			lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_FOLLOW);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:
					coordOffset_cLanguage_x = -20;
					screenLandscapeCoordinateObjOffset_temp = 0 - screenLandscapeCoordinate_objOffset;
					break;
				default:coordOffset_cLanguage_x = 0;
					screenLandscapeCoordinateObjOffset_temp = screenLandscapeCoordinate_objOffset;
					break;}
			lv_obj_align(cbDispSelect_screenVer, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 6);
			(devStatusRecordFlg_temp.devScreenLandscape_IF)?
				(lv_cb_set_checked(cbDispSelect_screenVer, false)):
				(lv_cb_set_checked(cbDispSelect_screenVer, true));
			cbDispSelect_screenHor = lv_cb_create(page_funSetOption, NULL);
			lv_obj_set_size(cbDispSelect_screenHor, 160, 15);
		//	lv_cb_set_text(cbDispSelect_screenHor, " Horizontal");
			lv_cb_set_text(cbDispSelect_screenHor, labelStrGet_bySysLanguage(tls_mSetting_horizontal));
			lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_POS);
			lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_FOLLOW);
			lv_obj_align(cbDispSelect_screenHor, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, coordOffset_cLanguage_x + screenLandscapeCoordinateObjOffset_temp + 35, 31);
			(devStatusRecordFlg_temp.devScreenLandscape_IF)?
				(lv_cb_set_checked(cbDispSelect_screenHor, true)):
				(lv_cb_set_checked(cbDispSelect_screenHor, false));
			lv_cb_set_style(cbDispSelect_screenVer, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
			lv_cb_set_style(cbDispSelect_screenHor, LV_CB_STYLE_BG, stylePageCb_screenLandscape);
			lv_cb_set_action(cbDispSelect_screenVer, funCb_cbOpreat_scrrenDispMethod_ver);
			lv_cb_set_action(cbDispSelect_screenHor, funCb_cbOpreat_scrrenDispMethod_hor);
			opreatTemp_screenLandscapeIf = devStatusRecordFlg_temp.devScreenLandscape_IF;
			lvObjAlign_temp = textSettingA_screenLandscape;

			#if(SCREENSAVER_RUNNING_ENABLE == 1)
				textSettingA_epidemicReport = lv_label_create(page_funSetOption, NULL);
			//	lv_label_set_text(textSettingA_epidemicReport, "COVID19 report:");
				lv_label_set_text(textSettingA_epidemicReport, labelStrGet_bySysLanguage(tls_mSetting_epidemicReport));
				lv_obj_set_style(textSettingA_epidemicReport, styleText_menuLevel_B);
				lv_obj_set_protect(textSettingA_epidemicReport, LV_PROTECT_POS);
				switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(textSettingA_epidemicReport, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 60);break;
					default:lv_obj_align(textSettingA_epidemicReport, lvObjAlign_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);break;}
				swSettingA_epidemicReportIF = lv_sw_create(page_funSetOption, NULL);
				lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_BG, bg_styleDevStatusRecordIF);
				lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_INDIC, indic_styleDevStatusRecordIF);
				lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_KNOB_ON, knob_on_styleDevStatusRecordIF);
				lv_sw_set_style(swSettingA_epidemicReportIF, LV_SW_STYLE_KNOB_OFF, knob_off_styleDevStatusRecordIF);
				lv_obj_set_protect(swSettingA_epidemicReportIF, LV_PROTECT_POS);
				switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(swSettingA_epidemicReportIF, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 65, 20);break;
					default:lv_obj_align(swSettingA_epidemicReportIF, textSettingA_epidemicReport, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);break;}
				lv_sw_set_anim_time(swSettingA_epidemicReportIF, 100);
				if(devStatusRecordFlg_temp.screensaverRunning_IF)lv_sw_on(swSettingA_epidemicReportIF);
				else lv_sw_off(swSettingA_epidemicReportIF);
				lv_sw_set_action(swSettingA_epidemicReportIF, funCb_epidemicReportIF);
				
				lvGui_settingSet_epidemicCtrlObj_opt(devStatusRecordFlg_temp.screensaverRunning_IF);
			#endif
		}break;

		case 4:{
			textSettingA_restartAllDevices = lv_label_create(page_funSetOption, NULL);
			lv_label_set_text(textSettingA_restartAllDevices, "Reboot system:");
			lv_obj_set_style(textSettingA_restartAllDevices, styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_restartAllDevices, LV_PROTECT_POS);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(textSettingA_restartAllDevices, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
				default:lv_obj_align(textSettingA_restartAllDevices, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
			btnSettingA_rebootAllDevices = lv_btn_create(page_funSetOption, NULL);
			lv_obj_set_size(btnSettingA_rebootAllDevices, 60, 20);
			lv_obj_set_protect(btnSettingA_rebootAllDevices, LV_PROTECT_POS);
			lv_btn_set_action(btnSettingA_rebootAllDevices, LV_BTN_ACTION_CLICK, funCb_btnActionClick_rebootAllDevices);
			switch(devLanguageCountry_identifyGet()){
				case languageCyId_Arabic:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
				default:lv_obj_align(btnSettingA_rebootAllDevices, textSettingA_restartAllDevices, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
			lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
			lv_btn_set_style(btnSettingA_rebootAllDevices, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
			textBtnRef_rebootAllDevices = lv_label_create(btnSettingA_rebootAllDevices, NULL);
			lv_label_set_recolor(textBtnRef_rebootAllDevices, true);
			lv_label_set_text(textBtnRef_rebootAllDevices, "#00FF40 >>>o<<<#");
			lv_obj_set_style(textBtnRef_rebootAllDevices, styleText_menuLevel_B);
			lv_obj_set_protect(textBtnRef_rebootAllDevices, LV_PROTECT_POS);
			lv_obj_align(textBtnRef_rebootAllDevices, NULL, LV_ALIGN_CENTER, 0, 0);
			lvObjAlign_temp = textSettingA_restartAllDevices;
		}break;

		case 5:{
			if(true == upgradeAvailable_flg){
				textSettingA_fwareUpgrade = lv_label_create(page_funSetOption, NULL);
				lv_label_set_text(textSettingA_fwareUpgrade, labelStrGet_bySysLanguage(tls_mSetting_upgrade));
				lv_obj_set_style(textSettingA_fwareUpgrade, styleText_menuLevel_B);
				lv_obj_set_protect(textSettingA_fwareUpgrade, LV_PROTECT_POS);
				switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(textSettingA_fwareUpgrade, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 20);break;
					default:lv_obj_align(textSettingA_fwareUpgrade, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);break;}
				btnSettingA_fwareUpgrade = lv_btn_create(page_funSetOption, NULL);
				lv_obj_set_size(btnSettingA_fwareUpgrade, 60, 20);
				lv_obj_set_protect(btnSettingA_fwareUpgrade, LV_PROTECT_POS);
				lv_btn_set_action(btnSettingA_fwareUpgrade, LV_BTN_ACTION_CLICK, funCb_btnActionClick_fwareUpgrade);
				switch(devLanguageCountry_identifyGet()){
					case languageCyId_Arabic:lv_obj_align(btnSettingA_fwareUpgrade, textSettingA_fwareUpgrade, LV_ALIGN_OUT_BOTTOM_RIGHT, 0 - screenLandscapeCoordinate_objOffset - 60, 20);break;
					default:lv_obj_align(btnSettingA_fwareUpgrade, textSettingA_fwareUpgrade, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);break;}
				lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_REL, styleBtn_specialTransparent);
				lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_PR, styleBtn_specialTransparent);
				lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_REL, styleBtn_specialTransparent);
				lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_PR, styleBtn_specialTransparent);
				textBtnRef_fwareUpgrade = lv_label_create(btnSettingA_fwareUpgrade, NULL);
				lv_label_set_recolor(textBtnRef_fwareUpgrade, true);
				lv_label_set_text(textBtnRef_fwareUpgrade, "#00FF40 >>>o<<<#");
				lv_obj_set_style(textBtnRef_fwareUpgrade, styleText_menuLevel_B);
				lv_obj_set_protect(textBtnRef_fwareUpgrade, LV_PROTECT_POS);
				lv_obj_align(textBtnRef_fwareUpgrade, NULL, LV_ALIGN_CENTER, 0, 0);
			}
		}break;

		default:break;
	}
}

void guiBussinessMenuSettingSubInterfaceRemove(void){

	currentGui_elementClear();
}

void lvGui_devTypeExSetElementRefresh_export(void){

	if(NULL != page_funSetOption){
		lvGui_devTypeExSetElementRefresh();
		// printf(">>>lvGui_devTypeExSetElementRefresh().\n");
	}
}


