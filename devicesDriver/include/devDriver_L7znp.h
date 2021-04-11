/**
 * @file devDriver_L7znp.h
 *
 */

#ifndef DEVDRIVER_L7ZNP_H
#define DEVDRIVER_L7ZNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "devDataManage.h"

/*--------------------------------------------------------------------------------------------------------
				L7 数据结构区间
---------------------------------------------------------------------------------------------------------*/
#define ZIGB_FRAME_HEAD_MOBILE					0xAA
#define ZIGB_FRAME_HEAD_SERVER					0xCC
#define ZIGB_FRAME_HEAD_HEARTB					0xAA

#define DTMODEKEEPACESS_FRAMEHEAD_ONLINE		0xFA //定时询访模式帧头-internet在线
#define DTMODEKEEPACESS_FRAMEHEAD_OFFLINE		0xFB //定时询访模式帧头-internet离线
#define	DTMODEKEEPACESS_FRAMECMD_ASR			0xA1 //定时询访模式帧命令 - 被动应答
#define	DTMODEKEEPACESS_FRAMECMD_PST			0xA2 //定时询访模式帧命令 - 主动上传

#define ZIGB_FRAMEHEAD_CTRLLOCAL				0xAA //常规控制帧头：本地
#define ZIGB_FRAMEHEAD_CTRLREMOTE				0xCC //常规控制帧头：远程
#define ZIGB_FRAMEHEAD_HEARTBEAT				0xAB //常规控制帧头：心跳<网关internet在线>
#define ZIGB_OFFLINEFRAMEHEAD_HEARTBEAT			0xBB //常规控制帧头：心跳<网关internet离线>

#define zigB_remoteDataTransASY_QbuffLen 		60  
#define zigB_remoteDataTransASY_txPeriod		200
#define zigB_remoteDataTransASY_txReapt			10	
#define zigB_remoteDataTransASY_txUartOnceWait	4	
	
#define zigB_ScenarioCtrlDataTransASY_QbuffLen 				100 
#define zigB_ScenarioCtrlDataTransASY_txPeriod				200 
#define zigB_ScenarioCtrlDataTransASY_txReapt				30	
#define zigB_devIrScenarioCtrlDataTransASY_QbuffLen			8 	//红外设备场景请求缓存队列
#define zigB_ScenarioCtrlDataTransASY_txTimeWaitOnceBasic	2	
#define zigB_ScenarioCtrlDataTransASY_txTimeWaitOnceStep	2
#define zigB_ScenarioCtrlDataTransASY_opreatOnceNum			10
	
#define L7_SWITCH_TYPE_SWBIT1	 	(0xA0 + 0x01) //设备类型，一位开关
#define L7_SWITCH_TYPE_SWBIT2	 	(0xA0 + 0x02) //设备类型，二位开关
#define L7_SWITCH_TYPE_SWBIT3	 	(0xA0 + 0x03) //设备类型，三位开关
#define L7_SWITCH_TYPE_CURTAIN		(0xA0 + 0x08) //设备类型，窗帘
	
#define L7_SWITCH_TYPE_DIMMER		(0x38 + 0x04) //设备类型，调光
#define L7_SWITCH_TYPE_FANS			(0x38 + 0x05) //设备类型，风扇
#define L7_SWITCH_TYPE_INFRARED		(0x38 + 0x06) //设备类型，红外转发器
#define L7_SWITCH_TYPE_SOCKETS		(0x38 + 0x07) //设备类型，插座
#define	L7_SWITCH_TYPE_SCENARIO		(0x38 + 0x09) //设备类型，场景开关
#define L7_SWITCH_TYPE_HEATER		(0x38 + 0x1F) //设备类型，热水器

#define DEVICE_VERSION_NUM			7 //设备版本号：L7 

#define DEV_MAC_LEN					6

#define ZIGB_PANID_MAXVAL     		0x3FFF //随机产生PANID最大值

#define DATATRANS_WORKMODE_HEARTBEAT	0x0A
#define DATATRANS_WORKMODE_KEEPACESS	0x0B

#define ZIGB_DATATRANS_WORKMODE			DATATRANS_WORKMODE_KEEPACESS 

#if(ZIGB_DATATRANS_WORKMODE == DATATRANS_WORKMODE_HEARTBEAT)
	#define PERIOD_HEARTBEAT_ASR		6  	
