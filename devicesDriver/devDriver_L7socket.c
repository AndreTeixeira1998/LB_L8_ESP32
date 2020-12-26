#include "devDriver_L7socket.h"

#include "mdf_common.h"
#include "mwifi.h"
#include "mlink.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <lwip/udp.h>

#include "devDriver_uartMoudle.h"
#include "devDriver_L7znp.h"

#define	DEV_L7_UDP_LOCAL_COMMUNICATION_PORT		8866

#define	DEV_L7_UDP_REMOTE_COMMUNICATION_ADDR	"192.168.10.1"
#define	DEV_L7_UDP_REMOTE_COMMUNICATION_PORT	8866

extern xQueueHandle xMsgQ_Socket2Zigb;
extern xQueueHandle xMsgQ_Zigb2Socket;
extern xQueueHandle xMsgQ_zigbFunRemind;

extern stt_scenarioOprateDats scenarioOprateDats;

extern uint8_t MACSTA_ID[DEV_MAC_LEN];
extern uint8_t MACAP_ID[DEV_MAC_LEN];
extern uint8_t MACDST_ID[DEV_MAC_LEN];

extern uint8_t SWITCH_TYPE;

xQueueHandle xMsgQ_datsFromSocketPort = NULL;

stt_scenarioOprateDats COLONY_DATAMANAGE_SCENE = {0}; 

bool nwkInternetOnline_IF = false;
bool deviceLock_flag = false;

static const char *TAG = "L7_socket communication";

static const int IPV4_GOTIP_BIT = BIT1;

static const char *payload = "Message from L8_uartMoudle.\n";

static const uint8_t debugLogOut_targetMAC[DEVL7MAC_LEN] = {0x20, 0x18, 0x12, 0x34, 0x56};

static xTaskHandle pxTaskHandle_l7ProcessSocketLocal = NULL;
static xTaskHandle pxTaskHandle_l7ProcessSocketRemote = NULL;

static bool internet_connFLG = false;

static int socketRemoteDt = 0,
		   socketLocalDt = 0;

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

/*本函数在socket数据发送前最后调用*///提前调用将影响校验码计算
static uint8_t dtasTX_loadBasic_CUSTOM(socketDataTrans_obj datsObj,
													     uint8_t dats_Tx[45],
													     uint8_t frame_Type,
													     uint8_t frame_CMD,
													        bool ifSpecial_CMD){	
						  
	switch(datsObj){
	
		/*服务器数据包*///45字节
		case DATATRANS_objFLAG_REMOTE:{
			
			uint8_t datsTemp[32] = {0};
		
			dats_Tx[0] 	= FRAME_HEAD_SERVER;
			
			memcpy(&datsTemp[0], &dats_Tx[1], 32);
			memcpy(&dats_Tx[13], &datsTemp[0], 32);	//帧头向后拉开，空出12字节
			memcpy(&dats_Tx[1],  &MACDST_ID[0], 6);	//远端MAC填充
			memcpy(&dats_Tx[8],  &MACSTA_ID[1], 5);	/*dats_Tx[7] 暂填0*///源MAC填充
			
			dats_Tx[1 + 12] = dataTransLength_objREMOTE;
			dats_Tx[2 + 12] = frame_Type;
			dats_Tx[3 + 12] = frame_CMD;
			
			if(!ifSpecial_CMD)dats_Tx[10 + 12] = SWITCH_TYPE;	//开关类型填充
			
			memcpy(&dats_Tx[5 + 12], &MACSTA_ID[1], 5);	//本地MAC填充
								  
			dats_Tx[4 + 12] = frame_Check(&dats_Tx[5 + 12], 28);	

			return dataTransLength_objREMOTE;
			
		}break;
		
		/*局域网数据包*///33字节
		case DATATRANS_objFLAG_LOCAL:{
		
			dats_Tx[0] 	= FRAME_HEAD_MOBILE;
			
			dats_Tx[1] 	= dataTransLength_objLOCAL;
			dats_Tx[2] 	= frame_Type;
			dats_Tx[3] 	= frame_CMD;
			
			if(!ifSpecial_CMD)dats_Tx[10] = SWITCH_TYPE;	//开关类型填充
			
			memcpy(&dats_Tx[5], &MACSTA_ID[1], 5);	//本地MAC填充
								  
			dats_Tx[4] 	= frame_Check(&dats_Tx[5], 28);

			return dataTransLength_objLOCAL;
			
		}break;
		
		default:break;
	}	
}

static void udpRemoteOperateDataSend(uint8_t *dats, uint16_t datsLen){

}

static void udpLocalOperateDataSend(uint8_t *dats, uint16_t datsLen){

}

static STATUS sockets_datsSend(socket_OBJ sObj, uint8_t dats[], uint16_t datsLen){

	STATUS res = FAIL;

	if(true == internet_connFLG){

		switch(sObj){
		
			case Obj_udpLocal_A:udpLocalOperateDataSend(dats, datsLen); res = OK; break;
			case Obj_udpRemote_B:udpRemoteOperateDataSend(dats, datsLen); res = OK;break;
			case Obj_tcpRemote_A:
			case Obj_tcpRemote_B:	
			default:res = FAIL;
		}
		
		vTaskDelay(40 / portTICK_PERIOD_MS); //socket通畅通信间隔 40ms
	}
	else{

		res = FAIL;
	}

	return res;
}

