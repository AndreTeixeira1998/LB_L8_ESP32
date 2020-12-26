/**
 * @file devDriver_uartMoudle.h
 *
 */

#ifndef DEVDRIVER_UARTMOUDLE_H
#define DEVDRIVER_UARTMOUDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDataManage.h"

//基本通信帧格式
/*---------------------------------------------------------------------------------------------------------
	D0			D1				D2				D3			D4			D5			Dn			Dn+1
	帧头			数据长度[HSB]		数据长度[LSB]		命令			数据1			数据2			数据n			校验码
----------------------------------------------------------------------------------------------------------*/

/*********************
 *      DEFINES
 *********************/
#define ITE_HA_DEV_MAC_LEN_MAX		6

/**********************
 *      TYPEDEFS
 **********************/
 enum{
	ctrlSubCmd_null = 0,
	ctrlSubCmd_L8DevStateCtrl,
	ctrlSubCmd_L7DevStateCtrl,
 }uartMeshReqCtrlSubCmd;

 enum{
	cfgSubCmd_null = 0,
 }uartMeshReqCfgSubCmd;

 enum{
	setSubCmd_null = 0,
	setSubCmd_L8roomParamSet,
	setSubCmd_L8networkParamSet,
	setSubCmd_L7networkParamSet,
	setSubCmd_L7networkFuncReq,
 }uartMeshReqSetSubCmd;

 enum{
	chkSubCmd_null = 0,
	chkSubCmd_hbList,
	
	chkSubCmd_L8DevDetailInfoList,
	chkSubCmd_L7DevUnitParamList,
	
	chkSubCmd_L8devUnitParamChgSycn,
	chkSubCmd_L7devUnitParamChgSycn,
	
	chkSubCmd_L8networkInfo,
	chkSubCmd_L7networkInfo,
	
	chkSubCmd_L8systemInfo,
	chkSubCmd_L8roomParamGet,

	chkSubCmd_L8SystemReady,
 }uartMeshReqChkSubCmd;
	
 typedef struct{
	uint8_t *dptr;
	uint16_t dptrLen;
 }uartMoudleDatsDtPkgAttr;

 typedef struct{

	uint8_t frameHead;
	uint8_t frameCheckNum;
	uint16_t dataLen;
	uint8_t opCmd;
	uint8_t *dataPtr;
 }stt_meshDevUartMoudleDTAttr;

 typedef enum{
 
	 blkInfoChgType_null = 0,
	 blkInfoChgType_state,
	 blkInfoChgType_name,
	 blkInfoChgType_icon,
 }blkInfoChgType;

 typedef enum{

	lanbonDevSeq_null = 0,
	lanbonDevSeq_L7,
	lanbonDevSeq_L8,
}lanbonDevSequenceDef;
	
 typedef struct{

	uint8_t nType;
	uint8_t devMac[ITE_HA_DEV_MAC_LEN_MAX];
	uint8_t devSeq;
	union{
		struct{
			uint8_t devState;
		}infoChgDats_state;

		struct{
			uint8_t itemIndex;
			char devName[DEV_CTRLOBJ_NAME_DETAILUD_LEN];
		}infoChgDats_name;

		struct{
			uint8_t itemIndex;
			uint8_t iconIndex;
		}infoChgDats_icon;
	}infoData;
}msgAttr_devBlockParamChange;

typedef struct{

	uint8_t devMac[ITE_HA_DEV_MAC_LEN_MAX];
	uint8_t devType;
}ITEHA_roomDataDevUnitParam; //个体设备单位数据

#define ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX	64
typedef struct{

	uint8_t roomAvailableFlg:1;
	uint8_t devUnitNum:7;
	ITEHA_roomDataDevUnitParam devUnitParam[128];
	char roomName[ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX];
}ITEHA_roomDataParamStorage;

typedef struct{

	wifi_config_t wifiCfgInfo;
	int devConnectedIf;
	int devRootMeshRole;
}sttAttrDatsInfoDevL8Network;

#define L8DEV_VERSION_DISCRIBE_STR_LEN_MAX	96
typedef struct{

	char versionInfo[L8DEV_VERSION_DISCRIBE_STR_LEN_MAX];
}sttAttrDatsInfoDevL8System;

typedef enum{

	DATATRANS_objFLAG_REMOTE = 0,
	DATATRANS_objFLAG_LOCAL
}socketDataTrans_obj;

typedef enum{

	Obj_udpLocal_A = 0,
	Obj_udpRemote_B,
	Obj_tcpRemote_A,
	Obj_tcpRemote_B
}socket_OBJ;

typedef enum{

	obj_toWIFI = 0,
	obj_toZigB,
	obj_toALL
}datsTrans_dstObj;

typedef struct{

	bool 				heartBeat_IF;	//是否为心跳包
	datsTrans_dstObj	dstObj;			//数据是给wifi还是zigb
	socket_OBJ 			portObj;		//数据对象（本地还是远端）
	uint8_t command;		//命令
	uint8_t dats[128];	//数据
	uint8_t datsLen;		//数据长度
}stt_socketDats;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_uartMoudle_moudleDeinit(void);
void devDriverBussiness_uartMoudle_moudleInit(void);
void devDriverBussiness_uartMoudle_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_UARTMOUDLE_H*/































