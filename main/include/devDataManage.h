/**
 * @file devDataManage.h
 *
 */

#ifndef DEVDATAMANAGE_H
#define DEVDATAMANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "mwifi.h"

#include "lvgl.h"

#include "os.h"

// /* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "devLanguageTab.h"

/**************************************
 *     SYSTEM FUNCTION DEFINE		  *
 **************************************/	
#define SYSFUNC_XQUEUESENDFROMISR_WITCH_FULLCHECK(a, b, c)		if(pdFALSE == xQueueIsQueueFullFromISR(a)){xQueueSendFromISR(a, b, c);}

#define USR_ERROR_CHECK(con, err, format, ...) do { \
        if (con) { \
            if(*format != '\0') \
                MDF_LOGW("<%s> " format, mdf_err_to_name(err), ##__VA_ARGS__); \
        } \
    } while(0)
/**************************************
 *     SWITCH VERSION CONTROL DEFINES *
 **************************************/
//#define L8_DEVICE_VERSION						0x08
#define L8_DEVICE_VERSION						0x18 //开关设备版本号（设备版本号高四位自增向后迭代，高四位为0，语音控制没有高位区分，要慢一点）//v1.0.24以后，此版本号报废，版本号以下面的 L8_DEVICE_VERSION_REF_DISCRIPTION 为准

#define L8_DEVICE_VERSION_REF_DISCRIPTION		"L8-v1.1.17[202104021600]" //固件说明描述版本号

#define SCREENSAVER_RUNNING_ENABLE				1	 //屏保显示使能

#define LVAPP_DISP_ELECPARAM_HIDDEN_EN			0	 //电量功能隐藏
#define LVAPP_DISP_MENU_TIMER_DELAYER_EN		1	 //定时及延时选项使能

#define DEVICE_MAC_ADDR_APPLICATION_LEN			6

#define DEVLIST_MANAGE_LISTNUM_MASK_NULL		0xFF	//设备数量为空时 掩码

#define L8_MESH_CONMUNICATE_RESTART_NOTICE_STR	"restart"
#define L8_MESH_CONMUNICATE_UPGSTBY_NOTICE_STR	"upgrade stand by"

#define DATAMANAGE_LANGUAGE_TAB_MAXNUM			20
#define DATAMANAGE_LANGUAGE_ABBRE_MAXLEN		6

#define COUNTER_DISENABLE_MASK_SPECIALVAL_U32	4294967295 	//32位计时counter失效掩码值
#define COUNTER_DISENABLE_MASK_SPECIALVAL_U16	65535		//16位计时counter失效掩码值
#define COUNTER_DISENABLE_MASK_SPECIALVAL_U8	255			//8位计时counter失效掩码值

#define COUNTER_COUNTING_DOWN_LEGAL_U32(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U32){if(x)x --;}	
#define COUNTER_COUNTING_DOWN_LEGAL_U16(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U16){if(x)x --;}	
#define COUNTER_COUNTING_DOWN_LEGAL_U8(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){if(x)x --;}	

#define DEVICE_MUTUAL_CTRL_GROUP_NUM			3		//设备互控组数量
#define MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP	5		//单个互控组内最多设备数量
#define DEVICE_MUTUALGROUP_INVALID_INSERT_A		0		//互控组号 无效值A:0
#define DEVICE_MUTUALGROUP_INVALID_INSERT_B		0xFF	//互控组号 无效值A:0xFF

#define DEV_AUTOMATION_SCENE_MAX_NUM			16		//自动化最大数目

#define L8_DEVICE_TYPEDEFINE_BASE_ADDR		(0x49)

#define TOUCHEVENT_FLG_BITHOLD_RESERVE		(0x00FF)

#define TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG	(1 << 0)
#define TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP	(1 << 1)

#define FTIC_EXTRA_REGISTERVAL_GES_mUP		0x10
#define FTIC_EXTRA_REGISTERVAL_GES_mDN		0x18
#define FTIC_EXTRA_REGISTERVAL_GES_mLT		0x14
#define FTIC_EXTRA_REGISTERVAL_GES_mRT		0x1C
#define FTIC_EXTRA_REGISTERVAL_GES_zIN		0x48
#define FTIC_EXTRA_REGISTERVAL_GES_zOUT		0x49

#define DEV_RUNNING_FLG_BIT_TIMER			(1 << 0)
#define DEV_RUNNING_FLG_BIT_DELAY			(1 << 1)
#define DEV_RUNNING_FLG_BIT_GREENMODE		(1 << 2)
#define DEV_RUNNING_FLG_BIT_NIGHTMODE		(1 << 3)
#define DEV_RUNNING_FLG_BIT_DEVLOCK			(1 << 4)
#define DEV_RUNNING_FLG_BIT_DEVHOSTIF		(1 << 5)
#define DEV_RUNNING_FLG_BIT_RESERVE_3		(1 << 6)
#define DEV_RUNNING_FLG_BIT_RESERVE_4		(1 << 8)
#define DEV_RUNNING_FLG_BIT_RESERVE_5		(1 << 9)
#define DEV_RUNNING_FLG_BIT_RESERVE_6		(1 << 10)
#define DEV_RUNNING_FLG_BIT_RESERVE_7		(1 << 11)
#define DEV_RUNNING_FLG_BIT_RESERVE_8		(1 << 12)
#define DEV_RUNNING_FLG_BIT_RESERVE_9		(1 << 13)
#define DEV_RUNNING_FLG_BIT_RESERVE_10		(1 << 14)
#define DEV_RUNNING_FLG_BIT_RESERVE_11		(1 << 15)

#define DEV_HEX_PROTOCOL_APPLEN_CONTROL			1	//16进制字节格式协议应用数据长度:普通控制
#define DEV_HEX_PROTOCOL_APPLEN_DEVLOCK			1	//16进制字节格式协议应用数据长度:设备锁定操作
#define DEV_HEX_PROTOCOL_APPLEN_TIMERSET		24	//16进制字节格式协议应用数据长度:普通定时设置
#define DEV_HEX_PROTOCOL_APPLEN_DELAYSET		3	//16进制字节格式协议应用数据长度:延时设置
#define DEV_HEX_PROTOCOL_APPLEN_GREENMODESET	2	//16进制字节格式协议应用数据长度:绿色模式设置
#define DEV_HEX_PROTOCOL_APPLEN_NIGHTMODESET	6	//16进制字节格式协议应用数据长度:夜间模式设置
#define DEV_HEX_PROTOCOL_APPLEN_EXTPARAMSET		5	//16进制字节格式协议应用数据长度:开关除普通控制外的额外参数设置

#define L8_NODEDEV_KEEPALIVE_PERIOD				249	//mesh内部心跳子节点生命周期

#define L8_DEV_LISTMANAGE_REALES_CONFIRM		40	//mesh内部心跳管理表确认可用时间，单位：s

