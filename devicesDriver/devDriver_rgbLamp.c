#include "devDriver_rgbLamp.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include <stdio.h>
#include "esp_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "devDriver_manage.h"

#include "light_driver.h"
#include "iot_light.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BELT) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_RGBLAMP_BULB)

 #define DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_R		26
 #define DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_G		32
 #define DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_B		33
 #define DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_C		27
 #define DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_W		14

 #define DEVDRIVER_RGBLAMP_FADE_PERIOD_MS		500
 #define DEVDRIVER_RGBLAMP_BLINK_PERIOD_MS		2000

 #define DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL  	0   /*!< If we connect with a IR receiver, the data is active low */
 #define DEV_RGBLAMP_RMT_TX_CARRIER_EN    	1   /*!< Enable carrier for IR transmitter test with IR led */

 #define DEV_RGBLAMP_RMT_RX_CHANNEL    		RMT_CHANNEL_7   /*!< RMT channel for receiver */
 #define DEV_RGBLAMP_RMT_RX_GPIO_NUM  		25     			/*!< GPIO number for receiver */
 #define DEV_RGBLAMP_RMT_CLK_DIV      		100    			/*!< RMT counter clock divider */
 #define DEV_RGBLAMP_RMT_TICK_10_US    		(80000000/DEV_RGBLAMP_RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

 #define DEV_RGBLAMP_NEC_HEADER_HIGH_US    	9000                         /*!< NEC protocol header: positive 9ms */
 #define DEV_RGBLAMP_NEC_HEADER_LOW_US     	4500                         /*!< NEC protocol header: negative 4.5ms*/
 #define DEV_RGBLAMP_NEC_BIT_ONE_HIGH_US    560                         /*!< NEC protocol data bit 1: positive 0.56ms */
 #define DEV_RGBLAMP_NEC_BIT_ONE_LOW_US    	(2250-DEV_RGBLAMP_NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
 #define DEV_RGBLAMP_NEC_BIT_ZERO_HIGH_US   560                         /*!< NEC protocol data bit 0: positive 0.56ms */
 #define DEV_RGBLAMP_NEC_BIT_ZERO_LOW_US   	(1120-DEV_RGBLAMP_NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
 #define DEV_RGBLAMP_NEC_BIT_END            560                         /*!< NEC protocol end: positive 0.56ms */
 #define DEV_RGBLAMP_NEC_BIT_MARGIN         150                          /*!< NEC parse margin time */

 #define DEV_RGBLAMP_NEC_ITEM_DURATION(d)  	((d & 0x7fff)*10/DEV_RGBLAMP_RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
 #define DEV_RGBLAMP_NEC_DATA_ITEM_NUM   	34  /*!< NEC code item number: header + 32bit data + end */
 #define DEV_RGBLAMP_rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

 extern void devRgbLamp_dispTrickParam_nvsSave(void *param, enumRgbLamp_nvsObj nvsObj);

 static const char *TAG = "lanbon_L8 - RGB Lamp";

 static const struct stt_rmtCtrlTrickColorTab{

	uint16_t ctrlCmd;
	stt_devRgbLampColorUnit tColor;

 }param_rmtCtrlTrickColorTab[DEV_RGBLAMP_RMT_COLOR_NUM] = {

	{0xfb04, {255,	0,	 0}}, {0xfa05, {  0, 255,  0}}, {0xf906, {  0,	 0,255}},
	{0xf708, {255,  32,	 0}}, {0xf30c, {255,  96,  0}}, {0xef10, {255, 196,	 0}}, 
	{0xeb14, {255, 255,  0}}, {0xf609, {  0, 255, 32}}, {0xf20d, {  0, 255, 96}}, 
	{0xee11, {  0, 255,196}}, {0xea15, {  0, 255,255}}, {0xf50a, { 32,	 0,255}}, 
	{0xf10e, {96,    0,255}}, {0xed12, {196,   0,255}}, {0xe916, {255,   0,255}},
 };

 static const stt_devRgbLampColorUnit lampColorDark = {0, 0, 0};
 static const stt_devRgbLampColorUnit lampColorTab_A[6] = {

	{255, 0, 0},{0, 255, 0},{0, 0, 255},
	{255, 0, 0},{0, 255, 0},{0, 0, 255},
 };
 static const stt_devRgbLampColorUnit lampColorTab_B[6] = {


	{255,  0, 0},{0, 255,  0},{0,  0, 255},
	{128,255, 0},{0, 128,255},{255,0, 128},
 };

 static bool devDriver_moudleInitialize_Flg = false;

 static TaskHandle_t *tHandle_rmtNec_rx = NULL;
 static TaskHandle_t *tHandle_devAssist = NULL;

 static xQueueHandle msgQh_devTipsTimerHandle = NULL;

 static stt_devRgbLampColorUnit rgbLampExcuteColorParam = {0};
 static stt_devRgbLampJumpParamTab devRgbLamp_sceneParam[DEV_RGBLAMP_SCENE_MAX_NUM] = {0};
 static stt_devRgbLampJumpParamTab devRgbLamp_opJumpParam = DEV_RGBLAMP_TRICK_JMPDISP_UNITPARAM_DEF;
 static stt_devRgbLampWhiteLightParam devRgbLamp_staticWhiteParam = {0, 50};
 static stt_devRgbLampColorLightParam devRgbLamp_staticColorParam = {{255, 0, 0}};
 static stt_devRgbLampMusicDispParam devRgbLamp_musicColorParam = {{0, 0, 0}};
 static stt_devRgbLampExcuteParam devRgbLamp_excuteParam = {.pd_excuteMode = DEV_RGBLAMP_TRICK_TYPE_WHITE};
 static const uint8_t devStaticColorExParam_brightnessMax = 10;
 static const uint8_t devDynamicDispExParam_speedMax = 10;
 static uint8_t devStaticColorExParam_brightness = 10;
 static uint8_t devDynamicDispExParam_speed = 5;

 static bool localRmtDynamicDisp_flg = false;

 static TimerHandle_t g_timer_lampJumpOpreat = NULL;

 static bool devTipsRunning_flg = false;
 static uint16_t volatile tipsCounterTimerLoop = 0;
 static TimerHandle_t t_timer_deviceOpreatTips = NULL;