static void udpProcessRemoteDataRcvHandle(uint8_t *pdata, uint16_t len){

	bool dataCorrect_FLG = false;
	
	stt_socketDats mptr_socketDats;

	if (pdata == NULL)return;

	if(!memcmp("654321", pdata, 6)){
		udpRemoteOperateDataSend((uint8_t *)"123456\n", 7);
	}
	else
	{
		/*数据处理*/
		if((*pdata == FRAME_HEAD_SERVER) &&
		   (*(pdata + 13) >= dataTransLength_objREMOTE) &&
		   (*(pdata + 14) >= FRAME_TYPE_MtoS_CMD) ){	//远端数据

			if(!memcmp(&MACSTA_ID[1], pdata + 2, 5)){

			   memcpy(MACDST_ID, pdata + 7, 6);  //远端MAC更新

			   mptr_socketDats.portObj = Obj_udpRemote_B;
			   mptr_socketDats.command = *(pdata + 15);
			   memcpy(pdata + 1, pdata + 13, len - 13); //数据提取远端MAC后前移十二位
			   mptr_socketDats.datsLen = len - 12;
			   memcpy(mptr_socketDats.dats, pdata, len - 12);
			   
			   mptr_socketDats.heartBeat_IF = false; //不是心跳包
			   mptr_socketDats.dstObj = obj_toWIFI;
			   dataCorrect_FLG = true;
			   
			}else{

			   mptr_socketDats.portObj = Obj_udpRemote_B;  //原封中转
			   mptr_socketDats.command = *(pdata + 15);
			   mptr_socketDats.datsLen = len;
			   memcpy(mptr_socketDats.dats, pdata, len);
			   
			   mptr_socketDats.heartBeat_IF = false;   //不是心跳包
			   mptr_socketDats.dstObj = obj_toZigB;
			   dataCorrect_FLG = true;
			}
		}
		else
		if( (*pdata == DTMODEKEEPACESS_FRAMEHEAD_ONLINE) && //定时询访机制帧头
		    (*(pdata + 1) == len) && 
		    (*(pdata + 8) == DTMODEKEEPACESS_FRAMECMD_ASR || *(pdata + 8) == DTMODEKEEPACESS_FRAMECMD_PST) ){

//			ESP_LOGI(TAG, "pag mark.\n");
//			ESP_LOGI(TAG, ">>>keepAcessPag get:%d.\n", len);

			mptr_socketDats.portObj = Obj_udpRemote_B;
			mptr_socketDats.command = *(pdata + 8);
			mptr_socketDats.datsLen = len;
			memcpy(mptr_socketDats.dats, pdata, len);

			if(!memcmp(&MACSTA_ID[1], pdata + 2, 5)){

				mptr_socketDats.heartBeat_IF = true;	
				mptr_socketDats.dstObj = obj_toWIFI;
				dataCorrect_FLG = true;
				
			}else{

				mptr_socketDats.heartBeat_IF = true;	
				mptr_socketDats.dstObj = obj_toZigB;
				dataCorrect_FLG = true;				
			}
		}
		else
		if( (*pdata == FRAME_HEAD_HEARTB) &&	//远端心跳
		    (*(pdata + 1) == dataHeartBeatLength_objSERVER) &&
			(*(pdata + 1) == len) ){   

			mptr_socketDats.portObj = Obj_udpRemote_B;
			mptr_socketDats.command = *(pdata + 2);
			mptr_socketDats.datsLen = len;
			memcpy(mptr_socketDats.dats, pdata, len);

//			printf_datsHtoA("[Tips_socketUDP_B]: get message:", pdata, len);

			if(!memcmp(&MACSTA_ID[1], pdata + (3), 5)){		//服务器帮忙往前挪了一个字字节
//			if(!memcmp(&MACSTA_ID[1], pdata + (3 + 1), 5)){
				
				mptr_socketDats.heartBeat_IF = true;    
				mptr_socketDats.dstObj = obj_toWIFI;
				dataCorrect_FLG = true;
			   
			}else{

				mptr_socketDats.heartBeat_IF = true;	
				mptr_socketDats.dstObj = obj_toZigB;
				dataCorrect_FLG = true;
			}
		}
		else
		if( (*pdata == FRAME_HEAD_MOBILE) && 
		    (*(pdata + 2) == FRAME_TYPE_MtoS_CMD) ){ //远端特殊指令

			bool specialCMD_IF = false;

//			printf_datsHtoA("scenario opreat coming dataHead:", pdata, 12);

			if((*(pdata + 3) == FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl) ||
			   (*(pdata + 3) == FRAME_MtoZIGBCMD_cmdCfg_scenarioDel) ||
			   (*(pdata + 3) == FRAME_MtoZIGBCMD_cmdCfg_scenarioReg)
			)specialCMD_IF = true; //指令数据甄别

			if(specialCMD_IF){ //一级非常规处理

				ESP_LOGI(TAG, "scenarioCtl cmd rcv, dataLen:%04d bytes<<<<.\n", len);

				switch(*(pdata + 3)){

					case FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl:{ //>>>场景控制<<<
					
						uint16_t dats_Len = (uint16_t)(*(pdata + 1)) * 6 + 11; //实际帧长（数据包帧长为操作开关个数）
						bool psd_rsv_flg = false; //口令验证标志
						static uint8_t psd_val_local = 0; //口令值本地记录

						if(psd_val_local != pdata[len - 2]){ //两次口令不同才有效，否则为重发数据，数据重发只回复不动作

							psd_val_local = pdata[len - 2];
							psd_rsv_flg = true;
						}
						
						if((dats_Len == len) && !memcmp(&MACSTA_ID[1], pdata + 4, 5)){ //特殊指令 MAC从第五字节开始
						
							uint16_t loop = 0;
							uint16_t pointTemp = 0;
							uint8_t local_scenarioRespond[11] = {0};
							
							memcpy(&local_scenarioRespond[0], pdata, 9); //前段复制
							memcpy(&local_scenarioRespond[9], &pdata[len - 2], 2); //后段复制 -帧尾和口令
//							espconn_sent(&infoTemp_connUDP_remote_B, local_scenarioRespond, 11); //即刻向服务器回码
							udpRemoteOperateDataSend(local_scenarioRespond, 11);

//							printf_datsHtoA("!!!===>>>server dats head: ", pdata, 9); //前9字节

							if(psd_rsv_flg){ //指令可用，触发动作

								scenarioOprateDats.devNode_num = *(pdata + 1); //集群数量填装
								scenarioOprateDats.scenarioCtrlOprate_IF = true; //场景集群操作使能
								for(loop = 0; loop < *(pdata + 1); loop ++){
							
									pointTemp = 9 + 6 * loop;
									memcpy(scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC, pdata + pointTemp, 5); //集群单位MAC填装
									scenarioOprateDats.scenarioOprate_Unit[loop].devNode_opStatus = *(pdata + pointTemp + 5); //集群单位操作状态填装
								}

								memset(&COLONY_DATAMANAGE_SCENE, 0, sizeof(stt_scenarioOprateDats));
								memcpy(&COLONY_DATAMANAGE_SCENE, &scenarioOprateDats, sizeof(stt_scenarioOprateDats)); //本地数据集中管理更新

								mptr_socketDats.dstObj = obj_toWIFI;
								mptr_socketDats.portObj = Obj_udpRemote_B;
								mptr_socketDats.command = *(pdata + 3);
								mptr_socketDats.datsLen = 0;
								mptr_socketDats.heartBeat_IF = false;	//不是心跳包
								
								xQueueSendToFront(xMsgQ_datsFromSocketPort, (void *)&mptr_socketDats, 0);
							}

//							ESP_LOGI(TAG, "scenario unitNum: %d.\n", COLONY_DATAMANAGE_SCENE.devNode_num);
						}
						else{ //打印错误分析

							ESP_LOGI(TAG, "scenarioCtl parsing fail, {frameDataLen:%04d<-->actualDataLen:%04d, frameTargetMAC:%02X %02X %02X %02X %02X, actualMAC:%02X %02X %02X %02X %02X}<<<<.\n", 
									  (*(pdata + 1)) * 6 + 11,
									  len,
									  *(pdata + 4),
									  *(pdata + 5),
									  *(pdata + 6),
									  *(pdata + 7),
									  *(pdata + 8),
									  MACSTA_ID[1],
									  MACSTA_ID[2],
									  MACSTA_ID[3],
									  MACSTA_ID[4],
									  MACSTA_ID[5]);
						}
						
					}break;

					case FRAME_MtoZIGBCMD_cmdCfg_scenarioReg:{ //>>>场景注册<<<

						uint8_t local_scenarioRespond[11] = {0}; //应答数据缓存
					
						memcpy(&local_scenarioRespond[0], pdata, 9); //前段复制
						memcpy(&local_scenarioRespond[9], &pdata[len - 2], 2); //后段复制 -帧尾和场景序号
//						espconn_sent(&infoTemp_connUDP_remote_B, local_scenarioRespond, 11); //即刻向服务器回码
						udpRemoteOperateDataSend(local_scenarioRespond, 11);

						uint16_t dats_Len = (uint16_t)(*(pdata + 1)) * 6 + 11; //实际帧长（数据包帧长为操作开关个数）

						if((dats_Len == len) && !memcmp(&MACSTA_ID[1], pdata + 4, 5)){ //特殊指令 MAC从第五字节开始
						
							uint16_t loop = 0;
							uint16_t pointTemp = 0;
							stt_scenarioDataLocalSave *scenarioDataSave_Temp = (stt_scenarioDataLocalSave *)os_zalloc(sizeof(stt_scenarioDataLocalSave)); //存储缓存
							
							memset(scenarioDataSave_Temp, 0x00, sizeof(stt_scenarioDataLocalSave));	//存储缓存清零					
							scenarioDataSave_Temp->devNode_num = *(pdata + 1); //存储缓存场景设备数量填装
							scenarioDataSave_Temp->scenarioDataSave_InsertNum = pdata[len - 2]; //存储缓存场景存储序号填装
							
							for(loop = 0; loop < *(pdata + 1); loop ++){ //存储缓存场景数据填装
						
								pointTemp = 9 + 6 * loop;
								memcpy(scenarioDataSave_Temp->scenarioOprate_Unit[loop].devNode_MAC, pdata + pointTemp, 5); //存储缓存场景单位MAC填装
								scenarioDataSave_Temp->scenarioOprate_Unit[loop].devNode_opStatus = *(pdata + pointTemp + 5); //存储缓存场景单位操作状态填装
							}

							scenarioDataSave_Temp->scenarioReserve_opt = scenarioOpt_enable; //数据可用标识置位

//							devParam_scenarioDataLocalSave(scenarioDataSave_Temp); //存储执行

							ESP_LOGI(TAG, ">>>>>>scenarioReg cmd coming(send by phone), insertNum:%d, devNodeNum:%d.\n", scenarioDataSave_Temp->scenarioDataSave_InsertNum,
																												 	 scenarioDataSave_Temp->devNode_num);
						}
						else{ //打印错误分析

							ESP_LOGI(TAG, "scenarioReg parsing fail, {frameDataLen:%04d<-->actualDataLen:%04d, frameTargetMAC:%02X %02X %02X %02X %02X, actualMAC:%02X %02X %02X %02X %02X}<<<<.\n", 
									  (*(pdata + 1)) * 6 + 11,
									  len,
									  *(pdata + 4),
									  *(pdata + 5),
									  *(pdata + 6),
									  *(pdata + 7),
									  *(pdata + 8),
									  MACSTA_ID[1],
									  MACSTA_ID[2],
									  MACSTA_ID[3],
									  MACSTA_ID[4],
									  MACSTA_ID[5]);
						}

					}break;

					default:break;
				}
			}
		}
		else{

			ESP_LOGI(TAG, ">>>data parsing fail with length:%d.\n", len);
		}

		if(dataCorrect_FLG){

			xQueueSend(xMsgQ_datsFromSocketPort, (void *)&mptr_socketDats, 0);
		}

//		printf_datsHtoA("[Tips_socketUDP_B]: get message:", pdata, len);
	}	
}