#define DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV	5	//设备设置定时业务开关响应值时，滑动条操作分量

#define DEV_MESH_PSD_DEFAULT()					{"psd456456123"}

#define SCALE_LOCATION_INFO_STR_MAX_LENGTH		128

#define GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE		40
#define GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE	   (120 * 20) //图片动态内存大小定义
#define MQTT_REMOTE_DATATRANS_PIC_DATABAG_LENGTH		976

#define GUI_BUSSINESS_HOME_BGROUND_PIC_PIXEL_SIZE	   (240 * 320) //背景底图像素大小

#define MQTT_REMOTE_DATATRANS_PIC_DATABAG_LENGTH_S		768
//#define MQTT_REMOTE_DATATRANS_PARAM_IP_DEF			{47, 52, 5, 108}
//#define MQTT_REMOTE_DATATRANS_PARAM_HOST_DEF			"112.124.61.191"
#define MQTT_REMOTE_DATATRANS_PARAM_HOST_DEF			"www.lanbonserver.com"
#define MQTT_REMOTE_DATATRANS_PARAM_PORT_DEF			8888
//#define MQTT_REMOTE_DATATRANS_PARAM_PORT_DEF			1883

#define MQTT_REMOTE_DATATRANS_USERNAME_DEF				"lanbon"
#define MQTT_REMOTE_DATATRANS_PASSWORD_DEF				"lanbon2019."

/********************************
 *     DEVIVE VERSION DEFINES 
 ********************************/	
#define PROGMA_FLASH_PARTITIONS_OFFSET_A		0x00
#define PROGMA_FLASH_PARTITIONS_OFFSET_B		0x20
#define DEVICE_FLASH_PARTITIONS_OFFSET			PROGMA_FLASH_PARTITIONS_OFFSET_B

#define DEVICE_DRIVER_METHOD_BY_SLAVE_MCU				0x01
#define DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE	0x02
	
#define DEVICE_DRIVER_DEFINITION				DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST		0
#endif

 #define DEV_TYPES_PANEL_DEF_NULL				0

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE) //--是否为磁保持继电器版本

 #define DEV_TYPES_PANEL_DEF_ALL_RESERVE		0xFA  //面板类型定义：所有可选
 #define DEV_TYPES_PANEL_DEF_SHARE_MIX			(0x01 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：六合一（多位开关和窗帘 ）
 #define DEV_TYPES_PANEL_DEF_INDEP_FANS			(0x02 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 02 = 33)面板类型定义：仅风扇 
 #define DEV_TYPES_PANEL_DEF_INDEP_DIMMER		(0x03 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)				 面板类型定义：仅调光
 #define DEV_TYPES_PANEL_DEF_INDEP_HEATER		(0x04 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 04 = 36)面板类型定义：仅热水器

 #define DEV_TYPES_PANEL_DEF_LC_1GANG	   				(0x09 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 09 = 41)面板类型定义：仅大电流一位开关

 #define DEV_TYPES_PANEL_DEF_RGBLAMP_BELT		(0x0A + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 10 = 42)面板类型定义：仅RGB灯带
 #define DEV_TYPES_PANEL_DEF_RGBLAMP_BULB		(0x0B + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 11 = 43)面板类型定义：仅RGB灯泡

 #define DEV_TYPES_PANEL_DEF_INDEP_INFRARED		(0x11 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 17 = 49)面板类型定义：仅红外转发器
 #define DEV_TYPES_PANEL_DEF_INDEP_SOCKET		(0x12 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 18 = 50)面板类型定义：仅插座
 #define DEV_TYPES_PANEL_DEF_INDEP_MOUDLE		(0x13 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 19 = 51)面板类型定义：仅三位模块

 #define DEV_TYPES_PANEL_DEF_MULIT_THERMO		(0x14 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 20 = 52)面板类型定义：恒温器[AC 220v 没有电量检测]
 #define DEV_TYPES_PANEL_DEF_THERMO_INDP_A		(0x15 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 21 = 53)面板类型定义：恒温器[AC 24v 有电量检测]

 #define DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER	(0x16 + DEVICE_FLASH_PARTITIONS_OFFSET)	//(20H[32] + 22 = 54)面板类型定义：太阳能系统管理器

 #define DEV_TYPES_PANEL_DEF_GAS_DETECTOR		(0x71 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 113 = 145)面板类型定义：燃气报警器
 #define DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR		(0x73 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 115 = 147)面板类型定义：烟雾报警器
 #define DEV_TYPES_PANEL_DEF_PIR_DETECTOR		(0x75 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 117 = 149)面板类型定义：红外热释电报警器
 #define DEV_TYPES_PANEL_DEF_RELAY_BOX			(0x77 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 119 = 151)面板类型定义：继电器模块

 #define DEV_TYPES_PANEL_DEF_UART_MOUDLE		(0x79 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：串口通信模块（用作室内机）

#else

 #define DEV_TYPES_PANEL_DEF_ALL_RESERVE 	    0xFB  //面板类型定义：所有可选
 #define DEV_TYPES_PANEL_DEF_SHARE_MIX		    (0x05 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：六合一（多位开关和窗帘 ）
 #define DEV_TYPES_PANEL_DEF_INDEP_FANS		    (0x06 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅风扇 
 #define DEV_TYPES_PANEL_DEF_INDEP_DIMMER	    (0x07 + DEVICE_FLASH_PARTITIONS_OFFSET) //(20H[32] + 07 = 33)面板类型定义：仅调光
 #define DEV_TYPES_PANEL_DEF_INDEP_HEATER	    (0x08 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅热水器

 #define DEV_TYPES_PANEL_DEF_LC_1GANG 			(0x0C + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅大电流一位开关

 #define DEV_TYPES_PANEL_DEF_RGBLAMP_BELT	    (0x0D + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅RGB灯带
 #define DEV_TYPES_PANEL_DEF_RGBLAMP_BULB 		(0x0E + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅RGB灯灯泡

 #define DEV_TYPES_PANEL_DEF_INDEP_INFRARED		(0x18 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅红外转发器
 #define DEV_TYPES_PANEL_DEF_INDEP_SOCKET		(0x19 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅插座
 #define DEV_TYPES_PANEL_DEF_INDEP_MOUDLE		(0x1A + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅三位模块 

 #define DEV_TYPES_PANEL_DEF_MULIT_THERMO		(0x1B + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：仅恒温器
 #define DEV_TYPES_PANEL_DEF_THERMO_INDP_A		(0x1C + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：恒温器-特殊类型A

 #define DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER	(0x1D + DEVICE_FLASH_PARTITIONS_OFFSET)	//(--)面板类型定义：太阳能系统管理器

 #define DEV_TYPES_PANEL_DEF_GAS_DETECTOR		(0x72 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：燃气传感器	
 #define DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR	   	(0x74 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：烟雾报警器
 #define DEV_TYPES_PANEL_DEF_PIR_DETECTOR		(0x76 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：红外热释电报警器
 #define DEV_TYPES_PANEL_DEF_RELAY_BOX			(0x78 + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：继电器模块

 #define DEV_TYPES_PANEL_DEF_UART_MOUDLE		(0x7A + DEVICE_FLASH_PARTITIONS_OFFSET) //(--)面板类型定义：串口通信模块（用作室内机）
