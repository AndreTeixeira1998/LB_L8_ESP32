#include "devDriver_L7znp.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "devDriver_manage.h"
#include "dataTrans_localHandler.h"
#include "bussiness_timerSoft.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "devDriver_manage.h"

#define HAL7_RING_BUFFER_LENGTH_MAX 			128

#define ZIGB_HARDWARE_RESET_LEVEL				false
#define ZIGB_HARDWARE_NORMAL_LEVEL				true

#define DEV_UART_L7ZNP_PORT	   					UART_NUM_2
#define DEV_UART_L7ZNP_RST_PIN  				(GPIO_NUM_4)
#define DEV_UART_L7ZNP_TXD_PIN  				(GPIO_NUM_2)
#define DEV_UART_L7ZNP_RXD_PIN  				(GPIO_NUM_15)
#define DEV_UART_L7ZNP_BAUDRATE 				(115200)

extern xQueueHandle msgQh_devListPageInfoChg;

xQueueHandle xMsgQ_Socket2Zigb = NULL,
			 xMsgQ_Zigb2Socket = NULL;

static const uint8_t l7PTL_FRAMEHEAD_CTRLLOCAL		= 0xAA,
			         l7PTL_FRAMEHEAD_CTRLREMOTE		= 0xCC,
			         l7PTL_FRAMEHEAD_HEARTBEAT		= 0xAB,
			         l7PTL_OFFLINEFRAMEHEAD_HEARTBEAT= 0xBB;

static const uint8_t l7PTL_FRAME_MtoSCMD_cmdControl           	= 0x10,	/*命令*///控制
					 l7PTL_FRAME_MtoSCMD_cmdConfigSearch		= 0x39,	/*命令*///配置搜索
					 l7PTL_FRAME_MtoSCMD_cmdQuery				= 0x11,	/*命令*///配置查询
					 l7PTL_FRAME_MtoSCMD_cmdInterface			= 0x15,	/*命令*///配置交互
					 l7PTL_FRAME_MtoSCMD_cmdReset				= 0x16,	/*命令*///复位
					 l7PTL_FRAME_MtoSCMD_cmdLockON				= 0x17,	/*命令*///上锁
					 l7PTL_FRAME_MtoSCMD_cmdLockOFF				= 0x18,	/*命令*///解锁
					 l7PTL_FRAME_MtoSCMD_cmdswTimQuery			= 0x19,	/*命令*///普通开关定时查询
					 l7PTL_FRAME_MtoSCMD_cmdConfigAP			= 0x50,	/*命令*///AP配置
					 l7PTL_FRAME_MtoSCMD_cmdBeepsON				= 0x1A,	/*命令*///开提示音/昼模式
					 l7PTL_FRAME_MtoSCMD_cmdBeepsOFF			= 0x1B,	/*命令*///关提示音/夜模式
					 l7PTL_FRAME_MtoSCMD_cmdWifiNwkCfg			= 0x1C,	/*命令*///wifi网络配置
					 l7PTL_FRAME_MtoSCMD_cmdftRecoverRQ			= 0x22,	/*命令*///查询是否支持恢复出厂设置
					 l7PTL_FRAME_MtoSCMD_cmdRecoverFactory		= 0x1F,	/*命令*///恢复出厂设置
					 l7PTL_FRAME_MtoSCMD_cmdCfg_swTim			= 0x14,	/*命令*///普通开关定时设置
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_PANID		= 0x40,	/*命令*///网关PANID配置
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_ctrlEachO	= 0x41,	/*命令*///普通开关互控端口设置
					 l7PTL_FRAME_MtoZIGBCMD_cmdQue_ctrlEachO	= 0x42,	/*命令*///普通开关互控端口查询
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_ledBackSet	= 0x43,	/*命令*///普通开关背景灯设置
					 l7PTL_FRAME_MtoZIGBCMD_cmdQue_ledBackSet	= 0x44,	/*命令*///普通开关背景灯查询
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_scenarioSet	= 0x45,	/*命令*///普通开关场景配置
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl	= 0x47,	/*命令*///普通开关场景控制
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_scenarioDel	= 0x48,	/*命令*///普通开关场景删除
					 l7PTL_FRAME_MtoZIGBCMD_cmdCfg_scenarioReg	= 0x50;	/*命令*///场景信息本地注册 --针对zigb场景开关子设备设置按键对应触发场景时的同步化操作，场景信息存在网关内，场景子设备只存场景号				 

static const uint8_t l7PTL_ZIGB_SYSCMD_NWKOPEN					= 0x68, //zigb系统指令，开放网络
					 l7PTL_ZIGB_SYSCMD_TIMESET					= 0x69, //zigb系统指令，对子节点进行网络时间同步设定
					 l7PTL_ZIGB_SYSCMD_DEVHOLD					= 0x6A, //zigb系统指令，设备网络挂起(用于更换网关，网管本身用不到)
					 l7PTL_ZIGB_SYSCMD_EACHCTRL_REPORT			= 0x6B, //zigb系统指令，子设备向网关汇报互控触发状态
					 l7PTL_ZIGB_SYSCMD_COLONYPARAM_REQPERIOD	= 0x6C, //zigb系统指令，集群控制本地受控状态周期性轮询应答(包括场景和互控)
					 l7PTL_ZIGB_SYSCMD_DATATRANS_HOLD			= 0x6D; //zigb系统指令，主动挂起周期性通信一段时间，之后恢复

static const uint8_t l7PTL_dataTransLength_objLOCAL = 33,
    				 l7PTL_dataTransLength_objREMOTE = 45,
    				 l7PTL_dataHeartBeatLength_objSERVER = 96;

static const uint8_t l7PTL_CTRLEATHER_PORT_NUMSTART	= 0x10,
         			 l7PTL_CTRLEATHER_PORT_NUMTAIL	= 0xFF;

static const uint8_t l7PTL_CTRLSECENARIO_RESPCMD_SPECIAL = 0xCE;

static const uint8_t zigB_reconnectCauseDataReqFailLoop	= 1;	//因为远端数据传输错误次数超出定义范围 判断zigbee模块重启

static const uint8_t zigB_ScenarioCtrlDataTransASY_timeRoundPause = 0;	//数据发送轮次间距暂歇时间系数 单位：视业务调用周期而定，近似单位为 10ms/per

static const uint8_t fragmentZNP_head = 0xFE,
					 fragmentZNP_frameMinLen = 6;

uint32_t systemUTC_current = 0;

uint8_t MACSTA_ID[DEV_MAC_LEN] = {0};
uint8_t MACAP_ID[DEV_MAC_LEN] = {0};
uint8_t MACDST_ID[DEV_MAC_LEN] = {1,1,1,1,1,1};

uint8_t SWITCH_TYPE = L7_SWITCH_TYPE_CURTAIN;	

stt_scenarioOprateDats scenarioOprateDats = {0};

xQueueHandle xMsgQ_zigbFunRemind = NULL;

static const char *TAG = "lanbon_L8 - L7znp";

static const uint8_t debugLogOut_targetMAC[DEVL7MAC_LEN] = {0x20, 0x18, 0x12, 0x34, 0x56};
static const uint8_t znpFrameCmdTab[3][2] = {

	{0x44, 0x81},
	{0x45, 0xCA},
	{0x44, 0x80},
};

static stt_L7smartLivingSysParam systemParamL7SmartLivingZnp = {0};

static TimerHandle_t localTimerL7Scheduling = NULL;

static xTaskHandle pxTaskHandle_l7ThreadZigbee = NULL;
static xTaskHandle pxTaskHandle_l7ProcessZnpUartRcv = NULL;

static xQueueHandle xMsgQ_uartToutDats_dataSysRespond = NULL, 		//串口数据接收超时断帧数据队列-协议栈或系统回复数据
					xMsgQ_uartToutDats_dataRemoteComing = NULL, 	//串口数据接收超时断帧数据队列-远端数据
					xMsgQ_uartToutDats_rmDataReqResp = NULL, 		//串口数据接收超时断帧数据队列-远端数据请求后的应答
					xMsgQ_uartToutDats_datsComingCtrlEach = NULL,  	//串口数据接收超时断帧数据队列-互控数据接收
					xMsgQ_uartToutDats_scenarioCtrlResp = NULL,    	//串口数据接收超时断帧数据队列-场景控制远端应答
					xMsgQ_timeStampGet = NULL;
					
static nwkStateAttr_Zigb *zigbDevList_Head = NULL;
static uint8_t *pRingBuffer = NULL;
static struct{
	uint16_t indexFront; //头标
	uint16_t indexRear; //尾标
	uint16_t loadLength; //有效长度
	uint16_t surplusLength; //剩余长度
}ringBufferAttrParam = {0};

static SemaphoreHandle_t xSemaphore_uartZnp = NULL;

static stt_dataRemoteReq localZigbASYDT_bufQueueRemoteReq[zigB_remoteDataTransASY_QbuffLen] = {0}; //常规远端数据请求发送队列
static uint8_t dataRemoteRequest_failCount = 0; //常规远端数据请求失败次数记录
static stt_dataScenarioReq localZigbASYDT_bufQueueScenarioReq[zigB_ScenarioCtrlDataTransASY_QbuffLen] = {0}; //场景控制远端数据请求发送队列
static uint8_t localZigbASYDT_scenarioCtrlReserveAllnum = 0; //当前剩余有效的场景操作单位数目
stt_devInfraredSceneReq localZigbASYDT_bufQueuedevIrScenarioReq[zigB_devIrScenarioCtrlDataTransASY_QbuffLen] = {0};

static bool scenarioCtrlOpreatStart_FLG = false; //场景操作开始标志

static uint8_t zigbNwkReserveNodeNum_currentValue = 0; //zigb网络内当前有效节点数量

static bool zigbNodeDevDetectManage_runningEN = false,
			zigbNodeDevDetectManage_runningFLG = false;

static bool nwkZigbOnline_IF = false;	//zigb网络在线标志
uint16_t nwkZigb_currentPANID = 0;
uint16_t sysZigb_random = 0x1234;
uint8_t zigbee_currentPanID_reslesCounter = ZIGBPANID_CURRENT_REALESPERIOD; //当前panid数据实时更新读取周期计时变量

static void functionRingBuffer_fragmentAdd(uint8_t dats[], uint16_t datsLen);

static void *usr_memmem(void *start, unsigned char s_len, void *find, unsigned char f_len){

	unsigned char len	= 0;
			char *p		= start, 
				 *q		= find;
	
	while((p - (char *)start + f_len) <= s_len){
	
		while(*p ++ == *q ++){
		
			len ++;
			if(len == f_len)return (p - f_len);
		}
		
		q 	= find;
		len = 0;
	}
	
	return NULL;
}

static int usr_memloc(uint8_t str2[], uint8_t num_s2, uint8_t str1[], uint8_t num_s1){

	int la = num_s1;
	int i, j;
	int lb = num_s2;
	for(i = 0; i < lb; i ++)
	{
		for(j = 0; j < la && i + j < lb && str1[j] == str2[i + j]; j ++);
		if(j == la)return i;
	}
	return -1;
}

static uint8_t numCheck_xor(uint8_t dats[], uint16_t indexStart, uint16_t datsLen){

	uint8_t res = 0;
	uint16_t loop = 0;

	for(loop = 0; loop < datsLen; loop ++)res ^= dats[indexStart + loop];

	return res;
}

static uint8_t frame_Check(uint8_t frame_temp[], uint8_t check_num){
  
	uint8_t loop 		= 0;
	uint8_t val_Check 	= 0;
	
	for(loop = 0; loop < check_num; loop ++){
	
		val_Check += frame_temp[loop];
	}
	
	val_Check  = ~val_Check;
	val_Check ^= 0xa7;
	
	return val_Check;
}

static void UartCallback_portL7zigb(void* arg1, uint32_t arg2){

	static BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(xSemaphore_uartZnp, &xHigherPriorityTaskWoken);
}

static uint8_t ZigB_TXFrameLoad(uint8_t frame[], uint8_t cmd[], uint8_t cmdLen, uint8_t dats[], uint8_t datsLen){		

	const uint8_t frameHead = fragmentZNP_head;	//ZNP,SOF帧头.
	uint8_t xor_check = datsLen; //异或校验，帧尾
	uint8_t loop = 0;
	uint8_t ptr = 0;
	
	frame[ptr ++] = frameHead;
	frame[ptr ++] = datsLen;
	
	memcpy(&frame[ptr],cmd,cmdLen);
	ptr += cmdLen;
	for(loop = 0;loop < cmdLen;loop ++)xor_check ^= cmd[loop];

	memcpy(&frame[ptr],dats,datsLen);
	ptr += datsLen;
	for(loop = 0;loop < datsLen;loop ++)xor_check ^= dats[loop];	
	
	frame[ptr ++] = xor_check;
	
	return ptr;
}

static void datsArray2HexPrintf(const char* tipsHead, uint8_t *dats, uint8_t datsLen){

	uint16_t datsLogLen = sizeof(uint8_t) * 3 * datsLen + 16;
	uint8_t *datsLogPtr = (uint8_t *)malloc(datsLogLen);
	uint8_t loop = 0;

	if(NULL != datsLogPtr){

		memset(datsLogPtr, 0, datsLogLen);

		for(loop = 0; loop < datsLen; loop ++){
			sprintf((char *)&datsLogPtr[loop * 3], "%02X ", *dats ++);
		}
		ESP_LOGI(TAG, "%s<datsLen: %d> %s.\n", tipsHead, datsLen, datsLogPtr);
		os_free(datsLogPtr);
	}
}

static void ZigB_sysCtrlFrameLoad(uint8_t datsTemp[], frame_zigbSysCtrl dats){

	datsTemp[0] = dats.command;
	datsTemp[1] = dats.datsLen;
	memcpy((char *)&datsTemp[2], (char *)dats.dats, dats.datsLen);
}

static void periphLocalPortUart_l7znp_datsReceiveLoopOnce(void){

	uint8_t *rptr = (uint8_t *)os_zalloc(HAL7_RING_BUFFER_LENGTH_MAX + 1);
	uint16_t dprxLen = 0;

	if(NULL != rptr){

		dprxLen = uart_read_bytes(DEV_UART_L7ZNP_PORT, rptr, HAL7_RING_BUFFER_LENGTH_MAX, 10 / portTICK_RATE_MS);
		if(dprxLen > 0){
			
			functionRingBuffer_fragmentAdd(rptr, dprxLen);
//			datsArray2HexPrintf("znpUart dats rcv", uartDatsTemp, uartDatsRcvLen);
//			ESP_LOGI(TAG, "znpUart dats rcv, len:%d\n", uartDatsRcvLen);
		}
		
		os_free(rptr);
	}
}

static void periphLocalPortUart_l7znp_send(uint8_t *dats, uint16_t datsLen){

	uart_write_bytes(DEV_UART_L7ZNP_PORT, (const char *)dats, datsLen);
}

static void periphLocalPortGpio_l7znp_resetPin(bool hLevel){

	(false == hLevel)?
		(gpio_set_level(DEV_UART_L7ZNP_RST_PIN, (uint32_t)1)):
		(gpio_set_level(DEV_UART_L7ZNP_RST_PIN, (uint32_t)0));
}

/*zigbee节点设备链表新增（注册）节点设备（将设备信息注册进链表）*/ 
static uint8_t zigbDev_eptCreat(nwkStateAttr_Zigb *pHead, nwkStateAttr_Zigb pNew){
	
	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow;
	uint8_t nCount = 0;
	bool flg_nodeDuplication = false;
	
	nwkStateAttr_Zigb *pNew_temp = (nwkStateAttr_Zigb *)malloc(sizeof(nwkStateAttr_Zigb));
	memset(pNew_temp, 0, sizeof(nwkStateAttr_Zigb));
	pNew_temp->nwkAddr 				= pNew.nwkAddr;
	memcpy(pNew_temp->macAddr, pNew.macAddr, DEVL7MAC_LEN);
	pNew_temp->devType 				= pNew.devType;
	pNew_temp->devDefClass			= pNew.devDefClass;
	pNew_temp->devStateData			= pNew.devStateData;
	pNew_temp->onlineDectect_LCount = pNew.onlineDectect_LCount;
	pNew_temp->next	   				= NULL;
	
	while(pAbove->next != NULL){

		if(!memcmp(pAbove->next->macAddr, pNew.macAddr, sizeof(uint8_t) * DEVL7MAC_LEN)){ //检测到重复节点

			flg_nodeDuplication = true;
			ESP_LOGI(TAG, "zigbDev list node duplication!\n");
			break;
		}
		
		nCount ++;
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}

	if(false == flg_nodeDuplication){ //节点创建

		pAbove->next = pNew_temp;
		return ++nCount;		
	}
	else{ //节点重复，取消创建
		
		os_free(pNew_temp);
		return nCount;
	}
}

