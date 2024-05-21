//----------------------------------------------------------------------------------------------------------------------------------

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define UIC_BUTTON_RUN_STOP           1
#define UIC_BUTTON_AUTO               2
#define UIC_BUTTON_MENU               3
#define UIC_BUTTON_S_PIC              4
#define UIC_BUTTON_S_WAV              5
#define UIC_BUTTON_H_CUR              6
#define UIC_BUTTON_V_CUR              7
#define UIC_BUTTON_NAV_RIGHT          8
#define UIC_BUTTON_NAV_UP             9
#define UIC_BUTTON_NAV_OK            10
#define UIC_BUTTON_NAV_DOWN          11
#define UIC_BUTTON_NAV_LEFT          12
#define UIC_BUTTON_MOVE_SPEED        13
#define UIC_BUTTON_CH1_ENABLE        14
#define UIC_BUTTON_CH1_CONF          15
#define UIC_BUTTON_CH2_ENABLE        16
#define UIC_BUTTON_CH2_CONF          17
#define UIC_BUTTON_TRIG_ORIG         18
#define UIC_BUTTON_TRIG_MODE         19
#define UIC_BUTTON_TRIG_EDGE         20
#define UIC_BUTTON_TRIG_CHX          21
#define UIC_BUTTON_TRIG_50_PERCENT   22
#define UIC_BUTTON_F1                23
#define UIC_BUTTON_F2                24
#define UIC_BUTTON_F3                25
#define UIC_BUTTON_F4                26
#define UIC_BUTTON_F5                27
#define UIC_BUTTON_F6                28
#define UIC_BUTTON_GEN               29
#define UIC_BUTTON_NEXT              30
#define UIC_BUTTON_LAST              31
#define UIC_BUTTON_DEL               32
#define UIC_BUTTON_SEE_ALL           33
#define UIC_BUTTON_SEL               34

#define UIC_ROTARY_SEL_ADD           35
#define UIC_ROTARY_SEL_SUB           36
#define UIC_ROTARY_CH1_POS_SUB       37
#define UIC_ROTARY_CH1_POS_ADD       38
#define UIC_ROTARY_CH2_POS_SUB       39
#define UIC_ROTARY_CH2_POS_ADD       40
#define UIC_ROTARY_TRIG_POS_SUB      41
#define UIC_ROTARY_TRIG_POS_ADD      42
#define UIC_ROTARY_TRIG_LEVEL_ADD    43
#define UIC_ROTARY_TRIG_LEVEL_SUB    44
#define UIC_ROTARY_SCALE_CH1_ADD     45
#define UIC_ROTARY_SCALE_CH1_SUB     46
#define UIC_ROTARY_SCALE_CH2_ADD     47
#define UIC_ROTARY_SCALE_CH2_SUB     48
#define UIC_ROTARY_TIME_SUB          49
#define UIC_ROTARY_TIME_ADD          50

#define UIC_BUTTON_OFF              200

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_user_input(void);

//----------------------------------------------------------------------------------------------------------------------------------


void sm_set_trigger_position(void);
void sm_set_trigger_level(void);
void sm_set_time_base(void);

void sm_set_channel_sensitivity(PCHANNELSETTINGS settings);
void sm_set_channel_position(PCHANNELSETTINGS settings);

void sm_do_50_percent_trigger_setup(void);

void sm_move_left_time_cursor_position(void);
void sm_move_right_time_cursor_position(void);

void sm_select_left_time_cursor(void);
void sm_select_right_time_cursor(void);

void sm_move_top_volt_cursor_position(void);
void sm_move_bottom_volt_cursor_position(void);

void sm_select_top_volt_cursor(void);
void sm_select_bottom_volt_cursor(void);

void sm_close_menu(void);
void sm_select_main_menu_item(void);

void sm_start_usb_export(void);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* STATEMACHINE_H */