#elif(ZIGB_DATATRANS_WORKMODE == DATATRANS_WORKMODE_KEEPACESS)
	#define PERIOD_HEARTBEAT_ASR		20  
	#define PERIOD_HEARTBEAT_PST		2	
#endif

#define ZIGB_UTCTIME_START						946684800UL

#define ZIGB_FRAME_HEARTBEAT_cmdOdd				0x23	
#define ZIGB_FRAME_HEARTBEAT_cmdEven			0x22	

#define USRCLUSTERNUM_CTRLEACHOTHER				3

#define ZIGB_ENDPOINT_CTRLSECENARIO				12 
#define ZIGB_ENDPOINT_CTRLNORMAL				13 
#define ZIGB_ENDPOINT_CTRLSYSZIGB				14 

#define ZIGB_CLUSTER_DEFAULT_DEVID				13
#define ZIGB_CLUSTER_DEFAULT_CULSTERID			13

#define DEVZIGB_DEFAULT							0x33

#define ZIGBNWKOPENTIME_DEFAULT					15

#define L7_DEV_STATE_REFRESH_CD_PERIOD			3	

#define ZIGBPANID_CURRENT_REALESPERIOD			50 //PANID数据实时更新周期 单位：s

typedef struct{

	uint8_t tzParam_H;
	uint8_t tzParam_M;
}sttL7_timeZone;

typedef struct{

	uint8_t time_Year;
	uint8_t time_Month;
	uint8_t time_Week;
	uint8_t time_Day;
	uint8_t time_Hour;
	uint8_t time_Minute;
	uint8_t time_Second;
}stt_deviceTime;
		
#define DEVL7MAC_LEN	5
typedef struct ZigB_nwkState_Form{

	uint16_t nwkAddr;				//网络短地址
	uint8_t macAddr[DEVL7MAC_LEN];	//设备MAC地址
	uint8_t devType;				//设备类型
	uint8_t devDefClass;			//设备记录类型
	uint8_t devStateData;			//设备状态值
	uint8_t devStateRefreshCdCnt;	//设备装填更新冷却时间
	uint16_t onlineDectect_LCount;	//心跳计时——实时更新
	
	struct ZigB_nwkState_Form *next;
}nwkStateAttr_Zigb;

typedef struct ZigB_Init_datsAttr{

	uint8_t zigbInit_reqCMD[2]; 	//请求-命令
	uint8_t zigbInit_reqDAT[96];	//请求-数据
	uint8_t reqDAT_num; 			//请求-数据长度
	uint8_t zigbInit_REPLY[96]; 	//请求-回复
	uint8_t REPLY_num;				//请求-重复次数
	uint16_t timeTab_waitAnsr;		//请求-等待回复超时时间
}datsAttr_ZigbInit;

typedef struct{

	uint8_t command; //命令
	uint8_t dats[32]; //数据
	uint8_t datsLen; //数据长度
}frame_zigbSysCtrl; //zigb系统数据通讯端口 通讯帧数据结构

typedef struct{

	uint8_t dats[128 + 25]; 
	uint8_t datsLen; 
}sttUartRcv_rmoteDatComming; 

typedef struct{

	uint8_t dats[32];
	uint8_t datsLen;
}sttUartRcv_sysDat; 

typedef struct{

	uint8_t dats[16];
	uint8_t datsLen;
}sttUartRcv_rmDatsReqResp; 

typedef struct{

	uint8_t prot_Ep; 
		
	uint8_t ctrlEachOther_dat;
	uint8_t ctrlEachOther_loop; 
}sttUartRcv_ctrlEachDat;

typedef struct{

	uint16_t respNwkAddr;
}sttUartRcv_scenarioCtrlResp;	

typedef struct{

	uint8_t cmdResp[2];
	uint8_t frameResp[96];
	uint8_t frameRespLen;
}datsZigb_reqGet;

typedef struct{

	uint16_t keepTxUntilCmp_IF:1; 
	uint16_t datsTxKeep_Period:15;
}remoteDataReq_method;

typedef struct{

	uint16_t deviveID; 
	uint8_t  endPoint; 
}devDatsTrans_portAttr; 