/*
 * @brief RMT receiver initialization
 */
 static void nec_rx_init(){

    rmt_config_t rmt_rx;
    rmt_rx.channel = DEV_RGBLAMP_RMT_RX_CHANNEL;
    rmt_rx.gpio_num = DEV_RGBLAMP_RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = DEV_RGBLAMP_RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = DEV_RGBLAMP_rmt_item32_tIMEOUT_US / 10 * (DEV_RGBLAMP_RMT_TICK_10_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);
 }

/*
 * @brief Check whether duration is around target_us
 */
 inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us){

    if(( DEV_RGBLAMP_NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us))
        && ( DEV_RGBLAMP_NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
        return true;
    } else {
        return false;
    }
 }

/*
 * @brief Check whether this value represents an NEC header
 */
 static bool nec_header_if(rmt_item32_t* item){

    if((item->level0 == DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL && item->level1 != DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEV_RGBLAMP_NEC_HEADER_HIGH_US, DEV_RGBLAMP_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEV_RGBLAMP_NEC_HEADER_LOW_US, DEV_RGBLAMP_NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
 }

/*
 * @brief Check whether this value represents an NEC data bit 1
 */
 static bool nec_bit_one_if(rmt_item32_t* item){
 
    if((item->level0 == DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL && item->level1 != DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEV_RGBLAMP_NEC_BIT_ONE_HIGH_US, DEV_RGBLAMP_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEV_RGBLAMP_NEC_BIT_ONE_LOW_US, DEV_RGBLAMP_NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
 }

/*
 * @brief Check whether this value represents an NEC data bit 0
 */
 static bool nec_bit_zero_if(rmt_item32_t* item){

    if((item->level0 == DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL && item->level1 != DEV_RGBLAMP_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEV_RGBLAMP_NEC_BIT_ZERO_HIGH_US, DEV_RGBLAMP_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEV_RGBLAMP_NEC_BIT_ZERO_LOW_US, DEV_RGBLAMP_NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
 }

/*
 * @brief Parse NEC 32 bit waveform to address and command.
 */
 static int nec_parse_items(rmt_item32_t* item, int item_num, uint16_t* addr, uint16_t* data){

    int w_len = item_num;
    if(w_len < DEV_RGBLAMP_NEC_DATA_ITEM_NUM) {
        return -1;
    }
    int i = 0, j = 0;
    if(!nec_header_if(item++)) {

//		printf("nec header err.\n");
	
        return -1;
    }
    uint16_t addr_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            addr_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            addr_t |= (0 << j);
        } else {

//			printf("nec addr bit_zero err.\n");
		
            return -1;
        }
        item++;
        i++;
    }
    uint16_t data_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            data_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            data_t |= (0 << j);
        } else {

//			printf("nec data bit_zero err.\n");
		
            return -1;
        }
        item++;
        i++;
    }
    *addr = addr_t;
    *data = data_t;
    return i;
 }

/**
 * @brief RMT receiver demo, this task will print each received NEC data.
 *
 */
 static void rmt_nec_rx_task(void *arg){

    int channel = DEV_RGBLAMP_RMT_RX_CHANNEL;
    nec_rx_init();
    RingbufHandle_t rb = NULL;
    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(channel, &rb);
    rmt_rx_start(channel, 1);
    while(rb) {
        size_t rx_size = 0;
        //try to receive data from ringbuffer.
        //RMT driver will push all the data it receives to its ringbuffer.
        //We just need to parse the value and return the spaces of ringbuffer.
        rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, (portTickType)portMAX_DELAY);
        if(item) {
            uint16_t rmt_addr;
            uint16_t rmt_cmd;
            int offset = 0;
            while(1) {
                //parse data value from ringbuffer.
                int res = nec_parse_items(item + offset, rx_size / 4 - offset, &rmt_addr, &rmt_cmd);
                if(res > 0) {
                    offset += res + 1;
					
					if(DEV_RGBLAMP_RMT_ADDR == rmt_addr){

						switch(rmt_cmd){

							case DEV_RGBLAMP_RMT_CMD_ON:{

								if(0 == devRgbLamp_excuteParam.pd_rsv){
									stt_devDataPonitTypedef dpTemp = {.devType_rgbLamp.devRgbLamp_enable = 1,};
									currentDev_dataPointSet(&dpTemp, true, false, false, true, true);
								}

							}break;
							
							case DEV_RGBLAMP_RMT_CMD_OFF:{

								if(1 == devRgbLamp_excuteParam.pd_rsv){
									stt_devDataPonitTypedef dpTemp = {.devType_rgbLamp.devRgbLamp_enable = 0,};
									currentDev_dataPointSet(&dpTemp, true, false, false, true, true);
								}
							}break;
							
							case DEV_RGBLAMP_RMT_CMD_BRTS_UP:{

								stt_devRgbLampColorUnit colorTemp = {0};

								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								if(true == localRmtDynamicDisp_flg){
								
									if(0 != devDynamicDispExParam_speed)
										devDynamicDispExParam_speed --;
									devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
								}else{
									
									uint8_t temp_brt = devRgbLamp_staticWhiteParam.c_brightness / 25;
									if(temp_brt < 10)temp_brt ++; 
									devRgbLamp_staticWhiteParam.c_brightness = temp_brt * 25;
									
									if(devStaticColorExParam_brightness < devStaticColorExParam_brightnessMax)
										devStaticColorExParam_brightness ++;
									rgbLampExcuteColorParam.c_r = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_r / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);
									rgbLampExcuteColorParam.c_g = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_g / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);
									rgbLampExcuteColorParam.c_b = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_b / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);								
									devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
								}
							}break;
							
							case DEV_RGBLAMP_RMT_CMD_BRTS_DN:{

								stt_devRgbLampColorUnit colorTemp = {0};

								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								if(true == localRmtDynamicDisp_flg){

									if(devDynamicDispExParam_speed < devDynamicDispExParam_speedMax)
										devDynamicDispExParam_speed ++;
									devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
								}else{
									
									uint8_t temp_brt = devRgbLamp_staticWhiteParam.c_brightness / 25;
									if(1 != temp_brt)temp_brt --; 
									devRgbLamp_staticWhiteParam.c_brightness = temp_brt * 25;
									
									if(devStaticColorExParam_brightness > 1)
										devStaticColorExParam_brightness --;
									rgbLampExcuteColorParam.c_r = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_r / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);
									rgbLampExcuteColorParam.c_g = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_g / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);
									rgbLampExcuteColorParam.c_b = (uint8_t)((float)devRgbLamp_staticColorParam.color.c_b / (float)devStaticColorExParam_brightnessMax * (float)devStaticColorExParam_brightness);
									devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
								}
							}break;

							case DEV_RGBLAMP_RMT_CMD_WHITE:{

								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								devRgbLamp_excuteParam.pd_excuteMode = DEV_RGBLAMP_TRICK_TYPE_WHITE;
								devRgbLamp_staticWhiteParam.c_brightness = 255;
								devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_WHITE);

							}break;

							case DEV_RGBLAMP_RMT_CMD_FLASH:
								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								devDynamicDispExParam_speed = 5;
								devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_FLASH);
								break;

							case DEV_RGBLAMP_RMT_CMD_STROBE:
								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								devDynamicDispExParam_speed = 5;
								devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_STROBE);
								break;

							case DEV_RGBLAMP_RMT_CMD_FADE:
								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								devDynamicDispExParam_speed = 5;
								devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_FADE);
								break;
						
							case DEV_RGBLAMP_RMT_CMD_SMOOTH:
								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								devDynamicDispExParam_speed = 5;
								devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_SMOOTH);
								break;
						
							default:{
						
								uint8_t loop = 0;

								if(0 == devRgbLamp_excuteParam.pd_rsv)break;
								for(loop = 0; loop < DEV_RGBLAMP_RMT_COLOR_NUM; loop ++){
								
									if(param_rmtCtrlTrickColorTab[loop].ctrlCmd == rmt_cmd){

										stt_devRgbLampColorLightParam paramTemp = {0};

										memcpy(&paramTemp.color, &param_rmtCtrlTrickColorTab[loop].tColor, sizeof(stt_devRgbLampColorUnit));
										rgbLamp_staticColorDisp_param_set(&paramTemp, true);
										devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_COLOR);
										devStaticColorExParam_brightness = devStaticColorExParam_brightnessMax - 1;
										break;
									}
								}
							}break;
						}
					}
						
                    ESP_LOGI(TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr, rmt_cmd);
                } else {
                    break;
                }
            }
            //after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void*) item);
        } 
		else 
		{
            break;
        }
    }
    vTaskDelete(NULL);
 }

 static void rgbLampAssist_task(void *arg){

	stt_devRgbLampAssistTkMsg rptr = {0}; 

	for(;;){

		if(NULL != msgQh_devTipsTimerHandle){

			if(xQueueReceive(msgQh_devTipsTimerHandle, &rptr, 5 / portTICK_RATE_MS) == pdTRUE){
			
				if(0 == rptr.dType){
			
					switch(rptr.dPoint){
			
						case 0:
							devDriverBussiness_rgbLamp_tipsTrig(rgbLampTipsType_normal, 0);
							break;
			
						default:break;
					}
				}
			}
		}

		vTaskDelay(20 / portTICK_RATE_MS);
	}
 
 	vTaskDelete(NULL);
 }

 static void devLamp_excuteLightOperate(stt_devRgbLampColorUnit color, uint16_t timeFade){

	iot_light_fade_with_time_x(CHANNEL_ID_RED, color.c_r, timeFade);
	iot_light_fade_with_time_x(CHANNEL_ID_GREEN, color.c_g, timeFade);
	iot_light_fade_with_time_x(CHANNEL_ID_BLUE, color.c_b, timeFade);
 }

 static void devLamp_trikRmtFlash_timer_cb(void *arg){

	static uint8_t paramIndexRcd = 0;

	if(paramIndexRcd < 5)paramIndexRcd ++;
	else paramIndexRcd = 0;

	devLamp_excuteLightOperate(lampColorTab_A[paramIndexRcd], 0);
 }

 static void devLamp_trikRmtStrobe_timer_cb(void *arg){

	const stt_devRgbLampColorUnit cDark = {0, 0, 0};
	static bool lightFull_flg = false;

	lightFull_flg = !lightFull_flg;
	(true == lightFull_flg)?
		(devLamp_excuteLightOperate(devRgbLamp_staticColorParam.color, 0)):
		(devLamp_excuteLightOperate(cDark, 0));
 }

 static void devLamp_trikRmtFade_timer_cb(void *arg){

	static bool lightFull_flg = false;
	// uint16_t fadePeriod = 400 * devDynamicDispExParam_speed;
	uint16_t fadePeriod = devDynamicDispExParam_speed * devDynamicDispExParam_speed * 100;

	if(0 == fadePeriod)fadePeriod = 200;
	lightFull_flg = !lightFull_flg;
	(true == lightFull_flg)?
		(devLamp_excuteLightOperate(devRgbLamp_staticColorParam.color, fadePeriod)):
		(devLamp_excuteLightOperate(lampColorDark, fadePeriod));
 }

 static void devLamp_trikRmtSmooth_timer_cb(void *arg){

	static uint8_t paramIndexRcd = 0;
	// uint16_t fadePeriod = 400 * devDynamicDispExParam_speed;
	uint16_t fadePeriod = devDynamicDispExParam_speed * devDynamicDispExParam_speed * 100;

	if(0 == fadePeriod)fadePeriod = 200;
	if(paramIndexRcd < 5)paramIndexRcd ++;
	else paramIndexRcd = 0;

	devLamp_excuteLightOperate(lampColorTab_B[paramIndexRcd], fadePeriod);
 }

 static void devLamp_opJump_timer_cb(void *arg){

	static uint8_t paramIndexRcd = 0;
	stt_devRgbLampColorUnit colorTemp = {0};
	
	if(devRgbLamp_opJumpParam.paramUnitScolor[paramIndexRcd].c_temperature ||
	   devRgbLamp_opJumpParam.paramUnitScolor[paramIndexRcd].c_brightness){

		iot_function_tb2hsv(devRgbLamp_opJumpParam.paramUnitScolor[paramIndexRcd].c_brightness,
							devRgbLamp_opJumpParam.paramUnitScolor[paramIndexRcd].c_temperature,
							&colorTemp.c_r,
							&colorTemp.c_g,
							&colorTemp.c_b);
	}else{

		memcpy(&colorTemp, &devRgbLamp_opJumpParam.paramUnitScolor[paramIndexRcd].pdJumpParam_c, sizeof(stt_devRgbLampColorUnit));
	}

	switch(devRgbLamp_opJumpParam.pd_method){

		case devLamp_sceneMode_method_static:{

			if(paramIndexRcd < devRgbLamp_opJumpParam.pd_tabNum - 1)paramIndexRcd ++;
			else paramIndexRcd = 0;
			devLamp_excuteLightOperate(colorTemp, 0);
		}break;
		
		case devLamp_sceneMode_method_jump:{

			static bool lightUp_flg = false;

			if(true == lightUp_flg){
				if(paramIndexRcd < devRgbLamp_opJumpParam.pd_tabNum - 1)paramIndexRcd ++;
				else paramIndexRcd = 0;
			}
			lightUp_flg = !lightUp_flg;
			(true == lightUp_flg)?
				(devLamp_excuteLightOperate(colorTemp, 0)):
				(devLamp_excuteLightOperate(lampColorDark, 0));
		}break;
		
		case devLamp_sceneMode_method_fade:{

			if(paramIndexRcd < devRgbLamp_opJumpParam.pd_tabNum - 1)paramIndexRcd ++;
			else paramIndexRcd = 0;
			devLamp_excuteLightOperate(colorTemp, (devRgbLamp_opJumpParam.pd_freqLevel + 1) * 150);
		}break;

		default:break;
	}
 }

 static void devLamp_tipsTimer_stopTrig(void){

	stt_devRgbLampAssistTkMsg sptr = {0}; 

	sptr.dType = 0;
	sptr.dPoint = 0;
	xQueueSend(msgQh_devTipsTimerHandle, &sptr, 1 / portTICK_PERIOD_MS);
 }
 
 static void devLamp_tips_a_timer_cb(void *arg){

	if(tipsCounterTimerLoop){

		tipsCounterTimerLoop --;
		(tipsCounterTimerLoop % 2)?
			(devLamp_excuteLightOperate(lampColorTab_A[0], 0)):
			(devLamp_excuteLightOperate(lampColorDark, 0));
	}
	else
	{
		devLamp_tipsTimer_stopTrig();
	}
 }

 static void devLamp_tips_b_timer_cb(void *arg){

	 if(tipsCounterTimerLoop != 0){
	 
		 tipsCounterTimerLoop --;
		 (tipsCounterTimerLoop % 2)?
			 (devLamp_excuteLightOperate(lampColorTab_A[1], 0)):
			 (devLamp_excuteLightOperate(lampColorDark, 0));
	 }
	 else
	 {
		 devLamp_tipsTimer_stopTrig();
	 }
 }
 
 static void devLamp_tips_c_timer_cb(void *arg){

 	 static uint8_t counterLoacl = 0;

	 if(tipsCounterTimerLoop != 0){

		 tipsCounterTimerLoop --;
		 if(0 == (tipsCounterTimerLoop % 2)){

			if(counterLoacl < 2)counterLoacl ++;
			else counterLoacl = 0;

			devLamp_excuteLightOperate(lampColorTab_A[counterLoacl], 0);
		 } 
	 }
	 else
	 {
		devLamp_tipsTimer_stopTrig();
	 }
 }

 void rgbLamp_staticWhiteDisp_param_get(stt_devRgbLampWhiteLightParam *p){

	memcpy(p, &devRgbLamp_staticWhiteParam, sizeof(stt_devRgbLampWhiteLightParam));
 }

 void rgbLamp_staticWhiteDisp_param_set(stt_devRgbLampWhiteLightParam *p, bool nvsRecord_IF){

	memcpy(&devRgbLamp_staticWhiteParam, p, sizeof(stt_devRgbLampWhiteLightParam));
	if(nvsRecord_IF)devRgbLamp_dispTrickParam_nvsSave(&devRgbLamp_staticWhiteParam, rgbLampNvsObj_pdStaticWhite);	
	devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_WHITE);
 }

 void rgbLamp_staticColorDisp_param_get(stt_devRgbLampColorLightParam *p){

	memcpy(p, &devRgbLamp_staticColorParam, sizeof(stt_devRgbLampColorLightParam));
 }

 void rgbLamp_staticColorDisp_param_set(stt_devRgbLampColorLightParam *p, bool nvsRecord_IF){

	memcpy(&devRgbLamp_staticColorParam, p, sizeof(stt_devRgbLampColorLightParam));
	if(nvsRecord_IF)devRgbLamp_dispTrickParam_nvsSave(&devRgbLamp_staticColorParam, rgbLampNvsObj_pdStaticColor);	
	memcpy(&rgbLampExcuteColorParam, &devRgbLamp_staticColorParam.color, sizeof(stt_devRgbLampColorUnit));
	devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_COLOR);
 }

 void rgbLamp_sceneDisp_param_get(stt_devRgbLampJumpParamTab *p, uint8_t sceneIndex){

	const stt_devRgbLampJumpParamTab devRgbLamp_sceneUnitParam_null = {0};
	stt_devRgbLampJumpParamTab paramTrickParam_unit = DEV_RGBLAMP_TRICK_JMPDISP_UNITPARAM_DEF;
	
	if(!memcmp(&devRgbLamp_sceneUnitParam_null, &devRgbLamp_sceneParam[sceneIndex], sizeof(stt_devRgbLampJumpParamTab))){

		paramTrickParam_unit.pd_sceIndex = sceneIndex;
		memcpy(&devRgbLamp_sceneParam[sceneIndex], &paramTrickParam_unit, sizeof(stt_devRgbLampJumpParamTab)); 
	}
	memcpy(p, &devRgbLamp_sceneParam[sceneIndex], sizeof(stt_devRgbLampJumpParamTab)); 
 }

 void rgbLamp_sceneDisp_param_set(stt_devRgbLampJumpParamTab *p, bool nvsRecord_IF){

	uint8_t loop = 0;

	memcpy(&devRgbLamp_sceneParam[devRgbLamp_opJumpParam.pd_sceIndex], p, sizeof(stt_devRgbLampJumpParamTab)); 
	memcpy(&devRgbLamp_opJumpParam, &devRgbLamp_sceneParam[devRgbLamp_opJumpParam.pd_sceIndex], sizeof(stt_devRgbLampJumpParamTab)); 
	if(nvsRecord_IF)devRgbLamp_dispTrickParam_nvsSave(&devRgbLamp_opJumpParam, rgbLampNvsObj_pdSceneUint);
	devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_SCENE);
 }

 void rgbLamp_musicColorDisp_param_get(stt_devRgbLampMusicDispParam *p){

	memcpy(p, &devRgbLamp_musicColorParam, sizeof(stt_devRgbLampMusicDispParam));
 }

 void rgbLamp_musicColorDisp_param_set(stt_devRgbLampMusicDispParam *p, bool nvsRecord_IF){

	memcpy(&devRgbLamp_musicColorParam, p, sizeof(stt_devRgbLampMusicDispParam));
	if(nvsRecord_IF)devRgbLamp_dispTrickParam_nvsSave(&devRgbLamp_musicColorParam, rgbLampNvsObj_pdMusicDisp);
	devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_MUSIC);
 }

 void rgbLamp_excuteParam_param_get(stt_devRgbLampExcuteParam *p){

	memcpy(p, &devRgbLamp_excuteParam, sizeof(stt_devRgbLampExcuteParam));
 }

 void rgbLamp_excuteParam_param_set(stt_devRgbLampExcuteParam *p, bool nvsRecord_IF){

	memcpy(&devRgbLamp_excuteParam, p, sizeof(stt_devRgbLampExcuteParam));
	if(nvsRecord_IF)devRgbLamp_dispTrickParam_nvsSave(&devRgbLamp_excuteParam, rgbLampNvsObj_pdExcuteParam);
	if(devRgbLamp_excuteParam.pd_sceneExcuteIndex != devRgbLamp_opJumpParam.pd_sceIndex){
		memcpy(&devRgbLamp_opJumpParam, &devRgbLamp_sceneParam[devRgbLamp_excuteParam.pd_sceneExcuteIndex], sizeof(stt_devRgbLampJumpParamTab));
	}
	if(1 == devRgbLamp_excuteParam.pd_rsv){
		devDriverBussiness_rgbLamp_trickOperation(devRgbLamp_excuteParam.pd_excuteMode);
	}
 }

 void devDriverBussiness_rgbLamp_tipsTrig(enumRgbLamp_tipsType tType, uint16_t loopKeep){

	const uint16_t tipsUnitPeriod = 100;

	if(NULL != t_timer_deviceOpreatTips){
		if (!xTimerStop(t_timer_deviceOpreatTips, portMAX_DELAY)) {
			MDF_LOGW("xTimerStop timer: %p", t_timer_deviceOpreatTips);
		}
		if (!xTimerDelete(t_timer_deviceOpreatTips, portMAX_DELAY)) {
			MDF_LOGW("xTimerDelete timer: %p", t_timer_deviceOpreatTips);
		}
		t_timer_deviceOpreatTips = NULL;
	}

	if(NULL != g_timer_lampJumpOpreat){
		if (!xTimerStop(g_timer_lampJumpOpreat, portMAX_DELAY)) {
			MDF_LOGW("xTimerStop timer: %p", g_timer_lampJumpOpreat);
		}
		if (!xTimerDelete(g_timer_lampJumpOpreat, portMAX_DELAY)) {
			MDF_LOGW("xTimerDelete timer: %p", g_timer_lampJumpOpreat);
		}
		g_timer_lampJumpOpreat = NULL;
	}

	if(rgbLampTipsType_normal == tType){

		devTipsRunning_flg = false;
		devLamp_excuteLightOperate(lampColorDark, 0);
		devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);

	}else{

		if(loopKeep == 0)tipsCounterTimerLoop = 2;
		else tipsCounterTimerLoop = 500 / tipsUnitPeriod * loopKeep;

		switch(tType){
			
			case rgbLampTipsType_a:
				devTipsRunning_flg = true;
				devLamp_tips_a_timer_cb(NULL);
				t_timer_deviceOpreatTips = xTimerCreate("tipsLampOpet", tipsUnitPeriod, true, NULL, devLamp_tips_a_timer_cb);
				xTimerStart(t_timer_deviceOpreatTips, tipsUnitPeriod);
				break;
			
			case rgbLampTipsType_b:
				devTipsRunning_flg = true;
				devLamp_tips_b_timer_cb(NULL);
				t_timer_deviceOpreatTips = xTimerCreate("tipsLampOpet", tipsUnitPeriod, true, NULL, devLamp_tips_b_timer_cb);
				xTimerStart(t_timer_deviceOpreatTips, tipsUnitPeriod);
				break;
				
			case rgbLampTipsType_c:
				devTipsRunning_flg = true;
				devLamp_tips_c_timer_cb(NULL);
				t_timer_deviceOpreatTips = xTimerCreate("tipsLampOpet", tipsUnitPeriod, true, NULL, devLamp_tips_c_timer_cb);
				xTimerStart(t_timer_deviceOpreatTips, tipsUnitPeriod);
				break;

			default:break;
		}
	}
 }

 void devDriverBussiness_rgbLamp_trickOperation(uint8_t trickType){

	const uint16_t timerPeriod_redu = 5;
	uint16_t timerPeriod_temp = 0;

	if(NULL != g_timer_lampJumpOpreat){

	    if (!xTimerStop(g_timer_lampJumpOpreat, portMAX_DELAY)) {
	        MDF_LOGW("xTimerStop timer: %p", g_timer_lampJumpOpreat);
	    }
	    if (!xTimerDelete(g_timer_lampJumpOpreat, portMAX_DELAY)) {
	        MDF_LOGW("xTimerDelete timer: %p", g_timer_lampJumpOpreat);
	    }
		g_timer_lampJumpOpreat = NULL;
	}

	((DEV_RGBLAMP_TRICK_TYPE_COLOR == devRgbLamp_excuteParam.pd_excuteMode) ||\
	 (DEV_RGBLAMP_TRICK_TYPE_WHITE == devRgbLamp_excuteParam.pd_excuteMode))?
		(localRmtDynamicDisp_flg = false):
		(localRmtDynamicDisp_flg = true);

	if(true == devTipsRunning_flg)return;
	if(0 == devRgbLamp_excuteParam.pd_rsv)return;
	if(DEV_RGBLAMP_TRICK_TYPE_REFRESH != trickType)devRgbLamp_excuteParam.pd_excuteMode = trickType;
	
	switch(devRgbLamp_excuteParam.pd_excuteMode){

		case DEV_RGBLAMP_TRICK_TYPE_WHITE:{

			stt_devRgbLampColorUnit colorTemp = {0};
			iot_function_tb2hsv(devRgbLamp_staticWhiteParam.c_brightness,
								devRgbLamp_staticWhiteParam.c_temperature,
								&colorTemp.c_r,
								&colorTemp.c_g,
								&colorTemp.c_b);
			devLamp_excuteLightOperate(colorTemp, 200);
		}break;
	
		case DEV_RGBLAMP_TRICK_TYPE_COLOR:
			devLamp_excuteLightOperate(rgbLampExcuteColorParam, 500);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_SCENE:
			devLamp_opJump_timer_cb(NULL);
			g_timer_lampJumpOpreat = xTimerCreate("trLampOper", (devRgbLamp_opJumpParam.pd_freqLevel + 1) * 150 + timerPeriod_redu,
												  true, NULL, devLamp_opJump_timer_cb);
			xTimerStart(g_timer_lampJumpOpreat, 0);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_MUSIC:
			devLamp_excuteLightOperate(devRgbLamp_musicColorParam.color, 100);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_BTCHG:
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_FLASH:
			timerPeriod_temp = 200 * devDynamicDispExParam_speed;
			if(0 == timerPeriod_temp)timerPeriod_temp = 50;
			devLamp_excuteLightOperate(lampColorDark, 0);
			devLamp_trikRmtFlash_timer_cb(NULL);
			g_timer_lampJumpOpreat = xTimerCreate("trLampOper", timerPeriod_temp + timerPeriod_redu,
												  true, NULL, devLamp_trikRmtFlash_timer_cb);
			xTimerStart(g_timer_lampJumpOpreat, 0);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_STROBE:
			timerPeriod_temp = 200 * devDynamicDispExParam_speed;
			if(0 == timerPeriod_temp)timerPeriod_temp = 50;
			devLamp_excuteLightOperate(lampColorDark, 0);
			devLamp_trikRmtStrobe_timer_cb(NULL);
			g_timer_lampJumpOpreat = xTimerCreate("trLampOper", timerPeriod_temp + timerPeriod_redu,
												  true, NULL, devLamp_trikRmtStrobe_timer_cb);
			xTimerStart(g_timer_lampJumpOpreat, 0);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_FADE:
			// timerPeriod_temp = 400 * devDynamicDispExParam_speed;
			timerPeriod_temp = devDynamicDispExParam_speed * devDynamicDispExParam_speed * 100;
			if(0 == timerPeriod_temp)timerPeriod_temp = 200;
			devLamp_excuteLightOperate(lampColorDark, 0);
			devLamp_trikRmtFade_timer_cb(NULL);
			g_timer_lampJumpOpreat = xTimerCreate("trLampOper", timerPeriod_temp + timerPeriod_redu,
												  true, NULL, devLamp_trikRmtFade_timer_cb);
			xTimerStart(g_timer_lampJumpOpreat, 0);
			break;
		
		case DEV_RGBLAMP_TRICK_TYPE_SMOOTH:
			// timerPeriod_temp = 400 * devDynamicDispExParam_speed;
			timerPeriod_temp = devDynamicDispExParam_speed * devDynamicDispExParam_speed * 100;
			if(0 == timerPeriod_temp)timerPeriod_temp = 200;
			devLamp_excuteLightOperate(lampColorDark, 0);
			devLamp_trikRmtSmooth_timer_cb(NULL);
			g_timer_lampJumpOpreat = xTimerCreate("trLampOper", timerPeriod_temp + timerPeriod_redu,
												  true, NULL, devLamp_trikRmtSmooth_timer_cb);
			xTimerStart(g_timer_lampJumpOpreat, 0);
			break;

		default:break;
	}
 }

 void devDriverBussiness_rgbLamp_moudleInit(void){

	light_driver_config_t driver_config = {
		.gpio_red 	   = DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_R,
		.gpio_green	   = DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_G,
		.gpio_blue	   = DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_B,
		.gpio_cold	   = DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_C,
		.gpio_warm	   = DEVDRIVER_RGBLAMP_GPIO_OUTPUT_IO_W,
		.fade_period_ms  = DEVDRIVER_RGBLAMP_FADE_PERIOD_MS,
		.blink_period_ms = DEVDRIVER_RGBLAMP_BLINK_PERIOD_MS,
	};

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType != devTypeDef_rgbLampBelt) &&\
	   (swCurrentDevType != devTypeDef_rgbLampBulb))return;
	if(devDriver_moudleInitialize_Flg)return;

	devRgbLamp_dispTrickParam_paramRecovery(); //数据恢复

	MDF_ERROR_ASSERT(light_driver_init(&driver_config));
	light_driver_set_switch(false);

	switch(swCurrentDevType){

		case devTypeDef_rgbLampBelt:{

			msgQh_devTipsTimerHandle = xQueueCreate(2, sizeof(stt_devRgbLampAssistTkMsg));

			xTaskCreate(rmt_nec_rx_task, "rmt_nec_rx_task", 1024 * 2, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_rmtNec_rx);
			xTaskCreate(rgbLampAssist_task, "rgbLampAssist_task", 1024 * 2, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, tHandle_devAssist);

		}break;

		default:break;
	}

	rgbLampExcuteColorParam.c_r =\
	rgbLampExcuteColorParam.c_g =\
	rgbLampExcuteColorParam.c_b = 0;
	devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
	devDriverBussiness_rgbLamp_tipsTrig(rgbLampTipsType_c, 2);
	vTaskDelay(2000 / portTICK_RATE_MS); //必须当即完成
 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

	
 #else
 