#endif

#define L8_DEVICE_TYPE_PANEL_DEF				DEV_TYPES_PANEL_DEF_INDEP_FANS //设备类型强制定义，记得调光类型一定改DEVICE_DRIVER_METHOD_BY_SLAVE_MCU，否则没用

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwThreeBit

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU) && (DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)
	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwTwoBit
 #else
 	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwThreeBit
 #endif

 	#define L8_HEADER_PARAM_DISP_TEMPERATURE_EN	0

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_fans

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_dimmer

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_heater

	#ifdef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #undef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #define LVAPP_DISP_ELECPARAM_HIDDEN_EN		1
	#endif

	#define L8_HEADER_PARAM_DISP_TEMPERATURE_EN	0

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_LC_1GANG)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_largeCurOneBit

//	#ifdef LVAPP_DISP_ELECPARAM_HIDDEN_EN
// 	 #undef LVAPP_DISP_ELECPARAM_HIDDEN_EN
// 	 #define LVAPP_DISP_ELECPARAM_HIDDEN_EN 	1
//	#endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BELT)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_rgbLampBelt

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BULB)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_rgbLampBulb

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_infrared

	#define INDEP_TEMPERATURE_MEARSURE_BY_DS18B20	1

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_socket

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_moudleSwCurtain

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_MULIT_THERMO)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_thermostatExtension

	#define DEVICE_THERMOSTAT_PEDESTAL_HEATER_RESERVE 1	 //是否使用热水器底座

	#if(DEVICE_THERMOSTAT_PEDESTAL_HEATER_RESERVE == 1)
	 #undef  L8_DEVICE_TYPE_DEFULT
	 #define L8_DEVICE_TYPE_DEFULT				devTypeDef_thermostat
	#endif

	#ifdef LVAPP_DISP_ELECPARAM_HIDDEN_EN
 	 #undef LVAPP_DISP_ELECPARAM_HIDDEN_EN
 	 #define LVAPP_DISP_ELECPARAM_HIDDEN_EN    1
	#endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_THERMO_INDP_A)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_thermostat

	#ifdef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #undef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #define LVAPP_DISP_ELECPARAM_HIDDEN_EN 	1
	#endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_voltageSensor

	#ifdef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #undef LVAPP_DISP_ELECPARAM_HIDDEN_EN
	 #define LVAPP_DISP_ELECPARAM_HIDDEN_EN 	1
	#endif

	#ifdef LVAPP_DISP_MENU_TIMER_DELAYER_EN
 	 #undef LVAPP_DISP_MENU_TIMER_DELAYER_EN
 	 #define LVAPP_DISP_MENU_TIMER_DELAYER_EN 	0
	#endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_GAS_DETECTOR)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_gasDetect

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_smokeDetect

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_PIR_DETECTOR)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_pirDetect

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RELAY_BOX)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_relayBox_1bit

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_UART_MOUDLE)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_uartMoudle

#endif

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_DIMMER)&&\
	  (L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_FANS)

//  #warning "当前类型不支持 SLAVE_MCU 定义，已切换到 SELF_HARDWARE 定义"
  #undef DEVICE_DRIVER_DEFINITION
  #define DEVICE_DRIVER_DEFINITION DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE
 #endif
#elif(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE)

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)||\
	  (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)

//  #warning "当前类型不支持 SELF_HARDWARE 定义，已切换到 SLAVE_MCU 定义"
  #undef DEVICE_DRIVER_DEFINITION
  #define DEVICE_DRIVER_DEFINITION DEVICE_DRIVER_METHOD_BY_SLAVE_MCU
 #endif
#endif

#if(SCREENSAVER_RUNNING_ENABLE == 1)

	#define SCREENSAVER_DISP_IDLE_TIME			20			//疫情屏保显示 空闲时间定义， 单位：s
	#define EPIDEMIC_DATA_REQ_PERIOD_TIME		1800		//疫情数据获取周期， 单位：s
#endif

#define STR_VERSION_REALES_REFERENCE_LAST_1		" 1.The L8switch border light can\n"\
												"   be set by user within the page\n"\
												"   'Setting' with the 'illumination set'.\n\n"\
												" 2.'Homeassistant' supported, enter\n"\
												"   Settings to enable this function.\n\n"\
												" 3.Add device management capabilities,\n"\
												"   enter Settings to enable this\n"\
												"   function.\n\n"\
												" 4.Significantly improved crash\n"\
												"   restart issues.\n\n"\
												" 5.The L8 mesh network is compatible with\n"\
												"   the new device \"Voltage Sensor\".\n\n"

#define STR_VERSION_REALES_REFERENCE_LAST_2		" 1.The Bezel light can be set, in the\n"\
												"   option 'Decorative lamp set' of \n"\
												"   the setting page.\n\n"\
												" 2.Add a new system languages, Portuguese\n\n"\
												" 3.Fix the issue that mutual control cannot\n"\
												"   be triggered during voice control.\n\n"

#define STR_VERSION_REALES_REFERENCE_DFT		" 1.Optimize touch accuracy and improve \n"\
																											"   experience;\n\n"\
																											" 2.Re-make some interface effects;\n\n"\
																											" 3.Improve system stability.\n\n"\

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT
												
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_MULIT_THERMO)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_LC_1GANG)

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)

	#define STR_VERSION_REALES_REFERENCE_T		" 1.Turn off voltage is divided into\n"\
																										"   three stages.\n\n"\
																										" 2.The screen display confusion problem is\n"\
																										"   completely solved.\n\n"\

	#define STR_VERSION_REALES_REFERENCE		STR_VERSION_REALES_REFERENCE_DFT

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BELT)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BULB)

#endif
		