/*zigbee节点提取从设备链表中，根据网络短地址;！！！谨记使用完节点信息后将内存释放！！！*/
static nwkStateAttr_Zigb *zigbDev_eptPutout_BYnwk(nwkStateAttr_Zigb *pHead, uint16_t nwkAddr, bool method){	//method = 1,源节点地址返回，操作返回内存影响源节点信息; method = 0,映射信息地址返回，操作返回内存，不影响源节点信息.
	
	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow;
	
	nwkStateAttr_Zigb *pTemp = (nwkStateAttr_Zigb *)malloc(sizeof(nwkStateAttr_Zigb));
	memset(pTemp, 0, sizeof(nwkStateAttr_Zigb));
	pTemp->next = NULL;
	
	while(!(pAbove->nwkAddr == nwkAddr) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(pAbove->nwkAddr == nwkAddr){
		
		if(!method){
			
			pTemp->nwkAddr 				= pAbove->nwkAddr;
			memcpy(pTemp->macAddr, pAbove->macAddr, DEVL7MAC_LEN);
			pTemp->devType 				= pAbove->devType;
			pTemp->devDefClass			= pAbove->devDefClass;
			pTemp->devStateData			= pAbove->devStateData;
			pTemp->onlineDectect_LCount = pAbove->onlineDectect_LCount;
		}else{
			
			os_free(pTemp);
			pTemp = pAbove;	
		}
		
		return pTemp;
	}else{
		
		os_free(pTemp);
		return NULL;
	}	
} 

/*zigbee节点提取从设备链表中，根据节点设备MAC地址和设备类型;！！！谨记使用完节点信息后将内存释放！！！*/
static nwkStateAttr_Zigb *zigbDev_eptPutout_BYpsy(nwkStateAttr_Zigb *pHead, uint8_t macAddr[DEVL7MAC_LEN], uint8_t devType, bool method){		//method = 1,源节点地址返回，操作返回内存影响源节点信息; method = 0,映射信息地址返回，操作返回内存，不影响源节点信息.

	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow = NULL;
	
	nwkStateAttr_Zigb *pTemp = (nwkStateAttr_Zigb *)malloc(sizeof(nwkStateAttr_Zigb));
	memset(pTemp, 0, sizeof(nwkStateAttr_Zigb));
	pTemp->next = NULL;
	
	while(!(!memcmp(pAbove->macAddr, macAddr, DEVL7MAC_LEN) && pAbove->devType == devType) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(!memcmp(pAbove->macAddr, macAddr, DEVL7MAC_LEN) && pAbove->devType == devType){
		
		if(!method){
			
			pTemp->nwkAddr 				= pAbove->nwkAddr;
			memcpy(pTemp->macAddr, pAbove->macAddr, DEVL7MAC_LEN);
			pTemp->devType 				= pAbove->devType;
			pTemp->devDefClass			= pAbove->devDefClass;
			pTemp->devStateData			= pAbove->devStateData;
			pTemp->onlineDectect_LCount = pAbove->onlineDectect_LCount;
		}else{

			os_free(pTemp);
			pTemp = pAbove;	
		}
		
		return pTemp;
	}else{
		
		os_free(pTemp);
		return NULL;
	}	
}

/*zigbee删除设备节点信息从链表中，根据节点设备网络短地址*/ 
static bool zigbDev_eptRemove_BYnwk(nwkStateAttr_Zigb *pHead, uint16_t nwkAddr){
	
	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow = NULL;
	
	nwkStateAttr_Zigb *pTemp = NULL;
	
	while(!(pAbove->nwkAddr == nwkAddr) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(pAbove->nwkAddr == nwkAddr){
		
		pTemp = pAbove;
		pFollow->next = pAbove->next;
		if(pTemp != NULL){

			os_free(pTemp);
			pTemp = NULL;
		}
		
		return true;
		
	}else{
		
		return false;
	}
}

/*zigbee删除设备节点信息从链表中，根据节点设备MAC地址和设备类型*/
static bool zigbDev_eptRemove_BYpsy(nwkStateAttr_Zigb *pHead,uint8_t macAddr[DEVL7MAC_LEN],uint8_t devType){
	
	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow;
	
	nwkStateAttr_Zigb *pTemp;
	
	while(!(!memcmp(pAbove->macAddr, macAddr, DEVL7MAC_LEN) && pAbove->devType == devType) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(!memcmp(pAbove->macAddr, macAddr, DEVL7MAC_LEN) && pAbove->devType == devType){
		
		 pTemp = pAbove;
		 pFollow->next = pAbove->next;
		 os_free(pTemp);
		 return true;
	}else{
		
		return false;
	}
}

/*zigbee节点设备链表长度测量*/
static uint8_t zigbDev_chatLenDectect(nwkStateAttr_Zigb *pHead){
	
	nwkStateAttr_Zigb *pAbove = pHead;
	nwkStateAttr_Zigb *pFollow;
	uint8_t loop = 0;
	
	while(pAbove->next != NULL){
		
		loop ++;
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}

	return loop;
}

/*zigbee节点设备信息链表遍历，将所有节点设备类型和设备MAC地址打包输出*/
static uint8_t ZigBdevDispList(nwkStateAttr_Zigb *pHead,uint8_t DevInform[]){
	
	nwkStateAttr_Zigb *Disp = pHead;
	uint8_t loop = 0;
	
	if(0 == zigbDev_chatLenDectect(pHead)){
		
		return 0;
	}

	while(Disp->next != NULL){
	
		Disp = Disp->next;
		
		memcpy(&DevInform[loop * (DEVL7MAC_LEN + 1)], Disp->macAddr, DEVL7MAC_LEN);
		DevInform[loop * (DEVL7MAC_LEN + 1) + 5] = Disp->devType;
		loop ++;
	}
	
	return loop;
}

static void functionRingBuffer_fragmentAdd(uint8_t dats[], uint16_t datsLen){

	bool frameDatsValid_flg = true;
	uint16_t loop = 0;

	//buffer未初始化
	if(NULL == pRingBuffer)return;

	//获取剩余空间
	ringBufferAttrParam.surplusLength = HAL7_RING_BUFFER_LENGTH_MAX - ringBufferAttrParam.loadLength;

	//剩余空间足够则进行缓存
	if(ringBufferAttrParam.surplusLength > datsLen){

		for(loop = 0; loop < datsLen; loop ++){
		
			pRingBuffer[ringBufferAttrParam.indexRear] = dats[loop];
			ringBufferAttrParam.indexRear = (ringBufferAttrParam.indexRear + 1) % HAL7_RING_BUFFER_LENGTH_MAX;
			ringBufferAttrParam.loadLength ++;
		}

		while(true == frameDatsValid_flg){

			while((pRingBuffer[ringBufferAttrParam.indexFront] != fragmentZNP_head) && (ringBufferAttrParam.loadLength > 0)){
			
				ringBufferAttrParam.indexFront = (ringBufferAttrParam.indexFront + 1) % HAL7_RING_BUFFER_LENGTH_MAX;
				ringBufferAttrParam.loadLength --;
			}

//			datsArray2HexPrintf("ringBuf refresh:", &pRingBuffer[ringBufferAttrParam.indexFront], ringBufferAttrParam.loadLength);
//			ESP_LOGI(TAG, "ringbf attr: index_f:%d, index_t:%d, loadLen:%d.\n", ringBufferAttrParam.indexFront, 
//																		 ringBufferAttrParam.indexRear,
//																		 ringBufferAttrParam.loadLength);

		  	if((pRingBuffer[ringBufferAttrParam.indexFront] == fragmentZNP_head) && (ringBufferAttrParam.loadLength >= fragmentZNP_frameMinLen)){ //锁定有效数据帧

				uint16_t frontIst_temp = (ringBufferAttrParam.indexFront + 1) % HAL7_RING_BUFFER_LENGTH_MAX;
				uint16_t frameLen = pRingBuffer[frontIst_temp];

				if((frameLen + 5) <= ringBufferAttrParam.loadLength){ //可疑有效帧总长度小于当前有效数据长度，进行有效性判断

					uint16_t datsFrameLen = frameLen + 5;
					uint8_t *datsFrame = (uint8_t *)malloc(sizeof(uint8_t) * datsFrameLen);
					uint16_t datsFrameAssign_ist = 0;

					if(NULL != datsFrame){

						memset(datsFrame, 0, sizeof(uint8_t) * datsFrameLen);

						for(uint16_t dIst = 0; dIst < datsFrameLen; dIst ++){ //有效帧数据进行缓存 -datsFrameLen是整个数据帧的长度
	
							datsFrameAssign_ist = (ringBufferAttrParam.indexFront + dIst) % HAL7_RING_BUFFER_LENGTH_MAX;
							datsFrame[dIst] = pRingBuffer[datsFrameAssign_ist];
						}

//						datsArray2HexPrintf("ZNP frame got:", datsFrame, datsFrameLen);
						
						if(datsFrame[frameLen + 4] == numCheck_xor(datsFrame, 1, frameLen + 3)){ //异或校验，验证有效性
	
							bool flg_znpFrameRcg = false;
	
							ringBufferAttrParam.indexFront += (frameLen + 5);
							ringBufferAttrParam.indexFront %= HAL7_RING_BUFFER_LENGTH_MAX;
							ringBufferAttrParam.loadLength -= (frameLen + 5);

//							ESP_LOGI(TAG, "znp frame catched.\n");
							
							for(uint8_t loop = 0; loop < 3; loop ++){
	
								if(!memcmp(znpFrameCmdTab[loop], &datsFrame[2], 2)){ //接收到串口数据帧：按类型填装进相应队列
	
									switch(loop){
	
										case 0: 
										case 1:{
	
											if((datsFrame[21] == l7PTL_FRAMEHEAD_CTRLLOCAL) &&\
											   (datsFrame[24] == l7PTL_FRAME_MtoSCMD_cmdConfigSearch)){ //远端数据
	
												sttUartRcv_rmoteDatComming sptr = {0};
												memcpy(sptr.dats, datsFrame, sizeof(uint8_t) * datsFrameLen);
												sptr.datsLen = datsFrameLen;
												xQueueSendToFront(xMsgQ_uartToutDats_dataRemoteComing, (void *)&sptr, (portTickType) 0);
	
											}else 
											if((datsFrame[10] > l7PTL_CTRLEATHER_PORT_NUMSTART) && (27 == datsFrameLen)){ //互控数据
	
												sttUartRcv_ctrlEachDat sptr = {0};
												sptr.prot_Ep			= datsFrame[10]; //终端点编号
												sptr.ctrlEachOther_dat	= datsFrame[21]; //互控数据
												sptr.ctrlEachOther_loop = datsFrame[22]; //互控剩余发送次数
												xQueueSend(xMsgQ_uartToutDats_datsComingCtrlEach, (void *)&sptr, (portTickType) 0);
	
											}else 
											if((l7PTL_CTRLSECENARIO_RESPCMD_SPECIAL == datsFrame[21]) && (26 == datsFrameLen)){ //场景响应数据
	
												sttUartRcv_scenarioCtrlResp sptr = {0};
												sptr.respNwkAddr = (((uint16_t)datsFrame[8] & 0x00FF) << 0)|
																   (((uint16_t)datsFrame[9] & 0x00FF) << 8); //远端响应短地址加载
												xQueueSend(xMsgQ_uartToutDats_scenarioCtrlResp, (void *)&sptr, (portTickType) 0);
	
											}else{ //远端数据
	
												sttUartRcv_rmoteDatComming sptr = {0};
												memcpy(sptr.dats, datsFrame, sizeof(uint8_t) * datsFrameLen);
												sptr.datsLen = datsFrameLen;
												xQueueSend(xMsgQ_uartToutDats_dataRemoteComing, (void *)&sptr, (portTickType) 0);
											}
											
											flg_znpFrameRcg = true;
											
										}break;
	
										case 2:{
	
											sttUartRcv_rmDatsReqResp sptr = {0};
											memcpy(sptr.dats, datsFrame, sizeof(uint8_t) * datsFrameLen);
											sptr.datsLen = datsFrameLen;
											xQueueSend(xMsgQ_uartToutDats_rmDataReqResp, (void *)&sptr, (portTickType) 0);
	
											flg_znpFrameRcg = true;
										
										}break;
	
										default:
//											Log.i("queue bussiness", "type unknown");
											break;
									}
									break;
								}
							}
	
							if(false == flg_znpFrameRcg){ //剩下就是系统数据
	
								sttUartRcv_sysDat sptr = {0};
								memcpy(sptr.dats, datsFrame, sizeof(uint8_t) * datsFrameLen);
								sptr.datsLen = datsFrameLen;
								xQueueSend(xMsgQ_uartToutDats_dataSysRespond, (void *)&sptr, (portTickType) 0);
							}
						}
						else
						{
							pRingBuffer[ringBufferAttrParam.indexFront] = 0;
						}
	
						os_free(datsFrame);
					}
				}
				else
				{
					if((frameLen + 5) >= HAL7_RING_BUFFER_LENGTH_MAX){ //可疑有效帧总长度大于queue总长度，直接判断无效，进行下一可疑帧筛选，防止等待溢出

						pRingBuffer[ringBufferAttrParam.indexFront] = 0;
					}
					else
					{ //否则待定

						frameDatsValid_flg = false;
					}
				}
			}
			else
			{
				frameDatsValid_flg = false;
			}
		}
	}
}

static bool zigb_datsRequest(		uint8_t frameREQ[],			//请求帧
								 	uint8_t frameREQ_Len,		//请求帧长度
								 	uint8_t resp_cmd[2],		//预期应答指令
								 	datsZigb_reqGet *datsRX,	//预期应答数据
								 	uint16_t timeWait, 			//超时时间
									remoteDataReq_method method){ //是否死磕

	sttUartRcv_sysDat	rptr_uartDatsRcv= {0};
	portBASE_TYPE 		xMsgQ_rcvResult = pdFALSE;
	uint16_t 			datsRcv_tout	= timeWait;

	if(!method.keepTxUntilCmp_IF)periphLocalPortUart_l7znp_send(frameREQ, frameREQ_Len); //非死磕，超时前只发一次
	
	while(datsRcv_tout --){

		vTaskDelay(1 / portTICK_RATE_MS);

		if(method.keepTxUntilCmp_IF){ //死磕模式下周期性发送指令

			if((datsRcv_tout % method.datsTxKeep_Period) == 0)periphLocalPortUart_l7znp_send(frameREQ, frameREQ_Len);
		}
	
		xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
		while(xMsgQ_rcvResult == pdTRUE){
			
			if((rptr_uartDatsRcv.dats[0] == fragmentZNP_head) &&
			   (rptr_uartDatsRcv.dats[rptr_uartDatsRcv.datsLen - 1] == numCheck_xor(&rptr_uartDatsRcv.dats[1], 0, rptr_uartDatsRcv.datsLen - 2)) &&
			   (!memcmp(&(rptr_uartDatsRcv.dats[2]), resp_cmd, 2))
			   ){

					memcpy(datsRX->cmdResp, &rptr_uartDatsRcv.dats[2], 2);
					memcpy(datsRX->frameResp, rptr_uartDatsRcv.dats, rptr_uartDatsRcv.datsLen);
					datsRX->frameRespLen = rptr_uartDatsRcv.datsLen;
					return true;
			}
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
		}
	}

	vTaskDelay(1 / portTICK_RATE_MS);

	return false;
}

static bool zigb_VALIDA_INPUT(	uint8_t REQ_CMD[2],		//指令
				       				uint8_t REQ_DATS[],	//数据
				       				uint8_t REQdatsLen,	//数据长度
				       				uint8_t ANSR_frame[], //响应帧
				       				uint8_t ANSRdatsLen, //响应帧长度
				       				uint8_t times,uint16_t timeDelay){ //循环次数，单次等待时间
					   
#define zigbDatsTransLen 128

	uint8_t *dataTXBUF = (uint8_t *)malloc(sizeof(uint8_t) * zigbDatsTransLen);
	uint8_t loop = 0;
	uint8_t datsTX_Len;
	uint16_t local_timeDelay = timeDelay;
	sttUartRcv_sysDat rptr_uartDatsRcv;
	portBASE_TYPE xMsgQ_rcvResult = pdFALSE;

	bool result_REQ = false;

	memset(dataTXBUF, 0, sizeof(uint8_t) * zigbDatsTransLen); //清洁缓存
	datsTX_Len = ZigB_TXFrameLoad(dataTXBUF, REQ_CMD, 2, REQ_DATS, REQdatsLen);

	for(loop = 0;loop < times;loop ++){

		uint16_t datsRcv_tout = timeDelay;

		periphLocalPortUart_l7znp_send(dataTXBUF, datsTX_Len);
		while(datsRcv_tout --){

			vTaskDelay(1 / portTICK_RATE_MS);
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
			while(xMsgQ_rcvResult == pdTRUE){	//遍历响应队列查找
			
				if(usr_memmem(rptr_uartDatsRcv.dats, rptr_uartDatsRcv.datsLen, ANSR_frame, ANSRdatsLen)){

					result_REQ = true;
					break;
				}
				xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
			}if(true == result_REQ)break;
		}if(true == result_REQ)break;
	}

	os_free(dataTXBUF);
	
	return result_REQ;
}

static bool zigb_clusterSet(uint16_t deviveID, uint8_t endPoint){

	const datsAttr_ZigbInit default_param = {{0x24,0x00},{0x0E,0x0D,0x00,0x0D,0x00,0x0D,0x00,0x01,0x00,0x00,0x01,0x00,0x00},0x0D,{0xFE,0x01,0x64,0x00,0x00,0x65},0x06,20}; //数据簇注册，默认参数
	const uint8_t frameResponse_Subs[6] = {0xFE,0x01,0x64,0x00,0xB8,0xDD}; //响应替补帧，若数据簇已被注册
	
#define paramLen_clusterSet 100

	uint8_t paramTX_temp[paramLen_clusterSet] = {0};

	bool resultSet = false;
	
	memcpy(paramTX_temp, default_param.zigbInit_reqDAT, default_param.reqDAT_num);
	paramTX_temp[0] = endPoint;
	paramTX_temp[3] = (uint8_t)((deviveID & 0x00ff) >> 0);
	paramTX_temp[4] = (uint8_t)((deviveID & 0xff00) >> 8);
	
	resultSet = zigb_VALIDA_INPUT((uint8_t *)default_param.zigbInit_reqCMD,
								  (uint8_t *)paramTX_temp,
								  default_param.reqDAT_num,
								  (uint8_t *)default_param.zigbInit_REPLY,
								  default_param.REPLY_num,
								  2, 	//2次以内无正确响应则失败
								  default_param.timeTab_waitAnsr);

	if(resultSet)return resultSet;
	else{

		return zigb_VALIDA_INPUT((uint8_t *)default_param.zigbInit_reqCMD,
								 (uint8_t *)paramTX_temp,
								 default_param.reqDAT_num,
								 (uint8_t *)frameResponse_Subs,
								 6,
								 2, 	//2次以内无正确响应则失败
								 default_param.timeTab_waitAnsr);
	}
}

static bool zigb_clusterMultiSet(devDatsTrans_portAttr devPort[], uint8_t Len){

	uint8_t loop = 0;

	for(loop = 0; loop < Len; loop ++){

		if(!zigb_clusterSet(devPort[loop].deviveID, devPort[loop].endPoint))return false;
	}

	return true;
}

static bool zigb_clusterCtrlEachotherCfg(void){

	uint8_t loop = 0;
	bool config_Result = false;

	uint8_t CTRLEATHER_PORT[USRCLUSTERNUM_CTRLEACHOTHER] = {0};

	for(loop = 0; loop < USRCLUSTERNUM_CTRLEACHOTHER; loop ++){

		if(CTRLEATHER_PORT[loop] > 0x10 && CTRLEATHER_PORT[loop] < 0xFF){

			config_Result = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, CTRLEATHER_PORT[loop]);
			if(!config_Result)break;
			
		}else{

			config_Result = true;
		}
	}

	return config_Result;
}

static bool zigbNetwork_OpenIF(bool opreat_Act, uint8_t keepTime){

	const datsAttr_ZigbInit default_param = {{0x26,0x08}, {0xFC,0xFF,0x00}, 0x03, {0xFE,0x01,0x66,0x08,0x00,0x6F}, 0x06, 500};	//命令帧，默认参数
#define nwOpenIF_paramLen 64

	bool result_Set = false;

	uint8_t paramTX_temp[nwOpenIF_paramLen] = {0};
	
	memcpy(paramTX_temp,default_param.zigbInit_reqDAT,default_param.reqDAT_num);
	if(true == opreat_Act)paramTX_temp[2] = keepTime;
	else paramTX_temp[2] = 0x00;
	
	result_Set = zigb_VALIDA_INPUT(	(uint8_t *)default_param.zigbInit_reqCMD,
									(uint8_t *)paramTX_temp,
									default_param.reqDAT_num,
									(uint8_t *)default_param.zigbInit_REPLY,
									default_param.REPLY_num,
									3,		//3次以内无正确响应则失败
									default_param.timeTab_waitAnsr);	

	if(result_Set)ESP_LOGI(TAG, "[Tips_uartZigb]: zigb nwkOpen success.\n");
	else ESP_LOGI(TAG, "[Tips_uartZigb]: zigb nwkOpen fail.\n");

	return result_Set;
}

static bool zigB_sysTimeSet(uint32_t timeStamp){

	const datsAttr_ZigbInit default_param = {{0x21,0x10},{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},0x0B,{0xFE,0x01,0x61,0x10,0x00},0x05,30}; //zigbee系统时间设置，默认参敿
	uint8_t timeStampArray[11] = {0};
	bool resultSet = false;
	uint32_t timeStamp_temp = timeStamp;

	sttL7_timeZone systemParamTimeZone = {0};

	if(systemParamTimeZone.tzParam_H <= 12){
	
		timeStamp_temp += (3600UL * (long)systemParamTimeZone.tzParam_H + 60UL * (long)systemParamTimeZone.tzParam_M); //时区正
		
	}else
	if(systemParamTimeZone.tzParam_H > 12 && systemParamTimeZone.tzParam_H <= 24){
	
		timeStamp_temp -= (3600UL * (long)(systemParamTimeZone.tzParam_H - 12) + 60UL * (long)systemParamTimeZone.tzParam_M); //时区负
		
	}else
	if(systemParamTimeZone.tzParam_H == 30 || systemParamTimeZone.tzParam_H == 31){ 
		
		timeStamp_temp += (3600UL * (long)(systemParamTimeZone.tzParam_H - 17) + 60UL * (long)systemParamTimeZone.tzParam_M); //时区特殊
	}
	
	timeStampArray[0] = (uint8_t)((timeStamp_temp & 0x000000ff) >> 0);
	timeStampArray[1] = (uint8_t)((timeStamp_temp & 0x0000ff00) >> 8);
	timeStampArray[2] = (uint8_t)((timeStamp_temp & 0x00ff0000) >> 16);
	timeStampArray[3] = (uint8_t)((timeStamp_temp & 0xff000000) >> 24);
	
	resultSet = zigb_VALIDA_INPUT((uint8_t *)default_param.zigbInit_reqCMD,
								  (uint8_t *)timeStampArray,
								  11,
								  (uint8_t *)default_param.zigbInit_REPLY,
								  default_param.REPLY_num,
								  2,	//2次以内无正确响应则失败
								  default_param.timeTab_waitAnsr);
	
//	ESP_LOGI(TAG, "[Tips_uartZigb]: zigbee sysTime set result: %d.\n", resultSet);
	if(resultSet){

//		ESP_LOGI(TAG, "[Tips_uartZigb]: zigbSystime set success.\n");
	}
	else ESP_LOGI(TAG, "[Tips_uartZigb]: zigbSystime set fail.\n");

	return resultSet;
}

static bool zigB_sysTimeGetRealesWithLocal(void){

	uint32_t timeStamp_temp = 0;
	datsZigb_reqGet *local_datsParam = (datsZigb_reqGet *)malloc(sizeof(datsZigb_reqGet));
	const uint8_t frameREQ_zigbSysTimeGet[5] = {0xFE, 0x00, 0x21, 0x11, 0x30};	//zigb系统时间获取指令帧
	const uint8_t cmdResp_zigbSysTimeGet[2] = {0x61, 0x11};	//zigb系统时间获取预期响应指令
	bool resultREQ = false;
	const remoteDataReq_method datsReq_method = {0};

	static stt_deviceTime systemTime_current = {0};
	static uint16_t sysTimeKeep_counter = 0;
	static uint32_t systemUTC_current = 0;

	memset(local_datsParam, 0, sizeof(datsZigb_reqGet));
	resultREQ = zigb_datsRequest((uint8_t *)frameREQ_zigbSysTimeGet,
								 5,
								 (uint8_t *)cmdResp_zigbSysTimeGet,
								 local_datsParam,
								 300,
								 datsReq_method);

	if(true == resultREQ){

		/*本地系统UTC更新*/
		timeStamp_temp |= (((uint32_t)(local_datsParam->frameResp[4]) << 0)  & 0x000000FF);
		timeStamp_temp |= (((uint32_t)(local_datsParam->frameResp[5]) << 8)  & 0x0000FF00);
		timeStamp_temp |= (((uint32_t)(local_datsParam->frameResp[6]) << 16) & 0x00FF0000);
		timeStamp_temp |= (((uint32_t)(local_datsParam->frameResp[7]) << 24) & 0xFF000000);
		systemUTC_current = timeStamp_temp + ZIGB_UTCTIME_START;  //zigb系统协议UTC补偿

		/*本地系统格式时间更新*/
		uint16_t Y_temp16 = ((uint16_t)local_datsParam->frameResp[13] << 0) | ((uint16_t)local_datsParam->frameResp[14] << 8);
		uint8_t  Y_temp8 = 0;
		uint8_t  M_temp8 = 0;
		
		uint8_t Y = (uint8_t)(Y_temp16 % 2000);
		uint8_t M = local_datsParam->frameResp[11];
		uint8_t D = local_datsParam->frameResp[12];
		uint8_t W = 0;
		
		/*计算缓存赋*/
		Y_temp8 = Y;
		if(M == 1 || M == 2){ //一月和二月当作去年十三月和十四月
		
			M_temp8 = M + 12;
			Y_temp8 --;
		}
		else M_temp8 = M;
		
		/*开始计算*/
		W =	 Y_temp8 + (Y_temp8 / 4) + 5 - 40 + (26 * (M_temp8 + 1) / 10) + D - 1;	//蔡勒公式
		W %= 7; 
		
		/*计算结果赋值*/
		W?(systemTime_current.time_Week = W):(systemTime_current.time_Week = 7);
		
		systemTime_current.time_Month = 	M;
		systemTime_current.time_Day = 		D;
		systemTime_current.time_Year = 		Y;
		
		systemTime_current.time_Hour = 		local_datsParam->frameResp[8];
		systemTime_current.time_Minute =	local_datsParam->frameResp[9];
		systemTime_current.time_Second = 	local_datsParam->frameResp[10];

		/*系统本地时间维持计时值校准更新*/
		sysTimeKeep_counter = systemTime_current.time_Minute * 60 + systemTime_current.time_Second; //本地时间维持更新

//		printf_datsHtoA("[Tips_uartZigb]: resultDats:", local_datsParam->frameResp, local_datsParam->frameRespLen);
	}

	os_free(local_datsParam);
	
//	return timeStamp_temp;
	return resultREQ;
}

static bool ZigB_resetInit(void){

	#define zigbInit_loopTry 	3
	#define zigbInit_onceWait 	6000
	
	const uint8_t initCmp_Frame[11] = {0xFE, 0x06, 0x41, 0x80, 0x01, 0x02, 0x00, 0x02, 0x06, 0x03, 0xC3};

	uint8_t loop = 0;
	uint16_t timeWait = 0;
	sttUartRcv_sysDat rptr_uartDatsRcv;
	portBASE_TYPE xMsgQ_rcvResult = pdFALSE;
	bool result_Init = false;

	for(loop = 0; loop < zigbInit_loopTry; loop ++){

		periphLocalPortGpio_l7znp_resetPin(ZIGB_HARDWARE_RESET_LEVEL);
		vTaskDelay(100 / portTICK_RATE_MS);
		periphLocalPortGpio_l7znp_resetPin(ZIGB_HARDWARE_NORMAL_LEVEL);

		ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee hwReset try loop : %d\n", loop + 1);

		timeWait = zigbInit_onceWait;
		while(timeWait --){

			vTaskDelay(1 / portTICK_RATE_MS);
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
			while(xMsgQ_rcvResult == pdTRUE){

				if(!memcmp(rptr_uartDatsRcv.dats, initCmp_Frame, 11)){

					result_Init = true;
					break;
				}
				xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataSysRespond, (void *)&rptr_uartDatsRcv, (portTickType) 0);
			}if(result_Init == true)break;
		}if(result_Init == true)break;
	}

	if(result_Init)ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee hwReset success!\n");
	else ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee hwReset fail!\n");

	return result_Init;
}

static uint16_t ZigB_getPanIDCurrent(void){

	uint16_t PANID_temp = 0;
	datsZigb_reqGet *local_datsParam = (datsZigb_reqGet *)malloc(sizeof(datsZigb_reqGet));
	const uint8_t frameREQ_zigbPanIDGet[6] = {0xFE, 0x01, 0x26, 0x06, 0x06, 0x27};	//zigb PANID获取指令帧
	const uint8_t cmdResp_zigbPanIDGet[2] = {0x66, 0x06};	//zigb PANID获取预期响应指令
	bool resultREQ = false;
	const remoteDataReq_method datsReq_method = {0};

	memset(local_datsParam, 0, sizeof(datsZigb_reqGet));
	resultREQ = zigb_datsRequest((uint8_t *)frameREQ_zigbPanIDGet,
								 6,
								 (uint8_t *)cmdResp_zigbPanIDGet,
								 local_datsParam,
								 300,
								 datsReq_method);
	
	if(true == resultREQ){

		PANID_temp |= (((uint16_t)(local_datsParam->frameResp[5]) << 0)	& 0x00FF);
		PANID_temp |= (((uint16_t)(local_datsParam->frameResp[6]) << 8)	& 0xFF00);

//		printf_datsHtoA("[Tips_uartZigb]: resultDats:", local_datsParam->frameResp, local_datsParam->frameRespLen);
	}

	os_free(local_datsParam);
	return PANID_temp;
}

static bool ZigB_getIEEEAddr(void){

	datsZigb_reqGet *local_datsParam = (datsZigb_reqGet *)malloc(sizeof(datsZigb_reqGet));
	const uint8_t frameREQ_zigbPanIDGet[6] 	= {0xFE, 0x01, 0x26, 0x06, 0x01, 0x20};	//zigb IEEE地址获取指令帧
	const uint8_t cmdResp_zigbPanIDGet[2] 	= {0x66, 0x06};	//zigb IEEE地址获取预期响应指令
	bool resultREQ = false;
	const remoteDataReq_method datsReq_method = {0};

	memset(local_datsParam, 0, sizeof(datsZigb_reqGet));
	resultREQ = zigb_datsRequest((uint8_t *)frameREQ_zigbPanIDGet,
								 6,
								 (uint8_t *)cmdResp_zigbPanIDGet,
								 local_datsParam,
								 500,
								 datsReq_method);

	if(true == resultREQ){

		uint16_t loop = 0;
		
//#if(DEV_MAC_SOURCE_DEF == DEV_MAC_SOURCE_ZIGBEE)
//		for(loop = 0; loop < DEV_MAC_LEN; loop ++){
//			MACSTA_ID[DEV_MAC_LEN - loop - 1] = local_datsParam->frameResp[5 + loop];
//		}
//#else
//#endif	
//		printf_datsHtoA("[Tips_uartZigb]: resultDats:", local_datsParam->frameResp, local_datsParam->frameRespLen);
	}

	os_free(local_datsParam);
	return resultREQ;
}

static bool ZigB_getRandom(void){

#define ZIGB_PANID_MAXVAL 1000

	uint16_t PANID_temp = 0;
	datsZigb_reqGet *local_datsParam = (datsZigb_reqGet *)malloc(sizeof(datsZigb_reqGet));
	const uint8_t frameREQ_zigbPanIDGet[5] 	= {0xFE, 0x00, 0x21, 0x0C, 0x2D};	//zigb 系统随机数获取指令帧
	const uint8_t cmdResp_zigbPanIDGet[2] 	= {0x61, 0x0C};	//zigb 系统随机数获取预期响应指令
	bool resultREQ = false;
	const remoteDataReq_method datsReq_method = {0};

	uint16_t sysZigb_random = 0;

	memset(local_datsParam, 0, sizeof(datsZigb_reqGet));
	resultREQ = zigb_datsRequest((uint8_t *)frameREQ_zigbPanIDGet,
								 5,
								 (uint8_t *)cmdResp_zigbPanIDGet,
								 local_datsParam,
								 300,
								 datsReq_method);

	if(true == resultREQ){

		sysZigb_random = 0;

		sysZigb_random |= (uint16_t)local_datsParam->frameResp[4] << 0;
		sysZigb_random |= (uint16_t)local_datsParam->frameResp[5] << 8;

		sysZigb_random %= ZIGB_PANID_MAXVAL;
	
//		printf_datsHtoA("[Tips_uartZigb]: resultDats:", local_datsParam->frameResp, local_datsParam->frameRespLen);
	}

	os_free(local_datsParam);
	return resultREQ;
}

static bool ZigB_inspectionSelf(bool hwReset_IF){ //是否硬件复位
	
	datsZigb_reqGet *local_datsParam = (datsZigb_reqGet *)malloc(sizeof(datsZigb_reqGet));

	const datsAttr_ZigbInit default_param = {{0x26,0x08}, {0xFC,0xFF,0x00}, 0x03, {0xFE,0x01,0x66,0x08,0x00,0x6F}, 0x06, 500};	//命令帧，默认参数
	const uint8_t frameREQ_zigbJoinNWK[5] 	= {0xFE, 0x00, 0x26, 0x00, 0x26};	//zigb激活网络指令帧
	const uint8_t cmdResp_zigbJoinNWK[2] 	= {0x45, 0xC0};	//zigb激活网络预期响应指令
	const remoteDataReq_method datsReq_method = {0};
	bool resultREQ = false;

	if(hwReset_IF)resultREQ = ZigB_resetInit();
	else resultREQ = true;
	
	if(true == resultREQ){

		memset(local_datsParam, 0, sizeof(datsZigb_reqGet));
		resultREQ = zigb_datsRequest((uint8_t *)frameREQ_zigbJoinNWK,
									 5,
									 (uint8_t *)cmdResp_zigbJoinNWK,
									 local_datsParam,
									 8000,
									 datsReq_method);
	}

	if(true == resultREQ){

//		printf_datsHtoA("[Tips_uartZigb]: resultDats:", local_datsParam->frameResp, local_datsParam->frameRespLen);

		if(local_datsParam->frameResp[4] != 0x09)resultREQ = false;
		else{

			resultREQ = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLSECENARIO);	//设备ID默认13，注册场景控制端点口—12
			if(resultREQ)resultREQ = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLNORMAL);	//设备ID默认13，注册常规端点口—13
			if(resultREQ)resultREQ = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLSYSZIGB); //设备ID默认13，注册系统交互端点口—14
			if(resultREQ)resultREQ = zigb_clusterCtrlEachotherCfg(); //互控端口注册
			if(resultREQ)resultREQ = zigbNetwork_OpenIF(0, 0); //关闭网络
		}
	}

	if(true == resultREQ){

		resultREQ = zigb_VALIDA_INPUT(	(uint8_t *)default_param.zigbInit_reqCMD,
										(uint8_t *)default_param.zigbInit_reqDAT,
										default_param.reqDAT_num,
										(uint8_t *)default_param.zigbInit_REPLY,
										default_param.REPLY_num,
										3,		//3次以内无正确响应则失败
										default_param.timeTab_waitAnsr);	
	}

	ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee inspection result is : %d\n", resultREQ);

	os_free(local_datsParam);
	return resultREQ;
}

