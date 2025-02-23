// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* SDK Includes */
#include "sdkconfig.h"
#include "esp_log.h"

#ifdef CONFIG_LVGL_GUI_ENABLE

/*C Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// /* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

/* I2C Includes */
#include "driver/i2c.h"
#include "iot_ft5x06.h"
#include "iot_i2c_bus.h"

/* lvgl includes */
#include "iot_lvgl.h"

/* lvgl calibration includes */
#include "lv_calibration.h"

/* FT5x06 Include */
#include "FT5x06.h"

/* usrApp Include */
#include "devDriver_manage.h"

extern EventGroupHandle_t xEventGp_screenTouch;
extern stt_touchEveInfo devTouchGetInfo;

static ft5x06_handle_t dev = NULL;

static void write_reg(uint8_t reg, uint8_t val)
{
    iot_ft5x06_write(dev, reg, 1, &val);
}

static uint8_t read_byte(uint8_t reg)
{
    uint8_t data;
    iot_ft5x06_read(dev, reg, 1, &data);
    return data;
}

static uint16_t read_word(uint8_t reg)
{
    uint8_t data[2];
    uint16_t result;
    iot_ft5x06_read(dev, reg, 2, &data);
    result = data[0] << 8 | data[1];
    return result;
}

/*Function pointer to read data. Return 'true' if there is still data to be read (buffered)*/
static bool ex_tp_read(lv_indev_data_t *data)
{
	stt_devStatusRecord devStatusParam_temp = {0};
	uint8_t touchPointNum = 0;
	uint8_t touch_gestrue = 0;
    static lv_coord_t x = 0xFFFF, y = 0xFFFF;
	static lv_point_t pointRecord[3] = {0}; //-参数对应业务保留，逻辑触发失能
	static bool touchReales_flg = true;
	const  uint16_t touchAvaliableCdPeriod = 6;
	static uint16_t touchAvaliableCdCounter = 0;

	enum_screenBkLight_status bkLightStatus_temp = devScreenBkLight_brightnessGet();
	
    data->state = LV_INDEV_STATE_REL;
    // please be sure that your touch driver every time return old (last clcked) value. 
    data->point.x = x;
    data->point.y = y;

	touchPointNum = read_byte(FT5x06_TOUCH_POINTS);
	// touch_gestrue = read_byte(FT5x06_GESTURE_ID); //手势寄存器暂时无效，应该是硬件不支持

	// if(touch_gestrue){
	// 	devTouchGetInfo.valGestrue = touch_gestrue;
	// 	if(xEventGp_screenTouch)xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
	// }

	// Only take a reading if we are touched.
	if (touchPointNum & 0x07){

		devScreenBkLight_weakUp();		
		if(bkLightStatus_temp != screenBkLight_statusEmpty){

			if(touchAvaliableCdCounter)touchAvaliableCdCounter --;
			else{

				devTouchGetInfo.touchNum = touchPointNum;
				for(uint8_t loop = 0; loop < touchPointNum; loop ++){
		
					devTouchGetInfo.touchPosition[loop].x = (lv_coord_t)(read_word(FT5x06_TOUCH1_XH + 6 * loop) & 0x0fff);
					devTouchGetInfo.touchPosition[loop].y = (lv_coord_t)read_word(FT5x06_TOUCH1_YH + 6 * loop);
				}
				if(xEventGp_screenTouch)xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG);
				
				/* Get the X, Y, values */
				data->point.x = (lv_coord_t)(read_word(FT5x06_TOUCH1_XH) & 0x0fff);
				data->point.y = (lv_coord_t)read_word(FT5x06_TOUCH1_YH);
				data->state = LV_INDEV_STATE_PR;
		
				// Apply calibration, rotation
				// Transform the co-ordinates
				if (lvgl_calibration_transform(&(data->point))) {
					lv_coord_t t;
					// Rescale X,Y if we are using self-calibration
	#ifdef CONFIG_LVGL_DISP_ROTATE_0

						devStatusRecordIF_paramGet(&devStatusParam_temp);
						if(devStatusParam_temp.devScreenLandscape_IF){ //横屏？
						
							t = data->point.y;
							data->point.y = 240 - 1 - data->point.x; //为了不修改所有文件的宏，强制将尺寸改为320 * 320，触摸修正手改
							data->point.x = t;
						}
						else
						{
							data->point.x = 240 - 1 - data->point.x; //为了不修改所有文件的宏，强制将尺寸改为320 * 320，触摸修正手改
							data->point.y = L8_DEV_SCREEN_SIZE_VER - 1 - data->point.y;
						}
	#elif defined(CONFIG_LVGL_DISP_ROTATE_90)
						t = data->point.x;
						data->point.x = L8_DEV_SCREEN_SIZE_HOR - 1 - data->point.y;
						data->point.y = t;
	#elif defined(CONFIG_LVGL_DISP_ROTATE_180)
						data->point.x = L8_DEV_SCREEN_SIZE_HOR - 1 - data->point.x;
						data->point.y = L8_DEV_SCREEN_SIZE_VER - 1 - data->point.y;
	#elif defined(CONFIG_LVGL_DISP_ROTATE_270)
						t = data->point.y;
						data->point.y = L8_DEV_SCREEN_SIZE_VER - 1 - data->point.x;
						data->point.x = t;
	#endif
					x = data->point.x;
					y = data->point.y;
				}
				else //未进行校正业务
				{
//					x = data->point.x;
//					y = data->point.y;
//			
					data->point.x = L8_DEV_SCREEN_SIZE_HOR - 1 - data->point.x;
					data->point.y = L8_DEV_SCREEN_SIZE_VER - 1 - data->point.y;
				}
			}

		}else{

				touchAvaliableCdCounter = touchAvaliableCdPeriod;
		}
	}
	else{

		touchReales_flg = true;
		memset(pointRecord, 0, sizeof(lv_point_t) * 3);
	}
		
    return false;
}

