/**
 * @file devDriver_L7socket.h
 *
 */

#ifndef DEVDRIVER_L7SOCKET_H
#define DEVDRIVER_L7SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "devDataManage.h"

#define FRAME_HEAD_MOBILE		0xAA
#define FRAME_HEAD_SERVER		0xCC
#define FRAME_HEAD_HEARTB		0xAA

#define DATATRANS_WORKMODE_HEARTBEAT		0x0A //心跳模式
#define DATATRANS_WORKMODE_KEEPACESS		0x0B //询访模式
#define ZIGB_DATATRANS_WORKMODE				DATATRANS_WORKMODE_KEEPACESS //定时类通讯模式选择

#define dataTransLength_objLOCAL			33
#define dataTransLength_objREMOTE			45
#define dataHeartBeatLength_objSERVER		96

#define FRAME_TYPE_MtoS_CMD					0xA0	/*数据类型*///手机到开关-WIFI
#define FRAME_TYPE_StoM_RCVsuccess			0x0A	/*数据类型*///开关到手机-WIFI
//#define FRAME_TYPE_MtoZB_CMD				0xA1	/*数据类型*///手机到开关-ZigB
//#define FRAME_TYPE_ZBtoM_CMD				0x1A	/*数据类型*///开关到手机-ZigB
#define FRAME_TYPE_StoM_RCVfail				0x0C
#define FRAME_TYPE_StoM_upLoad				0x0D
#define FRAME_TYPE_StoM_reaptSMK			0x0E

#define FRAME_HEARTBEAT_cmdOdd				0x23	/*命令*///偶数心跳包
#define FRAME_HEARTBEAT_cmdEven				0x22	/*命令*///奇数心跳包

#define FRAME_MtoSCMD_cmdControl			0x10	/*命令*///控制
#define FRAME_MtoSCMD_cmdConfigSearch		0x39	/*命令*///配置搜索
#define FRAME_MtoSCMD_cmdQuery				0x11	/*命令*///配置查询
#define FRAME_MtoSCMD_cmdInterface			0x15	/*命令*///配置交互
#define FRAME_MtoSCMD_cmdReset				0x16	/*命令*///复位
#define FRAME_MtoSCMD_cmdLockON				0x17	/*命令*///上锁
#define FRAME_MtoSCMD_cmdLockOFF			0x18	/*命令*///解锁
#define FRAME_MtoSCMD_cmdswTimQuery			0x19	/*命令*///普通开关定时查询
#define FRAME_MtoSCMD_cmdConfigAP			0x50	/*命令*///AP配置
#define FRAME_MtoSCMD_cmdBeepsON			0x1A	/*命令*///开提示音/昼模式
#define FRAME_MtoSCMD_cmdBeepsOFF			0x1B	/*命令*///关提示音/夜模式
#define FRAME_MtoSCMD_cmdWifiNwkCfg			0x1C	/*命令*///wifi网络配置
#define FRAME_MtoSCMD_cmdftRecoverRQ		0x22	/*命令*///查询是否支持恢复出厂设置
#define FRAME_MtoSCMD_cmdRecoverFactory		0x1F	/*命令*///恢复出厂设置
#define FRAME_MtoSCMD_cmdCfg_swTim			0x14	/*命令*///普通开关定时设置
#define FRAME_MtoZIGBCMD_cmdCfg_PANID		0x40	/*命令*///网关PANID配置
#define FRAME_MtoZIGBCMD_cmdCfg_ctrlEachO	0x41	/*命令*///普通开关互控端口设置
#define FRAME_MtoZIGBCMD_cmdQue_ctrlEachO	0x42	/*命令*///普通开关互控端口查询
#define FRAME_MtoZIGBCMD_cmdCfg_ledBackSet	0x43	/*命令*///普通开关背景灯设置 
#define FRAME_MtoZIGBCMD_cmdQue_ledBackSet	0x44	/*命令*///普通开关背景灯查询
#define FRAME_MtoZIGBCMD_cmdCfg_scenarioSet	0x45	/*命令*///普通开关场景配置
#define FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl	0x47	/*命令*///普通开关场景控制
#define FRAME_MtoZIGBCMD_cmdCfg_scenarioDel	0x48	/*命令*///普通开关场景删除
#define FRAME_MtoZIGBCMD_cmdCfg_scenarioReg 0x50	/*命令*///场景信息本地注册 --针对zigb场景开关子设备设置按键对应触发场景时的同步化操作，场景信息存在网关内，场景子设备只存场景号

#define	cmdConfigTim_normalSwConfig			0xA0	/*定时数据辨识*///普通时刻定时
#define cmdConfigTim_onoffDelaySwConfig		0xA1	/*定时数据辨识*///延时定时
#define cmdConfigTim_closeLoopSwConfig		0xA2	/*定时数据辨识*///绿色模式循环关闭定时
#define cmdConfigTim_nightModeSwConfig		0xA3	/*定时数据辨识*///夜间模式定时

void devL7SmartlivingUdp_remoteComunication_initialization(void);
void devL7SmartlivingUdp_remoteComunication_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_L7SOCKET_H*/



