static void ZigB_nwkReconnect(void){

	static uint8_t reconnectStep = 1;
	static uint8_t reconnectTryloop = 0;

	switch(reconnectStep){

		ESP_LOGI(TAG, "[Tips_uartZigb]: ZigbeeNwk reconnect start.\n");

		case 1:{

			if(ZigB_inspectionSelf(false)){
			
				reconnectTryloop = 0;
				reconnectStep = 2;
			}
			else
			{
				reconnectTryloop ++;
				ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee reconnectStep 1 try loop%d.\n", reconnectTryloop);
				if(reconnectTryloop > 3){
			
					reconnectTryloop = 0;
					reconnectStep = 1;
				}
			}
			
		}break;

		case 2:{

			reconnectStep = 1;
			
			xQueueReset(xMsgQ_timeStampGet);
			xQueueReset(xMsgQ_Socket2Zigb);
			xQueueReset(xMsgQ_zigbFunRemind);
			
			nwkZigbOnline_IF = true;
			
			ESP_LOGI(TAG, "[Tips_uartZigb]: ZigbeeNwk reconnect compelete.\n");
			
		}break;
	}
}

/*zigbee初始化*/
static bool ZigB_NwkCreat(uint16_t PANID, uint8_t CHANNELS){		

#define	zigbNwkCrateCMDLen 	10	//指令个数
	
#define loop_PANID		5	//指令索引
#define loop_CHANNELS	6	//指令索引

const datsAttr_ZigbInit ZigbInit_dats[zigbNwkCrateCMDLen] = {

	{	{0x41,0x00},	{0x00},					0x01,	{0xFE,0x06,0x41,0x80,0x02,0x02,0x00,0x02,0x06,0x03,0xC0},	0x0B,	5000},	//复位	
	{	{0x41,0x00},	{0x00},					0x01,	{0xFE,0x06,0x41,0x80,0x02,0x02,0x00,0x02,0x06,0x03,0xC0},	0x0B,	5000},	//复位
	{	{0x41,0x00},	{0x00},					0x01,	{0xFE,0x06,0x41,0x80,0x02,0x02,0x00,0x02,0x06,0x03,0xC0},	0x0B,	5000},	//复位
	{	{0x26,0x05},	{0x03,0x01,0x03},		0x03,	{0xFE,0x01,0x66,0x05,0x00,0x62},							0x06,	100	},	//寄存器初始化，参数清空
	{	{0x41,0x00},	{0x00},					0x01,	{0xFE,0x06,0x41,0x80,0x02,0x02,0x00,0x02,0x06,0x03,0xC0},	0x0B,	5000},	//二次复位
	{	{0x27,0x02},	{0x34,0x12},			0x02,	{0xFE,0x01,0x67,0x02,0x00,0x64},							0x06,	100	},	//PAN_ID设置
	{	{0x27,0x03},	{0x00,0x80,0x00,0x00},	0x04,	{0xFE,0x01,0x67,0x03,0x00,0x65},							0x06,	100	},	//信道寄存器配置
	{	{0x26,0x05},	{0x87,0x01,0x00},		0x03,	{0xFE,0x01,0x66,0x05,0x00,0x62},							0x06,	100	},	//角色设置（协调器）
	{	{0x26,0x00},	{0},					0x00,	{0xFE,0x01,0x45,0xC0,0x09,0x8D},							0x06,	8000},	//以既定角色入网/建立网络
	{	{0x26,0x08}, 	{0xFC,0xFF,0x00}, 		0x03, 	{0xFE,0x01,0x66,0x08,0x00,0x6F}, 							0x06, 	200 },	//创建成功后关闭网络
};

#define zigbNwkCrate_paramLen 100
	uint8_t paramTX_temp[zigbNwkCrate_paramLen] = {0};
	
	uint8_t  loop;
	uint32_t chnl_temp = 0x00000800UL << CHANNELS;
	
	for(loop = 1;loop < zigbNwkCrateCMDLen;loop ++){
		
		memset(paramTX_temp, 0, zigbNwkCrate_paramLen * sizeof(uint8_t));

		if(loop == 1){

			if(!ZigB_resetInit())loop = 0;
			else{

				ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee nwkCreat step:%d complete !!!\n", loop);
			}			
		}
		
		switch(loop){	//自选参参数替换默认参数
		
			case loop_PANID:{
			
				paramTX_temp[0] = (uint8_t)((PANID & 0x00ff) >> 0);
				paramTX_temp[1] = (uint8_t)((PANID & 0xff00) >> 8);
			}break;
			
			case loop_CHANNELS:{
			
				paramTX_temp[0] = (uint8_t)((chnl_temp & 0x000000ff) >>  0);
				paramTX_temp[1] = (uint8_t)((chnl_temp & 0x0000ff00) >>  8);
				paramTX_temp[2] = (uint8_t)((chnl_temp & 0x00ff0000) >> 16);
				paramTX_temp[3] = (uint8_t)((chnl_temp & 0xff000000) >> 24);
			}break;
			
			default:{
			
				memcpy(paramTX_temp,ZigbInit_dats[loop].zigbInit_reqDAT,ZigbInit_dats[loop].reqDAT_num);
				
			}break;
		}

		if(loop > 1){

			if(false == zigb_VALIDA_INPUT((uint8_t *)ZigbInit_dats[loop].zigbInit_reqCMD,
										  (uint8_t *)paramTX_temp,
										  ZigbInit_dats[loop].reqDAT_num,
										  (uint8_t *)ZigbInit_dats[loop].zigbInit_REPLY,
										  ZigbInit_dats[loop].REPLY_num,
										  3,
										  ZigbInit_dats[loop].timeTab_waitAnsr)
										 )loop = 0;
			else{
			
				ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee nwkCreat step:%d complete !!!\n", loop);
			}
		}
	}

	ESP_LOGI(TAG, "[Tips_uartZigb]: Zigbee nwkCreat all complete !!!\n");

	bool result_Set = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLSECENARIO); //设备ID默认13，注册场景控制端点口—12
	if(result_Set)result_Set =zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLNORMAL); //设备ID默认13，注册常规端点口—13
	if(result_Set)result_Set = zigb_clusterSet(ZIGB_CLUSTER_DEFAULT_DEVID, ZIGB_ENDPOINT_CTRLSYSZIGB); //设备ID默认13，注册系统交互端点口—14
	if(result_Set)result_Set = zigb_clusterCtrlEachotherCfg();

	return result_Set;
}