typedef enum datsZigb_structType{

	zigbTP_NULL = 0,
	zigbTP_MSGCOMMING,
	zigbTP_ntCONNECT,
}datsZigb_sttType;

typedef struct ZigB_datsRXAttr_typeMSG{

	bool	 ifBroadcast;	
	uint16_t Addr_from; 	
	uint8_t  srcEP; 		
	uint8_t  dstEP; 		
	uint16_t ClusterID; 	
	uint8_t  LQI;			
	uint8_t  datsLen;		
	uint8_t  dats[128]; 	
}datsAttr_ZigbRX_tpMSG;

typedef struct ZigB_datsRXAttr_typeONLINE{

	uint16_t nwkAddr_fresh; 	
}datsAttr_ZigbRX_tpONLINE;

typedef union ZigB_datsRXAttr{

	datsAttr_ZigbRX_tpMSG	 stt_MSG;
	datsAttr_ZigbRX_tpONLINE stt_ONLINE;
}ZigbAttr_datsZigbRX;

typedef struct ZigBAttr_datsRX{

	datsZigb_sttType datsType:4;
	ZigbAttr_datsZigbRX datsSTT;
}datsAttr_ZigbTrans;
		
#define dataRemote_RESPLEN 8
typedef struct{

	uint8_t dataReq[128 + 25];
	uint8_t dataReq_Len;
	uint8_t dataResp[dataRemote_RESPLEN];
	uint8_t dataResp_Len;

	uint16_t dataReqPeriod; 
	uint8_t repeat_Loop;	
}stt_dataRemoteReq;
		
#define dataScenario_RESPLEN 8
typedef struct{

	uint8_t dataReq[16];
	uint8_t dataReq_Len;
	uint16_t dataRespNwkAddr;

	uint16_t dataReqPeriod; 
	uint8_t repeat_Loop:7;	
	uint8_t scenarioOpreatCmp_flg:1; 
}stt_dataScenarioReq;

typedef struct{

	uint16_t dataRespNwkAddr;
	uint8_t cmdDataSequence[6]; //命令序列（最多6个）
	uint8_t excuteNum:4; //执行总数
	uint8_t excuteIndex:4; //执行索引
}stt_devInfraredSceneReq;

typedef struct{

	uint8_t devNode_MAC[DEVL7MAC_LEN];
	uint8_t devNode_opStatus;
}scenarioOprateUnit_Attr;

typedef struct{

	bool scenarioCtrlOprate_IF; //场景集群控制使能
	scenarioOprateUnit_Attr scenarioOprate_Unit[zigB_ScenarioCtrlDataTransASY_QbuffLen]; //集群节点MAC
	uint8_t devNode_num; //集群节点数目
}stt_scenarioOprateDats;

typedef struct{

	enum{

		zigbScenarioReverseCtrlCMD_scenarioCtrl = 0xCA,

	}command:8; //命令

	uint8_t scenario_Num; //场景号
	uint8_t dataOnceReserve_pWord; //数据包口令 -用于在通讯环境恶劣情况下的重发判断
}frame_zigbScenarioReverseCtrl; //zigb场景反向控制数据通讯端口 通讯帧数据结构

typedef struct{

	enum{

		scenarioOpt_enable = 0xBA,
		scenarioOpt_disable,

	}scenarioReserve_opt:8; //8bit限位
	
	uint8_t scenarioDataSave_InsertNum; //存储索引序号
	scenarioOprateUnit_Attr scenarioOprate_Unit[zigB_ScenarioCtrlDataTransASY_QbuffLen]; //集群节点MAC
	uint8_t devNode_num; //集群节点数目
}stt_scenarioDataLocalSave;

typedef enum{

	msgFun_nwkOpen = 0, //开放网络
	msgFun_nodeSystimeSynchronous, //UTC时间及时区下发至子设备同步
	msgFun_localSystimeZigbAdjust, //本地zigb系统时间与UTC时间同步
	msgFun_portCtrlEachoRegister, //互控端点（通讯簇）注册
	msgFun_panidRealesNwkCreat, //本地zigb主机panid更新
	msgFun_scenarioCrtl, //场景集群控制
	msgFun_dtPeriodHoldPst, //使子节点设备周期性远端通信挂起
	msgFun_dtPeriodHoldCancelAdvance,  //使子节点设备周期性远端通信挂起提前结束
}enum_zigbFunMsg; //zib系统功能触发 消息队列数据类型，功能枚举

