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
#define UIC_BUTTON_SAVE_PICTURE       4
#define UIC_BUTTON_SAVE_WAVE          5
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
#define UIC_BUTTON_PREVIOUS          31     //Marked LAST on the scope, but the action is previous
#define UIC_BUTTON_DELETE            32
#define UIC_BUTTON_SELECT_ALL        33
#define UIC_BUTTON_SELECT            34

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

#define SAMPLING_NOT_ENABLED          0
#define SAMPLING_ENABLED              1

#define TRACE_DISPLAY_NOT_ENABLED     0
#define TRACE_DISPLAY_ENABLED         1

//----------------------------------------------------------------------------------------------------------------------------------

#define MAIN_MENU_SCREEN_BRIGHTNESS   4
#define MAIN_MENU_GRID_BRIGHTNESS     5

#define MAIN_MENU_50_PERCENT          6
#define MAIN_MENU_XY_MODE             7

//----------------------------------------------------------------------------------------------------------------------------------

#define MOVE_SPEED_FAST              10
#define MOVE_SPEED_SLOW               1

//----------------------------------------------------------------------------------------------------------------------------------
//Navigation states
//----------------------------------------------------------------------------------------------------------------------------------

enum NavigationStates
{
  NAV_NO_ACTION = 0,
  NAV_TIME_VOLT_CURSOR_HANDLING,
  NAV_MAIN_MENU_HANDLING,
  NAV_FILE_VIEW_HANDLING,
  NAV_FILE_VIEW_SELECT_HANDLING,
  NAV_ITEM_VIEW_HANDLING,
  NAV_SLIDER_HANDLING,
  NAV_ON_OFF_HANDLING,
  NAV_MEASUREMENTS_MENU_HANDLING,
  NAV_CHANNEL_MENU_HANDLING,
};

//----------------------------------------------------------------------------------------------------------------------------------
//File viewing handling states
//----------------------------------------------------------------------------------------------------------------------------------

enum FileViewHandlingStates
{
  FILE_VIEW_NO_ACTION,
  FILE_VIEW_DEFAULT_CONTROL,
  FILE_VIEW_SELECT_CONTROL,
  FILE_VIEW_ITEM_CONTROL,
  FILE_VIEW_MENU_CONTROL,
};

//----------------------------------------------------------------------------------------------------------------------------------
//Button and rotary dial handling states
//----------------------------------------------------------------------------------------------------------------------------------

enum ButtonDialHandlingStates
{
  BUTTON_DIAL_NO_ACTION,
  BUTTON_DIAL_NORMAL_HANDLING,
  BUTTON_DIAL_MENU_HANDLING,
  BUTTON_DIAL_FILE_VIEW_HANDLING,
  BUTTON_DIAL_PICTURE_VIEW_HANDLING,
  BUTTON_DIAL_WAVE_VIEW_HANDLING,
  BUTTON_DIAL_MEASUREMENTS_MENU_HANDLING,
  BUTTON_DIAL_CHANNEL_MENU_HANDLING,
};

//----------------------------------------------------------------------------------------------------------------------------------

void sm_init(void);

void sm_handle_user_input(void);

//----------------------------------------------------------------------------------------------------------------------------------
//Navigation handling functions
//
//Act on user input on the navigation buttons (left, right, up, down, ok) and the selection rotary dial
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_time_volt_cursor(void);
void sm_handle_main_menu_actions(void);
void sm_handle_file_view_actions(void);
void sm_handle_file_view_select_actions(void);
void sm_handle_item_view_actions(void);
void sm_handle_slider_actions(void);
void sm_handle_on_off_actions(void);
void sm_handle_measurements_menu_actions(void);
void sm_handle_channel_menu_actions(void);

//----------------------------------------------------------------------------------------------------------------------------------
//File view handling functions
//
//Act on user input on the 5 white buttons below the display
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_file_view_control(void);
void sm_handle_file_view_select_control(void);
void sm_handle_item_view_control(void);

//----------------------------------------------------------------------------------------------------------------------------------
//Button and rotary dial handling functions
//
//Act on user input to the rest of the buttons and rotary dials
//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_normal_handling(void);
void sm_button_dial_file_view_handling(void);
void sm_button_dial_picture_view_handling(void);
void sm_button_dial_wave_view_handling(void);
void sm_button_dial_measurements_menu_handling(void);
void sm_button_dial_channel_menu_handling(void);

//----------------------------------------------------------------------------------------------------------------------------------
//Functions to handle specific tasks
//----------------------------------------------------------------------------------------------------------------------------------

void sm_close_menu(void);
void sm_restore_navigation_handling(void);

void sm_toggle_time_cursor(void);
void sm_toggle_volt_cursor(void);

void sm_switch_move_speed(void);

void sm_set_trigger_position(void);
void sm_set_trigger_level(void);

void sm_set_time_base(void);

void sm_toggle_channel_enable(PCHANNELSETTINGS settings);

void sm_set_channel_sensitivity(PCHANNELSETTINGS settings);
void sm_set_channel_position(PCHANNELSETTINGS settings);

void sm_do_50_percent_trigger_setup(void);

void sm_select_main_menu_item(void);

void sm_open_file_view(void);
void sm_close_view_screen(void);

void sm_open_file_view_item(void);

void sm_file_view_goto_next_item(void);
void sm_file_view_goto_previous_item(void);
void sm_file_view_goto_next_row(void);
void sm_file_view_goto_previous_row(void);

void sm_file_view_delete_current(void);

void sm_file_view_goto_next_item_on_page(void);
void sm_file_view_goto_previous_item_on_page(void);
void sm_file_view_goto_next_row_on_page(void);
void sm_file_view_goto_previous_row_on_page(void);

void sm_file_view_process_select(uint32 selectall);

void sm_file_view_delete_selected(void);

void sm_item_view_delete_current(void);

void sm_item_view_goto_next_item(void);
void sm_item_view_goto_previous_item(void);

void sm_slider_close(void);
void sm_slider_adjust(void);

void sm_on_off_close(void);
void sm_on_off_check(void);
void sm_on_off_select(void);

void sm_open_measurements_menu(uint32 slot);

void sm_open_channel_menu(PCHANNELSETTINGS settings);
void sm_select_channel_option(void);

//----------------------------------------------------------------------------------------------------------------------------------
//Next functions are for executing main menu items
//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_picture_file_viewing(void);
void sm_open_waveform_file_viewing(void);

void sm_open_brightness_setting(void);

void sm_open_on_off_setting(void);

void sm_do_base_calibration(void);

void sm_start_usb_export(void);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* STATEMACHINE_H */

//----------------------------------------------------------------------------------------------------------------------------------