static bool ZigB_datsRemoteRX(datsAttr_ZigbTrans *datsRX, uint32_t timeWait){
	
#define zigbcmdRX_Len 2
	const uint8_t cmdRX[zigbcmdRX_Len][2] = {
	
		{0x44,0x81},	//接收到无线节点数据
		{0x45,0xCA},	//无线节点上线通知
	};

	uint8_t *ptr = NULL;
	uint8_t loop = 0;
	
	sttUartRcv_rmoteDatComming rptr_uartDatsRcv;
	portBASE_TYPE xMsgQ_rcvResult = pdFALSE;

	datsRX->datsType = zigbTP_NULL;

	xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_dataRemoteComing, (void *)&rptr_uartDatsRcv,  (portTickType)timeWait);
	if(xMsgQ_rcvResult == pdTRUE){
	
		for(loop = 0;loop < zigbcmdRX_Len;loop ++){
		
			ptr = usr_memmem(rptr_uartDatsRcv.dats, rptr_uartDatsRcv.datsLen, (uint8_t*)cmdRX[loop], 2);
			
			if(ptr != NULL){
			
				switch(loop){
				
					case 0:{
					
						if(fragmentZNP_head == *(ptr - 2) && //信息格式校验:帧头与异或校验
						   rptr_uartDatsRcv.dats[rptr_uartDatsRcv.datsLen - 1] == numCheck_xor(&rptr_uartDatsRcv.dats[1], 0, rptr_uartDatsRcv.datsLen - 2)){		
						   
							/*验证通过，信息填装*/
							datsRX->datsSTT.stt_MSG.ifBroadcast = *(ptr + 10);
							datsRX->datsSTT.stt_MSG.Addr_from	= (((uint16_t)*(ptr + 6) & 0x00FF) << 0) + (((uint16_t)*(ptr + 7) & 0x00FF) << 8);
							datsRX->datsSTT.stt_MSG.srcEP		= *(ptr + 8);
							datsRX->datsSTT.stt_MSG.dstEP		= *(ptr + 9);
							datsRX->datsSTT.stt_MSG.ClusterID	= (((uint16_t)*(ptr + 4) & 0x00FF) << 0) + (((uint16_t)*(ptr + 5) & 0x00FF) << 8);
							datsRX->datsSTT.stt_MSG.LQI 		= *(ptr + 11);
							datsRX->datsSTT.stt_MSG.datsLen 	= *(ptr + 18);
							memset(datsRX->datsSTT.stt_MSG.dats, 0, datsRX->datsSTT.stt_MSG.datsLen * sizeof(uint8_t));
							memcpy(datsRX->datsSTT.stt_MSG.dats, (ptr + 19), *(ptr + 18));
							
							datsRX->datsType = zigbTP_MSGCOMMING;
							return true;
						}
						else{
						   
							rptr_uartDatsRcv.dats[usr_memloc((uint8_t *)rptr_uartDatsRcv.dats, rptr_uartDatsRcv.datsLen, (uint8_t *)cmdRX[loop], 2)] = 0xFF;	//非指定数据则主动污染本段后，再向后复柿
							loop --;	//原段信息向后复查
						}
					}break;
					
					case 1:{
					
						if(fragmentZNP_head == *(ptr - 2) && //信息格式校验:帧头与异或校验
						   rptr_uartDatsRcv.dats[rptr_uartDatsRcv.datsLen - 1] == numCheck_xor(&rptr_uartDatsRcv.dats[1], 0, rptr_uartDatsRcv.datsLen - 2)){
						
							/*验证通过，信息填装*/
							datsRX->datsSTT.stt_ONLINE.nwkAddr_fresh = (((uint16_t)*(ptr + 2) & 0x00FF) << 0) + (((uint16_t)*(ptr + 3) & 0x00FF) << 8);
							
							datsRX->datsType = zigbTP_ntCONNECT;
							return true;
						}
						else{
						   
							rptr_uartDatsRcv.dats[usr_memloc((uint8_t *)rptr_uartDatsRcv.dats, rptr_uartDatsRcv.datsLen, (uint8_t *)cmdRX[loop], 2)] = 0xFF;	//非指定数据则主动污染本段后，再向后复柿
							loop --;	//原段信息向后复查
						}
					}break;
					
					default:break;
				}
			}
		}
	}
	
	return false;
}

static bool ZigB_PANIDReales(bool inspection_IF){ //是否自检

//	stt_usrDats_privateSave *datsRead_Temp = devParam_flashDataRead();
//	stt_usrDats_privateSave datsSave_Temp = {0};
	
	uint16_t panID_temp = ZigB_getPanIDCurrent();
	uint16_t panID_default = 0x1235;
	uint16_t sysZigb_random = 0x1234;
	uint16_t nwkZigb_currentPANID = 0x1234;

	bool result = false; 
	static uint8_t inspectionFail_count = 0; //自检失败次数---防止更换模块不更换主控导致主控残留信息不一致而一直进行错误自检

	ESP_LOGI(TAG, "PANID_current is : 0x%04X.\n", panID_temp);
	ESP_LOGI(TAG, "PANID_flash is : 0x%04X.\n", panID_default);

//	datsRead_Temp->panID_default = 0;

	if((panID_default != panID_temp) || //PANID为零或
	   (!panID_default) || //与本地存储不相符
	   (inspectionFail_count >= 3)){ //或自检失败超过三次

		inspectionFail_count = 0;

		if((!panID_default) || (panID_default == 0xFFFF))panID_default = sysZigb_random; //本地存储PANID为空则赋随机值
		else{

			panID_default = panID_default;
		}

		nwkZigb_currentPANID = panID_default;
		panID_temp = panID_default;
//		devParam_flashDataSave(obj_panID_default, datsSave_Temp);
		
		result = ZigB_NwkCreat(panID_temp, 4);
		
	}else{

		nwkZigb_currentPANID = panID_default; //默认PANID更新

		if(inspection_IF){

			result = ZigB_inspectionSelf(1);
			
		}else{

			result = true;
		}
	}

//	if(datsRead_Temp)os_free(datsRead_Temp);

	if(result){

		ESP_LOGI(TAG, "panID reales success.\n");
		inspectionFail_count = 0;
	}
	else{

		ESP_LOGI(TAG, "panID reales fail.\n");
		inspectionFail_count ++;
	}

	return result;
}

static void ZigB_remoteDatsSend_straightforward(uint16_t DstAddr, //地址
														  uint8_t port, //端口
														  uint8_t dats[], //数据
								                          uint8_t datsLen){ //数据长度

	const uint8_t zigbProtocolCMD_dataSend[2] = {0x24,0x01};
	const uint8_t TransID = 13;
	const uint8_t Option = 0;
	const uint8_t Radius = 7;

	uint8_t buf_datsTX[32] = {0};
	uint8_t datsTX[48] = {0};
	uint8_t datsTX_Len = 0;

	buf_datsTX[0] = (uint8_t)((DstAddr & 0x00ff) >> 0);
	buf_datsTX[1] = (uint8_t)((DstAddr & 0xff00) >> 8);
	buf_datsTX[2] = port;
	buf_datsTX[3] = port;
	buf_datsTX[4] = ZIGB_CLUSTER_DEFAULT_CULSTERID;
	buf_datsTX[6] = TransID;
	buf_datsTX[7] = Option;
	buf_datsTX[8] = Radius;
	buf_datsTX[9] = datsLen;
	memcpy(&buf_datsTX[10],dats,datsLen);	

	datsTX_Len = ZigB_TXFrameLoad(datsTX, (uint8_t *)zigbProtocolCMD_dataSend, 2, (uint8_t *)buf_datsTX, datsLen + 10);

	periphLocalPortUart_l7znp_send(datsTX, datsTX_Len);
}

static bool ZigB_datsTX_ASY(	uint16_t 	DstAddr,
							  	  uint8_t  	SrcPoint,
							   	  uint8_t  	DstPoint,
							   	  uint8_t 	ClustID,
							   	  uint8_t  	dats[],
							   	  uint8_t  	datsLen,
							   	  stt_dataRemoteReq bufQueue[],
							   	  uint8_t   BQ_LEN){

	const uint8_t TransID = 13;
	const uint8_t Option = 0;
	const uint8_t Radius = 7;

	const uint8_t cmd_dataReq[2] = {0x24, 0x01};
	const uint8_t cmd_dataResp[2] = {0x44, 0x80};

#define zigbTX_datsTransLen_ASR (128 + 25)
	uint8_t buf_datsTX[zigbTX_datsTransLen_ASR] = {0};
#define zigbRX_datsTransLen_ASR (8 + 25)
	uint8_t buf_datsRX[zigbRX_datsTransLen_ASR] = {0};

	uint8_t loop = 0;

	for(loop = 0; loop < BQ_LEN; loop ++){

		if(!bufQueue[loop].repeat_Loop){

			memset(bufQueue[loop].dataReq, 0, sizeof(uint8_t) * zigbTX_datsTransLen_ASR);
			memset(bufQueue[loop].dataResp, 0, sizeof(uint8_t) * 8);

			//发送帧填装
			buf_datsTX[0] = (uint8_t)((DstAddr & 0x00ff) >> 0);
			buf_datsTX[1] = (uint8_t)((DstAddr & 0xff00) >> 8);
			buf_datsTX[2] = DstPoint;
			buf_datsTX[3] = SrcPoint;
			buf_datsTX[4] = ClustID;
			buf_datsTX[6] = TransID;
			buf_datsTX[7] = Option;
			buf_datsTX[8] = Radius;
			buf_datsTX[9] = datsLen;
			memcpy(&buf_datsTX[10], dats, datsLen);
			bufQueue[loop].dataReq_Len = ZigB_TXFrameLoad(bufQueue[loop].dataReq, (uint8_t *)cmd_dataReq, 2, (uint8_t *)buf_datsTX, datsLen + 10);

			//应答帧填装
			buf_datsRX[0] = 0x00;
			buf_datsRX[1] = SrcPoint;
			buf_datsRX[2] = TransID;

//			bufQueue[loop].dataResp_Len = 3;
			bufQueue[loop].dataResp_Len = ZigB_TXFrameLoad(bufQueue[loop].dataResp, (uint8_t *)cmd_dataResp, 2, (uint8_t *)buf_datsRX, 3);

			bufQueue[loop].repeat_Loop = zigB_remoteDataTransASY_txReapt; //使能发送 10 次

			if(!memcmp(bufQueue[loop].dataReq, bufQueue[loop - 1].dataReq, bufQueue[loop].dataReq_Len) && 
			   !memcmp(bufQueue[loop].dataResp, bufQueue[loop - 1].dataResp, bufQueue[loop].dataResp_Len)){ //去重

				memset(&bufQueue[loop - 1], 0, sizeof(stt_dataRemoteReq));
			}

			return true;
		}
	}

	ESP_LOGI(TAG, ">>>dataRM reqQ full.\n"); //发送缓存数组过满
	
	memcpy(&localZigbASYDT_bufQueueRemoteReq[0], &localZigbASYDT_bufQueueRemoteReq[zigB_remoteDataTransASY_QbuffLen / 2], sizeof(stt_dataRemoteReq) * zigB_remoteDataTransASY_QbuffLen / 2); //缓存整理，发送缓存组过满处理，发送缓存组清空掉一半
	memset(&localZigbASYDT_bufQueueRemoteReq[zigB_remoteDataTransASY_QbuffLen / 2], 0, sizeof(stt_dataRemoteReq) * zigB_remoteDataTransASY_QbuffLen / 2); //发送缓存组过满处理，发送缓存组清空掉一半

	return false; 
}														  