typedef enum{

	datsFrom_ctrlRemote = 0,
	datsFrom_ctrlLocal,
	datsFrom_heartBtRemote
}threadDatsPass_objDatsFrom;

typedef struct STTthreadDatsPass_conv{	//数据传输进程消息类型1：常规数据传输

	threadDatsPass_objDatsFrom datsFrom;	//数据来源
	uint8_t macAddr[5];
	uint8_t devType;
	uint8_t dats[128];
	uint8_t datsLen;
}stt_threadDatsPass_conv;

typedef struct STTthreadDatsPass_devQuery{	//数据传输进程消息类型2：设备列表请求

	uint8_t infoDevList[100];
	uint8_t infoLen;
}stt_threadDatsPass_devQuery;

typedef union STTthreadDatsPass_dats{	//数据实体（公用体），单次消息只存在一种类型数据

	stt_threadDatsPass_conv 	dats_conv;		//消息类型1数据
	stt_threadDatsPass_devQuery dats_devQuery;	//消息类型2数据
}stt_threadDatsPass_dats;

typedef enum threadDatsPass_msgType{	//数据传输进程消息类型枚举

	listDev_query = 0,
	conventional,
}threadDP_msgType;

typedef struct STTthreadDatsPass{	//数据传输进程消息数据类型

	threadDP_msgType msgType;		//数据类型
	stt_threadDatsPass_dats dats;	//数据实体
}stt_threadDatsPass;

typedef struct{

	uint8_t systemL7Flg_devLock:1;
	uint8_t systemL7Flg_rsv:7;

	uint16_t panId;
	uint8_t serverIP_default[4];
	uint8_t sysTimeZone_H;
	uint8_t sysTimeZone_M;
}stt_L7smartLivingSysParam;

/*=======================↓↓↓定时询访机制专用数据结构↓↓↓=============================*/
typedef struct agingDataSet_bitHold{ //使用指针强转时注意，agingCmd_swOpreat对应单字节最低位bit0,依此类推
	uint8_t agingCmd_swOpreat:1; //时效_开关状态操作 -bit0
	uint8_t agingCmd_devLock:1; //时效_设备锁设置 -bit1
	uint8_t agingCmd_delaySetOpreat:1; //时效_延时设置 -bit2
	uint8_t agingCmd_greenModeSetOpreat:1; //时效_绿色模式设置 -bit3
	uint8_t agingCmd_timerSetOpreat:1; //时效_定时设置 -bit4
	uint8_t agingCmd_nightModeSetOpreat:1; //时效_夜间模式设置 -bit5
	uint8_t agingCmd_bkLightSetOpreat:1; //时效_背光灯设置 -bit6
	uint8_t agingCmd_devResetOpreat:1; //时效_开关恢复出厂操作 -bit7
	
	uint8_t agingCmd_horsingLight:1; //时效_跑马灯设置 -bit0
	uint8_t agingCmd_switchBitBindSetOpreat:3; //时效_开关互控组号设置_针对三个开关位进行设置 -bit1...bit3
	uint8_t agingCmd_curtainOpPeriodSetOpreat:1; //时效_针对窗帘轨道时间设置 -bit4
	uint8_t agingCmd_infrareOpreat:1; //时效_针对针对红外转发器操作 -bit5
	uint8_t agingCmd_scenarioSwOpreat:1; //时效_针对场景开关操作 -bit6
	uint8_t agingCmd_timeZoneReset:1; //时效_时区校准操作 -bit7
	
	uint8_t agingCmd_byteReserve[4]; //4字节占位保留
	
}stt_agingDataSet_bitHold; //standard_length = 6Bytes