/* Input device interface */
/* Initialize your touchpad */
lv_indev_drv_t lvgl_indev_init()
{
    i2c_bus_handle_t i2c_bus = NULL;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_LVGL_TOUCH_SDA_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = CONFIG_LVGL_TOUCH_SCL_GPIO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 10000,
    };
    i2c_bus = iot_i2c_bus_create(CONFIG_LVGL_TOUCH_IIC_NUM, &conf);
    dev = iot_ft5x06_create(i2c_bus, FT5X06_ADDR_DEF);

    // Init default values. (From NHD-3.5-320240MF-ATXL-CTP-1 datasheet)
    // Valid touching detect threshold --def: 70
    write_reg(FT5x06_ID_G_THGROUP, 0x08);

    // valid touching peak detect threshold --def: 60
    write_reg(FT5x06_ID_G_THPEAK, 15);

    // Touch focus threshold --def: 16
    write_reg(FT5x06_ID_G_THCAL, 0xE9);

    // threshold when there is surface water --def: 60
    write_reg(FT5x06_ID_G_THWATER, 25);

    // threshold of temperature compensation --def: 10
    write_reg(FT5x06_ID_G_THTEMP, 20);

    // Touch difference threshold --def: 20
    write_reg(FT5x06_ID_G_THDIFF, 0x20);

    // Delay to enter 'Monitor' status (s) --def: 02
    write_reg(FT5x06_ID_G_TIME_ENTER_MONITOR, 0x02);

    // Period of 'Active' status (ms) --def: 12
    write_reg(FT5x06_ID_G_PERIODACTIVE, 0x03);

    // Timer to enter 'idle' when in 'Monitor' (ms) --def: 40
    write_reg(FT5x06_ID_G_PERIODMONITOR, 0x28);

    lv_indev_drv_t indev_drv;      /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
    indev_drv.read = ex_tp_read;            /*Library ready your touchpad via this function*/

    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
    return indev_drv;
}

#endif /* CONFIG_LVGL_GUI_ENABLE */