static bool ZigB_ScenarioTX_ASY( uint16_t DstAddr,
									  uint8_t SrcPoint,
									  uint8_t DstPoint,
									  uint8_t ClustID,
									  uint8_t dats[],
									  uint8_t datsLen,
									  stt_dataScenarioReq bufQueue[],
									  uint8_t BQ_LEN){

	const uint8_t TransID = 13;
	const uint8_t Option = 0;
	const uint8_t Radius = 7;

	const uint8_t cmd_dataReq[2] = {0x24, 0x01};
	const uint8_t cmd_dataResp[2] = {0x44, 0x80};

#define zigbTX_ScenarioLen_ASR 16
	uint8_t buf_datsTX[zigbTX_datsTransLen_ASR] = {0};	

	uint8_t loop = 0;

	for(loop = 0; loop < BQ_LEN; loop ++){

		if(!bufQueue[loop].repeat_Loop){

			memset(bufQueue[loop].dataReq, 0, sizeof(uint8_t) * 16);			

			//发送帧填装
			buf_datsTX[0] = (uint8_t)((DstAddr & 0x00ff) >> 0);
			buf_datsTX[1] = (uint8_t)((DstAddr & 0xff00) >> 8);
			buf_datsTX[2] = DstPoint;
			buf_datsTX[3] = SrcPoint;
			buf_datsTX[4] = ClustID;
			buf_datsTX[6] = TransID;
			buf_datsTX[7] = Option;
			buf_datsTX[8] = Radius;
			buf_datsTX[9] = datsLen;
			memcpy(&buf_datsTX[10], dats, datsLen);
			bufQueue[loop].dataReq_Len = ZigB_TXFrameLoad(bufQueue[loop].dataReq, (uint8_t *)cmd_dataReq, 2, (uint8_t *)buf_datsTX, datsLen + 10);

			//远端应答地址填装
			bufQueue[loop].dataRespNwkAddr = DstAddr;

			bufQueue[loop].repeat_Loop = zigB_ScenarioCtrlDataTransASY_txReapt; //使能发送 30 次
			bufQueue[loop].scenarioOpreatCmp_flg = 0;

			return true;
		}
	}

	ESP_LOGI(TAG, ">>>scenarioCT reqQ full.\n");

	return false; 
}

/*zigbee红外设备<特殊> 场景数据发送 - 非阻塞异步*/
static bool ZigB_devInfraredScenarioTX_ASY(uint16_t 	DstAddr,
												    uint8_t dataCtrlcmd){

	bool res = false;
	uint8_t loop = 0;

	for(loop = 0; loop < zigB_devIrScenarioCtrlDataTransASY_QbuffLen; loop ++){ //已存在，添加指令
		if(DstAddr == localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr){
			localZigbASYDT_bufQueuedevIrScenarioReq[loop].cmdDataSequence[localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum] = dataCtrlcmd;
			localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum ++;
			res = true;

			ESP_LOGI(TAG, ">>>irQ add, addr:0x%04X, ctrlCmd:%02x, num:%d.\n", localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr, 
																		   	  dataCtrlcmd,
																		   	  localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum);

			break;
		}
	}
	if(false == res){
		for(loop = 0; loop < zigB_devIrScenarioCtrlDataTransASY_QbuffLen; loop ++){
			if(0 == localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr){ //不存在，进行首次创建
				localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr = DstAddr;
				localZigbASYDT_bufQueuedevIrScenarioReq[loop].cmdDataSequence[localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum] = dataCtrlcmd;
				localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum ++;
				res = true;

				ESP_LOGI(TAG, ">>>irQ creat, addr:0x%04X, ctrlCmd:%02x, num:%d.\n", localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr, 
																			 		dataCtrlcmd,
																			 		localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum);

				ZigB_ScenarioTX_ASY( DstAddr, //首次创建时触发发送动作
									 ZIGB_ENDPOINT_CTRLSECENARIO,
									 ZIGB_ENDPOINT_CTRLSECENARIO,
									 ZIGB_CLUSTER_DEFAULT_CULSTERID,
									 &localZigbASYDT_bufQueuedevIrScenarioReq[loop].cmdDataSequence[localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteIndex ++],
									 1,
									 localZigbASYDT_bufQueueScenarioReq,
									 zigB_ScenarioCtrlDataTransASY_QbuffLen);
				break;
			}
		}
	}

	return res; 
}

static void ZigB_devInfraredScenarioExcution(uint16_t DstAddr){

	uint8_t loop = 0;
	
	for(loop = 0; loop < zigB_devIrScenarioCtrlDataTransASY_QbuffLen; loop ++){ //已存在，修改
		if(0 == localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr)continue;
		else{
			if((localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteIndex >= localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum) ||\
			   (0 == localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteNum)){
				memset(&localZigbASYDT_bufQueuedevIrScenarioReq[loop], 0, sizeof(stt_devInfraredSceneReq));
			}else
			if(DstAddr == localZigbASYDT_bufQueuedevIrScenarioReq[loop].dataRespNwkAddr){ //执行场景数据
				ZigB_ScenarioTX_ASY( DstAddr, //异步远端数据请求队列加载
									 ZIGB_ENDPOINT_CTRLSECENARIO,
									 ZIGB_ENDPOINT_CTRLSECENARIO,
									 ZIGB_CLUSTER_DEFAULT_CULSTERID,
									 &localZigbASYDT_bufQueuedevIrScenarioReq[loop].cmdDataSequence[localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteIndex],
									 1,
									 localZigbASYDT_bufQueueScenarioReq,
									 zigB_ScenarioCtrlDataTransASY_QbuffLen);
				localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteIndex ++;
				ESP_LOGI(TAG, ">>>Ir excuting, addr:0x%02X, index:%d.\n", DstAddr, localZigbASYDT_bufQueuedevIrScenarioReq[loop].excuteIndex);
			}
		}
	}
}