static void udpProcessLocalDataRcvHandle(uint8_t *pdata, uint16_t len){

	bool dataCorrect_FLG = false;
	
	stt_socketDats mptr_socketDats = {0};

	if(NULL == pdata)return;

	if(!memcmp("654321", pdata, 6)){
		udpLocalOperateDataSend((uint8_t *)"123456", 6);	
	}
	else{
		/*数据处理*///关键字节检验
		if( (*pdata == FRAME_HEAD_MOBILE) && 
		    (*(pdata + 2) == FRAME_TYPE_MtoS_CMD) ){

			bool specialCMD_IF = false;

			if((*(pdata + 3) == FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl) ||
			   (*(pdata + 3) == FRAME_MtoSCMD_cmdWifiNwkCfg))specialCMD_IF = true; //指令数据甄别

			if(specialCMD_IF){ //一级非常规处理

				switch(*(pdata + 3)){

					case FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl:{ //>>>场景控制<<<
					
						uint16_t dats_Len = (uint16_t)(*(pdata + 1)) * 6 + 10; //实际帧长（数据包帧长为操作开关个数）
						
						if((dats_Len == len) && !memcmp(&MACSTA_ID[1], pdata + 4, 5)){ //特殊指令 MAC从第五字节开始
						
							uint8_t loop = 0;
							uint8_t pointTemp = 0;
						
							scenarioOprateDats.devNode_num = *(pdata + 1); //集群数量填装
							scenarioOprateDats.scenarioCtrlOprate_IF = true; //场景集群操作使能
							for(loop = 0; loop < *(pdata + 1); loop ++){
						
								pointTemp = 9 + 6 * loop;
								memcpy(scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC, pdata + pointTemp, 5); //集群单位MAC填装
								scenarioOprateDats.scenarioOprate_Unit[loop].devNode_opStatus = *(pdata + pointTemp + 5); //集群单位操作状态填装
							}

							memset(&COLONY_DATAMANAGE_SCENE, 0, sizeof(stt_scenarioOprateDats));
							memcpy(&COLONY_DATAMANAGE_SCENE, &scenarioOprateDats, sizeof(stt_scenarioOprateDats)); //本地数据集中管理更新

							mptr_socketDats.dstObj = obj_toWIFI;
							mptr_socketDats.portObj = Obj_udpLocal_A;
							mptr_socketDats.command = *(pdata + 3);
							mptr_socketDats.datsLen = 0;
							mptr_socketDats.heartBeat_IF = false;	//不是心跳包
							
							xQueueSendToFront(xMsgQ_datsFromSocketPort, (void *)&mptr_socketDats, 0);
						}
						
					}break;

					case FRAME_MtoSCMD_cmdWifiNwkCfg:{

						uint16_t dats_Len = 4 + 32 + 64;

						if(dats_Len == len){

							char nwkWifi_stationCfgTemp_ssid[32] = {0},
							   	 nwkWifi_stationCfgTemp_psd[64] = {0};

							memcpy(nwkWifi_stationCfgTemp_ssid, pdata + (0 + 4), sizeof(uint8_t) * 32);
							memcpy(nwkWifi_stationCfgTemp_psd, pdata + (4 + 32), sizeof(uint8_t) * 64);

							mptr_socketDats.dstObj = obj_toWIFI;
							mptr_socketDats.portObj = Obj_udpLocal_A;
							mptr_socketDats.command = *(pdata + 3);
							mptr_socketDats.datsLen = 0;
							memcpy(mptr_socketDats.dats, pdata, len);
							mptr_socketDats.heartBeat_IF = false;	//不是心跳包
							
							xQueueSendToFront(xMsgQ_datsFromSocketPort, (void *)&mptr_socketDats, 0);
						}

					}break;

					default:break;
				}
			}
			else
			if((uint16_t)*(pdata + 1) == len){ //常规指令常规处理
			
				bool frameCodeCheck_PASS = false; // 校验码通过标志
				bool frameMacCheck_PASS  = false; // MAC通过标志

				if(*(pdata + 4) == frame_Check(pdata + 5, *(pdata + 1) - 5))frameCodeCheck_PASS = true;
				if(!memcmp(&MACSTA_ID[1], pdata + 5, 5))frameMacCheck_PASS = true;
				
				if( (*(pdata + 3) == FRAME_MtoSCMD_cmdConfigSearch) ){ //特殊指令不进行MAC地址校验	
				
					frameMacCheck_PASS = true;
				}
				if( (*(pdata + 3) == FRAME_MtoSCMD_cmdCfg_swTim) || //特殊指令不进行校验码校验
					(*(pdata + 3) == FRAME_MtoSCMD_cmdswTimQuery) ){
				
					frameCodeCheck_PASS = true;
				}
				
				if(frameMacCheck_PASS == true && frameCodeCheck_PASS)dataCorrect_FLG = true;
				
				if(dataCorrect_FLG == true){
				
					if( (*(pdata + 3) == FRAME_MtoSCMD_cmdConfigSearch) ){
				
						mptr_socketDats.dstObj = obj_toALL;
						ESP_LOGI(TAG, "rcv cfgRsp.\n");
					}
					else
					{
						mptr_socketDats.dstObj = obj_toWIFI;
					}
				
				}else{
				
					mptr_socketDats.dstObj = obj_toZigB;
				}
				
				mptr_socketDats.portObj = Obj_udpLocal_A;
				mptr_socketDats.command = *(pdata + 3);
				mptr_socketDats.datsLen = len;
				memcpy(mptr_socketDats.dats, pdata, len);
				mptr_socketDats.heartBeat_IF = false;	//不是心跳包
				
				xQueueSend(xMsgQ_datsFromSocketPort, (void *)&mptr_socketDats, 0);
			}
		}	
	}
}