//	light_driver_breath_start(100, 0, 80);
//	light_driver_set_switch(1);
//	light_driver_set_saturation(2);
//	devDriverBussiness_rgbLamp_trickOperation(3);
 #endif

	esp_log_level_set(TAG, ESP_LOG_INFO);

	devDriver_moudleInitialize_Flg = true;
 }

 void devDriverBussiness_rgbLamp_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	if(NULL != tHandle_rmtNec_rx){

		vTaskDelete(tHandle_rmtNec_rx);

		tHandle_rmtNec_rx = NULL;
	}

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
 #else

	MDF_ERROR_ASSERT(light_driver_deinit());
 #endif

	devDriver_moudleInitialize_Flg = false;
 }

 void devDriverBussiness_rgbLamp_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType == devTypeDef_rgbLampBelt) ||\
	   (swCurrentDevType == devTypeDef_rgbLampBulb)){

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

		
 #else

		if(1 == param->devType_rgbLamp.devRgbLamp_enable){

			devRgbLamp_excuteParam.pd_rsv = 1;
			memcpy(&rgbLampExcuteColorParam, &devRgbLamp_staticColorParam.color, sizeof(stt_devRgbLampColorUnit));
			if(!memcmp(&lampColorDark, &rgbLampExcuteColorParam, sizeof(stt_devRgbLampColorUnit))){
				rgbLampExcuteColorParam.c_r =\
				rgbLampExcuteColorParam.c_g =\
				rgbLampExcuteColorParam.c_b = 255;
				memcpy(&devRgbLamp_staticColorParam.color, &rgbLampExcuteColorParam, sizeof(stt_devRgbLampColorUnit));
			}devDriverBussiness_rgbLamp_trickOperation(devRgbLamp_excuteParam.pd_excuteMode);
		}else{

			devRgbLamp_excuteParam.pd_rsv = 0;
			devDriverBussiness_rgbLamp_trickOperation(DEV_RGBLAMP_TRICK_TYPE_REFRESH);
			devLamp_excuteLightOperate(lampColorDark, 200);
		}
 #endif
	}
 }
#endif






