/**********************
 *      TYPEDEFS
 **********************/
 enum{ //窗帘专用动作描述值枚举
 
	 devCurtain_actEnum_Open = 0,
	 devCurtain_actEnum_Puase,
	 devCurtain_actEnum_Close,
 }devCurtain_actEnum;
 
 enum{ //风扇专用动作描述值枚举
 
	 devFans_swEnum_Close = 0,
	 devFans_swEnum_sw1,
	 devFans_swEnum_sw2,
	 devFans_swEnum_sw3,
 }devFans_swEnum;

 enum{ //区分语言部分文字国家标识

	countryT_EnglishSerail = 0,
	countryT_Arabic,
	countryT_Hebrew,
 }countryTextPtrEnum;

 enum{ //节点状态
 
	 roleMeshStatus_noConnect = 0,
	 roleMeshStatus_root,
	 roleMeshStatus_node,
 };

 typedef union{
 
	 struct{
 
		 uint8_t swVal_bit1:1; 

		 uint8_t rsv:6;

		 uint8_t opCtrl_bit1:1; 
	 }devType_mulitSwitch_oneBit;
 
	 struct{

		 uint8_t swVal_bit1:1; 
		 uint8_t swVal_bit2:1; 

		 uint8_t rsv:4;

		 uint8_t opCtrl_bit1:1; 
		 uint8_t opCtrl_bit2:1; 
	 }devType_mulitSwitch_twoBit;
 
	 struct{
 
		 uint8_t swVal_bit1:1; 
		 uint8_t swVal_bit2:1; 
		 uint8_t swVal_bit3:1; 

		 uint8_t rsv:2;

		 uint8_t opCtrl_bit1:1; 
		 uint8_t opCtrl_bit2:1; 
		 uint8_t opCtrl_bit3:1; 
	 }devType_mulitSwitch_threeBit;
 
	 struct{
 
		 uint8_t devCurtain_actEnumVal:7;
		 uint8_t devCurtain_actMethod:1; // 1:滑条， 0:按钮
	 }devType_curtain;

	 struct{
 
		 uint8_t devHeater_swEnumVal;
	 }devType_heater;

	 struct{

	 	 uint8_t devThermostat_running_en:1; //bit0
	 	 uint8_t devThermostat_nightMode_en:1; //bit1
		 uint8_t devThermostat_tempratureTarget:6; //bit2-7
	 }devType_thermostat;

	 struct{
 
		 uint8_t devScenario_opNum;
	 }devType_scenario;

	 struct{
 
		 uint8_t devFans_swEnumVal;
	 }devType_fans;
 
	 struct{
 
		 uint8_t devDimmer_brightnessVal;
	 }devType_dimmer;

	 struct{

		 uint8_t devInfrared_irIst:7;
		 uint8_t devInfrared_actCmd:1; // 1:学习， 0:控制
	 }devType_infrared;

	 struct{
		
		 uint8_t devSocket_opSw:1;
	 }devType_socket;

	 struct{
		
		 uint8_t solarSysManager_voltageCur:7; //bit0 - bit6 : 实时电池电压
		 uint8_t solarSysManager_tempAlarmFlg:1; //bit7 : 温度报警标志
	 }devType_solarSysManager;

	 struct{

		uint8_t devRgbLamp_enable:1;
		uint8_t rsv:7;
	 }devType_rgbLamp;

	 struct{

		uint8_t gasConcentration:7; //浓度
		uint8_t alarmFlg:1; //报警标志
	 }devType_gasDetector;

	 struct{
	 
		uint8_t rsv:6; //保留
		uint8_t alarmFlg_smoke:1; //报警标志:烟雾
		uint8_t alarmFlg_co:1; //报警标志:一氧化碳
	 }devType_smokeDetector;

	 struct{
	 
		uint8_t rsv:7; //保留
		uint8_t alarmFlg:1; //报警标志
	 }devType_pirDetector;
 }stt_devDataPonitTypedef;

#define DEV_DETECTING_ALARM_DATA_LEN	32
typedef struct{

	uint8_t devType;
	//此处有一个byte的对齐
	uint16_t devTimeZone;
	uint8_t devAddr[MWIFI_ADDR_LEN];
	uint16_t devFirmwareVer;
	uint8_t alarmParam[DEV_DETECTING_ALARM_DATA_LEN];
}stt_devDetectingDataDef; //length为44
 
typedef enum{

	devTypeDef_null = 0,
	devTypeDef_mulitSwOneBit = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x01),
	devTypeDef_mulitSwTwoBit,
	devTypeDef_mulitSwThreeBit,
	devTypeDef_dimmer,
	devTypeDef_fans,
	devTypeDef_scenario,
	devTypeDef_curtain,
	devTypeDef_infrared,
	devTypeDef_socket,
	devTypeDef_moudleSwOneBit,
	devTypeDef_moudleSwTwoBit,
	devTypeDef_moudleSwThreeBit,
	devTypeDef_moudleSwCurtain,
	devTypeDef_relayBox_1bit = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x18), //97
	devTypeDef_relayBox_2bit,
	devTypeDef_relayBox_curtain,
	devTypeDef_thermostat = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x1E), //103
	devTypeDef_heater = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x1F),
	devTypeDef_thermostatExtension,
	devTypeDef_voltageSensor,
	devTypeDef_largeCurOneBit,
	devTypeDef_rgbLampBelt,
	devTypeDef_rgbLampBulb,
	devTypeDef_gasDetect,
	devTypeDef_smokeDetect,
	devTypeDef_pirDetect,
	devTypeDef_uartMoudle, //串口模块，室内机通信专用
}devTypeDef_enum;

typedef enum{

	dataManagement_msgType_null = 0,
	dataManagement_msgType_homePageCtrlObjTextChg,
	dataManagement_msgType_homePageCtrlObjIconChg,
	dataManagement_msgType_homePageThemeTypeChg,
	dataManagement_msgType_homePagePicGroundChg
}enumMsgType_dataManagement;

typedef struct{

	enumMsgType_dataManagement msgType;

	union _unn_dataDmHandle{

		struct _sttData_hpCtrlObjTextChg{

			uint8_t objChg_bitHold;
		}dataAb_hpCtrlObjTextChg;

		struct _sttData_hpCtrlObjIconChg{

			uint8_t objChg_bitHold;
		}dataAb_hpCtrlObjIconChg;

		struct _sttData_hpThemeTypeChg{

			uint8_t themeTypeChg_notice:1;
		}dataAb_hpThemeTypeChg;	

		struct _sttData_hpPicGroundChg{

			uint8_t picGroundChg_notice:1;
		}dataAb_hpPicGroundChg;	

	}msgData_dmHandle;

}stt_msgDats_dataManagementHandle;

typedef struct{

	uint8_t linkageRunning_proxmity_en:1;
	uint8_t linkageRunning_temprature_en:1;

	int8_t linkageCondition_tempratureVal;

	stt_devDataPonitTypedef linkageReaction_proxmity_swVal;
	uint8_t 				linkageReaction_proxmity_trigEn:1;
	
	uint8_t 				linkageReaction_proxmity_scrLightTrigIf:1;
	stt_devDataPonitTypedef linkageReaction_temprature_swVal;
	
}stt_paramLinkageConfig;

typedef struct{

	const uint16_t loopPeriod;
	uint16_t loopCounter;
}stt_timerLoop;

typedef struct{

	uint16_t loopPeriod;
	uint16_t loopCounter;
}stt_timerLoop_ext;

typedef struct{

	const uint32_t loopPeriod;
	uint32_t loopCounter;
}stt_timerLoop32BitCnt;

typedef struct{

	uint32_t loopPeriod;
	uint32_t loopCounter;
}stt_timerLoop32BitCnt_ext;