static void socketSetNoBlocking(int sock){

    int flag = fcntl(sock, F_GETFL, 0);
    if (flag < 0) {
        ESP_LOGE(TAG, "fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sock, F_SETFL, flag | O_NONBLOCK) < 0) {
        ESP_LOGE(TAG, "fcntl F_SETFL fail");
    }
}

static void udp_rcv_callback_dataTransLocal(		void *arg, 
													 struct udp_pcb *pcb, 
													 struct pbuf *p,
													 ip_addr_t *addr, 
													 u16_t port ){
	struct ip_addr my_ipaddr;
	unsigned char *temp = (unsigned char *)addr;
	char stBuf[32] = {0};
	
	IP4_ADDR(ip_2_ip4(&my_ipaddr), temp[0], temp[1], temp[2], temp[3]); // 保存源IP

	ESP_LOGI(TAG, "udp local data rcv, %s", (char *)p->payload);
	//udp_sendto(pcb, p, &my_ipaddr, port); // 将报文返回给原主机
	
	pbuf_free(p);
}

static void devL7SmartlivingUdpLocalCommunicationProcessTask(void *pvParameters){

	const char *dptr = "hellow world!!!\n";
	struct udp_pcb *pcb = NULL;
	ip_addr_t remoteIp = {0};
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(dptr), PBUF_RAM);
	tcpip_adapter_ip_info_t ip_info = {0};

	for(;;){ //等待网络连接
		vTaskDelay(100 / portTICK_PERIOD_MS);
		if((true == mwifi_is_connected()) && (MESH_ROOT == esp_mesh_get_layer())){
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			break;
		}
	}

	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info); //ip_info.ip.addr;

	pcb = udp_new();
	if(pcb == NULL){
		ESP_LOGE(TAG, "udp local creat fail.");
	}else{
		IP4_ADDR(ip_2_ip4(&remoteIp), 192, 168, 0, 121);
		//udp_disconnect(pcb);
		if(ERR_OK == udp_connect(pcb, &remoteIp, 8888)){ // 连接到指定的IP地址和端口

			pcb->local_port = 8888;
			udp_recv(pcb, udp_rcv_callback_dataTransLocal, NULL); // 注册报文处理回调				
			ESP_LOGI(TAG, "udp local connected, local_port %d.", pcb->local_port);
		}else{
			ESP_LOGI(TAG, "udp local connect fail.");
		}
	}

	p->payload = (void *)dptr;
	
	for(;;){

		udp_sendto(pcb, p, &remoteIp, 8888);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

static void udp_rcv_callback_dataTransRemote(		void *arg, 
													  struct udp_pcb *pcb, 
													  struct pbuf *p,
													  ip_addr_t *addr, 
													  u16_t port ){
	struct ip_addr my_ipaddr;
	unsigned char *temp = (unsigned char *)addr;
	char stBuf[32] = {0};
	
	IP4_ADDR(ip_2_ip4(&my_ipaddr), temp[0], temp[1], temp[2], temp[3]); // 保存源IP

	ESP_LOGI(TAG, "udp remote data rcv, %s", (char *)p->payload);
	//udp_sendto(pcb, p, &my_ipaddr, port); // 将报文返回给原主机
	
	pbuf_free(p);
}

static void devL7SmartlivingUdpRemoteCommunicationProcessTask(void *pvParameters){

	const char *dptr = "hellow world!!!\n";
	struct udp_pcb *pcb = NULL;
	ip_addr_t remoteIp = {0};
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(dptr), PBUF_RAM);
	tcpip_adapter_ip_info_t ip_info = {0};

	for(;;){ //等待网络连接
		vTaskDelay(100 / portTICK_PERIOD_MS);
		if((true == mwifi_is_connected()) && (MESH_ROOT == esp_mesh_get_layer())){
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			break;
		}
	}

	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info); //ip_info.ip.addr;

	pcb = udp_new();
	if(pcb == NULL){
		ESP_LOGE(TAG, "udp remote creat fail.");
	}else{
//		IP4_ADDR(ip_2_ip4(&remoteIp), 47, 52, 5, 108);
		IP4_ADDR(ip_2_ip4(&remoteIp), 112, 124, 61, 191);
		//udp_disconnect(pcb);
		if(ERR_OK == udp_connect(pcb, &remoteIp, 80)){ // 连接到指定的IP地址和端口

			pcb->local_port = 12345;
			udp_recv(pcb, udp_rcv_callback_dataTransRemote, NULL); // 注册报文处理回调				
			ESP_LOGI(TAG, "udp remote connected, local_port %d.", pcb->local_port);
		}else{
			ESP_LOGI(TAG, "udp remote connect fail.");
		}
	}

	p->payload = (void *)dptr;
	
	for(;;){

		udp_sendto(pcb, p, &remoteIp, 8888);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

static void devL7SmartlivingSocketsDataTransProcessTask(void *pvParameters){

	stt_threadDatsPass	mptr_S2Z;
	stt_threadDatsPass	rptr_Z2S;
	stt_socketDats		rptr_socketDats;
	portBASE_TYPE		xMsgQ_rcvResult = pdFALSE;

#define socketsData_repeatTxLen	128
	uint8_t repeatTX_buff[socketsData_repeatTxLen] = {0};
	uint8_t repeatTX_Len = 0;

#define socketsData_datsTxLen	128
	uint8_t datsTX_buff[socketsData_datsTxLen] = {0};
	uint8_t datsTX_Len = 0;

//	stt_usrDats_privateSave datsSave_Temp = {0};
//	stt_usrDats_privateSave *datsRead_Temp;
		
	bool socketRespond_IF = false;
	bool specialCMD_IF = false;

	bool local_ftyRecover_standbyFLG = false; //恢复出厂设置预动作标志

	uint8_t loop = 0;

	for(;;){

/*>>>>>>sockets数据处理<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
		xMsgQ_rcvResult = xQueueReceive(xMsgQ_datsFromSocketPort, (void *)&rptr_socketDats, 0);
		if(xMsgQ_rcvResult == pdTRUE){

			if(!rptr_socketDats.heartBeat_IF){	//常规数据处理<<---------------------------------------------------------------//

				ESP_LOGI(TAG, "[Tips_NWK-SKdats]: receive msg with cmd: %02X !!!\n", rptr_socketDats.command);

				memset(repeatTX_buff, 0, socketsData_repeatTxLen * sizeof(uint8_t)); //清缓存

				if((rptr_socketDats.dstObj == obj_toWIFI) || //数据是给网关的，则直接解析数据，远端数据已做缩减处理为和本地数据格式相同，直接解析数据即可
				   (rptr_socketDats.dstObj == obj_toALL)){ 

//				   beeps_usrActive(3, 25, 1);

					switch(rptr_socketDats.command){
		
						case FRAME_MtoSCMD_cmdControl:{

							//模块仅转发，不存在本身设备
		
						}break;
		
						case FRAME_MtoSCMD_cmdConfigSearch:{

							stt_L7smartLivingSysParam systemParamTemp = {0};

							if(deviceLock_flag)break; //上锁不应答，也不动作

							devDriverBussiness_uartMoudle_L7ZnpSysParamGet(&systemParamTemp);

							systemParamTemp.sysTimeZone_H = rptr_socketDats.dats[12];
							systemParamTemp.sysTimeZone_M = rptr_socketDats.dats[13];
							
//							mySocketUDPremote_B_serverChange(&rptr_socketDats.dats[6]);

							repeatTX_buff[11] = 0;
							repeatTX_buff[12] = DEVICE_VERSION_NUM; 

							repeatTX_buff[14] = (uint8_t)(systemParamTemp.panId >> 8);
							repeatTX_buff[15] = (uint8_t)(systemParamTemp.panId >> 0);

							repeatTX_buff[20] = 0x00;  //协调器网络短地址标记 <调试专用>
							repeatTX_buff[21] = 0x00;  //协调器网络短地址标记 <调试专用>

							devDriverBussiness_uartMoudle_L7ZnpSysParamSet(&systemParamTemp, true);

							specialCMD_IF		= false;
							socketRespond_IF	= !deviceLock_flag; //上锁不应答
						
						}break;
		
						case FRAME_MtoSCMD_cmdQuery:{

							//模块仅转发，不存在本身设备

						}break;
		
						case FRAME_MtoSCMD_cmdInterface:{}break;
		
						case FRAME_MtoSCMD_cmdReset:{}break;
		
						case FRAME_MtoSCMD_cmdLockON:{

							stt_L7smartLivingSysParam systemParamTemp = {0};

							devDriverBussiness_uartMoudle_L7ZnpSysParamGet(&systemParamTemp);
							systemParamTemp.systemL7Flg_devLock = 1;
							devDriverBussiness_uartMoudle_L7ZnpSysParamSet(&systemParamTemp, true);

							deviceLock_flag = true;
							
						}break;
		
						case FRAME_MtoSCMD_cmdLockOFF:{
							
							stt_L7smartLivingSysParam systemParamTemp = {0};

							devDriverBussiness_uartMoudle_L7ZnpSysParamGet(&systemParamTemp);
							systemParamTemp.systemL7Flg_devLock = 0;
							devDriverBussiness_uartMoudle_L7ZnpSysParamSet(&systemParamTemp, true);

							deviceLock_flag = false;
							
						}break;
		
						case FRAME_MtoSCMD_cmdswTimQuery:{

							//模块仅转发，不存在本身设备
							
						}break;
		
						case FRAME_MtoSCMD_cmdConfigAP:{
		
							//模块仅转发，不存在本身设备
						
						}break;
		
						case FRAME_MtoSCMD_cmdBeepsON:{ //蜂鸣器开命令代表夜间模式关闭

							//模块仅转发，不存在本身设备

						}break;
		
						case FRAME_MtoSCMD_cmdBeepsOFF:{ //蜂鸣器关命令代表夜间模式开启

							//模块仅转发，不存在本身设备
							
						}break;

						case FRAME_MtoSCMD_cmdWifiNwkCfg:{

							//模块仅转发，不存在本身设备

						}break;
		
						case FRAME_MtoSCMD_cmdftRecoverRQ:{
		
							socketRespond_IF = true; //支持恢复·
						
						}break;
		
						case FRAME_MtoSCMD_cmdRecoverFactory:{
		
							stt_L7smartLivingSysParam systemParamTemp = {0};
							devDriverBussiness_uartMoudle_L7ZnpSysParamSet(&systemParamTemp, true);
	
							socketRespond_IF = true;
						
						}break;
		
						case FRAME_MtoSCMD_cmdCfg_swTim:{
		
							//模块仅转发，不存在本身设备

						}break;

						case FRAME_MtoZIGBCMD_cmdCfg_PANID:{

							stt_L7smartLivingSysParam systemParamTemp = {0};
							bool PANID_changeRightNow = false;
						
							devDriverBussiness_uartMoudle_L7ZnpSysParamGet(&systemParamTemp);

							ESP_LOGI(TAG, "PANID_GET: 0x%02X%02X.\n", rptr_socketDats.dats[14], rptr_socketDats.dats[15]);
							if(rptr_socketDats.dats[16] == 0x01){ //强制设置

								PANID_changeRightNow = true;
							}
							else
							if(rptr_socketDats.dats[16] == 0x00){ //非强制设置

								if(systemParamTemp.panId > 0 && systemParamTemp.panId <= ZIGB_PANID_MAXVAL){ //合法

									PANID_changeRightNow = false;

								}else{ //非法

									PANID_changeRightNow = true;
								}
							}

							if(PANID_changeRightNow){ //立即更改并创建对应PANID网络

								systemParamTemp.panId = 0; //缓存清空
								systemParamTemp.panId |= ((uint16_t)rptr_socketDats.dats[14] << 8);
								systemParamTemp.panId |= ((uint16_t)rptr_socketDats.dats[15] << 0);
								devDriverBussiness_uartMoudle_L7ZnpSysParamSet(&systemParamTemp, true); //存储数据更新
								
								repeatTX_buff[14] = rptr_socketDats.dats[14];
								repeatTX_buff[15] = rptr_socketDats.dats[15];

								enum_zigbFunMsg mptr_zigbFunRm = msgFun_panidRealesNwkCreat;
								xQueueSend(xMsgQ_zigbFunRemind, (void *)&mptr_zigbFunRm, 0);
								
							}else{

								repeatTX_buff[14] = (uint8_t)(systemParamTemp.panId >> 8);
								repeatTX_buff[15] = (uint8_t)(systemParamTemp.panId >> 0);
							}
							
							socketRespond_IF = true;
						
						}break;

						case FRAME_MtoZIGBCMD_cmdCfg_ctrlEachO:{

							//模块仅转发，不存在本身设备

						}break;

						case FRAME_MtoZIGBCMD_cmdQue_ctrlEachO:{

							//模块仅转发，不存在本身设备

						}break;

						case FRAME_MtoZIGBCMD_cmdCfg_ledBackSet:{

							//模块仅转发，不存在本身设备

						}break;

						case FRAME_MtoZIGBCMD_cmdQue_ledBackSet:{

							//模块仅转发，不存在本身设备
						
						}break;

						case FRAME_MtoZIGBCMD_cmdCfg_scenarioCtl:{

							if(scenarioOprateDats.scenarioCtrlOprate_IF){

								uint8_t loop = 0;

								for(loop = 0; loop < scenarioOprateDats.devNode_num; loop ++){ //查找网关自身MAC是否在内

									if(!memcmp(&MACSTA_ID[1], scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC, 5)){ //本身存在于场景就响应动作

//										swCommand_fromUsr.objRelay = scenarioOprateDats.scenarioOprate_Unit[loop].devNode_opStatus;
//										swCommand_fromUsr.actMethod = relay_OnOff;

										/*场景是否触发互控?*/
										//if(SWITCH_TYPE == SWITCH_TYPE_SWBIT1 || SWITCH_TYPE == SWITCH_TYPE_SWBIT2 || SWITCH_TYPE == SWITCH_TYPE_SWBIT3)EACHCTRL_realesFLG |= (status_actuatorRelay ^ swCommand_fromUsr.objRelay); //有效互控位触发
										//else
										//if(SWITCH_TYPE == SWITCH_TYPE_CURTAIN)EACHCTRL_realesFLG = 1; //有效互控触发
										
										memset(&scenarioOprateDats.scenarioOprate_Unit[loop], 0, 1 * sizeof(scenarioOprateUnit_Attr)); //同时从场景控制表中剔除网关自身
										memcpy(&scenarioOprateDats.scenarioOprate_Unit[loop], &scenarioOprateDats.scenarioOprate_Unit[loop + 1], (zigB_ScenarioCtrlDataTransASY_QbuffLen - loop - 1) * sizeof(scenarioOprateUnit_Attr));
										scenarioOprateDats.devNode_num --;

										break;
									}
								}

								enum_zigbFunMsg mptr_zigbFunRm = msgFun_scenarioCrtl; //消息即刻转发至zigb线程进行场景集群控制下发
								xQueueSend(xMsgQ_zigbFunRemind, (void *)&mptr_zigbFunRm, 0);
								
//								for(loop = 0; loop < scenarioOprateDats.devNode_num; loop ++){ //log打印

//									ESP_LOGI(TAG, "devNode<%02d>MAC:[%02X %02X %02X %02X %02X] --opStatus:{%02X}.\n",
//											  loop,
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC[0],
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC[1],
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC[2],
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC[3],
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_MAC[4],
//											  scenarioOprateDats.scenarioOprate_Unit[loop].devNode_opStatus);
//								}
							}

						}break;

						default:{}break;
					}
		
					if(socketRespond_IF){
		
						socketDataTrans_obj dats_obj = DATATRANS_objFLAG_LOCAL;
		
						if(rptr_socketDats.portObj == Obj_udpLocal_A){	//数据传输对象判断
							dats_obj = DATATRANS_objFLAG_LOCAL;
						}else{
							dats_obj = DATATRANS_objFLAG_REMOTE;
						}
		
						repeatTX_Len = dtasTX_loadBasic_CUSTOM( dats_obj,		//发送前最后填装
																repeatTX_buff,				
																FRAME_TYPE_StoM_RCVsuccess,
																rptr_socketDats.command,
																specialCMD_IF);
		
						sockets_datsSend(rptr_socketDats.portObj, repeatTX_buff, repeatTX_Len);

//						printf_datsHtoA("[Tips_threadNet]: respondDats:", repeatTX_buff, repeatTX_Len);
					}

				}

				if((rptr_socketDats.dstObj == obj_toZigB) || //数据是给zigbee的，转发处理
				   (rptr_socketDats.dstObj == obj_toALL)){

					memset(&mptr_S2Z, 0, sizeof(stt_threadDatsPass));

					mptr_S2Z.msgType = conventional;
					memcpy(mptr_S2Z.dats.dats_conv.dats, rptr_socketDats.dats, rptr_socketDats.datsLen);
					mptr_S2Z.dats.dats_conv.datsLen = rptr_socketDats.datsLen;
					switch(rptr_socketDats.portObj){

						case Obj_udpLocal_A:{ //本地类型数据处理

							memcpy(mptr_S2Z.dats.dats_conv.macAddr, &rptr_socketDats.dats[5], DEV_MAC_LEN);
							mptr_S2Z.dats.dats_conv.devType = rptr_socketDats.dats[10];
							mptr_S2Z.dats.dats_conv.dats[0] = ZIGB_FRAMEHEAD_CTRLLOCAL;
							mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_ctrlLocal;
							   
						}break;

						case Obj_udpRemote_B:{ //远端服务器数据处理

							memcpy(mptr_S2Z.dats.dats_conv.macAddr, &rptr_socketDats.dats[17], DEV_MAC_LEN);
							mptr_S2Z.dats.dats_conv.devType = rptr_socketDats.dats[22];
							mptr_S2Z.dats.dats_conv.dats[0] = ZIGB_FRAMEHEAD_CTRLREMOTE;
							mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_ctrlRemote;

//							ESP_LOGI(TAG, "[Tips_NWK-SKdats]: mark: %02X %02X %02X %02X %02X.\n", 
//							mptr_S2Z.dats.dats_conv.macAddr[0],
//							mptr_S2Z.dats.dats_conv.macAddr[1],
//							mptr_S2Z.dats.dats_conv.macAddr[2],
//							mptr_S2Z.dats.dats_conv.macAddr[3],
//							mptr_S2Z.dats.dats_conv.macAddr[4]);

						}break;

						default:{

							mptr_S2Z.dats.dats_conv.dats[0] = ZIGB_FRAMEHEAD_CTRLREMOTE;
							mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_ctrlRemote;

						}break;
					}

				   xQueueSend(xMsgQ_Socket2Zigb, (void *)&mptr_S2Z, 0);

				}

				/*标志复位*/
				socketRespond_IF = false;
				specialCMD_IF = false;

			}
			else{ //心跳包处理<<---------------------------------------------------------------//

				switch(rptr_socketDats.dstObj){

					case obj_toWIFI:{ //给网关主机的

						//模块仅转发，不存在自身设备
						
					}break;

					case obj_toZigB:{ //给zigb子设备的

						memset(&mptr_S2Z, 0, sizeof(stt_threadDatsPass));
						
						mptr_S2Z.msgType = conventional;
						memcpy(mptr_S2Z.dats.dats_conv.dats, rptr_socketDats.dats, rptr_socketDats.datsLen);
						mptr_S2Z.dats.dats_conv.datsLen = rptr_socketDats.datsLen;
						mptr_S2Z.dats.dats_conv.datsFrom = datsFrom_heartBtRemote;

						switch(rptr_socketDats.dats[0]){

							case FRAME_HEAD_HEARTB:{

								memcpy(mptr_S2Z.dats.dats_conv.macAddr, &rptr_socketDats.dats[3], DEV_MAC_LEN);
//								memcpy(mptr_S2Z.dats.dats_conv.macAddr, &rptr_socketDats.dats[3 + 1], DEV_MAC_LEN); //服务器帮忙往前挪了一个字节
								mptr_S2Z.dats.dats_conv.devType = rptr_socketDats.dats[rptr_socketDats.datsLen - 3];
								mptr_S2Z.dats.dats_conv.dats[0] = ZIGB_FRAMEHEAD_HEARTBEAT;
								
							}break;

							case DTMODEKEEPACESS_FRAMEHEAD_ONLINE:{

								memcpy(mptr_S2Z.dats.dats_conv.macAddr, &rptr_socketDats.dats[2], DEV_MAC_LEN);
								mptr_S2Z.dats.dats_conv.devType = DEVZIGB_DEFAULT;

							}break;

							default:{}break;
						}

						if(!memcmp(debugLogOut_targetMAC ,mptr_S2Z.dats.dats_conv.macAddr, 5)){

							ESP_LOGI(TAG, "[Tips_NWK-SKdats]: node_hb rvc-<mac:%02X %02X %02X %02X %02X>,<cmd:%02X>,<dataLen:%d>.\n", 
										  mptr_S2Z.dats.dats_conv.macAddr[0],
										  mptr_S2Z.dats.dats_conv.macAddr[1],
										  mptr_S2Z.dats.dats_conv.macAddr[2],
										  mptr_S2Z.dats.dats_conv.macAddr[3],
										  mptr_S2Z.dats.dats_conv.macAddr[4],
										  rptr_socketDats.command,
										  rptr_socketDats.datsLen); 						
						}

//						ESP_LOGI(TAG, "[Tips_NWK-SKdats]: node_hb rvc-<mac:%02X %02X %02X %02X %02X>,<cmd:%02X>.\n", 
//									  mptr_S2Z.dats.dats_conv.macAddr[0],
//									  mptr_S2Z.dats.dats_conv.macAddr[1],
//									  mptr_S2Z.dats.dats_conv.macAddr[2],
//									  mptr_S2Z.dats.dats_conv.macAddr[3],
//									  mptr_S2Z.dats.dats_conv.macAddr[4],
//									  rptr_socketDats.command); 

						xQueueSend(xMsgQ_Socket2Zigb, (void *)&mptr_S2Z, 0);
						
					}break;

					default:{}break;
				}
			}
		}

