/**
 * @file dataTrans_localHandler.h
 *
 */

#ifndef DATATRANS_LOCALHANDLER_H
#define DATATRANS_LOCALHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

/*********************
 *      DEFINES
 *********************/
#define DEVMQTT_MESH_DATAREQ_TEMPLEN		64

#define USRAPP_LOCAL_MESH_DATATRANS_PERPACK_MAX_LEN		1024

#define L8DEV_MESH_CMD_CONTROL							0x10 /*R2N*/
#define L8DEV_MESH_CMD_DEVLOCK							0x17 /*R2N*/
#define L8DEV_MESH_CMD_DEVRESET							0x16 /*R2N*/
#define L8DEV_MESH_CMD_SET_MUTUALCTRL					0x41 /*R2N*/
#define L8DEV_MESH_CMD_SCENARIO_SET						0x45 /*R2N*/
#define L8DEV_MESH_CMD_SSMR_DEVLIST_SET					0x46 /*R2N*/
#define L8DEV_MESH_CMD_SCENARIO_CTRL					0x47 /*R2N*/
#define L8DEV_MESH_CMD_RGBLAMP_OP_PARAM_SET				0x48 /*R2N*/
#define L8DEV_MESH_CMD_AUTOMATION_SET					0x49 /*R2N*/
#define L8DEV_MESH_CMD_CTRLOBJ_TEXTSET					0x50 /*R2N*/
#define L8DEV_MESH_CMD_CTRLOBJ_ICONSET					0x51 /*R2N*/
#define L8DEV_MESH_CMD_UISET_THEMESTYLE					0x52 /*R2N*/
#define L8DEV_MESH_CMD_EXT_PARAM_SET					0x53 /*R2N*/
#define	L8DEV_MESH_CMD_NEIWORK_PARAM_CHG				0x54 /*R2N*/
#define	L8DEV_MESH_CMD_SYSTEM_PARAM_CHG					0x55 /*R2N*/
#define L8DEV_MESH_CMD_BGROUND_ISTSET					0x56 /*R2N*/
#define L8DEV_MESH_CMD_EPID_DATA_ISSUE					0x57 /*R2N*/
#define	L8DEV_MESH_CMD_SERVER_PARAM_CHG					0x58 /*R2N*/
#define	L8DEV_MESH_CMD_MQTTUSR_PARAM_CHG				0x59 /*R2N*/
#define	L8DEV_MESH_CMD_MQTTHA_PARAM_CHG	    			0x5A /*R2N*/
#define	L8DEV_MESH_CMD_ENVIR_INFO_BOARDCAST 			0x5B /*R2N*/
#define	L8DEV_MESH_CMD_SUNSCALE_LOCATION_INFO_BOARDCAST 0x5C /*R2N*/
#define L8DEV_MESH_CMD_SSMR_DEVLIST_STG_SET				0x5D /*R2N*/
#define L8DEV_MESH_CMD_SSMR_CTRLPARAM_STG_SET			0x5E /*R2N*/

#define	L8DEV_MESH_CMD_FWARE_CHECK						0x01 /*R2N | N2R(查询回复上报)*///固件版本check --用于查询固件升级是否可用

#define L8DEV_MESH_CMD_ROOT_FIRST_CONNECT				0x0E /*R2N*/ //根节点首次连接内部广播通知
#define L8DEV_MESH_CMD_SPEQUERY_NOTICE					0x0F /*R2N*/ //指定状态查询通知 --用于界面跳转

#define L8DEV_MESH_CMD_DEV_STATUS						0x11 /*R2N | N2R(查询回复上报)*/
#define L8DEV_MESH_CMD_SET_TIMER						0xA0 /*R2N | N2R(查询回复上报)*/
#define L8DEV_MESH_CMD_SET_DELAY						0xA1 /*R2N | N2R(查询回复上报)*/
#define L8DEV_MESH_CMD_SET_GREENMODE					0xA2 /*R2N | N2R(查询回复上报)*/
#define L8DEV_MESH_CMD_SET_NIGHTMODE					0xA3 /*R2N | N2R(查询回复上报)*/