typedef struct swDevStatus_reference{

	uint8_t statusRef_swStatus:3; //状态_设备开关状态 -bit0...bit2
	uint8_t statusRef_reserve:2; //状态_reserve -bit3...bit4
	uint8_t statusRef_swPush:3; //状态_推送占位 -bit5...bit7
	
	uint8_t statusRef_timer:1; //状态_定时器运行 -bit0
	uint8_t statusRef_devLock:1; //状态_设备锁 -bit1
	uint8_t statusRef_delay:1; //状态_延时运行 -bit2
	uint8_t statusRef_greenMode:1; //状态_绿色模式运行 -bit3
	uint8_t statusRef_nightMode:1; //状态_夜间模式运行 -bit4
	uint8_t statusRef_horsingLight:1; //状态_跑马灯运行 -bit5
	uint8_t statusRef_bitReserve:2; //状态__reserve -bit6...bit7
	
	uint8_t statusRef_byteReserve[2];	//状态__reserve -bytes2...3
	
}stt_swDevStatusReference_bitHold; //standard_length = 4Bytes

typedef struct dataPonit{

	stt_agingDataSet_bitHold			devAgingOpreat_agingReference; //时效操作占位, 6Bytes
	stt_swDevStatusReference_bitHold	devStatus_Reference; //设备状态占位, 4Bytes			
	uint8_t 							devData_timer[24]; //定时器数据, 24Bytes
	uint8_t 							devData_delayer; //延时数据, 1Bytes
	uint8_t 							devData_delayUpStatus; //延时到达时,开关响应状态 1Bytes
	uint8_t 							devData_greenMode; //绿色模式数据, 1Bytes
	uint8_t 							devData_nightMode[6]; //夜间模式数据, 6Bytes
	uint8_t 							devData_bkLight[2]; //背光灯颜色, 2Bytes
	uint8_t 							devData_devReset; //开关复位数据, 1Bytes
	uint8_t 							devData_switchBitBind[3]; //开关位互控绑定数据, 3Bytes

	union devClassfication{ //数据从此处开始分类
	
		struct funParam_curtain{ //窗帘
		
			uint8_t orbital_Period; //轨道周期时间
			
		}curtain_param;
		
		struct funParam_socket{ //插座
		
			uint8_t data_elePower[4]; //功率数据
			uint8_t data_eleConsum[3]; //电量数据
			uint8_t data_corTime; //当前电量对应小时段
			
			uint8_t dataDebug_powerFreq[4]; //debug数据	-power的检测频率
	
		}socket_param;

		struct funParam_infrared{
		
			uint8_t opreatAct; //操作指令
			uint8_t opreatInsert; //对应操作的红外指令序号
			uint8_t currentTemperature_integerPrt; //温度数据 -整数部分
			uint8_t currentTemperature_decimalPrt; //温度数据 -小数部分
			uint8_t currentOpreatReserveNum; //当前操作区分随机数口令 -用于区分是操作截止还是重发

			uint8_t irTimeAct_timeUpNum[8]; //八段红外定时响应指令号
			
		}infrared_param;

		struct funParam_scenarioSw{
		
			uint8_t scenarioOpreatCmd; //操作命令
			uint8_t scenarioKeyBind[3]; //对应按键绑定的场景号
			
		}scenarioSw_param;

	}union_devParam;

}stt_devOpreatDataPonit; //standard_length = 49Bytes + class_extension
/*=======================↑↑↑定时询访机制专用数据结构↑↑↑=============================*/

typedef struct{

	uint16_t nwkAddr;				//网络短地址
	uint8_t macAddr[DEVL7MAC_LEN];	//设备MAC地址
	uint8_t devType;				//设备类型
	uint8_t devDefClass;			//设备记录类型
	uint8_t devStateData;			//设备状态值
	uint16_t onlineDectect_LCount;	//心跳计时——实时更新
}paramAttr_l7DataTransNodeUnit;

typedef struct{

	uint16_t pan_id;
}sttAttrDatsInfoDevL7Network;

void smartLivingL7ApplicationRingBufferInitialization(void);
void smartLivingL7ApplicationRingBufferDeinit(void);
uint8_t *L7devListParamInfoTabGet(void);
void L7SystemZnpNetworkInfoGet(sttAttrDatsInfoDevL7Network *param);
void smartLivingL7DeviceControlExtution(uint8_t devMac[DEVL7MAC_LEN], uint8_t devState);

void devDriverBussiness_uartMoudle_L7ZnpSysParamGet(stt_L7smartLivingSysParam *param);
void devDriverBussiness_uartMoudle_L7ZnpSysParamSet(stt_L7smartLivingSysParam *param, bool nvsRecord_IF);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_L7ZNP_H*/


