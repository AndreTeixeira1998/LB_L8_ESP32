/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "sdkconfig.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu_delayer.h"
#include "gui_businessHome.h"

#include "gui_businessMenu_delayerSet.h"

void lvGui_businessMenu_delayer(lv_obj_t * obj_Parent){

	lvGui_businessMenu_delayerSet(obj_Parent);
}