/*>>>>>>开关状态改变，推送响应处理<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
		//不存在

/*>>>>>>zigbee消息数据处理<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
		xMsgQ_rcvResult = xQueueReceive(xMsgQ_Zigb2Socket, (void *)&rptr_Z2S, 0);
		if(xMsgQ_rcvResult == pdTRUE){

			memset(datsTX_buff, 0, socketsData_datsTxLen * sizeof(uint8_t)); //清缓存

			switch(rptr_Z2S.msgType){ //消息类型判断

				/*常规数据中转*/
				case conventional:{

//					ESP_LOGI(TAG, "[Tips_NWK-ZBmsg]: msgRcv from zigb <len: %d>\n", rptr_Z2S.dats.dats_conv.datsLen);

					switch(rptr_Z2S.dats.dats_conv.datsFrom){

						case datsFrom_ctrlLocal:{

							memcpy(datsTX_buff, rptr_Z2S.dats.dats_conv.dats, rptr_Z2S.dats.dats_conv.datsLen);
							datsTX_buff[0] = FRAME_HEAD_MOBILE;
							datsTX_Len = dataTransLength_objLOCAL;
							sockets_datsSend(Obj_udpLocal_A, datsTX_buff, datsTX_Len);
							
						}break;

						case datsFrom_ctrlRemote:{

							memcpy(datsTX_buff, rptr_Z2S.dats.dats_conv.dats, rptr_Z2S.dats.dats_conv.datsLen);
							datsTX_buff[0] = FRAME_HEAD_SERVER;
							datsTX_Len = dataTransLength_objREMOTE;
							sockets_datsSend(Obj_udpRemote_B, datsTX_buff, datsTX_Len);
							
						}break;

						case datsFrom_heartBtRemote:{

							if(nwkInternetOnline_IF){ //网关正常则向远端服务器发送心跳

								switch(rptr_Z2S.dats.dats_conv.dats[0]){
									
									case FRAME_HEAD_HEARTB:{
										
										memcpy(datsTX_buff, rptr_Z2S.dats.dats_conv.dats, rptr_Z2S.dats.dats_conv.datsLen);
										datsTX_buff[0] = FRAME_HEAD_HEARTB;
										datsTX_Len = dataHeartBeatLength_objSERVER;
										sockets_datsSend(Obj_udpRemote_B, datsTX_buff, datsTX_Len);
//										printf_datsHtoA("[Tips_NWK-ZBmsg]: zb_HB datsSend is:", datsTX_buff, datsTX_Len);
		
									}break;

									case DTMODEKEEPACESS_FRAMEHEAD_ONLINE:{

										memcpy(datsTX_buff, rptr_Z2S.dats.dats_conv.dats, rptr_Z2S.dats.dats_conv.datsLen);
										datsTX_buff[0] = DTMODEKEEPACESS_FRAMEHEAD_ONLINE;
										datsTX_Len = rptr_Z2S.dats.dats_conv.datsLen;
//										ESP_LOGI(TAG, "[Tips_socketUDP_B]: resTX:%d.\n", sockets_datsSend(Obj_udpRemote_B, datsTX_buff, datsTX_Len));
										sockets_datsSend(Obj_udpRemote_B, datsTX_buff, datsTX_Len);

									}break;

									default:{}break;
								}
							}
							else
							{ //网络离线则由网关自行回复给子设备心跳

								stt_threadDatsPass *mptrTemp_S2Z = &rptr_Z2S;

								switch(rptr_Z2S.dats.dats_conv.dats[0]){
									
									case FRAME_HEAD_HEARTB:{

										if(rptr_Z2S.dats.dats_conv.dats[2] == FRAME_HEARTBEAT_cmdEven){ //奇数心跳包
										
											mptrTemp_S2Z->dats.dats_conv.dats[0] = ZIGB_OFFLINEFRAMEHEAD_HEARTBEAT;
											xQueueSend(xMsgQ_Socket2Zigb, (void *)mptrTemp_S2Z, 0);
										
										}else
										if(rptr_Z2S.dats.dats_conv.dats[2] == FRAME_HEARTBEAT_cmdOdd){ //偶数心跳包
										
										
										}
										
									}break;
								
									case DTMODEKEEPACESS_FRAMEHEAD_ONLINE:{

										mptrTemp_S2Z->dats.dats_conv.dats[0] = DTMODEKEEPACESS_FRAMEHEAD_OFFLINE;
										xQueueSend(xMsgQ_Socket2Zigb, (void *)mptrTemp_S2Z, 0);

									}break;
								
									default:{}break;
								}
							}

						}break;

						default:{

							
						}break;
					}
				}break;

				/*子设备列表请求数据中转*/
				case listDev_query:{}break;

				default:break;
			}
		}

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void devL7SmartlivingUdp_remoteComunication_initialization(void){

	portBASE_TYPE xRes = pdFAIL;

	xMsgQ_datsFromSocketPort = xQueueCreate(30, sizeof(stt_threadDatsPass));

	xRes = xTaskCreate(devL7SmartlivingUdpLocalCommunicationProcessTask, "udpTaskL7Local", 1024 * 4, (void *)NULL, 5, &pxTaskHandle_l7ProcessSocketLocal);
	xRes = xTaskCreate(devL7SmartlivingUdpRemoteCommunicationProcessTask, "udpTaskL7Remote", 1024 * 4, (void *)NULL, 5, &pxTaskHandle_l7ProcessSocketRemote);
	esp_log_level_set(TAG, ESP_LOG_INFO);
}

void devL7SmartlivingUdp_remoteComunication_deinit(void){

	vTaskDelete(&pxTaskHandle_l7ProcessSocketLocal);
}









