#define L8DEV_MESH_CMD_SSMR_DEVLIST_REQ					0xB0 /*R2N | N2R(查询回复上报)*///太阳能电池管理器受控设备列表
#define L8DEV_MESH_CMD_RGBLAMP_OP_PARAM_REQ				0xB1 /*R2N | N2R(查询回复上报)*///RGB彩灯设置参数
#define L8DEV_MESH_CMD_SSMR_DEVLIST_STG_REQ				0xB2 /*R2N | N2R(查询回复上报)*///太阳能电池管理器阶段受控设备列表
#define L8DEV_MESH_CMD_SSMR_CTRLPARAM_STG_REQ			0xB3 /*R2N | N2R(查询回复上报)*///太阳能电池管理器阶段控制参数

#define L8DEV_MESH_CMD_MUTUAL_CTRL						0xEA /*N2N | N2R*/
#define L8DEV_MESH_CMD_SUPER_CTRL						0xEB /*N2N | N2R*/
#define L8DEV_MESH_CMD_SUPER_SYNC						0xEC /*N2N | N2R*/
#define L8DEV_MESH_CMD_ATMOS_CFG_NOTICE					0xED /*N2N | N2R*/
#define L8DEV_MESH_CMD_SYSTEM_REBOOT						0xEE /*N2N | N2R*/

#define L8DEV_MESH_CMD_DETAILDEVINFO_NOTICE				0xF9 /*R2N*/

#define L8DEV_MESH_CMD_DEVINFO_GET						0xFA /*R2N*/

#define L8DEV_MESH_HEARTBEAT_REQ						0xFB /*N2R*/

#define L8DEV_MESH_SYSTEMTIME_BOARDCAST					0xFC /*R2N*/

#define L8DEV_MESH_STATUS_SYNCHRO						0xFD /*N2R*/

#define L8DEV_MESH_FWARE_UPGRADE						0xFE /*N2R*///固件升级请求执行

#define L8DEV_MESH_CMD_DEVINFO_LIST_REQ					0x7F /*N2R*///子节点向根节点发送mesh内所有设备数据请求
#define L8DEV_MESH_CMD_EPID_DATA_REQ					0x80 /*N2R*///子节点向根节点发送疫情数据请求
#define L8DEV_MESH_CMD_DETAIL_INFO_REPORT				0x81 /*N2R*///子节点向根节点发送设备详细信息
#define L8DEV_MESH_CMD_HOMEASSISTANT_ONLINE				0x82 /*N2R*///子节点向根节点发送homeassistant上线通知
#define L8DEV_MESH_CMD_DETECTOR_ALARMREPORT				0x83 /*N2R*///报警类子节点设备向根节点发送警报

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void dataHandler_devNodeMeshData(const uint8_t *src_addr, const mlink_httpd_type_t *type, const void *dataRx, size_t dataLen);
void usrApp_devNodeStatusSynchronousInitiative(void);
void usrApp_devNodeCfgParamSynchronousInitiative(void);
void usrApp_devNodeHomeassistantOnlineNotice(void);
void devDetailParamManageList_rootNoticeTrig(uint8_t destMac[MWIFI_ADDR_LEN]);
void deviceDetailInfoListRequest_bussinessTrig(void);
void devHeartbeat_dataTrans_bussinessTrig(void);
void devDetailInfoUploadTrig(void);
void devDetailInfoList_request_trigByEvent(void);
void devAutomationTriggedScenarioExcution(void);

void datatransOpreation_heartbeatHold_realesRunning(void);
void datatransOpreation_epidDataIssue_realesRunning(void);
void datatransOpreation_heartbeatHold_set(uint16_t valSet);
void datatransOpreation_heartbeatHold_auto(void);
uint16_t datatransOpreation_heartbeatHold_get(void);

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_GAS_DETECTOR)||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SMOKE_DETECTOR)||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_PIR_DETECTOR)
 void usrApp_devNodeDetectingAlarmReportInitiative(void);
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SOLAR_SYS_MANAGER)
 void communicationAppDataPkgSetToStruct_devSSMRctrlParam(uint8_t *paramBytesPtr);
 uint8_t *communicationAppDataPkgGetWithBytes_devSSMRctrlParam(uint16_t *dpLen, stt_solarSysManagerCtrlOperateParam *param, bool addHeadCmd_if);
 void communicationAppDataPkgSetToStruct_devSSMRctrlDevList(uint8_t *paramBytesPtr);
 uint8_t *communicationAppDataPkgGetWithBytes_devSSMRctrlDevList(uint16_t *dpLen,
 																				stt_solarSysManagerDevList_nvsOpreat *devListParam, 
																			    uint8_t halfFlg,
																			    enum_solarSysManager_ctrlStage stage,
																			    bool addHeadCmd_if);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DATATRANS_LOCALHANDLER_H*/