typedef struct{

	uint8_t mutualCtrlGroup_insert; //互控组号
	uint8_t mutualCtrlDevNum; //该互控组内设备数量
	uint8_t mutualCtrlDevMacList[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP * MWIFI_ADDR_LEN]; //该互控组内设备MAC地址列表
}stt_devMutualGroupParam;

typedef struct{

	uint8_t routerInfo_ssid[32];
	uint8_t routerInfo_psd[64];
}stt_routerCfgInfo;

#define MQTT_HOST_DOMAIN_STRLEN	96
typedef struct{

	uint8_t  host_domain[MQTT_HOST_DOMAIN_STRLEN];
	uint16_t port_remote;
}stt_mqttCfgParam;

typedef struct{

	stt_mqttCfgParam hostConnServer;
	char usrName[32];
	char usrPsd[64];
}stt_mqttExServerCfgParam;

typedef struct{

	uint8_t devNum;
	uint8_t mGroupIst;
	struct stt_mutualInfoUnitDev{

		uint8_t devUnitMAC[MWIFI_ADDR_LEN];
		uint8_t devMutualBitIst;
		
	}unitDevMutualInfo[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP];
}stt_mutualCtrlInfoResp;

 typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devTempParam2Hex;

typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devPowerParam2Hex;

typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devElecsumParam2Hex;

typedef struct{

	uint8_t devTypedef;
	uint8_t devVersion;
	stt_devDataPonitTypedef devStatus_valCurrent;
	stt_devTempParam2Hex devFuncInfo_temprature;
	stt_devPowerParam2Hex devFuncInfo_elecPower;
}stt_blufiConfigDevInfo_resp;

#define DEVPARAMEXT_DT_LEN	4
typedef struct{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	uint8_t nodeDev_Type;
	uint8_t nodeDev_Status;
	uint16_t nodeDev_DevRunningFlg;
	stt_devTempParam2Hex nodeDev_dataTemprature;
	stt_devPowerParam2Hex nodeDev_dataPower;	
	uint8_t nodeDev_extFunParam[DEVPARAMEXT_DT_LEN];
}stt_devStatusInfoResp; //与stt_hbDataUpload不同，只获取需要的属性进行回复

typedef struct{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	stt_devElecsumParam2Hex nodeDev_dataElecsum;
}stt_devUnitElecsumReport;

typedef struct{

	uint16_t devStatusOnOffRecord_IF:1; //是否记忆开关状态
	uint16_t devElecsumDisp_IF:1; //电路参数显示类别（1：显示电流 0：显示功率）
	uint16_t devScreenLandscape_IF:1; //是否横屏（1：横屏 0：竖屏）
	uint16_t devUpgradeFirstRunning_FLG:1; //是否为第一次启动
	uint16_t screensaverRunning_IF:1; //屏保运行使能
	uint16_t homeassitant_En:1; //HA功能使能
	uint16_t devListManager_En:1;
	uint16_t devTempDispMethod:1; //设备温度表示方式（1：华氏度 0：摄氏度）
	uint16_t devStatusData_reserve:8; //reserve
}stt_devStatusRecord;

typedef struct{

	uint32_t sysParam_meshRoleForceAsChild:1;
	uint32_t sysParam_reserve:31;
}stt_devSystemKeyParamRecord;

typedef struct{

	uint8_t unitDevMac[MWIFI_ADDR_LEN];
	uint8_t unitDevOpreat_val;
}stt_scenarioUnitOpreatParam;

typedef struct{

	uint8_t scenarioInsert_num; //场景索引
	uint8_t scenarioDevice_sum; //场景内设备数量
}stt_scenarioSwitchData_nvsOpreatRef; 
/*单个场景数据最大存储长度：sizeof(stt_scenarioSwitchData_nvsOpreatRef) + 128 + 128*/ //数据说明头 + 数据上半部 + 数据下半部

typedef struct{

   uint8_t targetDevMac[MWIFI_ADDR_LEN];
   uint8_t devStateSet;
}stt_superCtrl_dtTransParam;