static void periphInit_l7UartMoudleComunicate(void){

	gpio_config_t io_conf = {0};
	const uart_config_t uart_config = {
		.baud_rate = DEV_UART_L7ZNP_BAUDRATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEV_UART_L7ZNP_RST_PIN);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
	
	uart_param_config(DEV_UART_L7ZNP_PORT, &uart_config);
	uart_set_pin(DEV_UART_L7ZNP_PORT, DEV_UART_L7ZNP_TXD_PIN, DEV_UART_L7ZNP_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	
	// We won't use a buffer for sending data.
	uart_driver_install(DEV_UART_L7ZNP_PORT, HAL7_RING_BUFFER_LENGTH_MAX * 2, 0, 0, NULL, 0);

	ESP_LOGI(TAG, "L7 zigbMoudle uart init complete!\n");
}

static void periphDeinit_l7UartMoudleComunicate(void){

	gpio_reset_pin(DEV_UART_L7ZNP_RST_PIN);
	uart_driver_delete(DEV_UART_L7ZNP_PORT);
}

static void timerCallbackSmartLivingApplication(TimerHandle_t xTimer){

	static struct{
		const uint16_t tPeriod;
		uint16_t tCounter;
	}tCntObj_1s = {1000, 0};

	//1s周期业务
	if(tCntObj_1s.tCounter < tCntObj_1s.tPeriod)tCntObj_1s.tCounter ++;
	else{
		tCntObj_1s.tCounter = 0;

		//链表管理周期
		if(false == zigbNodeDevDetectManage_runningFLG)zigbNodeDevDetectManage_runningFLG = true;
	}
}

static void smartLivingApplicationTimerInit(void){

	localTimerL7Scheduling = xTimerCreate((const char*           )"L7AppTimer",
                                       	  (TickType_t            )1 / portTICK_RATE_MS,
                                       	  (UBaseType_t           )pdTRUE,
                                       	  NULL,
                                       	  (TimerCallbackFunction_t)timerCallbackSmartLivingApplication);
	xTimerStart(localTimerL7Scheduling, 0 / portTICK_RATE_MS);
}

static void processUartdataReceiveLoop(void *pvParameters){

	for(;;){

		/********************
			*uart数据接收*
		********************/
		periphLocalPortUart_l7znp_datsReceiveLoopOnce();
//		periphLocalPortUart_l7znp_send("tx test.\n", 9);
//		ESP_LOGI(TAG, "uart1 dats send.\n");

		vTaskDelay(20 / portTICK_RATE_MS);	
	}

	vTaskDelete(NULL);
}

static void l7BussinessProcessLoop_Z2SMsgHandle(void){

	stt_threadDatsPass 	mptr_S2Z;
	stt_threadDatsPass 	rptr_Z2S;
	portBASE_TYPE 		xMsgQ_rcvResult = pdFALSE;

	xMsgQ_rcvResult = xQueueReceive(xMsgQ_Zigb2Socket, (void *)&rptr_Z2S, 0);
	if(xMsgQ_rcvResult == pdTRUE){

		memcpy(&mptr_S2Z, &rptr_Z2S, sizeof(stt_threadDatsPass));

		switch(rptr_Z2S.msgType){ //消息类型判断

			/*常规数据中转*/
			case conventional:{

				switch(rptr_Z2S.dats.dats_conv.datsFrom){

					case datsFrom_ctrlLocal:{

						
					}break;

					case datsFrom_ctrlRemote:{}break;

					case datsFrom_heartBtRemote:{

						switch(rptr_Z2S.dats.dats_conv.dats[0]){
							
							case ZIGB_FRAME_HEAD_HEARTB:{

								switch(rptr_Z2S.dats.dats_conv.dats[2]){

									case ZIGB_FRAME_HEARTBEAT_cmdEven:{

										mptr_S2Z.dats.dats_conv.dats[0] = ZIGB_OFFLINEFRAMEHEAD_HEARTBEAT;
										mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_heartBtRemote;

										xQueueSend(xMsgQ_Socket2Zigb, (void *)&mptr_S2Z, 0);

									}break;
										
									case ZIGB_FRAME_HEARTBEAT_cmdOdd:
									default:{}break;
								}
							
							}break;

							case DTMODEKEEPACESS_FRAMEHEAD_ONLINE:{

								mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_heartBtRemote;

								if(rptr_Z2S.dats.dats_conv.dats[8] == DTMODEKEEPACESS_FRAMECMD_ASR){

									uint8_t frameLen = mptr_S2Z.dats.dats_conv.dats[1];

									memset(&mptr_S2Z.dats.dats_conv.dats[15], 0, sizeof(stt_agingDataSet_bitHold));
									mptr_S2Z.dats.dats_conv.dats[frameLen - 1] = \
										frame_Check(&mptr_S2Z.dats.dats_conv.dats[1], frameLen - 2);
								}

								xQueueSend(xMsgQ_Socket2Zigb, (void *)&mptr_S2Z, 0);

							}break;

							default:{}break;
						}
						
					}break;

					default:{}break;
				}
			}break;

			/*子设备列表请求数据中转*/
			case listDev_query:{}break;

			default:break;
		}
	}
}

static void smartLivingApplicationProcessL7Bussiness(void *pvParameters){

#define zigB_mThread_dispLen  		150	//打印信息缓存
	char disp[zigB_mThread_dispLen] = {0};
#define zigB_mThread_datsMSG_Len 	100	//打印数据缓存
	char dats_MSG[zigB_mThread_datsMSG_Len] = {0};

#define zigB_datsTX_bufferLen 		128	//zigB数据发送缓存
	uint8_t	datsKernel_TXbuffer[zigB_datsTX_bufferLen] = {0};

	uint8_t local_insertRecord_datsReqNormal = 0; //静态值：本地普通非阻塞远端数据请求队列索引
	uint8_t local_insertRecord_datsReqScenario = 0; //静态值：场景集群控制非阻塞远端数据请求队列索引
	uint8_t local_scenarioRecord_dtRound = 0;  //静态值：当前为第几轮次记录
	uint16_t local_scenarioCtrl_counterRoundPause = 0; //数据发送轮次间距暂歇时间计数值
	uint8_t local_ctrlRecord_reserveLoopInsert = zigB_ScenarioCtrlDataTransASY_opreatOnceNum; //静态值：场景可用操作单元数目(分批次专用)

	datsAttr_ZigbTrans *local_datsRX = (datsAttr_ZigbTrans *)malloc(sizeof(datsAttr_ZigbTrans)); //数据接收缓存
	const uint16_t zigDev_lifeCycle = 240; //节点设备心跳周期（单位：s），周期内无心跳更新，节点设备将被判决死亡同时从链表中优化清除
	nwkStateAttr_Zigb *ZigbDevNew_temp = NULL; //节点设备信息缓存
	nwkStateAttr_Zigb ZigbDevNew_tempInsert; //节点设备插入链表前预缓存

	const uint16_t detectPeriod_zigbSlaveDevListLen = 200; //zib子设备链表长度检测周期 单位：约10ms
	uint16_t detectCounter_zigbSlaveDevListLen = 0; //zib子设备链表长度检测计时计数缓存

	stt_threadDatsPass mptr_Z2S = {0},	//通信线程发送消息队列缓存，zigbee通信主线程到socket通信主线程
					   rptr_S2Z = {0};	//通信线程接收消息队列缓存，socket通信主线程到zigbee通信主线程

	uint32_t rptr_timeStamp;
	enum_zigbFunMsg rptr_zigbFunRm;

	portBASE_TYPE xMsgQ_rcvResult = pdFALSE;

	zigbDevList_Head = (nwkStateAttr_Zigb *)malloc(sizeof(nwkStateAttr_Zigb)); //节点设备信息链表 表头创建
	memset(zigbDevList_Head, 0, sizeof(nwkStateAttr_Zigb)); //链表复位
	zigbDevList_Head->next = NULL; 

	memset(local_datsRX, 0, sizeof(datsAttr_ZigbTrans));

	vTaskDelay(100 / portTICK_RATE_MS); //线程启动延时

	while(!ZigB_resetInit())vTaskDelay(20 / portTICK_RATE_MS);

	ESP_LOGI(TAG, "ZIGB sysRandom get result is: %d.\n", ZigB_getRandom()); //PANID预取随机数更新
	vTaskDelay(10 / portTICK_RATE_MS);
	ESP_LOGI(TAG, "IEEE to MAC result is: %d.\n", ZigB_getIEEEAddr()); //MAC更新
	vTaskDelay(20 / portTICK_RATE_MS);
	while(!ZigB_PANIDReales(1)); //panid自检/更新	
	
	nwkZigbOnline_IF = true;

	for(;;){ //业务循环

		/*************************
			*zigb设备链表监管*
		*************************/
		if(zigbNodeDevDetectManage_runningEN && zigbNodeDevDetectManage_runningFLG){
		
			if(zigbNodeDevDetectManage_runningFLG)zigbNodeDevDetectManage_runningFLG = false;
		
			nwkStateAttr_Zigb *pHead_listDevInfo = (nwkStateAttr_Zigb *)zigbDevList_Head;
			if(zigbDev_chatLenDectect(pHead_listDevInfo)){
			
				while(pHead_listDevInfo->next != NULL){
				
					pHead_listDevInfo = pHead_listDevInfo->next;

					if(pHead_listDevInfo->devStateRefreshCdCnt > 0)pHead_listDevInfo->devStateRefreshCdCnt --; //状态更新冷却时间更新
					if(pHead_listDevInfo->onlineDectect_LCount > 0)pHead_listDevInfo->onlineDectect_LCount --; //zigb网络内节点数量更新
					else{
						
						uint16_t p_nwkRemove = pHead_listDevInfo->nwkAddr;
						
//						while(false == zigbDev_eptRemove_BYnwk((nwkStateAttr_Zigb *)pHead_listDevInfo,p_nwkRemove));
			
						zigbNwkReserveNodeNum_currentValue --; //节点总数更新
			
						ESP_LOGI(TAG, "[Tips_uartZigb]: nodeDev remove result is %d\n", zigbDev_eptRemove_BYnwk((nwkStateAttr_Zigb *)zigbDevList_Head, p_nwkRemove));
						ESP_LOGI(TAG, "[Tips_uartZigb]: nodeDev(nwkAddr: 0x%04X) has been optimized cause inactive.\n", p_nwkRemove);
					}
				}
			}
		}

		/************************
			*Z2S业务自行接管* --如果没有远程
		************************/
//		l7BussinessProcessLoop_Z2SMsgHandle();

		/*zigbee 当前实时PANID数值周期性读取更新*/
		if(!zigbee_currentPanID_reslesCounter){

			zigbee_currentPanID_reslesCounter = ZIGBPANID_CURRENT_REALESPERIOD;
			nwkZigb_currentPANID = ZigB_getPanIDCurrent();
		}

		if(nwkZigbOnline_IF){ //模块是否正常在线

			/****************************
				*zigb系统功能请求响应*
			*****************************/
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_zigbFunRemind, (void *)&rptr_zigbFunRm, 0);
			if(xMsgQ_rcvResult == pdTRUE){

				frame_zigbSysCtrl datsTemp_zigbSysCtrl = {0}; //系统控制数据帧缓存
				bool nodeCMDtranslate_EN = false; //远程数据传输使能

				switch(rptr_zigbFunRm){

					case msgFun_nwkOpen:{ //开发网络，使能新节点加入

						zigbNetwork_OpenIF(1, ZIGBNWKOPENTIME_DEFAULT); //自身响应网络开放请求

						/*zigbee网络开放应用层二次通知*///zigbee开放网络指令本身就是广播性质，可以不进行应用层二次广播补充
//						datsTemp_zigbSysCtrl.command = ZIGB_SYSCMD_NWKOPEN;
//						datsTemp_zigbSysCtrl.dats[0] = ZIGBNWKOPENTIME_DEFAULT;
//						datsTemp_zigbSysCtrl.datsLen = 1;

//						nodeCMDtranslate_EN = true;

					}break;

					case msgFun_nodeSystimeSynchronous:{ //设置子节点系统时间，进行网络时间同步

						uint32_t timeStmap_temp = 0UL;

//						if(nwkInternetOnline_IF){ //internet在线则获取sntp_UTC下发

//							timeStmap_temp = sntp_get_current_timestamp();
//							
//						}else{ //否则直接取本地UTC

//							timeStmap_temp = systemUTC_current;
//						}

						timeStmap_temp = systemUTC_current;
						
						datsTemp_zigbSysCtrl.command = l7PTL_ZIGB_SYSCMD_TIMESET;
						datsTemp_zigbSysCtrl.dats[0] = (uint8_t)((timeStmap_temp & 0x000000FF) >> 0); //UTC
						datsTemp_zigbSysCtrl.dats[1] = (uint8_t)((timeStmap_temp & 0x0000FF00) >> 8);
						datsTemp_zigbSysCtrl.dats[2] = (uint8_t)((timeStmap_temp & 0x00FF0000) >> 16);
						datsTemp_zigbSysCtrl.dats[3] = (uint8_t)((timeStmap_temp & 0xFF000000) >> 24);
						datsTemp_zigbSysCtrl.dats[4] = (uint8_t)(systemParamL7SmartLivingZnp.sysTimeZone_H); //时区_时
						datsTemp_zigbSysCtrl.dats[5] = (uint8_t)(systemParamL7SmartLivingZnp.sysTimeZone_M); //时区_分
						datsTemp_zigbSysCtrl.dats[6] = 0; //后期调整为下发时区，但不作时区补偿
						datsTemp_zigbSysCtrl.datsLen = 6;
						
						nodeCMDtranslate_EN = true;
					
					}break;

					case msgFun_localSystimeZigbAdjust:{ //读取zigbee内时间，并将系统本地时间与其同步

						zigB_sysTimeGetRealesWithLocal();
						nodeCMDtranslate_EN = false;
						
					}break;

					case msgFun_portCtrlEachoRegister:{ //立即注册互控端口

						bool result_Set = zigb_clusterCtrlEachotherCfg();
						nodeCMDtranslate_EN = false;
					
					}break;

					case msgFun_panidRealesNwkCreat:{ //PANID网络立即更新

						ZigB_PANIDReales(false);
						nodeCMDtranslate_EN = false;
					
					}break;

					case msgFun_scenarioCrtl:{ //场景控制即刻群发

						uint8_t loop = 0;
						uint8_t datsSend_temp[1] = {0};
						stt_scenarioOprateDats scenarioOprateDats_temp = {0}; //操作缓存，避免直接操作全局变量
						
						vTaskDelay(50); //等待场景控制响应回复队列冷却
						memcpy(&scenarioOprateDats_temp, &scenarioOprateDats, sizeof(stt_scenarioOprateDats));
						ESP_LOGI(TAG, ">>>scenario opreatNum:%d.\n", scenarioOprateDats_temp.devNode_num);
						memset(localZigbASYDT_bufQueueScenarioReq, 0, sizeof(stt_dataScenarioReq) * zigB_ScenarioCtrlDataTransASY_QbuffLen); //前次操作控制缓存冲刷
						xQueueReset(xMsgQ_uartToutDats_scenarioCtrlResp);  //前次回码队列缓存冲刷
						localZigbASYDT_scenarioCtrlReserveAllnum = scenarioOprateDats_temp.devNode_num; //当前剩余有效的场景操作单位数目更新
						local_scenarioRecord_dtRound = 0; //场景操作轮次缓存变量复位
						for(loop = 0; loop < scenarioOprateDats_temp.devNode_num; loop ++){

							nwkStateAttr_Zigb *infoZigbDevRet_temp = zigbDev_eptPutout_BYpsy(zigbDevList_Head, 
																							 scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC, 
																							 DEVZIGB_DEFAULT, 
																							 true);
							if(infoZigbDevRet_temp){ //网络短地址获取

								if(L7_SWITCH_TYPE_INFRARED == infoZigbDevRet_temp->devDefClass){ //为避免同一场景内有多条红外指令，红外设备场景操作进行特殊处理

									ZigB_devInfraredScenarioTX_ASY(infoZigbDevRet_temp->nwkAddr, scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_opStatus); //数据填装

								}else{ //非红外设备

									datsSend_temp[0] = scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_opStatus; //数据填装
									ZigB_ScenarioTX_ASY( infoZigbDevRet_temp->nwkAddr, //异步远端数据请求队列加载
	 													 ZIGB_ENDPOINT_CTRLSECENARIO,
														 ZIGB_ENDPOINT_CTRLSECENARIO,
														 ZIGB_CLUSTER_DEFAULT_CULSTERID,
														 datsSend_temp,
														 1,
														 localZigbASYDT_bufQueueScenarioReq,
														 zigB_ScenarioCtrlDataTransASY_QbuffLen);
									infoZigbDevRet_temp->onlineDectect_LCount = zigDev_lifeCycle; //更新节点设备在列表中的生命周期
								}

								infoZigbDevRet_temp->onlineDectect_LCount = zigDev_lifeCycle; //更新节点设备在列表中的生命周期

								infoZigbDevRet_temp = NULL;
								if(infoZigbDevRet_temp)os_free(infoZigbDevRet_temp);
								
							}
							else
							{
								localZigbASYDT_scenarioCtrlReserveAllnum --;
								ESP_LOGI(TAG, "scenario zigbAddr get fail with mac:%02X %02X %02X %02X %02X-(%2d).\n",	scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC[0],
																														scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC[1],
																														scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC[2],
																														scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC[3],
																														scenarioOprateDats_temp.scenarioOprate_Unit[loop].devNode_MAC[4],
																														loop);
							}
						}

//						memset(&scenarioOprateDats_temp, 0, sizeof(stt_scenarioOprateDats));  //数据复位

						if(!scenarioCtrlOpreatStart_FLG)scenarioCtrlOpreatStart_FLG = true; //场景控制操作开始

						datsTemp_zigbSysCtrl.command = l7PTL_ZIGB_SYSCMD_DATATRANS_HOLD; //主动使子设备通信挂起，为场景通信让路
						datsTemp_zigbSysCtrl.dats[0] = localZigbASYDT_scenarioCtrlReserveAllnum; //网络心跳挂起，挂起时间由场景内单位个数而定，个数越多挂起时间越长
						datsTemp_zigbSysCtrl.datsLen = 1;
						
						nodeCMDtranslate_EN = true;

					}break;

					case msgFun_dtPeriodHoldPst:{

						datsTemp_zigbSysCtrl.command = l7PTL_ZIGB_SYSCMD_DATATRANS_HOLD;
						datsTemp_zigbSysCtrl.dats[0] = 15; //默认挂起15s
						datsTemp_zigbSysCtrl.datsLen = 1;
						
						nodeCMDtranslate_EN = true;
						
					}break;

					case msgFun_dtPeriodHoldCancelAdvance:{

						datsTemp_zigbSysCtrl.command = l7PTL_ZIGB_SYSCMD_DATATRANS_HOLD;
						datsTemp_zigbSysCtrl.dats[0] = 1; //1s预取消
						datsTemp_zigbSysCtrl.datsLen = 1;
						
						nodeCMDtranslate_EN = true;

					}break;

					default:{

						nodeCMDtranslate_EN = false;
							
					}break;
				}

				if(nodeCMDtranslate_EN){

					bool TXCMP_FLG = false;

					memset(datsKernel_TXbuffer, 0, sizeof(uint8_t) * zigB_datsTX_bufferLen);
					ZigB_sysCtrlFrameLoad(datsKernel_TXbuffer, datsTemp_zigbSysCtrl);

					TXCMP_FLG = ZigB_datsTX_ASY(0xFFFF, 
												ZIGB_ENDPOINT_CTRLSYSZIGB,
												ZIGB_ENDPOINT_CTRLSYSZIGB,
												ZIGB_CLUSTER_DEFAULT_CULSTERID,
												(uint8_t *)datsKernel_TXbuffer,
												2 + datsTemp_zigbSysCtrl.datsLen, //命令长度 1 + 数据长度说明 1 + 数据长度 n 
												localZigbASYDT_bufQueueRemoteReq,
												zigB_remoteDataTransASY_QbuffLen);
				}
			}

			/****************************
				*zigb互控业务<暂无>*
			****************************/
			
			/*************************
				*zigb系统时间设置*
			*************************/
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_timeStampGet, (void *)&rptr_timeStamp, 0);
			if(xMsgQ_rcvResult == pdTRUE){
	
				if(rptr_timeStamp)zigB_sysTimeSet(rptr_timeStamp - ZIGB_UTCTIME_START); //zigbee UTC 负补偿
	
//				if(rptr_timeStamp == zigB_sysTimeGet())ESP_LOGI(TAG, "time right.\n");
	
//				ESP_LOGI(TAG, "[Tips_uartZigb]: zigbee msgQ is: 0x%08X.\n", rptr_timeStamp);
			}
	
			/*********************************
				*Internet网络交互数据处理*
			*********************************/
			xMsgQ_rcvResult = xQueueReceive(xMsgQ_Socket2Zigb, (void *)&rptr_S2Z, 0);
			if(xMsgQ_rcvResult == pdTRUE){
	
				switch(rptr_S2Z.msgType){
	
					case conventional:{
	
						uint16_t zigb_nwkAddrTemp = 0xFFFF;
						bool TXCMP_FLG = false;
	
						if((rptr_S2Z.dats.dats_conv.dats[3] == l7PTL_FRAME_MtoSCMD_cmdConfigSearch) && //若为配置指令，则广播
						   (rptr_S2Z.dats.dats_conv.datsFrom == datsFrom_ctrlLocal) ){	
						   
							const  uint8_t localPeriod_nwkTrig = 1;
							static uint8_t localCount_searchREQ = 2;
							
							if(localCount_searchREQ < localPeriod_nwkTrig)localCount_searchREQ ++;
							else{ //localPeriod_nwkTrig次搜索触发一次开放网络，取决于搜索码发送周期
							
								localCount_searchREQ = 0;
//								if(!deviceLock_flag)usrZigbNwkOpen_start(); //配置搜索时通知网内所有节点开放网络加入窗口,网关设备上锁情况下则不进行网络开放
							}

							ESP_LOGI(TAG, "[Tips_ZIGB-NWKmsg]: rcvMsg local cmd: %02X !!!\n", rptr_S2Z.dats.dats_conv.dats[3]);
							zigb_nwkAddrTemp = 0xFFFF;
						}
						else{
	
							nwkStateAttr_Zigb *infoZigbDevRet_temp = zigbDev_eptPutout_BYpsy(zigbDevList_Head, rptr_S2Z.dats.dats_conv.macAddr, DEVZIGB_DEFAULT, false);
							if(NULL != infoZigbDevRet_temp){
							
								zigb_nwkAddrTemp =	infoZigbDevRet_temp->nwkAddr;
								os_free(infoZigbDevRet_temp);
							
							}else{
	
								zigb_nwkAddrTemp = 0;
								ESP_LOGI(TAG, ">>>insert list fail:with mac: %02X %02X %02X %02X %02X.\n", rptr_S2Z.dats.dats_conv.macAddr[0],
																										   rptr_S2Z.dats.dats_conv.macAddr[1],
																										   rptr_S2Z.dats.dats_conv.macAddr[2],
																										   rptr_S2Z.dats.dats_conv.macAddr[3],
																										   rptr_S2Z.dats.dats_conv.macAddr[4]);
							}
						}
	
						if(zigb_nwkAddrTemp){
	
							memset(datsKernel_TXbuffer, 0, sizeof(uint8_t) * zigB_datsTX_bufferLen);
							memcpy(datsKernel_TXbuffer, rptr_S2Z.dats.dats_conv.dats, rptr_S2Z.dats.dats_conv.datsLen);
						
							TXCMP_FLG = ZigB_datsTX_ASY(zigb_nwkAddrTemp,	
														ZIGB_ENDPOINT_CTRLNORMAL,
														ZIGB_ENDPOINT_CTRLNORMAL,
														ZIGB_CLUSTER_DEFAULT_CULSTERID,
														(uint8_t *)datsKernel_TXbuffer,
														rptr_S2Z.dats.dats_conv.datsLen,
														localZigbASYDT_bufQueueRemoteReq,
														zigB_remoteDataTransASY_QbuffLen);
						}
						
					}break;
	
					case listDev_query:{
	
	
					}break;
	
					default:{
	
	
					}break;
				}
			}
			else{
	
//				ESP_LOGI(TAG, "[TEST]: receive msg fail, result: %d !!!\n", xResult);
			}

			/*****************************
				*zigb网络交互数据处理*
			*****************************/			
			if(true == ZigB_datsRemoteRX(local_datsRX, 0)){
	
				memset(disp, 0, zigB_mThread_dispLen * sizeof(char));
				memset(dats_MSG, 0, zigB_mThread_datsMSG_Len * sizeof(char));
	
				switch(local_datsRX->datsType){
	
					//======一级协议层数据类型：数据传输=====//
					case zigbTP_MSGCOMMING:{

						//端点口判定
						switch(local_datsRX->datsSTT.stt_MSG.srcEP){

							case ZIGB_ENDPOINT_CTRLSECENARIO:{ //端点口数据解析：场景反向控制，针对用于zigb子设备反向网关主动请求场景控制

								frame_zigbScenarioReverseCtrl datsTempRX_zigbScenarioReverseCtrl = {0};
								frame_zigbScenarioReverseCtrl datsTempTX_zigbScenarioReverseCtrl = {0};

								static uint8_t DataOnceReserve_pWord_local = 0; //本地口令值

								memcpy(&datsTempRX_zigbScenarioReverseCtrl, &(local_datsRX->datsSTT.stt_MSG.dats[0]), sizeof(frame_zigbScenarioReverseCtrl)); //数据结构化

//								ESP_LOGI(TAG, ">>>cmd:%02X, num:%02X, pWord:%02X.\n", datsTempRX_zigbScenarioReverseCtrl.command,
//																				  datsTempRX_zigbScenarioReverseCtrl.scenario_Num,
//																				  datsTempRX_zigbScenarioReverseCtrl.dataOnceReserve_pWord);

								if(DataOnceReserve_pWord_local != datsTempRX_zigbScenarioReverseCtrl.dataOnceReserve_pWord){ //通过口令判断是否为重复发送的数据
							
									DataOnceReserve_pWord_local = datsTempRX_zigbScenarioReverseCtrl.dataOnceReserve_pWord; //本地口令更新

									switch(datsTempRX_zigbScenarioReverseCtrl.command){

										case zigbScenarioReverseCtrlCMD_scenarioCtrl:{ //子命令，场景控制（来自子设备的反向控制）

											uint8_t loop = 0;

											ESP_LOGI(TAG, ">>>scenario reverseCtrl cmd coming(sned by devNode), insertNum:%d.\n", datsTempRX_zigbScenarioReverseCtrl.scenario_Num);

//											stt_scenarioDataLocalSave *scenarioCtrlData = devParam_scenarioDataLocalRead(datsTempRX_zigbScenarioReverseCtrl.scenario_Num); //网关内部存储场景数据读取
											stt_scenarioDataLocalSave *scenarioCtrlData = {0}; //网关内部存储场景数据读取

											if(scenarioCtrlData->scenarioReserve_opt == scenarioOpt_enable){ //场景数据是否可用判断

												memset(&scenarioOprateDats, 0, sizeof(stt_scenarioOprateDats));  //场景控制缓存数据复位
												memcpy(scenarioOprateDats.scenarioOprate_Unit, scenarioCtrlData->scenarioOprate_Unit, sizeof(scenarioOprateUnit_Attr) * zigB_ScenarioCtrlDataTransASY_QbuffLen); //场景数据赋值
												scenarioOprateDats.devNode_num = scenarioCtrlData->devNode_num; //场景设备数量赋值
												scenarioOprateDats.scenarioCtrlOprate_IF = false; //主动触发操作，此判断条件不需要置位
												
												for(loop = 0; loop < scenarioOprateDats.devNode_num; loop ++){ //查找网关自身MAC是否在内
												
													if(!memcmp(&MACSTA_ID[1], scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC, 5)){ //本身存在于场景就响应动作
												
//														swCommand_fromUsr.objRelay = scenarioOprateDats.scenarioOprate_Unit[loop].devNode_opStatus;
//														swCommand_fromUsr.actMethod = relay_OnOff;

														/*场景是否触发互控?*/
//														if(SWITCH_TYPE == SWITCH_TYPE_SWBIT1 || SWITCH_TYPE == SWITCH_TYPE_SWBIT2 || SWITCH_TYPE == SWITCH_TYPE_SWBIT3)EACHCTRL_realesFLG |= (status_actuatorRelay ^ swCommand_fromUsr.objRelay); //有效互控位触发
//														else
//														if(SWITCH_TYPE == SWITCH_TYPE_CURTAIN)EACHCTRL_realesFLG = 1; //有效互控触发
														
														memset(&scenarioOprateDats.scenarioOprate_Unit[loop], 0, 1 * sizeof(scenarioOprateUnit_Attr)); //同时从场景控制表中剔除网关自身
														memcpy(&scenarioOprateDats.scenarioOprate_Unit[loop], &scenarioOprateDats.scenarioOprate_Unit[loop + 1], (zigB_ScenarioCtrlDataTransASY_QbuffLen - loop - 1) * sizeof(scenarioOprateUnit_Attr));
														scenarioOprateDats.devNode_num --;
												
														break;
													}
												}
												
												enum_zigbFunMsg mptr_zigbFunRm = msgFun_scenarioCrtl; //消息即刻转发至zigb线程进行场景集群控制下发
												xQueueSend(xMsgQ_zigbFunRemind, (void *)&mptr_zigbFunRm, 0);
											}
											else{

												ESP_LOGI(TAG, ">>>current scenario data is not reserve, scenarioReserve_opt:%02X\n", scenarioCtrlData->scenarioReserve_opt);
											}

											if(scenarioCtrlData)os_free(scenarioCtrlData); //内存释放

										}break;

										default:{



										}break;
									}
								}

							}break;

							case ZIGB_ENDPOINT_CTRLNORMAL:{ //端点口数据解析：常规控制

//								/*数据包log输出*/
//								for(loop = 0;loop < local_datsRX->datsSTT.stt_MSG.datsLen;loop ++){
//								
//									sprintf((char *)&dats_MSG[loop * 3],"%02X ",local_datsRX->datsSTT.stt_MSG.dats[loop]);
//								}
//								ESP_LOGI(TAG, "[Tips_uartZigb]: datsRcv from 0x%04X<len:%d> : %s.\n", local_datsRX->datsSTT.stt_MSG.Addr_from, local_datsRX->datsSTT.stt_MSG.datsLen, dats_MSG);
			
								/*数据处理*/
								uint8_t devMAC_Temp[DEVL7MAC_LEN] = {0};
								threadDatsPass_objDatsFrom datsFrom_obj = datsFrom_ctrlLocal;
								switch(local_datsRX->datsSTT.stt_MSG.dats[0]){	//取MAC
								
									case ZIGB_FRAMEHEAD_CTRLLOCAL:{
			
										memcpy(devMAC_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[5]), DEVL7MAC_LEN); //数据包下标5-9为MAC地址,MAC前一位为校验码
										datsFrom_obj = datsFrom_ctrlLocal;
									
									}break;
									
									case ZIGB_FRAMEHEAD_CTRLREMOTE:{
								
										memcpy(devMAC_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[17]), DEVL7MAC_LEN); //数据包下标17-21为MAC地址,MAC前一位为校验码
										datsFrom_obj = datsFrom_ctrlRemote;
									
									}break;
								
									case ZIGB_FRAMEHEAD_HEARTBEAT:{
								
										memcpy(devMAC_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[3 + 1]), DEVL7MAC_LEN); //数据包下标4-8为MAC地址,MAC前一位为校验码
										datsFrom_obj = datsFrom_heartBtRemote;
									
									}break;

									case DTMODEKEEPACESS_FRAMEHEAD_ONLINE:{

										memcpy(devMAC_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[2]), DEVL7MAC_LEN); //数据包下标2-6为MAC地址,MAC前一位为校验码
										datsFrom_obj = datsFrom_heartBtRemote;
										
									}break;
								
									default:{
								
										memcpy(devMAC_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[5]), DEVL7MAC_LEN); //数据包下标5-9为MAC地址,MAC前一位为校验码
										datsFrom_obj = datsFrom_ctrlLocal;
										
									}break;
								}
								
								/*指定子设备数据探测，log输出*/
//								if(!memcmp(debugLogOut_targetMAC ,devMAC_Temp, 5)){
								if(1){

									/*数据包log输出*/
									ESP_LOGI(TAG, "[Tips_ZIGB-ZBdats]: rcv msg(Len: %d) from MAC:<%02X %02X %02X %02X %02X>-nwkAddr[%04X], kaCmd:%02X, devState:%02X.\n",
											local_datsRX->datsSTT.stt_MSG.datsLen,
											devMAC_Temp[0],
											devMAC_Temp[1],
											devMAC_Temp[2],
											devMAC_Temp[3],
											devMAC_Temp[4],
											local_datsRX->datsSTT.stt_MSG.Addr_from,
											local_datsRX->datsSTT.stt_MSG.dats[8],
											local_datsRX->datsSTT.stt_MSG.dats[21]);

//									switch(local_datsRX->datsSTT.stt_MSG.dats[9]){ //指定调试专用MAC 不同的设备类型打印调试数据

//										case SWITCH_TYPE_SOCKETS:{

//											uint8_t debugData_Temp[4] = {0};
//											char debug_str[12] = {0};

//											/*8266 sdk库printf()没有 %f浮点数 打印功能，所以需要自定义的ftoa转换*/
//											memcpy(debugData_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[72]), 4); //power频率
//											ftoa(debug_str, bytesTo_float(debugData_Temp), 5);
//											ESP_LOGI(TAG, "[dev_socket debugDats]: current powerFreq:%sHz,", debug_str);
//											
//											memcpy(debugData_Temp, &(local_datsRX->datsSTT.stt_MSG.dats[64]), 4); //power值
//											ftoa(debug_str, bytesTo_float(debugData_Temp), 5);
//											ESP_LOGI(TAG, "valPower:%sW,", debug_str);
//											
//											memcpy(&debugData_Temp[1], &(local_datsRX->datsSTT.stt_MSG.dats[68]), 3); //电量值
//											debugData_Temp[0] = 0;
//											ftoa(debug_str, bytesTo_float(debugData_Temp), 5);
//											ESP_LOGI(TAG, "valEleConsum:%skWh.\n", debug_str);

//										}break;

//										default:{}break;
//									}
								}

//								/*数据包log输出*/
//								ESP_LOGI(TAG, "[Tips_ZIGB-ZBdats]: rcv msg(Len: %d) from MAC:<%02X %02X %02X %02X %02X>-nwkAddr[%04X].\n",
//										  local_datsRX->datsSTT.stt_MSG.datsLen,
//										  devMAC_Temp[0],
//										  devMAC_Temp[1],
//										  devMAC_Temp[2],
//										  devMAC_Temp[3],
//										  devMAC_Temp[4],
//										  local_datsRX->datsSTT.stt_MSG.Addr_from);

								/*数据处理-节点设备链表更新*/
								ZigbDevNew_temp = zigbDev_eptPutout_BYnwk(zigbDevList_Head, local_datsRX->datsSTT.stt_MSG.Addr_from, true);
								if(NULL == ZigbDevNew_temp){	//判断是否为新增节点设备，是则更新生命周期，否则添加进链表
								
									if(local_datsRX->datsSTT.stt_MSG.Addr_from != 0 && local_datsRX->datsSTT.stt_MSG.datsLen >= (DEVL7MAC_LEN + 1)){	//数据来源判断（本地广播时自己也会收到 则不理会）和 数据长度判断（数据包含MAC地址和设备类型，因此长度必大于该长度之和）
									
										ZigbDevNew_tempInsert.nwkAddr = local_datsRX->datsSTT.stt_MSG.Addr_from;
										memcpy(ZigbDevNew_tempInsert.macAddr, devMAC_Temp, DEVL7MAC_LEN);
										ZigbDevNew_tempInsert.devType = DEVZIGB_DEFAULT;	//数据包下发默认设备（开关）类型
										ZigbDevNew_tempInsert.devDefClass = local_datsRX->datsSTT.stt_MSG.dats[9];
										ZigbDevNew_tempInsert.devStateData = local_datsRX->datsSTT.stt_MSG.dats[21];
										ZigbDevNew_tempInsert.onlineDectect_LCount = zigDev_lifeCycle;
										ZigbDevNew_tempInsert.next = NULL;
										
										zigbDev_eptCreat(zigbDevList_Head, ZigbDevNew_tempInsert);	//zigbee节点设备信息注册进链表
										zigbNwkReserveNodeNum_currentValue = zigbDev_chatLenDectect(zigbDevList_Head); //zigb网络内节点数量更新
//										ZigBdevListInfoSaveToFlash(zigbDevList_Head); //设备链表信息flalsh保存
									}
									
								}else{

									bool devStateRefreshEn = true;
									stt_devOpreatDataPonit *dptr = (stt_devOpreatDataPonit *)&local_datsRX->datsSTT.stt_MSG.dats[15]; //从下标15开始为dp数据

									if(memcmp(devMAC_Temp, ZigbDevNew_temp->macAddr, DEVL7MAC_LEN)){ //若网络地址相同的情况下，MAC地址不一致，则以最新的MAC地址为准，并进行更新
										ESP_LOGI(TAG, ">>>detect differ MAC, node[%04X]Info get reales.\n", ZigbDevNew_temp->nwkAddr);
										memcpy(ZigbDevNew_temp->macAddr, devMAC_Temp, DEVL7MAC_LEN); //以最新的MAC地址进行对应链表节点更新
									}

									if(dptr->devAgingOpreat_agingReference.agingCmd_swOpreat){ //标志占位判断
										if((DTMODEKEEPACESS_FRAMECMD_PST == local_datsRX->datsSTT.stt_MSG.dats[8])){
//											ESP_LOGI(TAG, ">>>devNode refresh cdcnt:%d.", ZigbDevNew_temp->devStateRefreshCdCnt);
											(ZigbDevNew_temp->devStateRefreshCdCnt > 0)? //被动回复(A2)情况下，判断冷却时间，防止连按时状态更新混乱
												(devStateRefreshEn = false):
												(devStateRefreshEn = true);
										}
									}

									if(true == devStateRefreshEn){ //链表开关状态更新使能
										ZigbDevNew_temp->devStateData = local_datsRX->datsSTT.stt_MSG.dats[21]; //设备状态更新
										if(NULL != msgQh_devListPageInfoChg){
											msgAttr_devBlockParamChange msgDats = {0};
											msgDats.nType = blkInfoChgType_state;
											msgDats.devSeq = lanbonDevSeq_L7;
											memcpy(msgDats.devMac, devMAC_Temp, sizeof(uint8_t) * DEVL7MAC_LEN);
											msgDats.infoData.infoChgDats_state.devState = local_datsRX->datsSTT.stt_MSG.dats[21];
											xQueueSend(msgQh_devListPageInfoChg, &msgDats, 1 / portTICK_PERIOD_MS);
										}
									}
									
									ZigbDevNew_temp->onlineDectect_LCount = zigDev_lifeCycle; //更新节点设备在列表中的生命周期
									ZigbDevNew_temp->devDefClass = local_datsRX->datsSTT.stt_MSG.dats[9]; //设备记录类型更新
									ZigbDevNew_temp = NULL;
									if(ZigbDevNew_temp)os_free(ZigbDevNew_temp); //缓存释放
								}
			
								/*数据处理-数据通过消息队列传送至socket通信主线程*///本机设备直接接管通信回复 
								if((local_datsRX->datsSTT.stt_MSG.dats[0] == ZIGB_FRAMEHEAD_CTRLLOCAL) && (local_datsRX->datsSTT.stt_MSG.dats[3] == l7PTL_FRAME_MtoSCMD_cmdConfigSearch)){ //本地配置指令添加 网络短地址 <供调试使用>

									local_datsRX->datsSTT.stt_MSG.dats[20] = (local_datsRX->datsSTT.stt_MSG.Addr_from & 0xFF00) >> 8;
									local_datsRX->datsSTT.stt_MSG.dats[21] = (local_datsRX->datsSTT.stt_MSG.Addr_from & 0x00FF) >> 0;
									local_datsRX->datsSTT.stt_MSG.dats[4] = frame_Check(&local_datsRX->datsSTT.stt_MSG.dats[5], 28);
								}
								
								mptr_Z2S.msgType = conventional;
								memcpy(mptr_Z2S.dats.dats_conv.dats, local_datsRX->datsSTT.stt_MSG.dats, local_datsRX->datsSTT.stt_MSG.datsLen);
								mptr_Z2S.dats.dats_conv.datsLen = local_datsRX->datsSTT.stt_MSG.datsLen;
								memcpy(mptr_Z2S.dats.dats_conv.macAddr, devMAC_Temp, DEVL7MAC_LEN);
								mptr_Z2S.dats.dats_conv.devType = DEVZIGB_DEFAULT;
								mptr_Z2S.dats.dats_conv.datsFrom = datsFrom_obj;
								xQueueSend(xMsgQ_Zigb2Socket, (void *)&mptr_Z2S, 0);
								
							}break;

							case ZIGB_ENDPOINT_CTRLSYSZIGB:{//端点口数据解析：zigbee系统控制交互专用端口


							}break;

							default:{//其余端口 		----为提高场景性能，关闭向内互控触发

							
							}break;
						}
	
					}break;
	
					//======协议层数据类型：新节点上线=====//
					case zigbTP_ntCONNECT:{
	
						ESP_LOGI(TAG, "[Tips_uartZigb]: new node[0x%04X] online.\n", local_datsRX->datsSTT.stt_ONLINE.nwkAddr_fresh);
						
					}break;
	
					default:{}break;
				}
			}
		}
		else
		{//zigb网络掉线处理

			ZigB_nwkReconnect();
		}

		/**************************************
			*zigb网络非阻塞远程数据持续传输*
		***************************************/	
		//互控业务 - 暂无
		
		//非阻塞远端数据传输
		{
			portBASE_TYPE xMsgQ_rcvResult = pdFALSE;
			sttUartRcv_rmDatsReqResp rptr_uartDatsRcv;
			sttUartRcv_scenarioCtrlResp rptr_scenarioCtrlResp;

			uint8_t loop_Insert_temp = 0; //索引缓存

			if(local_insertRecord_datsReqNormal < zigB_remoteDataTransASY_QbuffLen){ //异步非阻塞常规数据请求

				if(!localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReqPeriod && localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].repeat_Loop){
				
					localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReqPeriod = zigB_remoteDataTransASY_txPeriod; //轮发周期更新
					localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].repeat_Loop --; //轮发次数更新
				
					periphLocalPortUart_l7znp_send(localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq, localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq_Len);
					vTaskDelay(zigB_remoteDataTransASY_txUartOnceWait);

					if(!localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].repeat_Loop){

						ESP_LOGI(TAG, "preFail_rmDatatx warning, nwkAddr<0x%02X%02X>.\n", localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq[5], localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq[4]);

						if((localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq[5] == 0xff) && //远端地址为0xff时为广播传输
						   (localZigbASYDT_bufQueueRemoteReq[local_insertRecord_datsReqNormal].dataReq[4] == 0xff)){

						   if(dataRemoteRequest_failCount < zigB_reconnectCauseDataReqFailLoop){
						   
							   dataRemoteRequest_failCount ++;
							   
						   }else{
						   
							   dataRemoteRequest_failCount = 0;
							   nwkZigbOnline_IF = false; //远端广播数据传输失败次数超常，zigbee模块重启
						   }
						}
					}
				}
				local_insertRecord_datsReqNormal ++;
			}
			else{

				local_insertRecord_datsReqNormal = 0;
			}

			do{ //队列接收应答判断后将对应指令数据从发送队列中移除

				xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_rmDataReqResp, (void *)&rptr_uartDatsRcv, (portTickType) 0);
				if(xMsgQ_rcvResult == pdTRUE){

//					ESP_LOGI(TAG, ">>>:Qcome: Len-%02d, H-%02X, T-%02X.\n", rptr_uartDatsRcv.datsLen, rptr_uartDatsRcv.dats[0], rptr_uartDatsRcv.dats[rptr_uartDatsRcv.datsLen - 1]);

					while(loop_Insert_temp < zigB_remoteDataTransASY_QbuffLen){ //普通数据转发应答判断

						if( localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].repeat_Loop){

							if(!memcmp(rptr_uartDatsRcv.dats, localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp, localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp_Len)&&\
							   (localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].repeat_Loop < (zigB_remoteDataTransASY_txReapt - 2))){ //最少发2次

								localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].repeat_Loop = 0; //有正确的应答响应，提前结束数据发送
								memset(localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp, 0, sizeof(uint8_t) * localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp_Len); //对应应答缓存清零
								memcpy(&localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp], &localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp + 1], (zigB_remoteDataTransASY_QbuffLen - loop_Insert_temp - 1) * sizeof(stt_dataRemoteReq)); //缓存整理

