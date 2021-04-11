/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "sdkconfig.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu_timerSet.h"

#include "gui_businessMenu_timer.h"
#include "gui_businessHome.h"

void lvGui_businessMenu_timer(lv_obj_t * obj_Parent){

	lvGui_businessMenu_timerSet(obj_Parent);
}