#define DEVSCENARIO_NVSDATA_HALFOPREAT_NUM		128 //场景数据半部存储数量
typedef struct{

	stt_scenarioSwitchData_nvsOpreatRef dataRef;
	stt_scenarioUnitOpreatParam dataHalf_A[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
	stt_scenarioUnitOpreatParam dataHalf_B[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
}stt_scenarioSwitchData_nvsOpreat;

typedef enum{ //触发类别

	autoTrigType_null = 0,
	autoTrigType_sunScale = 0xA0,
	autoTrigType_devState = 0xA1,
}enumAutomationTrigConditionType;

typedef struct{

	uint8_t timeWeekBitHold;
	uint8_t timeA_h;
	uint8_t timeA_m;
	uint8_t timeB_h;
	uint8_t timeB_m;	
}stt_aramAutomationEffectTime;

#define DEVAUTOMATION_TRIG_CONDITION_VAL_LEN	8
typedef struct{ //自动化操作参数

	uint8_t serialNum;
	uint8_t existMask;
	stt_aramAutomationEffectTime effTime;
	uint8_t automationTrigType;
	uint8_t conditionDevMac[MWIFI_ADDR_LEN];
	uint8_t conditionValue[DEVAUTOMATION_TRIG_CONDITION_VAL_LEN];
	uint8_t sceneDevNum;
	stt_scenarioUnitOpreatParam sceneParam[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
}stt_paramAutomationExcution;

typedef struct{ //nvs操作专用

	uint8_t opNum;
	stt_paramAutomationExcution *dats;
}stt_automationExcutionNvsOp;

typedef struct{

	uint8_t unitDevMac[MWIFI_ADDR_LEN];
	uint8_t unitDevStateRcd;
	uint8_t unitDevTFCD_counter:7;
	uint8_t unitDevTFCD_counterEn:1;
}stt_solarSysManager_ctrlUnit;

typedef struct{

	uint8_t devUnit_Sum;
	stt_solarSysManager_ctrlUnit dataHalf_A[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
	stt_solarSysManager_ctrlUnit dataHalf_B[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
}stt_solarSysManagerDevList_nvsOpreat;

typedef struct sttListAttrSolarSysManager{

	stt_solarSysManager_ctrlUnit paramCtrlUnit;
	struct sttListAttrSolarSysManager *next;
}stt_listAttrSolarSysManager;

#define DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN 	232 //红外转单条指令操作数据缓存
typedef struct{

	uint8_t ir_dataOpreat_buf[DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN];
}stt_infraredSwitchData_nvsOpreat;

typedef struct{

	stt_infraredSwitchData_nvsOpreat dats;
	char 							 nvsKeyWord[16];
}stt_devInfraredNvsOpreatParam;

#define GUIBUSSINESS_CTRLOBJ_MAX_NUM 3
typedef struct{

	uint8_t countryFlg;
	uint8_t dataBtnTextDisp[GUIBUSSINESS_CTRLOBJ_MAX_NUM][GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE];
}stt_dataDisp_guiBussinessHome_btnText;

typedef struct{

	uint8_t    valGestrue;

	uint8_t    touchNum;
	lv_point_t touchPosition[5];
}stt_touchEveInfo;

typedef struct{

	uint8_t bGround_keyTheme_ist;
	uint8_t bGround_picOrg_ist;

	union unn_devUiKindIndex{
		uint8_t devFan_uiKindIndex;
	}devUiKindIndex;
}stt_bGroundThemeParam;

typedef enum{

	saveObj_swStatus = 0,
	saveObj_infoTimer_normal,
	saveObj_infoTimer_nightMode,
	saveObj_infoTimer_greenMode,
	saveObj_devTimeZone,
	saveObj_devEelcSum,
	saveObj_devRunning_flg,
	saveObj_devTypeDef,
	saveObj_devStatusRecordIF,
	saveObj_devSysKeyParam,
	saveObj_routerBssid,
	saveObj_routerBssidRcd,
	saveObj_mutualCtrlInfo,
	saveObj_devDriver_linkageConfigParam_set,
	saveObj_devGuiBussinessHome_btnTextDisp,
	saveObj_devGuiBussinessHome_btnIconDisp,
	saveObj_devGuiBussinessHome_btnTextPic_A,
	saveObj_devGuiBussinessHome_btnTextPic_B,
	saveObj_devGuiBussinessHome_btnTextPic_C,
	saveObj_devGuiBussinessHome_bGroundPic,
	saveObj_devGuiBussinessHome_themeParam,

	saveObj_devDriver_atmosLightRunningParam_set,

	saveObj_devDriver_tempratureCal_valDat,
	saveObj_devDriver_iptRecalibration_set,
	saveObj_devDriver_screenRunningParam_set,

	saveObj_devCurtain_runningParam,

	saveObj_devDimmer_cfgParam,
	saveObj_devFan_cfgParam,

	saveObj_devInfrared_paramDats,
	saveObj_devInfrared_timerUpIstTab,

	saveObj_devThermostatExSw_statusDats,

	saveObj_devScenario_paramDats_0,
	saveObj_devScenario_paramDats_1,
	saveObj_devScenario_paramDats_2,

	saveObj_solarSysManager_devList,
	saveObj_solarSysManager_devOptParam,
	saveObj_solarSysManager_devCtrlRunningParam,

	saveObj_uartMoudleL7SysParam,

	saveObj_dtMqttCfgParam,
	saveObj_dtHaMqttCfgParam,

	saveObj_timeAutomationScenraioParam,

	saveObj_timeSunriseAndSunset,

	saveObj_systemLanguage,

	saveObj_routerConfigInfo,

	saveObj_devHeater_customTimeParam,

	saveObj_greenMode_usrCfg,

	saveObj_epidCyLocation,
	saveObj_sunScaleLocation,

	saveObj_paramMagRelayTest,

}enum_dataSaveObj;

typedef enum{

	caliStatus_noCfm = 0,
	caliStatus_Y,
	caliStatus_N,
}enum_touchCaliRsv;

#define DEV_CTRLOBJ_NAME_DETAILUD_LEN	12

/*-------------------------------开关控制基础显示块-------------------------------------------------*/
typedef struct struct_devCtrlObjGraphBlockDataBase{ //添加属性将导致 新旧版本 数据结构无法对齐

	uint8_t ctrlObj_devMac[MWIFI_ADDR_LEN];
	uint8_t ctrlObj_devType;

	uint8_t devStatusVal;

	uint8_t objIcon_ist[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	char objCtrl_name[GUIBUSSINESS_CTRLOBJ_MAX_NUM][DEV_CTRLOBJ_NAME_DETAILUD_LEN];

	lv_obj_t *graphBaseBlock;
	lv_obj_t *imgCtrlObj[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	LV_OBJ_FREE_NUM_TYPE lv_fNum_objImgCtrl[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	lv_obj_t *ctrlObjIcon[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	lv_obj_t *nameTextDispObj[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	
}stt_devCtrlObjGraphBlockDataBase;

typedef struct struct_devCtrlObjGraphBlockOperationParam{ //添加属性将导致 新旧版本 数据结构无法对齐

	lv_obj_t *objBaseBlockSelect_cb;
	LV_OBJ_FREE_NUM_TYPE lv_fNum_objBlockCb;
	uint8_t objBlockSelectStage:4;

}stt_devCtrlObjGraphBlockOperationParam;

typedef struct struct_nodeObj_listManageDevCtrlBase{ //添加属性将导致 新旧版本 数据结构无法对齐

	stt_devCtrlObjGraphBlockDataBase nodeData;
	stt_devCtrlObjGraphBlockOperationParam nodeOpParam;

	struct struct_nodeObj_listManageDevCtrlBase *next;
	
}stt_nodeObj_listManageDevCtrlBase;

/*-------------------------------开关属性数据管理链表 数据结构--------------------------------------*/
typedef struct struct_devInfoDetailUpload_2Server{

	uint8_t nodeDev_Version[2];

}stt_devInfoDetailUpload_2Server;

typedef struct struct_devInfoDetailUpload_2Root{

	uint8_t devType;
	uint8_t devSelf_iconIst[GUIBUSSINESS_CTRLOBJ_MAX_NUM];
	char devSelf_name[GUIBUSSINESS_CTRLOBJ_MAX_NUM][DEV_CTRLOBJ_NAME_DETAILUD_LEN];
	uint8_t devSelf_status;

}stt_devInfoDetailUpload_2Root;

typedef struct struct_devInfoDetailUpload_2Common{


}stt_devInfoDetailUpload_2Common;

typedef struct struct_devInfoDetailUpload{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];

	stt_devInfoDetailUpload_2Server data2Server;
	stt_devInfoDetailUpload_2Root data2Root;
	stt_devInfoDetailUpload_2Common dataCommon;

}stt_devInfoDetailUpload;

typedef struct struct_nodeDev_detailInfoManage{

	stt_devInfoDetailUpload dataManage;

	struct struct_nodeDev_detailInfoManage *next;
	
}stt_nodeDev_detailInfoManage;


/*-------------------------------心跳数据管理链表 数据结构--------------------------------------*/
typedef struct struct_hbDataUpload{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	uint8_t nodeDev_Type;
	uint8_t nodeDev_Status;
	uint8_t nodeDev_mautualInfo[DEVICE_MUTUAL_CTRL_GROUP_NUM];
	uint16_t nodeDev_runningFlg;
	stt_devTempParam2Hex nodeDev_dataTemprature;
	stt_devPowerParam2Hex nodeDev_dataPower;
	stt_devElecsumParam2Hex nodeDev_dataElecsum;
	uint8_t nodeDev_extFunParam[DEVPARAMEXT_DT_LEN];

}stt_hbDataUpload;

typedef struct struct_nodeDev_hbDataManage{

	stt_hbDataUpload dataManage;
	uint16_t nodeDevKeepAlive_counter;

	struct struct_nodeDev_hbDataManage *next;
	
}stt_nodeDev_hbDataManage;

#if(SCREENSAVER_RUNNING_ENABLE == 1)

 typedef struct{

	const uint8_t idlePeriod;
	uint8_t idleCounter;

	uint8_t flg_screensaverDataRefresh:1;

	uint8_t flg_screensaverPageCreat:1;
	uint8_t en_idleCounter:1;
 }stt_screensaverDispAttr;

 typedef struct{

	uint32_t epidData_cure;
	uint32_t epidData_confirmed;
	uint32_t epidData_deaths;

	const uint16_t reqTimePeriod;
	uint16_t reqTimeCounter;
 }stt_epidemicReqRunningAttr;
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern xQueueHandle msgQh_dataManagementHandle;

extern bool mutualCtrlTrigIf_A; 
extern bool mutualCtrlTrigIf_B;
extern bool mutualCtrlTrigIf_C;

extern const char meshPsd_default[20];

extern uint8_t *dataPtr_bGroundPic;
extern uint8_t bGroundPic_reserveFlg;

#if(SCREENSAVER_RUNNING_ENABLE == 1)
	
 extern stt_screensaverDispAttr screensaverDispAttrParam;
 extern stt_epidemicReqRunningAttr epidemicDataRunningParam;
#endif

//extern uint8_t *dataPtr_btnTextImg_sw_A;
//extern uint8_t *dataPtr_btnTextImg_sw_B;
//extern uint8_t *dataPtr_btnTextImg_sw_C;

uint8_t numCheckMethod_customLanbon(uint8_t *dats, uint8_t len);

void dispApplication_epidCyLocation_set(uint8_t cyIst, bool nvsRecord_IF);
uint8_t dispApplication_epidCyLocation_get(void);

uint8_t systemDevice_startUpTime_get(void);
uint16_t systemDevice_currentVersionGet(void);

void gui_bussinessHome_btnText_dataReales(uint8_t picIst, uint8_t *picDataBuf, uint8_t *dataLoad, uint16_t dataLoad_len, uint8_t dataBagIst, bool lastFrame_If);
void gui_bussinessHome_bGroundPic_dataReales(uint8_t *picDataBuf, uint8_t *dataLoad, uint16_t dataLoad_len, uint16_t dataBagIst, bool lastFrame_If);

void flgSet_gotRouterOrMeshConnect(bool valSet);
bool flgGet_gotRouterOrMeshConnect(void);

void devRouterRecordBssid_Set(uint8_t bssid[6], bool nvsRecord_IF);
void devRouterRecordBssid_Get(uint8_t bssid[6]);

void devRouterConnectBssid_Set(uint8_t bssid[6], bool nvsRecord_IF);
void devRouterConnectBssid_Get(uint8_t bssid[6]);

void usrMeshApplication_rootFirstConNoticeTrig(void);
bool usrMeshApplication_rootFirstConNoticeActionRserveGet(void);

void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if);
enum_touchCaliRsv usrApp_devIptdrv_paramRecalibration_get(void);

void usrAppHomepageBtnTextDisp_paramSet(stt_dataDisp_guiBussinessHome_btnText *param, bool nvsRecord_IF);
void usrAppHomepageBtnTextDisp_paramSet_specified(uint8_t objNum, 
															   uint8_t nameTemp[GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE], 
															   uint8_t nameLen, 
															   uint8_t cyFlg,
															   bool nvsRecord_IF);
void usrAppHomepageBtnTextDisp_defaultLoad(devTypeDef_enum devType, bool nvsRecord_IF);
void usrAppHomepageBtnTextDisp_paramGet(stt_dataDisp_guiBussinessHome_btnText *param);
uint8_t countryFlgGetByAbbre(char *countryAbbre);

void usrAppHomepageBtnIconNumDisp_paramSet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM], bool nvsRecord_IF);
void usrAppHomepageBtnIconNumDisp_defaultLoad(devTypeDef_enum devType, bool nvsRecord_IF);
void usrAppHomepageBtnIconNumDisp_paramGet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM]);
lv_img_dsc_t *usrAppHomepageBtnIconDisp_dataGet(uint8_t iconNum);

void devMeshSignalVal_Reales(void);
int8_t devMeshSignalFromParentVal_Get(void);
void devMeshNodeNum_Reales(void);
uint16_t devMeshNodeNum_Get(void);

void devMutualCtrlGroupInfo_Clr(void);
void devMutualCtrlGroupInfo_Set(stt_devMutualGroupParam *mutualGroupParam, uint8_t opreatBit, bool nvsRecord_IF);
void devMutualCtrlGroupInfo_Get(stt_devMutualGroupParam mutualGroupParam[DEVICE_MUTUAL_CTRL_GROUP_NUM]);
void devMutualCtrlGroupInfo_groupInsertGet(uint8_t groupInsert[DEVICE_MUTUAL_CTRL_GROUP_NUM]);
bool devMutualCtrlGroupInfo_unitCheckByInsert(stt_devMutualGroupParam *mutualGroupParamUnit, uint8_t *groupNum, uint8_t paramInsert);

void devSystemOpration_linkageConfig_paramSet(stt_paramLinkageConfig *param, bool nvsRecord_IF);
void devSystemOpration_linkageConfig_paramGet(stt_paramLinkageConfig *param);

uint16_t currentDevRunningFlg_paramGet(void);
void currentDevRunningFlg_paramSet(uint16_t valFlg, bool nvsRecord_IF);

void meshNetwork_connectReserve_IF_set(bool param);
bool meshNetwork_connectReserve_IF_get(void);

void mqttRemoteConnectCfg_paramSet(stt_mqttCfgParam *param, bool nvsRecord_IF);
void mqttRemoteConnectCfg_paramGet(stt_mqttCfgParam *param);

void mqttHaMqttServer_paramSet(stt_mqttExServerCfgParam *param, bool nvsRecord_IF);
void mqttHaMqttServer_paramGet(stt_mqttExServerCfgParam *param);

void currentRouterCfgInfo_paramSet(stt_routerCfgInfo *param, bool nvsRecord_IF);
void currentRouterCfgInfo_paramGet(stt_routerCfgInfo *param);

uint8_t nvsDataOpreation_devAutomationParam_set(stt_automationExcutionNvsOp *param);
void nvsDataOpreation_devAutomationParamGet_bySerialNum(stt_automationExcutionNvsOp *param);
void nvsDataOpreation_devAutomationParamGet_byStorageNum(stt_automationExcutionNvsOp *param);
void devAutomationSceneExcuteIndex_set(uint8_t index);
uint8_t devAutomationSceneExcuteIndex_get(void);

void devSunScaleLocationInfo_set(char *infoStr, bool nvsRecord_IF);
void devSunScaleLocationInfo_get(char *infoStr);

void devAutomationExcutionDetectJudge_byConditionDevState(uint8_t devMac[MWIFI_ADDR_LEN], 
																		 uint8_t devType,
																		 uint8_t devStateVal);
void devAutomationExcutionDetectJudge_byConditionSunScale(uint8_t timeWeek, 
																		 uint8_t timeScale_hour, 
																		 uint8_t timeScale_min);

stt_blufiConfigDevInfo_resp *devBlufiConfig_respInfoGet(void);

void nvsDataOpreation_devInfraredParam_set(uint8_t irDats[DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN], uint8_t cmdInsert);
stt_infraredSwitchData_nvsOpreat *nvsDataOpreation_devInfraredParam_get(uint8_t cmdInsert);

stt_scenarioSwitchData_nvsOpreat *nvsDataOpreation_devScenarioParam_get(uint8_t scenarioIst);

void devSystemInfoLocalRecord_preSaveTest(void);
void devSystemInfoLocalRecord_normalClear(void);
void devSystemInfoLocalRecord_allErase(void);
void devSystemInfoLocalRecord_initialize(void);
void devSystemInfoLocalRecord_save(enum_dataSaveObj obj, void *dataSave);

void L8devHbDataManageList_delSame(stt_nodeDev_hbDataManage *head);
uint8_t L8devHbDataManageList_nodeNumDetect(stt_nodeDev_hbDataManage *pHead);
uint8_t *L8devHbDataManageList_listGet(stt_nodeDev_hbDataManage *pHead);
stt_mutualCtrlInfoResp *L8devMutualCtrlInfo_Get(stt_nodeDev_hbDataManage *pHead, uint8_t mutualCtrlGroupIst);
uint8_t *L8DevListGet_Type1(stt_nodeDev_hbDataManage *pHead, uint8_t targetDevType);
uint8_t *L8devStatusInfoGet(stt_nodeDev_hbDataManage *pHead);
uint8_t *L8devElecsumInfoGet(stt_nodeDev_hbDataManage *pHead);
uint8_t *L8devInfoDetailManageList_data2ServerTabGet(stt_nodeDev_detailInfoManage *pHead);
uint8_t *L8devInfoDetailManageList_data2RootTabGet(stt_nodeDev_detailInfoManage *pHead);
void L8devHbDataManageList_listDestory(stt_nodeDev_hbDataManage *pHead);
uint8_t L8devHbDataManageList_nodeCreat(stt_nodeDev_hbDataManage *pHead, stt_nodeDev_hbDataManage *pNew);
stt_nodeDev_hbDataManage *L8devHbDataManageList_nodeGet(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method);
bool L8devHbDataManageList_nodeRemove(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]);
void L8devHbDataManageList_bussinessKeepAliveManagePeriod1s(stt_nodeDev_hbDataManage *pHead_hb, 
																		   stt_nodeDev_detailInfoManage *pHead_di,
																		   stt_nodeObj_listManageDevCtrlBase *pHead_gb);

uint8_t L8devInfoDetailManageList_nodeNumDetect(stt_nodeDev_detailInfoManage *pHead);
uint8_t L8devInfoDetailManageList_nodeCreat(stt_nodeDev_detailInfoManage *pHead, stt_nodeDev_detailInfoManage *pNew);
stt_nodeDev_detailInfoManage * L8devInfoDetailManageList_nodeGet(stt_nodeDev_detailInfoManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method);
bool L8devInfoDetailManageList_nodeRemove(stt_nodeDev_detailInfoManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]);
void L8devInfoDetailManageList_listDestory(stt_nodeDev_detailInfoManage *pHead);

uint8_t lvglUsrApp_devCtrlBlockBaseManageList_nodeNumDetect(stt_nodeObj_listManageDevCtrlBase *pHead);
uint8_t lvglUsrApp_devCtrlBlockBaseManageList_nodeAdd(stt_nodeObj_listManageDevCtrlBase *pHead, stt_nodeObj_listManageDevCtrlBase *pNew, bool onlyDataReales);
bool lvglUsrApp_devCtrlBlockBaseManageList_nodeRemove(stt_nodeObj_listManageDevCtrlBase *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]);
stt_nodeObj_listManageDevCtrlBase * lvglUsrApp_devCtrlBlockBaseManageList_nodeGet(stt_nodeObj_listManageDevCtrlBase *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method);
stt_nodeObj_listManageDevCtrlBase * lvglUsrApp_devCtrlBlockBaseManageList_nodeGetByLvObjFreenum(stt_nodeObj_listManageDevCtrlBase *pHead, LV_OBJ_FREE_NUM_TYPE fNumHead, bool method);
stt_nodeObj_listManageDevCtrlBase * lvglUsrApp_devCtrlBlockBaseManageList_nodeGetByLvSelectCbFreenum(stt_nodeObj_listManageDevCtrlBase *pHead, LV_OBJ_FREE_NUM_TYPE fNum, bool method);
void lvglUsrApp_devCtrlBlockBaseManageList_listDestory(stt_nodeObj_listManageDevCtrlBase *pHead);

void devRgbLamp_dispTrickParam_paramRecovery(void);
uint8_t *devRgbLamp_dispTrickParam_paramGet(void);
void devRgbLamp_dispTrickParam_paramSet(void *param, uint8_t trType, bool nvsRecord_IF);

void L8devHeartbeatFunctionParamLoad(stt_hbDataUpload *nodeDev_dataTemp);
void L8devDetailInfoParamLoad(stt_devInfoDetailUpload *nodeDev_dataTemp);

bool usrAppMethod_mwifiMacAddrRemoveFromList(uint8_t *macAddrList, uint8_t macAddrList_num, uint8_t macAddrRemove[MWIFI_ADDR_LEN]);

void usrApplication_systemRestartTrig(uint8_t trigDelay);

void functionSpecialUsrApp_floatToHex(stt_devElecsumParam2Hex *param, float fData);
void functionEndianSwap(uint8_t *pData, int startIndex, int length);
void functionStrPsdToHidden(char *psdStr);
uint16_t functionFloatGetInteger(float v);
uint8_t functionFloatGetDecimalW2Bit(float v);
void functionDebugDptrToHex_printf(const char *tag, uint8_t *p, uint8_t len);

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)
uint8_t listOperationNodeNumDetect_SSMRctrlUnit(stt_listAttrSolarSysManager *pHead);
void listOperationNodeCreat_SSMRctrlUnit(stt_listAttrSolarSysManager *pHead, stt_listAttrSolarSysManager *pNew);
void listOperationNodeRemove_SSMRctrlUnit(stt_listAttrSolarSysManager *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]);
stt_listAttrSolarSysManager *listOperationNodeGet_SSMRctrlUnit(stt_listAttrSolarSysManager *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method);
void listOperationDestory_SSMRctrlUnit(stt_listAttrSolarSysManager *pHead);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDATAMANAGE_H*/



