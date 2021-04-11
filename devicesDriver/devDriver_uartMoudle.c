#include "devDriver_uartMoudle.h"
#include "devDriver_L7znp.h"
#include "devDriver_L7socket.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "devDriver_manage.h"
#include "dataTrans_localHandler.h"
#include "bussiness_timerSoft.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_UART_MOUDLE)

 #define DEV_UART_MOUDLE_PORT		UART_NUM_1
 #define DEV_UART_MOUDLE_TXD_PIN 	(GPIO_NUM_18)
 #define DEV_UART_MOUDLE_RXD_PIN 	(GPIO_NUM_19)
 #define DEV_UART_MOUDLE_BAUDRATE	(921600)

 #define DEV_UARTMOUDLE_DT_FRAME_HEAD			0xFC

 #define DEV_UARTMOUDLE_PROTOCOL_CMD_CONTROL		0x30
 #define DEV_UARTMOUDLE_PROTOCOL_CMD_CONFIG			0x31
 #define DEV_UARTMOUDLE_PROTOCOL_CMD_SETTING		0x32
 #define DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK			0x33

 extern xQueueHandle xMsgQ_zigbFunRemind;

 extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;
 extern stt_nodeDev_detailInfoManage *listHead_nodeInfoDetailManage;

 extern void devDriverBussiness_scnarioSwitch_uintDevCtrlExcuteExpose(uint8_t dstMac[MWIFI_ADDR_LEN], uint8_t opreatVal);
 extern void devUartMoudle_roomGroupParam_nvsSet(ITEHA_roomDataParamStorage *roomParam, uint8_t groupIndex);
 extern void devUartMoudle_roomGroupParam_nvsGet(ITEHA_roomDataParamStorage *roomParam, uint8_t groupIndex);

 xQueueHandle msgQh_devListPageInfoChg = NULL;

 static const char *TAG = "lanbon_L8 - devUartMoudle";

 static const uint16_t fragmentDevUartMoudle_frameMinLen = 5;

 static const uint16_t DEV_UART_MOUDLE_RX_BUF_SIZE = 1024;
 static uint16_t RING_BUFFER_LENGTH_MAX = 0;

 static bool devDriver_moudleInitialize_Flg = false;

 static TaskHandle_t *tHandle_uartMoudleProcess = NULL;

 static uint8_t *pRingBuffer = NULL; //环形缓存
 static struct{
	uint16_t indexFront; //头标
	uint16_t indexRear; //尾标
	uint16_t loadLength; //有效长度
	uint16_t surplusLength; //剩余长度
 }ringBufferAttrParam = {0};

 static void devDriverBussiness_uartMoudle_periphInit(void){

	 const uart_config_t uart_config = {
		 .baud_rate = DEV_UART_MOUDLE_BAUDRATE,
		 .data_bits = UART_DATA_8_BITS,
		 .parity = UART_PARITY_DISABLE,
		 .stop_bits = UART_STOP_BITS_1,
		 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	 };
		 
	 uart_param_config(DEV_UART_MOUDLE_PORT, &uart_config);
	 uart_set_pin(DEV_UART_MOUDLE_PORT, DEV_UART_MOUDLE_TXD_PIN, DEV_UART_MOUDLE_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	 
	 // We won't use a buffer for sending data.
	 uart_driver_install(DEV_UART_MOUDLE_PORT, DEV_UART_MOUDLE_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
 }
 
 static void devDriverBussiness_uartMoudle_periphDeinit(void){

	uart_driver_delete(DEV_UART_MOUDLE_PORT);
	gpio_reset_pin(DEV_UART_MOUDLE_TXD_PIN);
	gpio_reset_pin(DEV_UART_MOUDLE_RXD_PIN);
 }

 static uint8_t numCheck_xor(uint8_t dats[], uint16_t indexStart, uint16_t datsLen){
 
	uint8_t res = 0;
	uint16_t loop = 0;
 
	for(loop = 0; loop < datsLen; loop ++)res ^= dats[indexStart + loop];
 
	return res;
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
		 free(datsLogPtr);
	 }
 }

 static void devMeshMoudleDataSendToUart(uint8_t cmdReq, uint8_t *dptr, uint16_t datsLen){

	uint8_t *dptrTx = (uint8_t *)os_zalloc(sizeof(uint8_t) * (datsLen + 5));

	if(NULL != dptrTx){

		uint16_t datsTxLen = 0;

		dptrTx[0] = DEV_UARTMOUDLE_DT_FRAME_HEAD;
		dptrTx[1] = (uint8_t)((datsLen & 0xFF00) >> 8);
		dptrTx[2] = (uint8_t)((datsLen & 0x00FF) >> 0);
		dptrTx[3] = cmdReq;
		memcpy(&dptrTx[4], dptr, sizeof(uint8_t) * datsLen);
		dptrTx[datsLen + 4] = numCheck_xor(dptrTx, 1, datsLen + 3);

		datsTxLen = uart_write_bytes(DEV_UART_MOUDLE_PORT, (const char *)dptrTx, datsLen + 5);
//		ESP_LOGI(TAG, "devMeshMoudle data tx, realLen:%d, tarLen:%d.", datsTxLen, datsLen + 5);

		os_free(dptrTx);
	}
 }

 static void devMeshUartMoudleDevUnitCtrlExcute(uint8_t dstMac[MWIFI_ADDR_LEN], uint8_t opreatVal, bool mulit_if){
 
	 mdf_err_t ret = MDF_OK;
	 
	 const mlink_httpd_type_t type_L8mesh_cst = {
	 
		 .format = MLINK_HTTPD_FORMAT_HEX,
	 };
	 mwifi_data_type_t data_type = {
		 
		 .compression = true,
		 .communicate = MWIFI_COMMUNICATE_UNICAST,
	 };

	 uint8_t meshDatsRequesTempLen = 1 + sizeof(stt_superCtrl_dtTransParam);
	 uint8_t *meshDatsRequest_ptr = NULL;
	 uint8_t devSelfMac[MWIFI_ADDR_LEN] = {0};
	 stt_superCtrl_dtTransParam *dataCtrlPtr = NULL;

	 if(false == mwifi_is_connected())return;
	 
	 esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);
	 if(!memcmp(devSelfMac, dstMac, sizeof(uint8_t) * MWIFI_ADDR_LEN))return; //拒绝自己调用自己

	 meshDatsRequest_ptr = (uint8_t *)os_zalloc(sizeof(uint8_t) * meshDatsRequesTempLen);
	 if(NULL != meshDatsRequest_ptr){

		 (true == mulit_if)? //是否为批量控制
		 	(meshDatsRequest_ptr[0] = L8DEV_MESH_CMD_SCENARIO_SET):
		 	(meshDatsRequest_ptr[0] = L8DEV_MESH_CMD_SUPER_CTRL);
		 dataCtrlPtr = (stt_superCtrl_dtTransParam *)&meshDatsRequest_ptr[1];
		 memcpy(dataCtrlPtr->targetDevMac, dstMac, sizeof(uint8_t) * MWIFI_ADDR_LEN);
		 dataCtrlPtr->devStateSet = opreatVal;
		 
		 memcpy(&(data_type.custom), &type_L8mesh_cst, sizeof(uint32_t));
		 
		 if(esp_mesh_get_layer() == MESH_ROOT){
		 
			 ret = mwifi_root_write((const uint8_t *)dstMac, 1, &data_type, meshDatsRequest_ptr, 1 + sizeof(stt_superCtrl_dtTransParam), true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
			 MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 
		 }
		 else
		 {
			 ret = mwifi_write((const uint8_t *)dstMac, &data_type, meshDatsRequest_ptr, 1 + sizeof(stt_superCtrl_dtTransParam), true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
			 MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> heartbeat mwifi_root_write", mdf_err_to_name(ret));
		 }

		 os_free(meshDatsRequest_ptr);
	 }
 }

 static void devMeshUartMoudleCheckExcuteL8DevNetworkInfoNotice(void){

	size_t subCmdLen = 1;
	size_t devNwkInfoLen = sizeof(sttAttrDatsInfoDevL8Network);
	size_t dptrLen = subCmdLen + devNwkInfoLen;
	uint8_t *dptr = (uint8_t *)os_zalloc(dptrLen);

	if(NULL != dptr){

		uartMoudleDatsDtPkgAttr dataPkg = {0};
		uint8_t *subCmd = NULL;
		sttAttrDatsInfoDevL8Network *infoDptr = NULL;
		uint16_t ist = 0;

		subCmd = (uint8_t *)&dptr[ist];
		*subCmd = chkSubCmd_L8networkInfo;
		ist += subCmdLen;

		//核心信息填装
		infoDptr = (sttAttrDatsInfoDevL8Network *)&dptr[ist];
		esp_wifi_get_config(ESP_IF_WIFI_STA, &infoDptr->wifiCfgInfo); //wifi信息
		(mwifi_is_connected())? //连接状态
			(infoDptr->devConnectedIf = 1):
			(infoDptr->devConnectedIf = 0);
		infoDptr->devRootMeshRole =\
			esp_mesh_get_layer(); //连接角色
		ist += devNwkInfoLen;

		dataPkg.dptr = dptr;
		dataPkg.dptrLen = dptrLen;
		devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, dataPkg.dptr, dataPkg.dptrLen);

		os_free(dptr);
	}
 }

 static void devMeshUartMoudleCheckExcuteL7DevNetworkInfoNotice(void){

	size_t subCmdLen = 1;
	size_t devNwkInfoLen = sizeof(sttAttrDatsInfoDevL7Network);
	size_t dptrLen = subCmdLen + devNwkInfoLen;
	uint8_t *dptr = (uint8_t *)os_zalloc(dptrLen);

	if(NULL != dptr){

		uartMoudleDatsDtPkgAttr dataPkg = {0};
		uint8_t *subCmd = NULL;
		sttAttrDatsInfoDevL7Network *infoDptr = NULL;
		uint16_t ist = 0;

		subCmd = (uint8_t *)&dptr[ist];
		*subCmd = chkSubCmd_L7networkInfo;
		ist += subCmdLen;

		//核心信息填装
		infoDptr = (sttAttrDatsInfoDevL8Network *)&dptr[ist];
		L7SystemZnpNetworkInfoGet(infoDptr);
		ist += devNwkInfoLen;

		dataPkg.dptr = dptr;
		dataPkg.dptrLen = dptrLen;
		devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, dataPkg.dptr, dataPkg.dptrLen);

		os_free(dptr);
	}
 }

 static void devMeshUartMoudleCheckExcuteL8DevSystemInfoNotice(void){

	 size_t subCmdLen = 1;
	 size_t devSysInfoLen = sizeof(sttAttrDatsInfoDevL8System);
	 size_t dptrLen = subCmdLen + devSysInfoLen;
	 uint8_t *dptr = (uint8_t *)os_zalloc(dptrLen);
	 
	 if(NULL != dptr){
	 
		 uartMoudleDatsDtPkgAttr dataPkg = {0};
		 uint8_t *subCmd = NULL;
		 sttAttrDatsInfoDevL8System *infoDptr = NULL;
		 uint16_t ist = 0;
	 
		 subCmd = (uint8_t *)&dptr[ist];
		 *subCmd = chkSubCmd_L8systemInfo;
		 ist += subCmdLen;

		 //核心信息填装
		 infoDptr = (sttAttrDatsInfoDevL8System *)&dptr[ist];
		 strcpy(infoDptr->versionInfo, L8_DEVICE_VERSION_REF_DISCRIPTION); //版本描述
		 ist += devSysInfoLen;
	 
		 dataPkg.dptr = dptr;
		 dataPkg.dptrLen = dptrLen;
		 devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, dataPkg.dptr, dataPkg.dptrLen);
	 
		 os_free(dptr);
	 }
 }

 static void devMeshUartMoudleRespFunction_control(void *arg){

	uint8_t *dptr = (uint8_t *)arg;
	
	uint8_t *subCmd = NULL;
	size_t subCmdLen = 1;
	uint16_t ist = 0;

	subCmd = (uint8_t *)&dptr[ist];
	ist += subCmdLen;

	switch(*subCmd){

		case ctrlSubCmd_L8DevStateCtrl:{

			struct tabHead_dataDiscrip{
				uint8_t devNum;
			}*dataTabHead = (struct tabHead_dataDiscrip *)&dptr[ist];  //数量描述 + 房间名
			size_t	dataTabHead_len = sizeof(struct tabHead_dataDiscrip);
			uint8_t loop = 0;
			stt_scenarioUnitOpreatParam *dataUnitCtrl = NULL;
			bool mulitCtrl_flg = false;

			ist += dataTabHead_len;
			for(loop = 0; loop < dataTabHead->devNum; loop ++){

				ist += sizeof(stt_scenarioUnitOpreatParam) * loop;
				dataUnitCtrl = (stt_scenarioUnitOpreatParam *)&dptr[ist];
				(dataTabHead->devNum > 1)?(mulitCtrl_flg = true):(mulitCtrl_flg = false);
				devMeshUartMoudleDevUnitCtrlExcute(dataUnitCtrl->unitDevMac, dataUnitCtrl->unitDevOpreat_val, mulitCtrl_flg);
				ESP_LOGI(TAG, "devL8Ctrl "MACSTR" complete, state:%02XH", MAC2STR(dataUnitCtrl->unitDevMac), dataUnitCtrl->unitDevOpreat_val);
			}

		}break;
		
		case ctrlSubCmd_L7DevStateCtrl:{

			struct tabHead_dataDiscrip{
				uint8_t devNum;
			}*dataTabHead = (struct tabHead_dataDiscrip *)&dptr[ist];  //数量描述 + 房间名
			size_t	dataTabHead_len = sizeof(struct tabHead_dataDiscrip);
			uint8_t loop = 0;
			scenarioOprateUnit_Attr *dataUnitCtrl = NULL;

			ist += dataTabHead_len;
			for(loop = 0; loop < dataTabHead->devNum; loop ++){
				
				ist += sizeof(scenarioOprateUnit_Attr) * loop;
				dataUnitCtrl = (scenarioOprateUnit_Attr *)&dptr[ist];
				smartLivingL7DeviceControlExtution(dataUnitCtrl->devNode_MAC, dataUnitCtrl->devNode_opStatus);
				ESP_LOGI(TAG, "devL7Ctrl dev[mac:%02x:%02x:%02x:%02x:%02x] complete, state:%02XH", 
							  dataUnitCtrl->devNode_MAC[0],
							  dataUnitCtrl->devNode_MAC[1],
							  dataUnitCtrl->devNode_MAC[2],
							  dataUnitCtrl->devNode_MAC[3],
							  dataUnitCtrl->devNode_MAC[4],
							  dataUnitCtrl->devNode_opStatus);
			}

		}break;

		default:break;
	}

 }

 static void devMeshUartMoudleRespFunction_config(void *arg){

 	
 }

 static void devMeshUartMoudleRespFunction_setting(void *arg){

	 uint8_t *dptr = (uint8_t *)arg;
	 
	 uint8_t *subCmd = NULL;
	 size_t subCmdLen = 1;
	 uint16_t ist = 0;
	 
	 subCmd = (uint8_t *)&dptr[ist];
	 ist += subCmdLen;
 
 //  ESP_LOGI(TAG, "handle satrt, scmd:%02X", dptr[0]);
	 
	 switch(*subCmd){
 
		 case setSubCmd_L8roomParamSet:{

			ITEHA_roomDataParamStorage *roomParam = (ITEHA_roomDataParamStorage *)os_zalloc(sizeof(ITEHA_roomDataParamStorage));

			if(NULL != roomParam){

				uint8_t loop = 0;
				struct tabHead_dataDiscrip{
					uint8_t groupIndex;
					uint8_t devNum;
					char roomName[ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX];
				}*dataTabHead = NULL;  //数量描述 + 房间名
				uint8_t *dataHandle = &dptr[1];
				ITEHA_roomDataDevUnitParam *ptr_devUnitParam = NULL;
				size_t subCmd = 1;
				size_t dataTabHead_len = sizeof(struct tabHead_dataDiscrip);
				size_t devUnitParamLen = sizeof(ITEHA_roomDataDevUnitParam);
				
				dataTabHead = (struct tabHead_dataDiscrip *)dataHandle;
				roomParam->devUnitNum = dataTabHead->devNum;
				if(roomParam->devUnitNum)roomParam->roomAvailableFlg = 1;
				memcpy(roomParam->roomName, dataTabHead->roomName, sizeof(uint8_t) * ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX);
				for(loop = 0; loop < roomParam->devUnitNum; loop ++){
					ptr_devUnitParam = (ITEHA_roomDataDevUnitParam *)&dataHandle[dataTabHead_len + devUnitParamLen * loop];
					memcpy(&roomParam->devUnitParam[loop], ptr_devUnitParam, sizeof(ITEHA_roomDataDevUnitParam));
				}

				ESP_LOGI(TAG, "cmd setting handle, roomParam set, roomIndex:%02X", dataTabHead->groupIndex);	
				devUartMoudle_roomGroupParam_nvsSet(roomParam, dataTabHead->groupIndex);

				os_free(roomParam);
			}
		 }break;

		 case setSubCmd_L7networkFuncReq:{

			size_t devFuncTypeLen = sizeof(uint8_t);
			enum_zigbFunMsg sptr_zigbFunRm = msgFun_nwkOpen;

			uint8_t *funcReq = (uint8_t *)&dptr[ist];
			ist += devFuncTypeLen;
			xQueueSend(xMsgQ_zigbFunRemind, (void *)&sptr_zigbFunRm, 0);

		 }break;

		 default:break;
	 }
 }

 static void devMeshUartMoudleRespFunction_check(void *arg){

	uint8_t *dptr = (uint8_t *)arg;
	
	uint8_t *subCmd = NULL;
	size_t subCmdLen = 1;
	uint16_t ist = 0;
	
	subCmd = (uint8_t *)&dptr[ist];
	ist += subCmdLen;

//	ESP_LOGI(TAG, "handle satrt, scmd:%02X", dptr[0]);
	
	switch(*subCmd){

		case chkSubCmd_hbList:{ //获取到数据请求

			uint8_t *infoListPtr = L8devStatusInfoGet(listHead_nodeDevDataManage);
		
			if(NULL != infoListPtr){

				uint8_t *sptr = NULL;
			
				if(DEVLIST_MANAGE_LISTNUM_MASK_NULL != infoListPtr[0]){
					
					uint8_t infoListTotalLen = sizeof(stt_devStatusInfoResp) * infoListPtr[0] + 1;
					
					sptr = (uint8_t *)os_zalloc(infoListTotalLen + 1);
					if(NULL != sptr){

						sptr[0] = chkSubCmd_hbList; //子命令
						memcpy(&sptr[1], infoListPtr, sizeof(uint8_t) * infoListTotalLen);
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, infoListTotalLen + 1);
						os_free(sptr);
					}
				}
				else
				{
					sptr = (uint8_t *)os_zalloc(1 + 1);
					if(NULL != sptr){

						sptr[0] = chkSubCmd_hbList; //子命令
						sptr[1] = DEVLIST_MANAGE_LISTNUM_MASK_NULL;
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, 1 + 1);
						os_free(sptr);
					}
				}
				os_free(infoListPtr);
			}
		}break;

		case chkSubCmd_L8DevDetailInfoList:{ //获取到数据请求

			uint8_t *l8infoListPtr = NULL;

			devDetailInfoList_request_trigByEvent(); //请求一次设备列表
			vTaskDelay(1500 / portTICK_RATE_MS);
			l8infoListPtr = L8devInfoDetailManageList_data2RootTabGet(listHead_nodeInfoDetailManage);

			if(NULL != l8infoListPtr){

				uint8_t *sptr = NULL;
				
//				ESP_LOGI(TAG, "handle step_A");

				if(DEVLIST_MANAGE_LISTNUM_MASK_NULL != l8infoListPtr[0]){

					struct tabHead_dataDiscrip{
					
						uint16_t dataTabCompleteLen;
						uint8_t dataNullForExt;
						uint8_t devNodeNum;
					}*dataTabHead = (struct tabHead_dataDiscrip *)&l8infoListPtr[0];
					sptr = (uint8_t *)os_zalloc(dataTabHead->dataTabCompleteLen + 1);
					if(NULL != sptr){

//						ESP_LOGI(TAG, "handle step_B");

						sptr[0] = chkSubCmd_L8DevDetailInfoList; //子命令
						memcpy(&sptr[1], l8infoListPtr, sizeof(uint8_t) * dataTabHead->dataTabCompleteLen);
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, dataTabHead->dataTabCompleteLen + 1);
						os_free(sptr);
					}
				}
				else
				{
					sptr = (uint8_t *)os_zalloc(1 + 1);
					if(NULL != sptr){

//						ESP_LOGI(TAG, "handle step_B");
					
						sptr[0] = chkSubCmd_L8DevDetailInfoList; //子命令
						sptr[1] = DEVLIST_MANAGE_LISTNUM_MASK_NULL;
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, 1 + 1);
						os_free(sptr);
					}
				}
				os_free(l8infoListPtr);
			}

		}break;

		case chkSubCmd_L7DevUnitParamList:{ //获取到数据请求

			uint8_t *l7infoListPtr = L7devListParamInfoTabGet();

			if(NULL != l7infoListPtr){

				uint8_t *sptr = NULL;

				if(0 != l7infoListPtr[0]){

					struct tabHead_dataDiscrip{
					
						uint16_t dataTabCompleteLen;
						uint8_t dataNullForExt;
						uint8_t devNodeNum;
					}*dataTabHead = (struct tabHead_dataDiscrip *)&l7infoListPtr[0];
					sptr = (uint8_t *)os_zalloc(dataTabHead->dataTabCompleteLen + 1);
					if(NULL != sptr){

						sptr[0] = chkSubCmd_L7DevUnitParamList; //子命令
						memcpy(&sptr[1], l7infoListPtr, sizeof(uint8_t) * dataTabHead->dataTabCompleteLen);
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, dataTabHead->dataTabCompleteLen + 1);
						os_free(sptr);
					}
				}
				else
				{
					sptr = (uint8_t *)os_zalloc(1 + 1);
					if(NULL != sptr){
					
						sptr[0] = chkSubCmd_L7DevUnitParamList; //子命令
						sptr[1] = DEVLIST_MANAGE_LISTNUM_MASK_NULL;
						devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, 1 + 1);
						os_free(sptr);
					}
				}
				os_free(l7infoListPtr);
			}

		}break;

		case chkSubCmd_L7networkInfo:{ //获取到数据请求

			devMeshUartMoudleCheckExcuteL7DevNetworkInfoNotice();
		
		}break;
		
		case chkSubCmd_L8networkInfo:{ //获取到数据请求

			devMeshUartMoudleCheckExcuteL8DevNetworkInfoNotice();

		}break;
		
		case chkSubCmd_L8systemInfo:{ //获取到数据请求

			devMeshUartMoudleCheckExcuteL8DevSystemInfoNotice();

		}break;
		
		case chkSubCmd_L8roomParamGet:{ //获取到数据请求

			ITEHA_roomDataParamStorage *ptr_roomParam = (ITEHA_roomDataParamStorage *)os_zalloc(sizeof(ITEHA_roomDataParamStorage));

			if(NULL != ptr_roomParam){

				struct tabHead_dataRxDiscrip{
					uint8_t groupIndex;
				}*dataRxTabHead = (struct tabHead_dataRxDiscrip *)&dptr[1];
				devUartMoudle_roomGroupParam_nvsGet(ptr_roomParam, dataRxTabHead->groupIndex);

				struct tabHead_dataDiscrip{
					uint8_t groupIndex;
					uint8_t devNum;
					char roomName[ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX];
				}*dataTabHead = NULL;  //数量描述 + 房间名
				size_t subCmd = 1;
				size_t dataTabHead_len = sizeof(struct tabHead_dataDiscrip);
				size_t devUnitParamLen = sizeof(ITEHA_roomDataDevUnitParam);
				size_t sptrLen = subCmd + dataTabHead_len + devUnitParamLen * ptr_roomParam->devUnitNum;
				uint8_t *sptr = (uint8_t *)os_zalloc(sptrLen);
			
				ESP_LOGI(TAG, "cmd check handle, roomParam check, roomIndex:%02X", dataRxTabHead->groupIndex);
				
				if(NULL != sptr){
				
					uint8_t loop = 0;
					uint16_t ist = 0;
					uint8_t *p = NULL;
					uartMoudleDatsDtPkgAttr dataPkg = {0};
				
					memset(sptr, 0, sptrLen);
				
					sptr[0] = chkSubCmd_L8roomParamGet;
					ist += subCmd;
					
					dataTabHead = (struct tabHead_dataDiscrip *)&sptr[ist];
					dataTabHead->groupIndex = dataRxTabHead->groupIndex;
					dataTabHead->devNum = ptr_roomParam->devUnitNum;
					memcpy(dataTabHead->roomName, ptr_roomParam->roomName, sizeof(char) * ITEHA_DEV_ROOM_GROUP_NAME_LEN_MAX);
					ist += dataTabHead_len;
				
					for(loop = 0; loop < dataTabHead->devNum; loop ++){
				
						ist += (devUnitParamLen * loop);
						p = &sptr[ist];
						memcpy(p, &ptr_roomParam->devUnitParam[loop], devUnitParamLen);
					}	
				
					dataPkg.dptr = sptr;
					dataPkg.dptrLen = sptrLen;
					devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, dataPkg.dptr, dataPkg.dptrLen);
				
					os_free(sptr);
				}

				os_free(ptr_roomParam);
			}

		}break;

		default:break;
	}
 }

 static void functionRingBuffer_fragmentAdd(uint8_t dats[], uint16_t datsLen){

	bool frameDatsValid_flg = true;
	uint16_t loop = 0;

	//buffer未初始化
	if(NULL == pRingBuffer)return;

	//获取剩余空间
	ringBufferAttrParam.surplusLength = RING_BUFFER_LENGTH_MAX - ringBufferAttrParam.loadLength;

	//剩余空间足够则进行缓存
	if(ringBufferAttrParam.surplusLength > datsLen){

		for(loop = 0; loop < datsLen; loop ++){ //数据装填

			pRingBuffer[ringBufferAttrParam.indexRear] = dats[loop];
			ringBufferAttrParam.indexRear = (ringBufferAttrParam.indexRear + 1) % RING_BUFFER_LENGTH_MAX;
			ringBufferAttrParam.loadLength ++;
		}

//		ESP_LOGI(TAG, "check step_A");

		while(true == frameDatsValid_flg){

			while((pRingBuffer[ringBufferAttrParam.indexFront] != DEV_UARTMOUDLE_DT_FRAME_HEAD) && (ringBufferAttrParam.loadLength > 0)){
			
				ringBufferAttrParam.indexFront = (ringBufferAttrParam.indexFront + 1) % RING_BUFFER_LENGTH_MAX;
				ringBufferAttrParam.loadLength --;
			}

			if((pRingBuffer[ringBufferAttrParam.indexFront] == DEV_UARTMOUDLE_DT_FRAME_HEAD) && (ringBufferAttrParam.loadLength >= fragmentDevUartMoudle_frameMinLen)){ //锁定有效数据帧

				uint16_t frontIst_dLenHSB = (ringBufferAttrParam.indexFront + 1) % RING_BUFFER_LENGTH_MAX, //帧头下标+1获取数据长度HSB
					     frontIst_dLenLSB = (ringBufferAttrParam.indexFront + 2) % RING_BUFFER_LENGTH_MAX; //帧头下标+2获取数据长度LSB
				uint16_t frameLen = (pRingBuffer[frontIst_dLenHSB] << 8) | (pRingBuffer[frontIst_dLenLSB] << 0);

//				ESP_LOGI(TAG, "check step_B");

				if((frameLen + 5) <= ringBufferAttrParam.loadLength){ //可疑有效帧总长度小于当前有效数据长度，进行有效性判断

					uint16_t datsFrameLen = frameLen + 5;
					uint8_t *datsFrame = (uint8_t *)os_zalloc(sizeof(uint8_t) * datsFrameLen);
					uint16_t datsFrameAssign_ist = 0;

//					ESP_LOGI(TAG, "check step_C");

					if(NULL != datsFrame){

						memset(datsFrame, 0, sizeof(uint8_t) * datsFrameLen);
						
						for(uint16_t dIst = 0; dIst < datsFrameLen; dIst ++){ //有效帧数据进行缓存 -datsFrameLen是整个数据帧的长度
	
							datsFrameAssign_ist = (ringBufferAttrParam.indexFront + dIst) % RING_BUFFER_LENGTH_MAX;
							datsFrame[dIst] = pRingBuffer[datsFrameAssign_ist];
						}	

						if(datsFrame[frameLen + 4] == numCheck_xor(datsFrame, 1, frameLen + 3)){ //异或校验，验证有效性

							bool flg_znpFrameRcg = false;
							uint8_t *rptr = NULL;
	
							ringBufferAttrParam.indexFront += (frameLen + 5);
							ringBufferAttrParam.indexFront %= RING_BUFFER_LENGTH_MAX;
							ringBufferAttrParam.loadLength -= (frameLen + 5);

							ESP_LOGI(TAG, "devMeshMoudle legal frame get, totalLen:%d, cmd:%02XH.\n", frameLen + 5, datsFrame[3]);

							switch(datsFrame[3]){ //命令解析

								case DEV_UARTMOUDLE_PROTOCOL_CMD_CONTROL:{ //控制

									rptr = &datsFrame[4];
									devMeshUartMoudleRespFunction_control((void *)rptr);
									flg_znpFrameRcg = true;
								}break;
								
								case DEV_UARTMOUDLE_PROTOCOL_CMD_CONFIG:{ //配置

									flg_znpFrameRcg = true;
								}break;
								
								case DEV_UARTMOUDLE_PROTOCOL_CMD_SETTING:{ //设置

									rptr = &datsFrame[4];
									devMeshUartMoudleRespFunction_setting((void *)rptr);
									flg_znpFrameRcg = true;
								}break;
								
								case DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK:{ //查询

									rptr = &datsFrame[4];
									devMeshUartMoudleRespFunction_check((void *)rptr);
									flg_znpFrameRcg = true;
								}break;

								default:break;
							}

							if(false == flg_znpFrameRcg){ //杂项命令数据

								
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
					if((frameLen + 5) >= RING_BUFFER_LENGTH_MAX){ //可疑有效帧总长度大于queue总长度，直接判断无效，进行下一可疑帧筛选，防止等待溢出

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

 static void DevUartMoudleProcessLoopOncePartUartDataRcv(void){

	uint8_t *rptr = (uint8_t *)os_zalloc(DEV_UART_MOUDLE_RX_BUF_SIZE + 1);
	uint16_t dprxLen = 0;

	if(NULL != rptr){

		dprxLen = uart_read_bytes(DEV_UART_MOUDLE_PORT, rptr, DEV_UART_MOUDLE_RX_BUF_SIZE, 10 / portTICK_RATE_MS);
		if(dprxLen > 0){
			
			functionRingBuffer_fragmentAdd(rptr, dprxLen);
//			ESP_LOGI(TAG, "devMeshMoudle uart data rcv, len:%d.\n", dprxLen);
//			datsArray2HexPrintf("uart data rcv", rptr, dprxLen);
		}
		
		os_free(rptr);
	}
 }

 static void DevUartMoudleProcessMsgRcvPassiveLoop(void){

	msgAttr_devBlockParamChange msgDats = {0};

	if(pdTRUE == xQueueReceive(msgQh_devListPageInfoChg, &msgDats, 0)){

		size_t subCmdLen = 1;
		uint8_t *sptr = (uint8_t *)os_zalloc(sizeof(msgAttr_devBlockParamChange) + subCmdLen);
		if(NULL != sptr){

			if(lanbonDevSeq_L7 == msgDats.devSeq)
				sptr[0] = chkSubCmd_L7devUnitParamChgSycn; //子命令
			else
			if(lanbonDevSeq_L8 == msgDats.devSeq)
				sptr[0] = chkSubCmd_L8devUnitParamChgSycn; //子命令
			memcpy((msgAttr_devBlockParamChange *)&sptr[1], &msgDats, sizeof(msgAttr_devBlockParamChange));
			devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, sptr, sizeof(msgAttr_devBlockParamChange) + 1);
//			ESP_LOGI(TAG, "devSycnMsg send, nType:%d.", msgDats.nType);
			os_free(sptr);
		}
	}
 }

 static void processTaskDevUartMoudleBussiness(void* arg){

	const uint16_t vTaskLoopIdle_time = 20;
	uint8_t dataTx_temp[1] = {0};
	static stt_timerLoop timerTestUartTx = {
		2000 / vTaskLoopIdle_time,
		0
	};

	for(;;){

		DevUartMoudleProcessLoopOncePartUartDataRcv();
		DevUartMoudleProcessMsgRcvPassiveLoop();
		if(timerTestUartTx.loopCounter < timerTestUartTx.loopPeriod)timerTestUartTx.loopCounter ++;
		else{
			timerTestUartTx.loopCounter = 0;
//			devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_SETTING, dataTx_temp, 1);
		}

		vTaskDelay(vTaskLoopIdle_time / portTICK_RATE_MS);
	}

	vTaskDelete(NULL);
 }

 void devMeshUartMoudleCheckExcuteSystemReadyNoticeExcute(void){

	size_t subCmd = 1;
	uint8_t *sptr = (uint8_t *)os_zalloc(sizeof(uint8_t) * subCmd);
	uartMoudleDatsDtPkgAttr dataPkg = {0};

	if(NULL != sptr){

		sptr[0] = chkSubCmd_L8SystemReady;
		dataPkg.dptr = sptr;
		dataPkg.dptrLen = 1;

		devMeshMoudleDataSendToUart(DEV_UARTMOUDLE_PROTOCOL_CMD_CHECK, dataPkg.dptr, dataPkg.dptrLen);
		os_free(sptr);
	}
 }
 
 void devDriverBussiness_uartMoudle_moudleInit(void){

	 devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	 
	 if(swCurrentDevType != devTypeDef_uartMoudle)return;
	 if(devDriver_moudleInitialize_Flg)return;

	 esp_log_level_set(TAG, ESP_LOG_INFO);

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else
 
	devDriverBussiness_uartMoudle_periphInit();
#endif

	RING_BUFFER_LENGTH_MAX = DEV_UART_MOUDLE_RX_BUF_SIZE * 3; //常量初始化

	pRingBuffer = (uint8_t *)os_zalloc(sizeof(uint8_t) * RING_BUFFER_LENGTH_MAX);
	xTaskCreate(processTaskDevUartMoudleBussiness, "devUartProcessTk", 1024 * 8, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_uartMoudleProcess);

	msgQh_devListPageInfoChg = xQueueCreate(10, sizeof(msgAttr_devBlockParamChange));

	smartLivingL7ApplicationRingBufferInitialization();
	devL7SmartlivingUdp_remoteComunication_initialization();

	devMeshUartMoudleCheckExcuteSystemReadyNoticeExcute(); //系统准备完成通知

	devDriver_moudleInitialize_Flg = true;
 }
 
 void devDriverBussiness_uartMoudle_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
#else

	devDriverBussiness_uartMoudle_periphDeinit();
#endif

	if(NULL != tHandle_uartMoudleProcess){

		vTaskDelete(tHandle_uartMoudleProcess);
		tHandle_uartMoudleProcess = NULL;
	}

	smartLivingL7ApplicationRingBufferDeinit();
	devL7SmartlivingUdp_remoteComunication_deinit();

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_uartMoudle_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_uartMoudle){
		
	/*业务空置，独立串口模块不存在控制业务*/

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else

#endif
	}
 }

#endif
