//								ESP_LOGI(TAG, "bingo.\n");

								break; //当前可用应答使用完毕，一次应答只能用一次，不能重复共用
								
							}else{

//								ESP_LOGI(TAG, ">>>:mShud: Len-%02d, h-%02X, t-%02X.\n", localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp_Len, localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp[0], localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp[localZigbASYDT_bufQueueRemoteReq[loop_Insert_temp].dataResp_Len - 1]);
							}
						}
						loop_Insert_temp ++;	
					}
					loop_Insert_temp = 0;  //谨记清零,单次应答只用单次取消指令下达
				}
			}
			while(xMsgQ_rcvResult == pdTRUE);

			if(localZigbASYDT_scenarioCtrlReserveAllnum){ //场景异步发送操作是否可用
			
				zigbNodeDevDetectManage_runningEN = false;
				
				if(local_scenarioCtrl_counterRoundPause)local_scenarioCtrl_counterRoundPause --; //轮次间歇判断执行
				else{

					do{
	
						xMsgQ_rcvResult = xQueueReceive(xMsgQ_uartToutDats_scenarioCtrlResp, (void *)&rptr_scenarioCtrlResp, (portTickType) 0);
						if(xMsgQ_rcvResult == pdTRUE){
	
							while(loop_Insert_temp < localZigbASYDT_scenarioCtrlReserveAllnum){
	
								if(localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp].repeat_Loop){
							
									if(rptr_scenarioCtrlResp.respNwkAddr == localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp].dataRespNwkAddr){

										ZigB_devInfraredScenarioExcution(localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp].dataRespNwkAddr); //红外设备场景操作接力
									
										memset(&localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp], 0, sizeof(stt_dataScenarioReq)); //对应缓存清零
										memcpy(&localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp], &localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp + 1], (zigB_ScenarioCtrlDataTransASY_QbuffLen - loop_Insert_temp - 1) * sizeof(stt_dataScenarioReq)); //缓存整理
										localZigbASYDT_scenarioCtrlReserveAllnum --; //成功单位剔除
	
//										if(!(localZigbASYDT_scenarioCtrlReserveAllnum % zigB_ScenarioCtrlDataTransASY_opreatOnceNum))ESP_LOGI(TAG, ">>>scenarioCtrl unitReserve:%d.\n", localZigbASYDT_scenarioCtrlReserveAllnum);
										if(!localZigbASYDT_scenarioCtrlReserveAllnum)ESP_LOGI(TAG, ">>>scenarioCtrl allComplete.\n");
										
//										ESP_LOGI(TAG, "bingo.\n");
									
										break; //当前可用应答使用完毕，一次应答只能用一次，不能重复共用
										
									}else{
							
//										ESP_LOGI(TAG, ">>>addrShud: %04X, addrQ:%04X.\n", localZigbASYDT_bufQueueScenarioReq[loop_Insert_temp].dataRespNwkAddr, rptr_scenarioCtrlResp.respNwkAddr);
									}
									
								}
	
								loop_Insert_temp ++;	
							}
	
							loop_Insert_temp = 0;  //谨记清零,单次应答只用单次取消指令下达
						}
					}
					while(xMsgQ_rcvResult == pdTRUE);
	
					if(local_insertRecord_datsReqScenario < localZigbASYDT_scenarioCtrlReserveAllnum){ //异步非阻塞场景控制数据请求
	
						if(!localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReqPeriod && localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].repeat_Loop){
						
							localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReqPeriod = zigB_ScenarioCtrlDataTransASY_txPeriod; //轮发周期更新
							localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].repeat_Loop --; //轮发次数更新
						
							periphLocalPortUart_l7znp_send(localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReq, localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReq_Len);
							vTaskDelay(zigB_ScenarioCtrlDataTransASY_txTimeWaitOnceBasic + ((zigB_ScenarioCtrlDataTransASY_txReapt - localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].repeat_Loop) * zigB_ScenarioCtrlDataTransASY_txTimeWaitOnceStep)); //为保证稳定通信，单轮批次内，重发次数越多，发送等待时间步距越长，即等待时间越来越长
						
							if(!localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].repeat_Loop){

								ZigB_devInfraredScenarioExcution(localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataRespNwkAddr); //红外设备场景操作接力

								localZigbASYDT_scenarioCtrlReserveAllnum --; //垂死单位剔除
								memcpy(&localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario], &localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario + 1], (zigB_ScenarioCtrlDataTransASY_QbuffLen - local_insertRecord_datsReqScenario - 1) * sizeof(stt_dataScenarioReq)); //缓存整理
								ESP_LOGI(TAG, "preFail_qScenario warning, nwkAddr<0x%02X%02X>.\n", localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReq[5], localZigbASYDT_bufQueueScenarioReq[local_insertRecord_datsReqScenario].dataReq[4]);
							}
						}
	
						local_insertRecord_datsReqScenario ++;
					}
					else
					{
						local_insertRecord_datsReqScenario = 0;

						local_scenarioCtrl_counterRoundPause = zigB_ScenarioCtrlDataTransASY_timeRoundPause; //每轮次间歇计数值更新
	
						ESP_LOGI(TAG, ">>>scenario dataReq round:%d cmp, unitReserve:%d.\n", local_scenarioRecord_dtRound, localZigbASYDT_scenarioCtrlReserveAllnum);
						local_scenarioRecord_dtRound ++; //轮次记录值更新

						if(local_scenarioRecord_dtRound > (zigB_ScenarioCtrlDataTransASY_txReapt * 2)){ //业务异常，强制结束操作

							localZigbASYDT_scenarioCtrlReserveAllnum = 0;
							ESP_LOGI(TAG, ">>>scenarioCtrl err with time out, forced termination opreation.\n");
						}
					}
				}
			}
			else
			{
				zigbNodeDevDetectManage_runningEN = true;
				local_scenarioRecord_dtRound = 0; //轮次记录值复位
				
				if(scenarioCtrlOpreatStart_FLG){ //场景操作结束
				
					scenarioCtrlOpreatStart_FLG = false;
				
					enum_zigbFunMsg mptr_zigbFunRm = msgFun_dtPeriodHoldCancelAdvance; //结束心跳挂起
					xQueueSend(xMsgQ_zigbFunRemind, (void *)&mptr_zigbFunRm, 0);
				}
			}
		}

		vTaskDelay(5 / portTICK_RATE_MS);
	}

	vTaskDelete(NULL);
}

uint8_t *L7devListParamInfoTabGet(void){

	const uint8_t devUnitInfoAttrLen = sizeof(paramAttr_l7DataTransNodeUnit);
	uint8_t *listDataTab = NULL;
	struct tabHead_dataDiscrip{
		uint16_t dataTabCompleteLen;
		uint8_t dataNullForExt;
		uint8_t devNodeNum;
	}dataTabHead = {0};	
	uint8_t dataTabHead_Len = sizeof(struct tabHead_dataDiscrip);
	uint8_t listDevNum = zigbDev_chatLenDectect(zigbDevList_Head);
	uint8_t loopCount = 0;
	
	dataTabHead.dataTabCompleteLen = devUnitInfoAttrLen * listDevNum + dataTabHead_Len;
	listDataTab = os_zalloc(sizeof(uint8_t) * dataTabHead.dataTabCompleteLen);
	
	if(NULL != listDataTab){

		if(NULL != zigbDevList_Head){
			
			nwkStateAttr_Zigb *pAbove = zigbDevList_Head;
			nwkStateAttr_Zigb *pFollow;
			paramAttr_l7DataTransNodeUnit devUnitParamTemp = {0};
			
			while(pAbove->next != NULL){

				pFollow = pAbove;
				pAbove	= pFollow->next;

				//缓存数据更新
				devUnitParamTemp.nwkAddr = pAbove->nwkAddr;
				memcpy(devUnitParamTemp.macAddr,
					   pAbove->macAddr,
					   sizeof(uint8_t) * DEVL7MAC_LEN);
				devUnitParamTemp.devType = pAbove->devType;
				devUnitParamTemp.devDefClass = pAbove->devDefClass;
				devUnitParamTemp.devStateData = pAbove->devStateData;
				devUnitParamTemp.onlineDectect_LCount = pAbove->onlineDectect_LCount;
				
//				ESP_LOGI(TAG, "L7ParamList getting, pkg dev type:%02XH.", devUnitParamTemp.devDefClass);

				memcpy(&listDataTab[devUnitInfoAttrLen * loopCount + dataTabHead_Len], &devUnitParamTemp, sizeof(paramAttr_l7DataTransNodeUnit)); //缓存装填
				
				loopCount ++;
			}
		}

		//描述头数据更新
		dataTabHead.dataNullForExt = 0;
		dataTabHead.devNodeNum = listDevNum;
		memcpy(listDataTab, &dataTabHead, sizeof(uint8_t) * dataTabHead_Len);
	}

	return listDataTab;
}

void smartLivingL7DeviceControlExtution(uint8_t devMac[DEVL7MAC_LEN], uint8_t devState){

//    struct timeval tv;
//    struct tm *tt;
	nwkStateAttr_Zigb *infoZigbDevRet_temp = zigbDev_eptPutout_BYpsy(zigbDevList_Head, 
																	 devMac, 
																	 DEVZIGB_DEFAULT, 
																	 true);
	static uint8_t psCode = 0; //口令值本地记录

	if(NULL != infoZigbDevRet_temp){

		stt_threadDatsPass mptr_S2Z = {0};	
		uint8_t frameTotelLen = 15 + sizeof(stt_devOpreatDataPonit) + 2;
		stt_devOpreatDataPonit devDp_keepaccessDataTemp = {0};

//		//时间获取
//		gettimeofday(&tv, NULL);
//		tt = localtime(&tv.tv_sec);

//		ESP_LOGI(TAG, "devL7Node found, mac:%02X-%02X-%02X-%02X-%02X.\n", devMac[0], devMac[1], devMac[2], devMac[3], devMac[4]);

		//数据预填
		mptr_S2Z.msgType = conventional;
		mptr_S2Z.dats.dats_conv.dats[0] = DTMODEKEEPACESS_FRAMEHEAD_ONLINE;
		mptr_S2Z.dats.dats_conv.dats[1] = frameTotelLen;
		memcpy(&mptr_S2Z.dats.dats_conv.dats[2], devMac, 5);
		mptr_S2Z.dats.dats_conv.dats[8] = DTMODEKEEPACESS_FRAMECMD_ASR;
		mptr_S2Z.dats.dats_conv.dats[9] = infoZigbDevRet_temp->devDefClass;

		//操作值装填
		devDp_keepaccessDataTemp.devAgingOpreat_agingReference.agingCmd_swOpreat = 1;
		devDp_keepaccessDataTemp.devStatus_Reference.statusRef_swStatus = devState;
		memcpy(&mptr_S2Z.dats.dats_conv.dats[15], &devDp_keepaccessDataTemp, sizeof(stt_devOpreatDataPonit));

		//口令值装填
//		mptr_S2Z.dats.dats_conv.dats[frameTotelLen - 2] = (uint8_t)tt->tm_sec; //使用当前秒做口令值
		mptr_S2Z.dats.dats_conv.dats[frameTotelLen - 2] = psCode++; //自定义口令，每次都刷新

		//校验码计算
		mptr_S2Z.dats.dats_conv.dats[frameTotelLen - 1] =\
			frame_Check(&mptr_S2Z.dats.dats_conv.dats[1], frameTotelLen - 2);

		memcpy(mptr_S2Z.dats.dats_conv.macAddr, devMac, 5);
		mptr_S2Z.dats.dats_conv.datsLen = frameTotelLen;
		mptr_S2Z.dats.dats_conv.devType = infoZigbDevRet_temp->devDefClass;
		mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_ctrlRemote;
		
		xQueueSend(xMsgQ_Socket2Zigb, (void *)&mptr_S2Z, 0);	

		infoZigbDevRet_temp->devStateData = devState;
		infoZigbDevRet_temp->devStateRefreshCdCnt = L7_DEV_STATE_REFRESH_CD_PERIOD; //刷新时间冷却
	}
	else
	{
//		ESP_LOGI(TAG, "devL7Node not found, mac:%02X%02X%02X%02X%02X.\n", devMac[0], devMac[1], devMac[2], devMac[3], devMac[4]);
	}
}

uint8_t *smartLivingL7DeviceStateGet(uint8_t devMac[DEVL7MAC_LEN]){

	uint8_t *resptr = NULL;
	nwkStateAttr_Zigb *infoZigbDevRet_temp = zigbDev_eptPutout_BYpsy(zigbDevList_Head, 
																	 devMac, 
																	 DEVZIGB_DEFAULT, 
																	 true);
	
	if(NULL != infoZigbDevRet_temp){
		resptr = &infoZigbDevRet_temp->devStateData;
	}

	return resptr;
}

nwkStateAttr_Zigb *zigbDevListHead_get(void){

	return zigbDevList_Head;
}

void L7SystemZnpNetworkInfoGet(sttAttrDatsInfoDevL7Network *param){

	if(NULL != param){
		param->pan_id = ZigB_getPanIDCurrent();
	}
}

void devDriverBussiness_uartMoudle_L7ZnpSysParamGet(stt_L7smartLivingSysParam *param){

	memcpy(param, &systemParamL7SmartLivingZnp, sizeof(stt_L7smartLivingSysParam));
}

void devDriverBussiness_uartMoudle_L7ZnpSysParamSet(stt_L7smartLivingSysParam *param, bool nvsRecord_IF){

	memcpy(&systemParamL7SmartLivingZnp, param, sizeof(stt_L7smartLivingSysParam));
	if(true == nvsRecord_IF)
		devSystemInfoLocalRecord_save(saveObj_uartMoudleL7SysParam, &systemParamL7SmartLivingZnp);
}

void smartLivingL7ApplicationRingBufferInitialization(void){

	portBASE_TYPE xRes = pdFAIL;

	pRingBuffer = (uint8_t *)malloc(sizeof(uint8_t) * HAL7_RING_BUFFER_LENGTH_MAX);
	memset(pRingBuffer, 0, sizeof(uint8_t) * HAL7_RING_BUFFER_LENGTH_MAX);
	ringBufferAttrParam.indexFront = ringBufferAttrParam.indexRear = 0;

	xMsgQ_Socket2Zigb 					= xQueueCreate(30, sizeof(stt_threadDatsPass));
	xMsgQ_Zigb2Socket 					= xQueueCreate(30, sizeof(stt_threadDatsPass));

	xMsgQ_uartToutDats_dataSysRespond 	= xQueueCreate(30, sizeof(sttUartRcv_sysDat));
 	xMsgQ_uartToutDats_dataRemoteComing = xQueueCreate(40, sizeof(sttUartRcv_rmoteDatComming));
	xMsgQ_uartToutDats_rmDataReqResp 	= xQueueCreate(30, sizeof(sttUartRcv_rmDatsReqResp));
	xMsgQ_uartToutDats_scenarioCtrlResp = xQueueCreate(50, sizeof(sttUartRcv_scenarioCtrlResp));
	xMsgQ_uartToutDats_datsComingCtrlEach = xQueueCreate(30, sizeof(sttUartRcv_ctrlEachDat));
	xMsgQ_timeStampGet 					= xQueueCreate(5, sizeof(uint32_t));
	xMsgQ_zigbFunRemind					= xQueueCreate(5, sizeof(enum_zigbFunMsg));

	periphInit_l7UartMoudleComunicate();
	smartLivingApplicationTimerInit();

	xRes = xTaskCreate(processUartdataReceiveLoop, "ProcessZnpUartRcv", 1536, (void *)NULL, 5, &pxTaskHandle_l7ProcessZnpUartRcv);
	xRes = xTaskCreate(smartLivingApplicationProcessL7Bussiness, "l7Thread_Zigbee", 1024 * 6, (void *)NULL, 5, &pxTaskHandle_l7ThreadZigbee);

	esp_log_level_set(TAG, ESP_LOG_INFO);
}

void smartLivingL7ApplicationRingBufferDeinit(void){

	vTaskDelete(&pxTaskHandle_l7ProcessZnpUartRcv);
	vTaskDelete(&pxTaskHandle_l7ThreadZigbee);

	xTimerDelete(localTimerL7Scheduling, (portTickType) 0);
	
	periphDeinit_l7UartMoudleComunicate();
	
	xQueueReset(xMsgQ_Socket2Zigb);
	xQueueReset(xMsgQ_Zigb2Socket);
	xQueueReset(xMsgQ_uartToutDats_dataSysRespond);
	xQueueReset(xMsgQ_uartToutDats_dataRemoteComing);
	xQueueReset(xMsgQ_uartToutDats_rmDataReqResp);
	xQueueReset(xMsgQ_uartToutDats_scenarioCtrlResp);
	xQueueReset(xMsgQ_uartToutDats_datsComingCtrlEach);
	xQueueReset(xMsgQ_timeStampGet);
	xQueueReset(xMsgQ_zigbFunRemind);
	
	os_free(pRingBuffer);
}


















