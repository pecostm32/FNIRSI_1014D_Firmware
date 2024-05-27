//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "scope_functions.h"
#include "statemachine.h"
#include "timer.h"
#include "fpga_control.h"
#include "spi_control.h"
#include "sd_card_interface.h"
#include "display_lib.h"
#include "ff.h"
#include "user_interface_functions.h"
#include "usb_interface.h"
#include "variables.h"

#include "sin_cos_math.h"

#include <string.h>

#if 0
//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_usb_screen(void)
{
  //Clear the whole screen
  display_set_fg_color(0x00000000);
  display_fill_rect(0, 0, 800, 480);

  //Set the light color for the equipment borders
  display_set_fg_color(0x00AAAAAA);

  //Draw the computer screen
  display_fill_rounded_rect(470, 115, 250, 190, 2);
  display_fill_rect(580, 305, 30, 20);
  display_fill_rounded_rect(550, 325, 90, 10, 2);
  display_fill_rect(550, 331, 89, 4);

  //Draw the scope
  display_fill_rounded_rect(80, 200, 180, 135, 2);

  //Draw the cable
  display_fill_rect(210, 188, 10, 12);
  display_fill_rect(213, 154, 4, 36);
  display_fill_rect(213, 150, 152, 4);
  display_fill_rect(361, 154, 4, 106);
  display_fill_rect(361, 260, 98, 4);
  display_fill_rect(458, 257, 12, 10);

  //Fill in the screens with a blue color
  display_set_fg_color(0x00000055);
  display_fill_rect(477, 125, 235, 163);
  display_fill_rect(88, 210, 163, 112);

  //Draw a dark border around the blue screens
  display_set_fg_color(0x00111111);
  display_draw_rect(477, 125, 235, 163);
  display_draw_rect(88, 210, 163, 112);

  //Display the text with font 5 and the light color
  display_set_font(&font_3);
  display_set_fg_color(0x00AAAAAA);
  display_text(125, 254, "ON / OFF");

  //Start the USB interface
  usb_device_enable();

  //Wait for the user to touch the scope ON / OFF section to quit
  while(1)
  {
    //Scan the touch panel for touch
//    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the text field
      if((xtouch >= 90) && (xtouch <= 250) && (ytouch >= 210) && (ytouch <= 320))
      {
        //Touched the text field so wait until touch is released and then quit
//        tp_i2c_wait_for_touch_release();

        break;
      }
    }
  }

  //Stop the USB interface
  usb_device_disable();

  //Re-sync the system files
  scope_sync_thumbnail_files();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_right_control_menu(void)
{
  //Setup for clearing right menu bar
  display_set_fg_color(0x00000000);

  //Clear the right bar for the control buttons
  display_fill_rect(730, 0, 70, 480);

  //Display the control button
  scope_control_button(0);

  //Check in which state the right menu is in
  if(scopesettings.rightmenustate == 0)
  {
    //Main control state so draw the always used buttons
    scope_t_cursor_button(0);
    scope_v_cursor_button(0);
    scope_measures_button(0);
    scope_save_picture_button(0);

    //Check if in wave view mode
    if(scopesettings.waveviewmode == 0)
    {
      //Main control mode buttons
      scope_run_stop_button(0);
      scope_auto_set_button(0);
      scope_save_wave_button(0);
    }
    else
    {
      //Wave view mode buttons
      scope_previous_wave_button(0);
      scope_next_wave_button(0);
      scope_delete_wave_button(0);
    }
  }
  else
  {
    //Channel sensitivity state
    scope_channel_sensitivity_control(&scopesettings.channel1, 0, 0);
    scope_channel_sensitivity_control(&scopesettings.channel2, 0, 0);

    //Check if in wave view mode
    if(scopesettings.waveviewmode == 0)
    {
      //Main control mode
      scope_50_percent_trigger_button(0);
    }
    else
    {
      //Wave view mode
      scope_show_grid_button(0);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_right_file_menu(void)
{
  //Set black color for background
  display_set_fg_color(0x00000000);

  //Clear the right menu bar
  display_fill_rect(730, 0, 70, 480);

  //Add the buttons
  scope_return_button(0);
  scope_select_all_button(0);
  scope_select_button(0);
  scope_delete_button(0);
  scope_page_up_button(0);
  scope_page_down_button(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_bottom_file_menu(int mode)
{
  //Check if background needs to be saved
  if(mode == VIEW_BOTTON_MENU_INIT)
  {
    //Save the screen rectangle where the menu will be displayed
    display_copy_rect_from_screen(0, 390, 800, 90);
  }

  //Check if it needs to be drawn
  if(mode & VIEW_BOTTON_MENU_SHOW)
  {
    //Draw the background in grey
    display_set_fg_color(0x00202020);
    display_fill_rect(0, 390, 800, 90);

    //Draw the filename in white
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_2);
    display_text(20, 392, viewfilename);
    
    //Setup the buttons
    scope_bmp_return_button(0);    
    scope_bmp_delete_button(0);
    scope_bmp_previous_button(0);
    scope_bmp_next_button(0);
    
    //Signal menu is visible
    viewbottommenustate = VIEW_BOTTON_MENU_SHOW | VIEW_BOTTOM_MENU_ACTIVE;
  }
  else
  {
    //Hide the menu bar
    display_copy_rect_to_screen(0, 390, 800, 90);

    //Signal menu is not visible
    viewbottommenustate = VIEW_BOTTON_MENU_HIDE | VIEW_BOTTOM_MENU_ACTIVE;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Right side bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_control_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Grey color for inactive button
    display_set_fg_color(0x00404040);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 5, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(739, 5, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(748, 22, "CTRL");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_run_stop_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 65, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 65, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 75, "RUN/");
  display_text(746, 90, "STOP");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_auto_set_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 125, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 125, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 135, "AUTO");
  display_text(753, 150, "SET");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_previous_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 65, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 65, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(749, 75, "prev");
  display_text(748, 90, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_next_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 125, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 125, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(749, 135, "next");
  display_text(748, 150, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_t_cursor_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 185, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 185, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 195, "T");
  display_text(765, 195, "CU");
  display_text(746, 210, "RSOR");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_v_cursor_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 245, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 245, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 255, "V");
  display_text(765, 255, "CU");
  display_text(746, 270, "RSOR");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_measures_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 305, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 305, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 315, "MEAS");
  display_text(746, 330, "URES");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_picture_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 365, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 365, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 375, "SAVE");
  display_text(753, 390, "PIC");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 435, "SAVE");
  display_text(746, 450, "WAVE");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_delete_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(745, 436, "delete");
  display_text(748, 449, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_50_percent_trigger_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(752, 436, "50%");
  display_text(749, 449, "TRIG");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_grid_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(748, 436, "show");
  display_text(752, 449, "grid");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_sensitivity_control(PCHANNELSETTINGS settings, int type, int mode)
{
  uint32 y;

  //Check if V+ is active or inactive
  if((type == 0) && (mode != 0))
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }

  //Top button
  display_fill_rounded_rect(739, settings->voltdivypos, 51, 50, 2);
  
  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, settings->voltdivypos, 51, 50, 2);

  //Check if V- is active or inactive
  if((type != 0) && (mode != 0))
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }

  //Y position o f bottom button
  y = settings->voltdivypos + 86;
  
  //Bottom button
  display_fill_rounded_rect(739, y, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, y, 51, 50, 2);
  
  //Display V+ and V- the text in larger font
  display_set_font(&font_0);
  
  //Check if V+ is active or inactive
  if((type == 0) && (mode != 0))
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }
  else
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Top button text
  display_text(757, settings->voltdivypos + 18, "V+");

  //Check if V- is active or inactive
  if((type != 0) && (mode != 0))
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }
  else
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Bottom button text
  display_text(757, settings->voltdivypos + 104, "V-");

  //Display the channel identifier bar with the channel color
  display_set_fg_color(settings->color);
  display_fill_rect(739, settings->voltdivypos + 56, 51, 24);

  //Display the channel identifier in black
  display_set_fg_color(0x00000000);
  display_set_font(&font_2);
  display_text(754, settings->voltdivypos + 61, settings->buttontext);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_return_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 14, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 14, 63, 58, 2);
  
  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(747, 34, "return");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_select_all_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else if(mode == 1)
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //White for enabled button
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 93, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 93, 63, 58, 2);
  
  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 106, "select");
  display_text(758, 120, "all");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_select_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else if(mode == 1)
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //White for enabled button
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 173, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 173, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 193, "select");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_delete_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 253, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 253, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 273, "delete");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_page_up_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 333, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 333, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(750, 345, "page");
  display_text(758, 360, "up");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_page_down_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 413, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 413, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(750, 425, "page");
  display_text(748, 442, "down");
}

//----------------------------------------------------------------------------------------------------------------------------------
// Bitmap control bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_return_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(40, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(40, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(return_arrow_icon, 79, 436, 41, 27);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_delete_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(240, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(240, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(waste_bin_icon, 284, 433, 31, 33);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_previous_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(440, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(440, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(previous_picture_icon, 483, 438, 33, 24);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_next_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(640, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(640, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(next_picture_icon, 683, 438, 33, 24);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Top bar function
//----------------------------------------------------------------------------------------------------------------------------------

void scope_menu_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark blue background
    display_set_fg_color(0x00000078);
  }
  else
  {
    //Active so pale yellow background
    display_set_fg_color(0x00FFFF80);
  }

  //Draw the background
  display_fill_rect(0, 0, 80, 38);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black foreground
    display_set_fg_color(0x00000000);
  }

  //Draw the menu symbol
  display_fill_rect(6,  11, 7, 7);
  display_fill_rect(15, 11, 7, 7);
  display_fill_rect(6,  20, 7, 7);
  display_fill_rect(15, 20, 7, 7);

  //Display the text
  display_set_font(&font_3);
  display_text(32, 11, "MENU");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_return_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the background
  display_fill_rect(0, 0, 80, 38);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00000000);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFFFF);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(return_arrow_icon, 20, 5, 41, 27);
}
#endif
//----------------------------------------------------------------------------------------------------------------------------------

void scope_run_stop_text(void)
{
  //Check if run or stop mode
  if(scopesettings.runstate == 0)
  {
    //Run mode. Black box
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Stop mode. Red box
    display_set_fg_color(0x00FF0000);
  }

  //Fill the box
  display_fill_rect(RUN_STOP_TEXT_XPOS, RUN_STOP_TEXT_YPOS, RUN_STOP_TEXT_WIDTH, RUN_STOP_TEXT_HEIGHT);

  //Select the font for the text
  display_set_font(&font_3);

  //Check if run or stop mode
  if(scopesettings.runstate == 0)
  {
    //Run mode. White text
    display_set_fg_color(0x00FFFFFF);
    display_text(RUN_STOP_TEXT_XPOS + 4, RUN_STOP_TEXT_YPOS + 1, "RUN");
  }
  else
  {
    //Stop mode. Black text
    display_set_fg_color(0x00000000);
    display_text(RUN_STOP_TEXT_XPOS + 1, RUN_STOP_TEXT_YPOS + 1, "STOP");
  }
}
#if 0
//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_settings(PCHANNELSETTINGS settings, int mode)
{
  int8 **vdtext;

  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(settings->buttonxpos, CH_BUTTON_YPOS, CH_BUTTON_BG_WIDTH, CH_BUTTON_BG_HEIGHT);

  //Check if channel is enabled or disabled
  if(settings->enable == 0)
  {
    //Disabled so off colors
    //Check if inactive or active
    if(mode == 0)
    {
      //Inactive, grey menu button
      display_set_fg_color(0x00444444);
    }
    else
    {
      //Active, light grey menu button
      display_set_fg_color(0x00BBBBBB);
    }
  }
  else
  {
    //Enabled so on colors
    //Check if inactive or active
    if(mode == 0)
    {
      //Inactive, channel 1 color menu button
      display_set_fg_color(settings->color);
    }
    else
    {
      //Active, blue menu button
      display_set_fg_color(settings->touchedcolor);
    }
  }

  //Fill the button
  display_fill_rect(settings->buttonxpos, CH_BUTTON_YPOS, CH_BUTTON_WIDTH, CH_BUTTON_HEIGHT);

  //Select the font for the text
  display_set_font(&font_2);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(settings->buttonxpos + 30, CH_BUTTON_YPOS, CH_BUTTON_BG_WIDTH - 30, CH_BUTTON_BG_HEIGHT);
  }

  //Display the channel identifier text
  display_text(settings->buttonxpos + 5, CH_BUTTON_YPOS + 11, settings->buttontext);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Check on which coupling is set
  if(settings->coupling == 0)
  {
    //DC coupling
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 3, "DC");
  }
  else
  {
    //AC coupling
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 3, "AC");
  }

  //Print the probe magnification factor
  switch(settings->magnification)
  {
    case 0:
      //Times 1 magnification
      display_text(settings->buttonxpos + 63, CH_BUTTON_YPOS + 3, "1X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[0];
      break;

    case 1:
      //Times 10 magnification
      display_text(settings->buttonxpos + 61, CH_BUTTON_YPOS + 3, "10X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[1];
      break;

    default:
      //Times 100 magnification
      display_text(settings->buttonxpos + 59, CH_BUTTON_YPOS + 3, "100X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[2];
      break;
  }

  //Display the sensitivity when in range
  if(settings->displayvoltperdiv < 7)
  {
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 19, vdtext[settings->displayvoltperdiv]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acqusition_settings(int mode)
{
  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(ACQ_BUTTON_XPOS, ACQ_BUTTON_YPOS, ACQ_BUTTON_BG_WIDTH, ACQ_BUTTON_BG_HEIGHT);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, green menu button
    display_set_fg_color(TRIGGER_COLOR);
  }
  else
  {
    //Active, magenta menu button
    display_set_fg_color(0x00FF00FF);
  }

  //Fill the button
  display_fill_rect(ACQ_BUTTON_XPOS, ACQ_BUTTON_YPOS, ACQ_BUTTON_WIDTH, ACQ_BUTTON_HEIGHT);

  //Select the font for the text
  display_set_font(&font_2);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(ACQ_BUTTON_XPOS + 30, ACQ_BUTTON_YPOS, ACQ_BUTTON_BG_WIDTH - 30, ACQ_BUTTON_BG_HEIGHT);
  }

  //Display the acquisition identifier text
  display_text(ACQ_BUTTON_XPOS + 4, ACQ_BUTTON_YPOS + 11, "ACQ");

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Only display the text when in range of the text array
  if(scopesettings.samplerate < (sizeof(acquisition_speed_texts) / sizeof(int8 *)))
  {
    //Display the text from the table
    display_text(ACQ_BUTTON_XPOS + 38, ACQ_BUTTON_YPOS + 3, (int8 *)acquisition_speed_texts[scopesettings.samplerate]);
  }

  //Only display the text when in range of the text array
  if(scopesettings.timeperdiv < (sizeof(time_div_texts) / sizeof(int8 *)))
  {
    //Display the text from the table
    display_text(ACQ_BUTTON_XPOS + 38, ACQ_BUTTON_YPOS + 19, (int8 *)time_div_texts[scopesettings.timeperdiv]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_move_speed(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark blue background
    display_set_fg_color(0x00000078);
  }
  else
  {
    //Active so pale yellow background
    display_set_fg_color(0x00FFFF80);
  }

  //Draw the background
  display_fill_rect(493, 5, 40, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black text
    display_set_fg_color(0x00000000);
  }

  //Select the font for the text
  display_set_font(&font_3);

  //Display the common text
  display_text(496, 6, "move");

  //Check on which speed is set
  if(scopesettings.movespeed == 0)
  {
    display_text(501, 21, "fast");
  }
  else
  {
    display_text(499, 21, "slow");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_settings(int mode)
{
  int8 *modetext = 0;

  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(570, 5, 110, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, green menu button
    display_set_fg_color(TRIGGER_COLOR);
  }
  else
  {
    //Active, magenta menu button
    display_set_fg_color(0x00FF00FF);
  }

  //Fill the button
  display_fill_rect(570, 5, 31, 35);

  //Select the font for the text
  display_set_font(&font_4);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(601, 5, 68, 35);
  }

  //Display the channel identifier text
  display_text(582, 15, "T");

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Check on which trigger mode is set
  switch(scopesettings.triggermode)
  {
    case 0:
      modetext = "Auto";
      break;

    case 1:
      modetext = "Single";
      break;

    case 2:
      modetext = "Normal";
      break;
  }

  //Select the font for the texts
  display_set_font(&font_2);

  //Check if valid setting
  if(modetext)
  {
    //Display the selected text if so
    display_text(606, 7, modetext);
  }

  //Draw the trigger edge symbol
  display_draw_vert_line(642, 27, 38);

  //Draw the arrow based on the selected edge
  if(scopesettings.triggeredge == 0)
  {
    //rising edge
    display_draw_horz_line(27, 642, 645);
    display_draw_horz_line(38, 639, 642);
    display_draw_horz_line(32, 641, 643);
    display_draw_horz_line(33, 640, 644);
  }
  else
  {
    //falling edge
    display_draw_horz_line(27, 639, 642);
    display_draw_horz_line(38, 642, 645);
    display_draw_horz_line(32, 640, 644);
    display_draw_horz_line(33, 641, 643);
  }

  //Check on which channel is used for triggering
  switch(scopesettings.triggerchannel)
  {
    //Channel 1
    case 0:
      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, dark channel 1 trigger color box
        display_set_fg_color(CHANNEL1_TRIG_COLOR);
      }
      else
      {
        //Active, some blue box
        display_set_fg_color(0x003333FF);
      }

      //Fill the channel background
      display_fill_rect(605, 25, 28, 15);

      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, black text
        display_set_fg_color(0x00000000);
      }
      else
      {
        //Active, white text
        display_set_fg_color(0x00FFFFFF);
      }

      //Display the text
      display_text(608, 26, "CH1");
      break;

    //Channel 2
    case 1:
      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, dark cyan box
        display_set_fg_color(CHANNEL2_TRIG_COLOR);
      }
      else
      {
        //Active, some red box
        display_set_fg_color(0x00FF3333);
      }

      //Fill the channel background
      display_fill_rect(605, 25, 28, 15);

      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, black text
        display_set_fg_color(0x00000000);
      }
      else
      {
        //Active, white text
        display_set_fg_color(0x00FFFFFF);
      }

      //Display the text
      display_text(608, 26, "CH2");
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_battery_status(void)
{
  //Prepare the battery symbol in a working buffer to avoid flicker
  display_set_screen_buffer(displaybuffer1);

  //Clear the background
  display_set_fg_color(0x00000000);
  display_fill_rect(701, 5, 25, 13);

  //Draw an empty battery symbol in white
  display_set_fg_color(0x00FFFFFF);
  display_fill_rect(701, 9, 2, 4);
  display_fill_rect(703, 5, 22, 12);

  //Check if there is any charge
  if(scopesettings.batterychargelevel)
  {
    //Keep charge level on max if above
    if(scopesettings.batterychargelevel > 20)
    {
      //Max for displaying the level
      scopesettings.batterychargelevel = 20;
    }

    //Check if charge level is low
    if(scopesettings.batterychargelevel < 4)
    {
      //Draw the level indicator in red
      display_set_fg_color(0x00FF0000);
    }
    else
    {
      //Draw the level indicator in dark green
      display_set_fg_color(0x0000BB00);
    }

    //Draw the indicator based on the level
    display_fill_rect(724 - scopesettings.batterychargelevel, 6, scopesettings.batterychargelevel, 10);
  }

  //Draw the battery charging indicator when plugged in
  if(scopesettings.batterycharging)
  {
#if 0
    //Some light blue color
    display_set_fg_color(0x002222FF);

    //Draw an arrow when charging
    display_draw_horz_line(10, 708, 718);
    display_draw_horz_line(11, 708, 718);
    display_draw_horz_line(12, 708, 718);
    display_draw_vert_line(719, 8, 14);
    display_draw_vert_line(720, 9, 13);
    display_draw_vert_line(721, 10, 12);
    display_draw_vert_line(722, 11, 11);
#else
    //Some orange color
    display_set_fg_color(0x00FF6A00);

    //Draw a lightning bolt when charging
    display_draw_horz_line( 7, 715, 716);
    display_draw_horz_line( 8, 713, 716);
    display_draw_horz_line( 9, 711, 715);
    display_draw_horz_line(10, 709, 715);
    display_draw_horz_line(11, 707, 711);
    display_draw_horz_line(12, 705, 709);
    display_draw_horz_line(11, 713, 715);
    display_draw_horz_line(10, 719, 723);
    display_draw_horz_line(11, 717, 721);
    display_draw_horz_line(12, 713, 719);
    display_draw_horz_line(13, 713, 717);
    display_draw_horz_line(14, 712, 715);
    display_draw_horz_line(15, 712, 713);
#endif
  }

  //Copy it to the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(701, 5, 25, 13);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Menu functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_main_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(0, 46, 149, 233);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(0, 46, 149, 233);

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(104, 9, 140);
  display_draw_horz_line(163, 9, 140);
  display_draw_horz_line(222, 9, 140);

  //Display the menu items
  scope_main_menu_system_settings(0);
  scope_main_menu_picture_view(0);
  scope_main_menu_waveform_view(0);
  scope_main_menu_usb_connection(0);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(0, 46, 149, 233, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_system_settings(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(9, 59, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(system_settings_icon, 21, 63, 15, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(69, 60, "System");
  display_text(68, 76, "settings");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_picture_view(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 116, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(picture_view_icon, 17, 122, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(73, 119, "Picture");
  display_text(79, 135, "view");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_waveform_view(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 175, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(waveform_view_icon, 17, 181, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(62, 178, "Waveform");
  display_text(79, 194, "view");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_usb_connection(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 235, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(usb_icon, 20, 239, 18, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(80, 237, "USB");
  display_text(60, 253, "connection");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_channel_menu(PCHANNELSETTINGS settings)
{
  uint32 xstart;
  uint32 xend;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

  //Line start and end x positions
  xstart = settings->menuxpos + 14;
  xend   = settings->menuxpos + CH_MENU_WIDTH - 14;

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(CH_MENU_YPOS +  62, xstart, xend);
  display_draw_horz_line(CH_MENU_YPOS + 124, xstart, xend);
  display_draw_horz_line(CH_MENU_YPOS + 188, xstart, xend);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(settings->menuxpos + 15, CH_MENU_YPOS +  10, "open");
  display_text(settings->menuxpos + 22, CH_MENU_YPOS +  29, "CH");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS +  72, "open");
  display_text(settings->menuxpos + 19, CH_MENU_YPOS +  91, "FFT");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 136, "coup");
  display_text(settings->menuxpos + 18, CH_MENU_YPOS + 154, "ling");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 201, "probe");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 219, "mode");

  //Display the actual settings
  scope_channel_enable_select(settings);
  scope_channel_fft_show(settings);
  scope_channel_coupling_select(settings);
  scope_channel_probe_magnification_select(settings);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT, 69906);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_enable_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so dark grey box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 16, 32, 22);
  }
  else
  {
    //Enabled so dark grey box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 16, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so channel color box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 16, 32, 22);
  }
  else
  {
    //Enabled so channel color box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 16, 32, 22);
  }

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so white on text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Enabled so black on text
    display_set_fg_color(0x00000000);
  }

  //Display the on text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 19, "ON");

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so black off text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Enabled so white off text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the off text
  display_text(settings->menuxpos + 133, CH_MENU_YPOS + 19, "OFF");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_fft_show(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so dark grey box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 78, 32, 22);
  }
  else
  {
    //Enabled so dark grey box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 78, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so channel color box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 78, 32, 22);
  }
  else
  {
    //Enabled so channel color box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 78, 32, 22);
  }

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so white on text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Enabled so black on text
    display_set_fg_color(0x00000000);
  }

  //Display the on text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 81, "ON");

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so black off text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Enabled so white off text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the off text
  display_text(settings->menuxpos + 133, CH_MENU_YPOS + 81, "OFF");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_coupling_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so dark grey box behind ac text
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 142, 32, 22);
  }
  else
  {
    //AC so dark grey box behind dc text
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 142, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so channel color box behind dc text
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 142, 32, 22);
  }
  else
  {
    //AC so channel color box behind ac text
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 142, 32, 22);
  }

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so black DC text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //AC so white or grey DC text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the DC text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 145, "DC");

  //Check if coupling is DC or AC
  if(settings->coupling == 0)
  {
    //DC so white or grey AC text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //AC so black AC text
    display_set_fg_color(0x00000000);
  }

  //Display the off text
  display_text(settings->menuxpos + 135, CH_MENU_YPOS + 145, "AC");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_probe_magnification_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00181818);

  //Check if coupling is dc or ac
  switch(settings->magnification)
  {
    case 0:
      //dark grey times 10 and 100 magnification
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;

    case 1:
      //dark grey times 1 and 100 magnification
      display_fill_rect(settings->menuxpos +  78, CH_MENU_YPOS + 199, 20, 38);
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;

    default:
      //dark grey times 1 and 10 magnification
      display_fill_rect(settings->menuxpos +  78, CH_MENU_YPOS + 199, 20, 38);
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      break;
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if which magnification to highlight
  switch(settings->magnification)
  {
    case 0:
      //Highlight times 1 magnification
      display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 199, 20, 38);
      break;

    case 1:
      //Highlight times 10 magnification
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      break;

    default:
      //Highlight times 100 magnification
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;
  }

  //Check if magnification is 1x
  if(settings->magnification == 0)
  {
    //Yes so black 1X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 1X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 1X text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 201, "1");
  display_text(settings->menuxpos + 83, CH_MENU_YPOS + 219, "X");

  //Check if magnification is 10x
  if(settings->magnification == 1)
  {
    //Yes so black 10X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 10X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 10X text
  display_text(settings->menuxpos + 113, CH_MENU_YPOS + 201, "10");
  display_text(settings->menuxpos + 115, CH_MENU_YPOS + 219, "X");

  //Check if magnification is 100x
  if(settings->magnification > 1)
  {
    //Yes so black 100X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 100X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 100X text
  display_text(settings->menuxpos + 142, CH_MENU_YPOS + 201, "100");
  display_text(settings->menuxpos + 149, CH_MENU_YPOS + 219, "X");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_acquisition_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);

  //A black line between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(ACQ_MENU_YPOS +  158, ACQ_MENU_XPOS + 8, ACQ_MENU_XPOS + ACQ_MENU_WIDTH - 8);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(ACQ_MENU_XPOS + 111, ACQ_MENU_YPOS +   8, "Sample Rate");
  display_text(ACQ_MENU_XPOS +  97, ACQ_MENU_YPOS + 166, "Time per Division");

  //Display the actual settings
  scope_acquisition_speed_select();
  scope_acquisition_timeperdiv_select();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT, 69906);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquisition_speed_select(void)
{
  uint32 i,x,y;

  //Select the font for the texts
  display_set_font(&font_2);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00383838);

  //Clear the boxes for the not selected items
  for(i=0;i<(sizeof(acquisition_speed_texts) / sizeof(int8 *));i++)
  {
    if(i != scopesettings.samplerate)
    {
      x = ((i & 3) * 72) + 10;
      y = ((i >> 2) * 23) + 33;

      display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);
    }
  }

  //Set channel 2 color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Get the position of the selected item
  x = ((scopesettings.samplerate & 3) * 72) + 10;
  y = ((scopesettings.samplerate >> 2) * 23) + 33;

  //Highlight the selected item
  display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);

  for(i=0;i<(sizeof(acquisition_speed_texts) / sizeof(int8 *));i++)
  {
    if(i != scopesettings.samplerate)
    {
      //Check if in stop mode
      if(scopesettings.runstate)
      {
        //When stopped select option is disabled so texts shown in light grey
        display_set_fg_color(0x00606060);
      }
      else
      {
        //When running available not selected texts shown in white
        display_set_fg_color(0x00FFFFFF);
      }
    }
    else
    {
      //Selected texts in black
      display_set_fg_color(0x00000000);
    }

    //Calculate the position of this text
    x = ((i & 3) * 72) + acquisition_speed_text_x_offsets[i];
    y = ((i >> 2) * 23) + 36;

    //Display the text from the table
    display_text(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, (int8 *)acquisition_speed_texts[i]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquisition_timeperdiv_select(void)
{
  uint32 c,i,x,y;

  //Select the font for the texts
  display_set_font(&font_2);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00383838);

  //Clear the boxes for the not selected items
  for(i=0;i<(sizeof(time_div_texts) / sizeof(int8 *));i++)
  {
    //Settings displayed from smallest to highest value
    c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - i;

    if(c != scopesettings.timeperdiv)
    {
      x = ((i & 3) * 72) + 10;
      y = ((i >> 2) * 23) + 191;

      display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);
    }
  }

  //Set channel 2 color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Get the position of the selected item
  c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - scopesettings.timeperdiv;
  x = ((c & 3) * 72) + 10;
  y = ((c >> 2) * 23) + 191;

  //Highlight the selected item
  display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);

  for(i=0;i<(sizeof(time_div_texts) / sizeof(int8 *));i++)
  {
    //Settings displayed from smallest to highest value
    c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - i;

    //Check if the current text is the selected on
    if(c != scopesettings.timeperdiv)
    {
      //When not check if the current on is a viable candidate for full screen trace display
      if(viable_time_per_div[scopesettings.samplerate][c])
      {
        //Available but viable not selected texts in white
        display_set_fg_color(0x00FFFFFF);
      }
      else
      {
        //Not viable but available not selected texts in grey
        display_set_fg_color(0x00686868);
      }
    }
    else
    {
      //Selected texts in black
      display_set_fg_color(0x00000000);
    }

    //Calculate the position of this text
    x = ((i & 3) * 72) + time_div_text_x_offsets[c];
    y = ((i >> 2) * 23) + 194;

    //Display the text from the table
    display_text(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, (int8 *)time_div_texts[c]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_trigger_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(560, 46, 172, 186);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(560, 46, 172, 186);

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(107, 570, 722);
  display_draw_horz_line(168, 570, 722);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(570,  56, "trigger");
  display_text(570,  75, "mode");
  display_text(570, 118, "trigger");
  display_text(570, 137, "edge");
  display_text(570, 182, "trigger");
  display_text(570, 200, "channel");

  //Display the actual settings
  scope_trigger_mode_select();
  scope_trigger_edge_select();
  scope_trigger_channel_select();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(560, 46, 172, 186, 56415);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_mode_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00181818);

  //Check which trigger mode is selected
  switch(scopesettings.triggermode)
  {
    case 0:
      //dark grey single and normal
      display_fill_rect(661, 57, 20, 38);
      display_fill_rect(692, 57, 21, 38);
      break;

    case 1:
      //dark grey auto and normal
      display_fill_rect(629, 57, 20, 38);
      display_fill_rect(692, 57, 21, 38);
      break;

    default:
      //dark grey auto and single
      display_fill_rect(629, 57, 20, 38);
      display_fill_rect(661, 57, 20, 38);
      break;
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check if which trigger mode to highlight
  switch(scopesettings.triggermode)
  {
    case 0:
      //Highlight auto mode
      display_fill_rect(629, 57, 20, 38);
      break;

    case 1:
      //Highlight single mode
      display_fill_rect(661, 57, 20, 38);
      break;

    default:
      //Highlight normal mode
      display_fill_rect(692, 57, 21, 38);
      break;
  }

  //Check if trigger mode is auto
  if(scopesettings.triggermode == 0)
  {
    //Yes so black auto text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the auto text
  display_text(631, 58, "au");
  display_text(633, 75, "to");

  //Check if trigger mode is single
  if(scopesettings.triggermode == 1)
  {
    //Yes so black single text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White single text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey single text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the single text
  display_text(666, 56, "si");
  display_text(663, 66, "ng");
  display_text(665, 79, "le");

  //Check if trigger mode is normal
  if(scopesettings.triggermode > 1)
  {
    //Yes so black normal text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White normal text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey normal text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the normal text
  display_text(695, 56, "no");
  display_text(694, 66, "rm");
  display_text(696, 79, "al");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_edge_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so dark grey box behind falling
    display_fill_rect(671, 125, 45, 22);
  }
  else
  {
    //Falling so dark grey box behind rising
    display_fill_rect(626, 125, 40, 22);
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so trigger color box behind rising
    display_fill_rect(626, 125, 40, 22);
  }
  else
  {
    //Falling so trigger color box behind falling
    display_fill_rect(671, 125, 45, 22);
  }

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so black rising text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //For falling edge trigger check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the rising text
  display_text(629, 127, "rising");

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //For rising edge trigger check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //Falling so black falling text
    display_set_fg_color(0x00000000);
  }

  //Display the falling text
  display_text(674, 127, "falling");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_channel_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so dark grey box behind CH2 text
    display_fill_rect(680, 188, 32, 22);
  }
  else
  {
    //2 so dark grey box behind CH1 text
    display_fill_rect(632, 188, 32, 22);
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so trigger color box behind CH1 text
    display_fill_rect(632, 188, 32, 22);
  }
  else
  {
    //2 so trigger color box behind CH2 text
    display_fill_rect(680, 188, 32, 22);
  }

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so black CH1 text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //2 so white or grey CH1 text
    if((scopesettings.waveviewmode == 0) && scopesettings.channel1.enable)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the CH1 text
  display_text(635, 191, "CH1");

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so white or grey CH2 text
    if((scopesettings.waveviewmode == 0) && scopesettings.channel2.enable)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //2 so black CH2 text
    display_set_fg_color(0x00000000);
  }

  //Display the CH2 text
  display_text(683, 191, "CH2");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_system_settings_menu(void)
{
  int y;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(150, 46, 244, 353);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(150, 46, 244, 353);

  //Five black lines between the settings
  display_set_fg_color(0x00000000);

  for(y=104;y<350;y+=59)
  {
    display_draw_horz_line(y, 159, 385);
  }

  //Display the menu items
  scope_system_settings_screen_brightness_item(0);
  scope_system_settings_grid_brightness_item(0);
  scope_system_settings_trigger_50_item();
  scope_system_settings_calibration_item(0);
  scope_system_settings_x_y_mode_item();
  scope_system_settings_confirmation_item();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(150, 46, 244, 353, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_screen_brightness_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 59, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(screen_brightness_icon, 171, 63, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(231, 60, "Screen");
  display_text(220, 76, "brightness");

  //Show the actual setting
  scope_system_settings_screen_brightness_value();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_screen_brightness_value(void)
{
  //Draw the yellow background
  display_set_fg_color(0x00FFFF00);
  display_fill_rect(332, 67, 32, 15);

  //Display the number with fixed width font and black color
  display_set_font(&font_0);
  display_set_fg_color(0x00000000);
  display_decimal(337, 68, scopesettings.screenbrightness);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_grid_brightness_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 116, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(grid_brightness_icon, 171, 122, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(240, 119, "Grid");
  display_text(220, 135, "brightness");

  //Show the actual setting
  scope_system_settings_grid_brightness_value();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_grid_brightness_value(void)
{
  //Draw the yellow background
  display_set_fg_color(0x00FFFF00);
  display_fill_rect(332, 124, 32, 15);

  //Display the number with fixed width font and black color
  display_set_font(&font_0);
  display_set_fg_color(0x00000000);
  display_decimal(337, 125, scopesettings.gridbrightness);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_trigger_50_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(trigger_50_percent_icon, 171, 181, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(229, 178, "Always");
  display_text(217, 194, "trigger 50%");

  //Show the state
  scope_display_slide_button(326, 183, scopesettings.alwaystrigger50);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_calibration_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 235, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(baseline_calibration_icon, 171, 239, 24, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(225, 237, "Baseline");
  display_text(219, 253, "calibration");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_x_y_mode_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(x_y_mode_display_icon, 171, 297, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(223, 295, "X-Y mode");
  display_text(231, 311, "display");

  //Show the state
  scope_display_slide_button(326, 299, scopesettings.xymodedisplay);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_confirmation_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(confirmation_icon, 171, 356, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(217, 354, "Notification");
  display_text(213, 370, "confirmation");

  //Show the state
  scope_display_slide_button(326, 358, scopesettings.confirmationmode);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_calibration_start_text(void)
{
  //Save the screen under the baseline calibration start text
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(395, 222, 199, 59);

  //Setup the text in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 199, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 199, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(409, 227, "Please unplug");
  display_text(409, 243, "the probe and");
  display_text(409, 259, "USB first !");

  //Add the ok button
  scope_display_ok_button(517, 230, 0);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(395, 222, 199, 59, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_calibrating_text(void)
{
  //Restore the screen from under the calibration start text to get rid of it
  display_set_source_buffer(displaybuffer2);
  display_copy_rect_to_screen(395, 222, 199, 59);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 110, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 110, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(409, 243, "Calibrating...");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_calibration_done_text(void)
{
  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 110, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 110, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(414, 235, "Calibration");
  display_text(416, 251, "successful");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_measures_menu(void)
{
  int item;
  int channel;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(231, 263, 499, 214);

  //Draw the edge in black
  display_set_fg_color(0x00000000);
  display_draw_rect(231, 263, 499, 214);

  //Three horizontal black lines between the settings
  display_draw_horz_line(288, 232, 729);
  display_draw_horz_line(350, 232, 729);
  display_draw_horz_line(412, 232, 729);

  //Vertical separator between the channel sections
  display_draw_vert_line(481, 264, 476);

  //Vertical separators between the items
  display_draw_vert_line(294, 289, 476);
  display_draw_vert_line(356, 289, 476);
  display_draw_vert_line(418, 289, 476);
  display_draw_vert_line(544, 289, 476);
  display_draw_vert_line(606, 289, 476);
  display_draw_vert_line(668, 289, 476);

  //Channel 1 top bar
  display_set_fg_color(CHANNEL1_COLOR);
  display_fill_rect(232, 264, 248, 23);

  //Channel 2 top bar
  display_set_fg_color(CHANNEL2_COLOR);
  display_fill_rect(482, 264, 247, 23);

  //Display the channel identifier text in black
  display_set_fg_color(0x00000000);
  display_set_font(&font_2);
  display_text(240, 269, "CH1");
  display_text(490, 269, "CH2");

  //Display the menu items
  for(channel=0;channel<2;channel++)
  {
    //For each channel 12 items
    for(item=0;item<12;item++)
    {
      //Draw the separate items
      scope_measures_menu_item(channel, item);
    }
  }

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_right_rect_onto_screen(231, 263, 499, 214, 75646);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_measures_menu_item(int channel, int item)
{
  uint16  xpos;
  uint16  ypos;
  char   *text;

  //Set the x position offset for the given channel
  if(channel == 0)
  {
    //Channel 1 is on the left side
    xpos = 232;
  }
  else
  {
    //Channel 2 is on the right side
    xpos = 482;
  }

  //Set the text and the position for the given item
  switch(item)
  {
    case 0:
      text  = "Vmax";
      xpos += 15;
      ypos  = 312;
      break;

    case 1:
      text  = "Vmin";
      xpos += 79;
      ypos  = 312;
      break;

    case 2:
      text  = "Vavg";
      xpos += 141;
      ypos  = 312;
      break;

    case 3:
      text  = "Vrms";
      xpos += 203;
      ypos  = 312;
      break;

    case 4:
      text  = "Vpp";
      xpos += 19;
      ypos  = 376;
      break;

    case 5:
      text  = "Vp";
      xpos += 86;
      ypos  = 376;
      break;

    case 6:
      text  = "Freq";
      xpos += 143;
      ypos  = 376;
      break;

    case 7:
      text  = "Cycle";
      xpos += 201;
      ypos  = 376;
      break;

    case 8:
      text  = "Tim+";
      xpos += 17;
      ypos  = 437;
      break;

    case 9:
      text  = "Tim-";
      xpos += 80;
      ypos  = 437;
      break;

    case 10:
      text  = "Duty+";
      xpos += 138;
      ypos  = 437;
      break;

    case 11:
      text  = "Duty-";
      xpos += 202;
      ypos  = 437;
      break;

    default:
      return;
  }

  //Check if item is on or off
  if(scopesettings.measuresstate[channel][item] == 0)
  {
    //Off so some dark grey text
    display_set_fg_color(0x00444444);
  }
  else
  {
    //On so white text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(xpos, ypos, text);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_slider(uint16 xpos, uint16 ypos, uint8 position)
{
  //Save the screen under the screen brightness slider
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(xpos, ypos, 331, 58);

  //Setup the slider menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(xpos, ypos, 331, 58);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(xpos, ypos, 331, 58);

  //Display the actual slider
  scope_display_slider(xpos, ypos, position);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(xpos, ypos, 331, 58, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_slider(uint16 xpos, uint16 ypos, uint8 position)
{
  uint16 x = xpos + 20;
  uint16 y = ypos + 24;
  uint16 w = (291 * position) / 100;
  uint16 ys = ypos + 23;
  uint16 ye = ypos + 35;

  //Clear the background first
  display_set_fg_color(0x00181818);
  display_fill_rect(xpos + 8, ypos + 17, 315, 24);

  //Draw the first part of the slider bar in a yellow color
  display_set_fg_color(0x00FFFF00);
  display_fill_rounded_rect(x, y, w, 10, 2);

  //Adjust positions for the grey part
  x += w;
  w  = 291 - w;

  //Draw the last part of the slider bar in a light grey color
  display_set_fg_color(0x00666666);
  display_fill_rounded_rect(x, y, w, 10, 2);

  //Adjust positions for drawing the knob
  x -= 11;
  y -= 6;

  //Draw the knob
  display_set_fg_color(0x00AAAAAA);
  display_fill_rounded_rect(x, y, 22, 22, 2);

  //Draw the black lines on the knob
  display_set_fg_color(0x00000000);
  display_draw_vert_line(x +  6, ys, ye);
  display_draw_vert_line(x + 11, ys, ye);
  display_draw_vert_line(x + 16, ys, ye);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_move_slider(uint16 xpos, uint16 ypos, uint8 *position)
{
  uint16 xs = xpos + 20;
  uint16 value;
  int16 filter = xtouch - prevxtouch;

  //Check if update needed
  if((filter > -3) && (filter < 3))
  {
    //When change in movement less the absolute 3 don't process
    return(0);
  }

  //Save for next filter check
  prevxtouch = xtouch;

  //Make sure it stays in allowed range
  if(xtouch <= xs)
  {
    //Below slider keep it on 0
    value = 0;
  }
  else if(xtouch >= (xpos + 311))
  {
    //Above slider keep it on max
    value = 100;
  }
  else
  {
    //Based on xtouch position calculate a new position from 0 to 100
    value = ((xtouch - xs) * 100) / 291;
  }

  //Update the position variable
  *position = value;

  //Show the new position on screen
  scope_display_slider(xpos, ypos, value);

  //Signal there is change
  return(1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_slide_button(uint16 xpos, uint16 ypos, uint8 state)
{
  uint16 linex      = xpos + 8;
  uint16 lineystart = ypos + 6;
  uint16 lineyend   = ypos + 15;
  uint16 buttonx    = xpos + 4;
  uint32 edgecolor  = 0x00444444;
  uint32 fillcolor  = 0x00888888;

  if(state == 1)
  {
    //Displace the lines and button by 19 pixels
    linex   += 19;
    buttonx += 19;

    //Set the enabled colors
    edgecolor  = 0x00008800;
    fillcolor  = 0x0000FF00;
  }

  //Draw the background
  display_set_fg_color(fillcolor);
  display_fill_rounded_rect(xpos, ypos, 45, 21, 2);

  //Draw the edge
  display_set_fg_color(edgecolor);
  display_draw_rounded_rect(xpos, ypos, 45, 21, 2);

  //Draw button in dark grey
  display_set_fg_color(0x00444444);
  display_fill_rect(buttonx, ypos + 4, 19, 13);

  //Draw lines in black
  display_set_fg_color(0x00000000);
  display_draw_vert_line(linex,     lineystart, lineyend);
  display_draw_vert_line(linex + 3, lineystart, lineyend);
  display_draw_vert_line(linex + 6, lineystart, lineyend);
  display_draw_vert_line(linex + 9, lineystart, lineyend);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_ok_button(uint16 xpos, uint16 ypos, uint8 mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00333333);
  }
  else
  {
    //Active so light grey background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(xpos, ypos, 66, 44);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black foreground
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(xpos + 24, ypos + 14, "OK");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_adjust_timebase(void)
{
  //Check if touch within the trace display region
  if((previousxtouch > 2) && (previousxtouch < 720) && (previousytouch > 50) && (previousytouch < 470))
  {
    //Check if touch on the left of the center line
    if(previousxtouch < 358)
    {
      //Check if not already on the highest setting (50S/div)
      if(scopesettings.timeperdiv > 0)
      {
        //Go up in time by taking one of the setting
        scopesettings.timeperdiv--;
      }
    }
    //Check if touch on the right of the center line
    else if(previousxtouch > 362)
    {
      //Check if not already on the lowest setting (10nS/div)
      if(scopesettings.timeperdiv < ((sizeof(time_div_texts) / sizeof(int8 *)) - 1))
      {
        //Go down in time by adding one to the setting
        scopesettings.timeperdiv++;
      }
    }

    //For time per div set with tapping on the screen the direct relation between the time per div and the sample rate is set
    //but only when the scope is running. Otherwise the sample rate of the acquired buffer still is valid.
    if(scopesettings.runstate == 0)
    {
      //Set the sample rate that belongs to the selected time per div setting
      scopesettings.samplerate = time_per_div_sample_rate[scopesettings.timeperdiv];
    }

    //Set the new setting in the FPGA
    fpga_set_sample_rate(scopesettings.samplerate);

    //Show he new setting on the display
    scope_acqusition_settings(0);
  }
}
#endif
//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_grid(void)
{
  uint32 color;
  register uint32 i;

  //Only draw the grid when something will show (not in the original code)
  if(scopesettings.gridbrightness > 3)
  {
    //Calculate a grey shade based on the grid brightness setting
    color = (scopesettings.gridbrightness * 255) / 100;
    color = (color << 16) | (color << 8) | color;

    //Set the color for drawing
    display_set_fg_color(color);

    //Draw the edge
    display_draw_rect(2, 46, 726, 404);

    //Draw the center lines
    display_draw_horz_line(249,  2, 726);
    display_draw_vert_line(364, 46, 448);

    //Draw the ticks on the x line
    for(i=4;i<726;i+=5)
    {
      display_draw_vert_line(i, 247, 251);
    }

    //Draw the ticks on the y line
    for(i=49;i<448;i+=5)
    {
      display_draw_horz_line(i, 362, 366);
    }

    //Draw the horizontal dots
    for(i=99;i<448;i+=50)
    {
      display_draw_horz_dots(i, 4, 726, 5);
    }

    //Draw the vertical dots
    for(i=14;i<726;i+=50)
    {
      display_draw_vert_dots(i, 49, 448, 5);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_calculate_trigger_vertical_position()
{
  PCHANNELSETTINGS settings;

  //Select the channel based on the current trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  int32 position;

  //Center the trigger level around 0 point
  position = scopesettings.triggerlevel - 128;

  //Adjust it for the current volt per div setting
  position = (position * signal_adjusters[settings->displayvoltperdiv]) >> VOLTAGE_SHIFTER;

  //Add the trace center to it
  position = settings->traceposition + position;

  //Limit to extremes
  if(position < 0)
  {
    position = 0;
  }
  else if(position > 401)
  {
    position = 401;
  }

  //Set as new position
  scopesettings.triggerverticalposition = position;
}


//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquire_trace_data(void)
{
  uint32 data;

  //Check if running and not in a trace or cursor displacement state
  if((scopesettings.runstate == 0) && (touchstate == 0))
  {
    //Set the trigger level
    fpga_set_trigger_level();

    //Write the time base setting to the FPGA
    fpga_set_time_base(scopesettings.timeperdiv);

    //Sampling with trigger circuit enabled
    scopesettings.samplemode = 1;

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

#if 0    
    //Check if cut short with touch
    if(havetouch)
    {
      //If so skip the rest
      return;
    }
#endif
    
    //Check if in single mode
    if(scopesettings.triggermode == 1)
    {
      //Switch to stopped
      scopesettings.runstate = 1;

      //Show this on the screen
      ui_display_run_stop_text();
    }

    //Get trigger point information
    //Later on used to send to the FPGA with command 0x1F
    data = fpga_prepare_for_transfer();
    
    //After 800 samples the FPGA starts checking on a trigger, and it can run past the end of the buffer back to the beginning
    //To determine the starting point of from where to read the data it is possible to extend it to this 800 samples as the before
    //and read 1600 samples per ADC

    //Just using the same calculation for every setting solves the frequency calculation error
    //The signal representation still is correct and the trigger point seems to be more valid also
    //The original uses time base dependent processing here, but this seems to do the trick on all ranges
    //The software needs to verify the trigger to make it more stable
    if(data < 750)
    {
      //Less then 750 make it bigger (4095 - 750 = 3345)
      //Reading is also circular. 4096 becomes 0.
      data = data + 3345;
    }
    else
    {
      //More then 750 make it smaller
      data = data - 750;
    }

    //Only need a single count variable for both channels, since they run on the same sample rate
    //This can be changed to a global define
    scopesettings.nofsamples  = SAMPLES_PER_ADC;
    scopesettings.samplecount = SAMPLE_COUNT;

    //Check if channel 1 is enabled
    if(scopesettings.channel1.enable)
    {
      //Get the samples for channel 1
      fpga_read_sample_data(&scopesettings.channel1, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && (scopesettings.triggerchannel == 0))
      {
        //Use the channel 1 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel1.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }

    //Check if channel 2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Get the samples for channel 2
      fpga_read_sample_data(&scopesettings.channel2, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && scopesettings.triggerchannel)
      {
        //Use the channel 2 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel2.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }


    //Need to improve on this for a more stable displaying. On the low sample rate settings it seems to flip between two positions.
    //Determine the trigger position based on the selected trigger channel
    scope_process_trigger(scopesettings.nofsamples);

  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_process_trigger(uint32 count)
{
  uint8  *buffer;
  uint32  index;
  uint32  level = scopesettings.triggerlevel;
  uint32  sample1;
  uint32  sample2;

  //Select the trace buffer to process based on the trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    //Channel 1 buffer
    buffer = (uint8 *)channel1tracebuffer;
  }
  else
  {
    //Channel 2 buffer
    buffer = (uint8 *)channel2tracebuffer;
  }

  disp_have_trigger = 0;

  //Set a starting point for checking on trigger
  //Count is half a sample buffer!!
  index = count - 10;
  count = 20;

  //Need a better check here, maybe over a wider range of samples

  while(count--)
  {
    sample1 = buffer[index];
    sample2 = buffer[index + 1];

    if(((scopesettings.triggeredge == 0) && (sample1 < level) && (sample2 >= level)) ||
       ((scopesettings.triggeredge == 1) && (sample1 >= level) && (sample2 < level)))
    {
      //Set the current index as trigger point
      disp_trigger_index = index;

      //Signal trigger has been found
      disp_have_trigger = 1;

      //Done with checking
      break;
    }

    //Select next sample to check
    index++;
  }
}

#if 0
//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_do_baseline_calibration(void)
{
  uint32 flag = 1;
  uint32 voltperdiv;

  //Disable the trigger circuit
  scopesettings.samplemode = 0;

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = SAMPLE_COUNT;
  scopesettings.nofsamples  = SAMPLES_PER_ADC;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Clear the compensation values before doing the calibration
  calibrationsettings.adc1compensation = 0;
  calibrationsettings.adc2compensation = 0;

  //Setup for channel 1 calibration
  calibrationsettings.voltperdivcommand = 0x33;
  calibrationsettings.offsetcommand     = 0x32;
  calibrationsettings.adc1command       = 0x20;
  calibrationsettings.adc2command       = 0x21;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel1tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel1.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel1.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel1.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Setup for channel 2 calibration
  calibrationsettings.voltperdivcommand = 0x36;
  calibrationsettings.offsetcommand     = 0x35;
  calibrationsettings.adc1command       = 0x22;
  calibrationsettings.adc2command       = 0x23;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel2tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel2.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel2.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel2.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Load the normal operation settings back into the FPGA
  fpga_set_channel_voltperdiv(&scopesettings.channel1);
  fpga_set_channel_offset(&scopesettings.channel1);
  fpga_set_channel_voltperdiv(&scopesettings.channel2);
  fpga_set_channel_offset(&scopesettings.channel2);
  fpga_set_sample_rate(scopesettings.samplerate);

  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

#define HIGH_DC_OFFSET   500
#define LOW_DC_OFFSET   1200

uint32 scope_do_channel_calibration(void)
{
  uint32 flag = 1;
  int32  samplerate;
  int32  voltperdiv;
  int32  highaverage;
  int32  lowaverage;
  int32  dcoffsetstep;
  int32  compensationsum = 0;

  //Calibrate for the hardware sensitivity settings
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start with the first set of averages
    samplerateindex = 0;

    //Do the measurements on two sample rates. 200MSa/s and 50KSa/s
    for(samplerate=0;samplerate<18;samplerate+=11)
    {
      //Set the selected sample rate
      fpga_set_sample_rate(samplerate);

      //Set the matching time base
      fpga_set_time_base(sample_rate_time_per_div[samplerate]);

      //Set the high DC offset in the FPGA (Lower value returns higher ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = HIGH_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done or touch panel active
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as one reading here. Only use ADC1 data
      highaverage = calibrationsettings.adc1rawaverage;

      //Set the low DC offset in the FPGA (Higher value returns lower ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = LOW_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done or touch panel active
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as another reading here. Only use ADC1 data
      lowaverage = calibrationsettings.adc1rawaverage;

      //Calculate the DC offset step for a single ADC bit change for this volt/div setting
      //Low DC offset minus high DC offset (1200 - 500) = 700. Scaled up for fixed point calculation ==> 700 << 20 = 734003200
      dcoffsetstep = 734003200 / (highaverage - lowaverage);

      //Calculate the average DC offset settings for both the low to center as the high to center readings
      highaverage = HIGH_DC_OFFSET + (((highaverage - 128) * dcoffsetstep) >> 20);
      lowaverage  = LOW_DC_OFFSET  - (((128 - lowaverage) * dcoffsetstep) >> 20);

      //Set the result for this sample rate and volt per division setting
      samplerateaverage[samplerateindex][voltperdiv] = (highaverage + lowaverage) / 2;

      //Save the dc offset step for final calibration after compensation data has been determined
      sampleratedcoffsetstep[samplerateindex][voltperdiv] = dcoffsetstep;

      //Select the next set of sample indexes
      samplerateindex++;
    }
  }

  //Set the sample rate to 2MSa/s
  fpga_set_sample_rate(6);

  //Set the matching time base
  fpga_set_time_base(sample_rate_time_per_div[6]);

  //Average and test the results on a third sample rate
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the average of the two measurements as the DC offset to work with
    calibrationsettings.dc_calibration_offset[voltperdiv] = (samplerateaverage[0][voltperdiv] + samplerateaverage[1][voltperdiv]) / 2;

    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Set the new DC channel offset in the FPGA
    fpga_set_channel_offset(&calibrationsettings);

    //Wait 50ms before sampling
    timer0_delay(50);

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(&calibrationsettings, 100);

    //Check if the average reading is outside allowed range
    if((calibrationsettings.adc1rawaverage < 125) || (calibrationsettings.adc1rawaverage > 131))
    {
      //When deviation is more then 3, signal it as a failure
      flag = 0;
    }

    //Sum the ADC differences
    compensationsum += (calibrationsettings.adc2rawaverage - calibrationsettings.adc1rawaverage);
  }

  //Calculate the average of the ADC difference
  compensationsum /= 6;

  //Split the difference on the two ADC's
  calibrationsettings.adc1compensation = compensationsum / 2;
  calibrationsettings.adc2compensation = -1 * (compensationsum - calibrationsettings.adc1compensation);

  //Check if the found result is within limits
  if((compensationsum < -20) || (compensationsum > 20))
  {
    //Not so clear the flag
    flag = 0;
  }

  //Adjust the center point DC offsets with the found compensation values
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //The DC offset is based on the pre compensated ADC1 reading, so need to adjust with the average DC offset step times the ADC1 compensation value
    calibrationsettings.dc_calibration_offset[voltperdiv] = (int16)calibrationsettings.dc_calibration_offset[voltperdiv] + (((int32)calibrationsettings.adc1compensation * (((int32)sampleratedcoffsetstep[0][voltperdiv] + (int32)sampleratedcoffsetstep[1][voltperdiv]) / 2)) >> 20);
  }

  //Return the result of the tests. True if all tests passed
  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_do_50_percent_trigger_setup(void)
{
  //Check which channel is the active trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    //Use the channel 1 center value
    scopesettings.triggerlevel = scopesettings.channel1.center;
  }
  else
  {
    //Use the channel 2 center value
    scopesettings.triggerlevel = scopesettings.channel2.center;
  }

  //Set the trigger vertical position position to match the new trigger level
  scope_calculate_trigger_vertical_position();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_do_auto_setup(void)
{
  PCHANNELSETTINGS settings;

  int32  screentime;
  int32  timeperdiv;
  int32  samplerate;

  uint32 dochannel1 = scopesettings.channel1.enable;
  uint32 dochannel2 = scopesettings.channel2.enable;

  //No need to do auto setup if no channel is enabled
  if((dochannel1 == 0) && (dochannel2 == 0))
  {
    return;
  }

  //Disable the trigger circuit
  scopesettings.samplemode = 0;
  
  //Modification on 09-03-2022. Begin
  //Switch back to AUTO trigger mode to avoid lockup in NORMAL or SINGLE mode
  scopesettings.triggermode = 0;
  
  //Show it on the display
  scope_trigger_settings(0);
  
  //Also set the FPGA to the AUTO trigger mode
  fpga_set_trigger_mode();
  
  //To make sure the scope will be running after auto set, set the run mode to running
  scopesettings.runstate = 0;
  
  //Show this on the screen
  scope_run_stop_text();
  //End. 09-03-2022

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = SAMPLE_COUNT;
  scopesettings.nofsamples  = SAMPLES_PER_ADC;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Setup channel 1 if enabled
  if(dochannel1)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel1.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel1);
  }

  //Setup channel 2 if enabled
  if(dochannel2)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel2.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel2);
  }

  //Select the channel to work with
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  //Wait 50ms to allow the relays to settle
  timer0_delay(50);

  //Do the measurements on four sample rates. 200MSa/s, 2MSa/s, 20KSa/s and 1000Sa/s
  for(samplerate=0;samplerate<4;samplerate++)
  {
    //Set the selected sample rate
    fpga_set_sample_rate(samplerate_for_autosetup[samplerate]);

    //Set the matching time base
    fpga_set_time_base(sample_rate_time_per_div[samplerate_for_autosetup[samplerate]]);

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(settings, 100);

    //Check if there is a frequency reading and break the loop if so
    if(settings->frequencyvalid)
    {
      break;
    }
  }

  //When there is a frequency reading determine the needed time per division setting
  if(settings->frequencyvalid)
  {
    //Can't use the frequency here since it is based on the scopesettings.samplerate variable, which is not used here
    //Calculate the time in nanoseconds for getting three periods on the screen
    screentime = (float)settings->periodtime * sample_time_converters[samplerate_for_autosetup[samplerate]];

    //Match the found time to the nearest time per division setting
    for(timeperdiv=0;timeperdiv<24;timeperdiv++)
    {
      //When the found time is higher than the selected time per division quit the search
      if(screentime > time_per_div_matching[timeperdiv])
      {
        break;
      }
    }

    //Check if not on the first setting
    if(timeperdiv)
    {
      //If so take one of to get to the right one to use. Also ensures not selecting a non existing setting if none found
      timeperdiv--;
    }

    //Select the found time per division
    scopesettings.timeperdiv = timeperdiv;
    scopesettings.samplerate = time_per_div_sample_rate[timeperdiv];
  }
  else
  {
    //Set a default sample rate and time per division setting
    scopesettings.timeperdiv = 12;
    scopesettings.samplerate = time_per_div_sample_rate[12];
  }

  //Range the input sensitivity on the enabled channels
  //Check on which bottom check level needs to be used
  //When both channels are enabled and in normal display mode use separate sections of the screen for each channel.
  if(dochannel1 && dochannel2 && (scopesettings.xymodedisplay == 0))
  {
    //Both channels enabled then use a lower level. Smaller section of the display available per channel so lower value
    scopesettings.channel1.maxscreenspace = 1900;
    scopesettings.channel2.maxscreenspace = 1900;

    //Give both traces it's own location on screen
    scopesettings.channel1.traceposition = 300;
    scopesettings.channel2.traceposition = 100;

    //Get the sample data for the not trigger source channel when a valid frequency has been determined
    if(settings->frequencyvalid)
    {
      //Check which channel is the trigger source
      if(scopesettings.triggerchannel == 0)
      {
        //Channel 1 is trigger source so get channel 2 data
        fpga_read_sample_data(&scopesettings.channel2, 100);
      }
      else
      {
        //Channel 2 is trigger source so get channel 1 data
        fpga_read_sample_data(&scopesettings.channel1, 100);
      }
    }
  }
  else
  {
    //Only one channel enabled then more screen space available for it so higher value
    scopesettings.channel1.maxscreenspace = 3900;
    scopesettings.channel2.maxscreenspace = 3900;

    //Used channel will be set on the middle of the display
    scopesettings.channel1.traceposition = 200;
    scopesettings.channel2.traceposition = 200;
  }

  //Set the new sample rate in the FPGA
  fpga_set_sample_rate(scopesettings.samplerate);

  //Show the new settings
  scope_acqusition_settings(0);

  //Check the channels on valid voltage readings with the already done conversion, but only when valid frequency
  if(settings->frequencyvalid)
  {
    //Check if channel 1 is enabled and check its range if so
    if(dochannel1)
    {
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 is enabled and check its range if so
    if(dochannel2)
    {
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }
  }

  //If one or either channel not in range another conversion is needed
  if(dochannel1 || dochannel2)
  {
    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel1.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel2.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel2);
    }

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start the conversion and wait until done
    fpga_do_conversion();

    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel1, 100);

      //Check the range again
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel2, 100);

      //Check the range again
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }

    //If one or either channel not in range another conversion is needed
    if(dochannel1 || dochannel2)
    {
      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel1.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel2.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel2);
      }

      //Wait 50ms to allow the relays to settle
      timer0_delay(50);

      //Start the conversion and wait until done
      fpga_do_conversion();

      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel1, 100);

        //Check the range again
        dochannel1 = scope_check_channel_range(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel2, 100);

        //Check the range again
        dochannel2 = scope_check_channel_range(&scopesettings.channel2);
      }

      //If one or either channel not in range the least sensitive setting needs to be used
      if(dochannel1 || dochannel2)
      {
        //Check if channel 1 still not done
        if(dochannel1)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel1.samplevoltperdiv = 0;
        }

        //Check if channel 2 still not done
        if(dochannel2)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel2.samplevoltperdiv = 0;
        }
      }
    }
  }

  //Check if channel 1 is enabled and set the new settings if so
  if(scopesettings.channel1.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel1);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel1.displayvoltperdiv = scopesettings.channel1.samplevoltperdiv;
    
    //Update the display
    scope_channel_settings(&scopesettings.channel1, 0);
  }

  //Check if channel 2 is enabled and set the new settings if so
  if(scopesettings.channel2.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel2);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel2.displayvoltperdiv = scopesettings.channel2.samplevoltperdiv;
    
    //Update the display
    scope_channel_settings(&scopesettings.channel2, 0);
  }

  //Adjust the trigger level to 50% setting
  scope_do_50_percent_trigger_setup();
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_check_channel_range(PCHANNELSETTINGS settings)
{
  uint32 screenpixels;
  uint32 range;
  uint32 notdone = 0;

  //Convert the peak peak reading to screen pixels
  screenpixels = (settings->peakpeak * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Check if there is any signal at all to avoid divide by zero
  if(screenpixels)
  {
    //Calculate a fixed point range to determine the volts per division setting, based on the available screen space
    range = settings->maxscreenspace / screenpixels;

    //Check if signal is at least 10 times smaller then the max range
    if(range >= 100)
    {
      //If so go three sensitivity settings up. (e.g. 500mV/div ==> 50mV/div)
      settings->samplevoltperdiv = 3;
    }
    //Else check if signal is at least 5 times smaller then the max range
    else if(range >= 50)
    {
      //If so go two sensitivity settings up. (e.g. 500mV/div ==> 100mV/div)
      settings->samplevoltperdiv += 2;
    }
    //Else check if signal is at least 2.5 times smaller then the max range
    else if(range >= 25)
    {
      //If so go one sensitivity setting up. (e.g. 500mV/div ==> 200mV/div)
      settings->samplevoltperdiv += 1;
    }
    //Else check if signal is bigger then the max range
    else if(range < 10)
    {
      //Out of range so not done
      notdone = 1;
    }

    //Check if volt per division not out of range
    if(settings->samplevoltperdiv > 6)
    {
      //Keep it on max if so
      settings->samplevoltperdiv = 6;
    }
  }
  else
  {
    //When there is no signal use the most sensitive setting
    settings->samplevoltperdiv = 6;
  }

  return(notdone);
}

#endif
//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_trace_data(void)
{
  //See if it is possible to rework this to fixed point. A 32 bit mantissa is not accurate enough though

  //On the scope the last pixel interleaving is not working properly. Don't know why.

  //Check out the sin x/x interpolation downloaded from keysight (Sine wave reproduction using Sinc interpolation - v.1.0.py)

  //This needs more code to skip when stopped and no changes are made. It seems to be ok without this, but what is not needed is not needed
  //In the display trace routine the display only needs to be redrawn when certain conditions apply
  //  The user changes the sample rate or the time per div setting. Might need to block the setting of the sample rate when stopped because that makes no change to the current sample buffer.
  //  The user moves pointers around. Trace up and down should work, and trigger left and right. Changing the trigger level should not do anything, or even be disabled



  //Need to compensate for the position being on the left side of the pointer
  uint32 triggerposition = scopesettings.triggerhorizontalposition + 7;

  //Check if a trigger position has been found
  if(disp_have_trigger == 0)
  {
    //When not use the center of the sample buffer
    disp_trigger_index = scopesettings.samplecount / 2;
  }

  //Make sure the two settings are in range of the tables!!!!
  //SKipp displaying if not????


  //The amount of x positions needed per sample is based on the number of pixels per division, the set time per division and the sample rate.
  disp_xpos_per_sample = (50.0 * frequency_per_div[scopesettings.timeperdiv]) / sample_rate[scopesettings.samplerate];

  //This gives the step size for going through the samples. Is also the linear interleaving step for the y direction
  disp_sample_step = 1.0 / disp_xpos_per_sample;

  //The displayable x range is based on the number of samples and the number of x positions needed per sample
  //Halved to allow trigger position to be in the center
  double xrange = (scopesettings.samplecount * disp_xpos_per_sample) / 2.0;

  //x range needs to be at least 1 pixel
  if(xrange < 1.0)
  {
    xrange = 1.0;
  }
  else if(xrange > 725.0)
  {
    //Limit on max screen pixels to avoid disp_xend becoming 0x80000000 due to overflow
    xrange = 725.0;
  }

  //Calculate the start and end x coordinates
  disp_xstart = triggerposition - xrange;
  disp_xend = triggerposition + xrange;

  //Limit on actual start of trace display
  if(disp_xstart < 3)
  {
    disp_xstart = 3;
  }

  //And limit on the actual end of the trace display
  if(disp_xend > 725)
  {
    disp_xend = 725;
  }

  //Determine first sample to use based on a full screen worth of samples and the trigger position in relation to the number of pixels on the screen
  disp_first_sample = disp_trigger_index - (((725.0 / disp_xpos_per_sample) * triggerposition) / 725.0) - 1;

  //If below the first sample limit it on the first sample
  if(disp_first_sample < 0)
  {
    disp_first_sample = 0;
  }

  //This makes sure no reading outside the buffer can occur
  if(disp_sample_step > ((scopesettings.samplecount) / 2))
  {
    disp_sample_step = (scopesettings.samplecount) / 2;
  }




  //If samplestep > 1 might be an idea to draw the in between samples on the same x position to avoid aliasing
  //If sample step < 1 then skip drawing on x positions. The draw line function does the linear interpolation



  //Use a separate buffer to clear the screen. Also used as source to copy back to the screen
  display_set_screen_buffer(displaybuffer1);
  display_set_source_buffer(displaybuffer1);

  //Clear the trace portion of the screen
  display_set_fg_color(0x00000000);
  display_fill_rect(6, 59, 699, 399);

  //Check if not in waveform view mode with grid disabled
  if((scopesettings.waveviewmode == 0) || scopesettings.gridenable == 0)
  {
    //Draw the grid lines and dots based on the grid brightness setting
    ui_draw_grid();
  }

  //Check if scope is in normal display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //The calculations done above need to go here??


    //Check if channel1 is enabled
    if(scopesettings.channel1.enable)
    {
      //This can be reduced in parameters by using the channel structure as input and add the color as item in the structure

      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel1);
    }

    //Check if channel2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel2);
    }

    //Displaying of FFT needs to be added here.

  }
  else
  {
    //Scope set to x y display mode
    //Set x-y mode display trace color
    display_set_fg_color(XYMODE_COLOR);

    uint32 index = disp_trigger_index - 315;
    uint32 last = index + 730;

    //Need two samples per channel
    uint32 x1,x2;
    uint32 y1,y2;

    //Get the samples for the first point
    x1 = scope_get_x_sample(&scopesettings.channel1, index);
    y1 = scope_get_y_sample(&scopesettings.channel2, index);
    
    //Handle all the needed samples
    for(;index<last;index++)
    {
      //Get the samples for the next point
      x2 = scope_get_x_sample(&scopesettings.channel1, index);
      y2 = scope_get_y_sample(&scopesettings.channel2, index);

      //Draw the line between these two points
      display_draw_line(x1, y1, x2, y2);

      //Swap the points for drawing the next part
      x1 = x2;
      y1 = y2;
    }
  }

  //Draw the cursors with their text and measurement display
  ui_draw_time_cursors();
  ui_draw_volt_cursors();
  ui_display_selected_text();
  ui_display_cursor_measurements();

  //Draw the signal center, trigger level and trigger position pointers
  ui_draw_pointers();

  //Update the measurements in the six slots on the screen
  ui_update_measurements();

  //Check if in waveform view
  if(scopesettings.waveviewmode)
  {
    //Display the file name
    //Use white text and font_0
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_0);
    display_text(550, 48, viewfilename);
  }
  
  //Copy it to the actual screen buffer
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(6, 59, 699, 399);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_x_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }

  //Limit the sample on max displayable
  if(sample > 401)
  {
    sample = 401;
  }

  //The x center position has an extra offset compared to the y trace position
  return(sample + 165);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_y_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }

  //Limit the sample on max displayable
  if(sample > 401)
  {
    sample = 401;
  }

  //Display y coordinates are inverted to signal orientation
  return(448 - sample);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_channel_trace(PCHANNELSETTINGS settings)
{
  double inputindex;

  register int32 previousindex;
  register int32 currentindex;
  register int32 sample1 = 0;
  register int32 sample2 = 0;
  register uint32 lastx = disp_xstart;
  register uint32 xpos = disp_xstart + 1;

  register PDISPLAYPOINTS tracepoints = settings->tracepoints;

  //Set the trace color for the current channel
  display_set_fg_color(settings->color);

  //Get the processed sample
  sample1 = scope_get_y_sample(settings, disp_first_sample);

  //Store the first sample in the trace points buffer
  tracepoints->x = lastx;
  tracepoints->y = sample1;
  tracepoints++;

  //Start with one tracepoint
  settings->noftracepoints = 1;

  //Step to the next input index
  inputindex = disp_first_sample + disp_sample_step;

  //The previous index is the index of the first sample
  previousindex = disp_first_sample;

  //Process the sample data to screen data
  for(; xpos < disp_xend; inputindex += disp_sample_step, xpos++)
  {
    //Get the current integer index into the sample buffer
    currentindex = inputindex;

    //Check if linear approximation needs to be done. (Only when step < 1) pixels are skipped if so.
    if(currentindex != previousindex)
    {
      //Set new previous index
      previousindex = currentindex;

      //Get the processed sample
      sample2 = scope_get_y_sample(settings, currentindex);

      //Store the second sample in the screen buffer
      tracepoints->x = xpos;
      tracepoints->y = sample2;
      tracepoints++;

      //One more tracepoint
      settings->noftracepoints++;

      //Need to draw a line here
      display_draw_line(lastx, sample1, xpos, sample2);

      sample1 = sample2;

      lastx = xpos;
    }
  }

  //When step less then 1 the last pixel needs to be interpolated between current sample and next sample.
  if(disp_sample_step < 1.0)
  {
    //Calculate the scaler for the last y value based on the x distance from the last drawn position to the end of the screen
    //divided by the x distance it takes to where the next position should be drawn (Number of x steps per sample)
    double scaler =  (725.0 - lastx) / disp_xpos_per_sample;    // (1 / samplestep);

    //Get the processed sample
    sample2 = scope_get_y_sample(settings, inputindex);

    sample2 = sample1 + ((double)((double)sample2 - (double)sample1) / scaler);

    //Store the last sample in the screen buffer
    tracepoints->x = xpos;
    tracepoints->y = sample2;
    tracepoints++;

    //One more tracepoint
    settings->noftracepoints++;

    //Draw the last line
    display_draw_line(lastx, sample1, xpos, sample2);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_measurements(void)
{
#if 0  
  //Check if channel1 is active
  if(scopesettings.channel1.enable)
  {
    //Display the enabled measurements if it is active
    scope_display_channel_measurements(&scopesettings.channel1, scopesettings.measuresstate[0], 5, CHANNEL1_COLOR);
  }

  //Check if channel2 is active
  if(scopesettings.channel2.enable)
  {
    //Display the enabled measurements if it is active
    scope_display_channel_measurements(&scopesettings.channel2, scopesettings.measuresstate[1], 366, CHANNEL2_COLOR);
  }
#endif  
}

//----------------------------------------------------------------------------------------------------------------------------------
#if 0
const MEASUREMENTFUNCTION measurementfunctions[12] =
{
  scope_display_vmax,
  scope_display_vmin,
  scope_display_vavg,
  scope_display_vrms,
  scope_display_vpp,
  scope_display_vp,
  scope_display_freq,
  scope_display_cycle,
  scope_display_time_plus,
  scope_display_time_min,
  scope_display_duty_plus,
  scope_display_duty_min
};

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_channel_measurements(PCHANNELSETTINGS settings, uint8 *measuresstate, uint32 xstart, uint32 color)
{
  int i;
  int x,xe,y;

  x  = xstart;
  xe = xstart + 359;
  y  = 456;
  
  //Process the twelve possible measurements for this channel
  for(i=0;i<12;i++)
  {
    //Check if the current one is enabled
    if(measuresstate[i])
    {
      //Set gray background for the measurements tile
      display_set_fg_color(0x00404040);

      //Fill rounded rectangle for the tile
      display_fill_rounded_rect(x, y, 118, 22, 2);

      //Set the channel color for the name part
      display_set_fg_color(color);
      
      //Fill rounded rectangle for the name part
      display_fill_rounded_rect(x + 3, y + 3, 45, 16, 2);
      
      //Use black text and font_0 for the name
      display_set_fg_color(0x00000000);
      display_set_font(&font_0);
      display_text(x + 6, y + 3, (char *)measurement_names[i]);

      //Measurement in white text
      display_set_fg_color(0x00FFFFFF);
      
      //Erase any previous text
      measurementtext[0] = 0;
      
      //Display the data for this measurement
      measurementfunctions[i](settings);
      
      //Display the formated data
      display_text(x + 55, y + 3, measurementtext);

      //Select next column
      x += 120;
      
      //Check if past the last for this row
      if(x > xe)
      {
        //Back to beginning of the rows
        x = xstart;
        
        //Up to the row above
        y -= 24;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vmax(PCHANNELSETTINGS settings)
{
  //For the maximum take of the center ADC value
  scope_display_voltage(settings, settings->max - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vmin(PCHANNELSETTINGS settings)
{
  //For the minimum take of the center ADC value
  scope_display_voltage(settings, settings->min - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vavg(PCHANNELSETTINGS settings)
{
  //For the average take of the center ADC value
  scope_display_voltage(settings, settings->average - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vrms(PCHANNELSETTINGS settings)
{
  //The rms has already been centered during the summation so use it as is
  scope_display_voltage(settings, settings->rms);

#if 0
  //This code has moved to FPGA file and has been improved
  //Determine the two absolute extremes
  int32 min = 128 - settings->min;
  int32 max = settings->max - 128;
  int32 vrms = max;
  
  //Use the biggest of the two
  if(min > max)
  {
    vrms = min;
  }
  
  //Calculate the root mean square
  vrms = (vrms * 7071) / 10000;
  
  //Use the below the center value when it is the biggest
  scope_display_voltage(settings, vrms);
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vpp(PCHANNELSETTINGS settings)
{
  //For the peak peak just use the value as is
  scope_display_voltage(settings, settings->peakpeak);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_vp(PCHANNELSETTINGS settings)
{
  //Determine the two absolute extremes
  int32 min = 128 - settings->min;
  int32 max = settings->max - 128;
  
  //Display the biggest of the two
  if(min > max)
  {
    //Use the below the center value when it is the biggest
    scope_display_voltage(settings, min);
  }
  else
  {
    //Use the above the center value when it is the biggest
    scope_display_voltage(settings, max);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_freq(PCHANNELSETTINGS settings)
{
  if(settings->frequencyvalid)
  {
    //Format the frequency for displaying
    scope_print_value(measurementtext, settings->frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "", "Hz");
  }
  else
  {
    strcpy(measurementtext, "xxxHz");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_cycle(PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    scope_print_value(measurementtext, (((uint64)settings->periodtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "", "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_time_plus(PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    scope_print_value(measurementtext, (((uint64)settings->hightime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "", "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_time_min(PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    scope_print_value(measurementtext, (((uint64)settings->lowtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "", "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_duty_plus(PCHANNELSETTINGS settings)
{
  char *buffer;
  
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    buffer = scope_print_decimal(measurementtext, (((uint64)settings->hightime * 1000) / settings->periodtime), 1, 0);
    
    //Add the duty cycle sign
    strcpy(buffer, "%");
    
  }
  else
  {
    strcpy(measurementtext, "xx%");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_duty_min(PCHANNELSETTINGS settings)
{
  char *buffer;
  
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    buffer = scope_print_decimal(measurementtext, (((uint64)settings->lowtime * 1000) / settings->periodtime), 1, 0);
    
    //Add the duty cycle sign
    strcpy(buffer, "%");
    
  }
  else
  {
    strcpy(measurementtext, "xx%");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Simple non optimized function for string copy that returns the position of the terminator
//----------------------------------------------------------------------------------------------------------------------------------

char *strcpy(char *dst, const char *src)
{
  while(*src)
  {
    *dst++ = *src++;
  }

  //Terminate the copy
  *dst = 0;

  return(dst);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_voltage(PCHANNELSETTINGS settings, int32 value)
{
  PVOLTCALCDATA vcd;
  int32         volts;
  
  //Calculate the voltage based on the channel settings
  vcd = (PVOLTCALCDATA)&volt_calc_data[settings->magnification][settings->displayvoltperdiv];

  //Adjust the data for the correct voltage per div setting
  volts = (value * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Scale the data based on the two volt per div settings when they differ
  //This is needed when the screen is frozen and zooming is applied
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    volts = (volts * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Multiply with the scaling factor for the channel settings
  volts *= vcd->mul_factor;

  //Format the voltage for displaying
  scope_print_value(measurementtext, volts, vcd->volt_scale, "", "V");
}

//----------------------------------------------------------------------------------------------------------------------------------


void scope_print_value(char *buffer, int32 value, uint32 scale, char *header, char *sign)
{
  uint32 negative = 0;
  
  //Copy the header into the string buffer
  buffer = strcpy(buffer, header);

  //Check if negative value
  if(value < 0)
  {
    //Negate if so and signal negative sign needed
    value = -value;
    negative = 1;
  }
  
  //Need to find the magnitude scale for the input
  //The calculations are based on fixed point
  while(value >= 100000)
  {
    //Skip to the next magnitude
    scale++;

    //Bring the value in range
    value /= 1000;
  }

  //Format the remainder for displaying. Only 3 digits are allowed to be displayed
  if(value < 1000)
  {
    //Less then 1000 means x.yy
    buffer = scope_print_decimal(buffer, value, 2, negative);
  }
  else if(value < 10000)
  {
    //More then 1000 but less then 10000 means xx.y
    value /= 10;
    buffer = scope_print_decimal(buffer, value, 1, negative);
  }
  else
  {
    //More then 10000 and less then 100000 means xxx
    value /= 100;
    buffer = scope_print_decimal(buffer, value, 0, negative);
  }

  //Make sure scale is not out of range
  if(scale > 7)
  {
    scale = 7;
  }

  //Add the magnitude scaler
  buffer = strcpy(buffer, magnitude_scaler[scale]);

  //Add the type of measurement sign
  strcpy(buffer, sign);
}

//----------------------------------------------------------------------------------------------------------------------------------

char *scope_print_decimal(char *buffer, int32 value, uint32 decimals, uint32 negative)
{
  char   b[12];
  uint32 i = 12;   //Start beyond the array since the index is pre decremented
  uint32 s;

  //For value 0 no need to do the work
  if(value == 0)
  {
    //Value is zero so just set 0 character
    b[--i] = '0';
  }
  else
  {
    //Process the digits
    while(value)
    {
      //Set current digit to decreased index
      b[--i] = (value % 10) + '0';

      //Check if decimal point needs to be placed
      if(i == 12 - decimals)
      {
        //If so put it in
        b[--i] = '.';
      }

      //Take of the current digit
      value /= 10;
    }
  }

  //Check if negative number and if so put a minus in front of it
  if(negative)
    b[--i] = '-';
  
  //Determine the size of the string
  s = 12 - i;

  //Copy to the buffer
  memcpy(buffer, &b[i], s);

  //terminate the string
  buffer[s] = 0;

  //Return the position of the terminator to allow appending
  return(&buffer[s]);
}
#endif
//----------------------------------------------------------------------------------------------------------------------------------
// Configuration data functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_load_configuration_data(void)
{
#if 0
  //Get the settings data form the flash memory
  sys_spi_flash_read(0x001FD000, (uint8 *)settingsworkbuffer, sizeof(settingsworkbuffer));
#else
  //Load the settings data from its sector on the SD card
  if(sd_card_read(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer) != SD_OK)
  {
    settingsworkbuffer[2] = 0;
    settingsworkbuffer[3] = 0;
  }
#endif

  //Restore the settings from the loaded data
  scope_restore_config_data();

  //Set the FPGA commands for channel 1
  scopesettings.channel1.enablecommand     = 0x02;
  scopesettings.channel1.couplingcommand   = 0x34;
  scopesettings.channel1.voltperdivcommand = 0x33;
  scopesettings.channel1.offsetcommand     = 0x32;
  scopesettings.channel1.adc1command       = 0x20;
  scopesettings.channel1.adc2command       = 0x21;

  //Set the menu data for channel 1
  scopesettings.channel1.color            = CHANNEL1_COLOR;
  scopesettings.channel1.highlightboxdata = &channel_1_highlight_box;
  scopesettings.channel1.boxdata          = &channel_1_box;
  scopesettings.channel1.boxtext          = &channel_1_box_text;
  scopesettings.channel1.infoxpos         = 517;
  scopesettings.channel1.infoypos         = 6;
  
  //Set the trace and display buffer pointers for channel 1
  scopesettings.channel1.tracebuffer  = (uint8 *)channel1tracebuffer;
  scopesettings.channel1.tracepoints = channel1pointsbuffer;

  //Set the FPGA commands for channel 2
  scopesettings.channel2.enablecommand     = 0x03;
  scopesettings.channel2.couplingcommand   = 0x37;
  scopesettings.channel2.voltperdivcommand = 0x36;
  scopesettings.channel2.offsetcommand     = 0x35;
  scopesettings.channel2.adc1command       = 0x22;
  scopesettings.channel2.adc2command       = 0x23;

  //Set the menu and button data for channel 2
  scopesettings.channel2.color            = CHANNEL2_COLOR;
  scopesettings.channel2.highlightboxdata = &channel_2_highlight_box;
  scopesettings.channel2.boxdata          = &channel_2_box;
  scopesettings.channel2.boxtext          = &channel_2_box_text;
  scopesettings.channel2.infoxpos         = 622;
  scopesettings.channel2.infoypos         = 6;

  //Set the trace and display buffer pointers for channel 2
  scopesettings.channel2.tracebuffer = (uint8 *)channel2tracebuffer;
  scopesettings.channel2.tracepoints = channel2pointsbuffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_configuration_data(void)
{
  //Save the settings for writing to the flash
  scope_save_config_data();

#if 0
  //Write it to the flash
  sys_spi_flash_write(0x001FD000, (uint8 *)settingsworkbuffer, sizeof(settingsworkbuffer));
#else
  //Write the data to its sector on the SD card
  sd_card_write(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer);
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_reset_config_data(void)
{
  uint32 index;
  
  //Load a default configuration in case of settings in flash being corrupted

  //Enable channel 1, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in top part of the screen
  scopesettings.channel1.enable            = 1;
  scopesettings.channel1.coupling          = 0;
  scopesettings.channel1.magnification     = 0;
  scopesettings.channel1.displayvoltperdiv = 0;
  scopesettings.channel1.samplevoltperdiv  = 0;
  scopesettings.channel1.fftenable         = 0;
  scopesettings.channel1.traceposition     = 300;

  //Enable channel 2, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in bottom part of the screen
  scopesettings.channel2.enable            = 1;
  scopesettings.channel2.coupling          = 0;
  scopesettings.channel2.magnification     = 0;
  scopesettings.channel2.displayvoltperdiv = 0;
  scopesettings.channel2.samplevoltperdiv  = 0;
  scopesettings.channel2.fftenable         = 0;
  scopesettings.channel2.traceposition     = 100;

  //Set trigger mode to auto, trigger edge to rising, trigger channel to channel 1, trigger position and trigger screen offset to center of the screen
  scopesettings.triggermode     = 0;
  scopesettings.triggeredge     = 0;
  scopesettings.triggerchannel  = 0;
  scopesettings.triggerhorizontalposition = 362;
  scopesettings.triggerverticalposition   = 200;

  //Set move speed to fast
  scopesettings.movespeed = 0;
  userinterfacedata.movespeed = 10;

  //Set time base to 20uS/div
  scopesettings.timeperdiv = 12;

  //Set the related acquisition speed which is 5MHz
  scopesettings.samplerate = 5;

  //Enable some default measurements
  scopesettings.measurementitems[0].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[0].channel         = 0;
  scopesettings.measurementitems[0].index           = 4;                            //Vpp

  scopesettings.measurementitems[1].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[1].channel         = 0;
  scopesettings.measurementitems[1].index           = 2;                            //Vavg

  scopesettings.measurementitems[2].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[2].channel         = 0;
  scopesettings.measurementitems[2].index           = 6;                            //Freq
  
  scopesettings.measurementitems[3].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[3].channel         = 1;
  scopesettings.measurementitems[3].index           = 4;                            //Vpp

  scopesettings.measurementitems[4].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[4].channel         = 1;
  scopesettings.measurementitems[4].index           = 2;                            //Vavg

  scopesettings.measurementitems[5].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[5].channel         = 1;
  scopesettings.measurementitems[5].index           = 6;                            //Freq

  //Turn time cursor off and set some default positions
  scopesettings.timecursorsenable   = 0;
  scopesettings.timecursor1position = 183;
  scopesettings.timecursor2position = 547;

  //Turn volt cursor of and set some default positions
  scopesettings.voltcursorsenable   = 0;
  scopesettings.voltcursor1position = 167;
  scopesettings.voltcursor2position = 328;

  //Set screen brightness to high, grid brightness to low, always 50% trigger off, x-y display mode off and confirmation mode enabled
  scopesettings.screenbrightness = 100;
  scopesettings.gridbrightness   = 25;
  scopesettings.alwaystrigger50  = 0;
  scopesettings.xymodedisplay    = 0;
  scopesettings.confirmationmode = 1;
  
  //Set default channel calibration values
  for(index=0;index<7;index++)
  {
    //Set FPGA center level
    scopesettings.channel1.dc_calibration_offset[index] = 750;
    scopesettings.channel2.dc_calibration_offset[index] = 750;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_config_data(void)
{
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Set a version number for checking if the settings match the current firmware
  settingsworkbuffer[2] = SETTING_SECTOR_VERSION_HIGH;
  settingsworkbuffer[3] = SETTING_SECTOR_VERSION_LOW;
  
  //Point to the channel 1 settings
  ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];
  
  //Save the channel 1 settings
  *ptr++ = scopesettings.channel1.enable;
  *ptr++ = scopesettings.channel1.coupling;
  *ptr++ = scopesettings.channel1.magnification;
  *ptr++ = scopesettings.channel1.displayvoltperdiv;
  *ptr++ = scopesettings.channel1.samplevoltperdiv;
  *ptr++ = scopesettings.channel1.fftenable;
  *ptr++ = scopesettings.channel1.traceposition;

  //Point to the channel 2 settings
  ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];
  
  //Save the channel 2 settings
  *ptr++ = scopesettings.channel2.enable;
  *ptr++ = scopesettings.channel2.coupling;
  *ptr++ = scopesettings.channel2.magnification;
  *ptr++ = scopesettings.channel2.displayvoltperdiv;
  *ptr++ = scopesettings.channel2.samplevoltperdiv;
  *ptr++ = scopesettings.channel2.fftenable;
  *ptr++ = scopesettings.channel2.traceposition;

  //Point to the trigger settings
  ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];
  
  //Save trigger settings
  *ptr++ = scopesettings.timeperdiv;
  *ptr++ = scopesettings.samplerate;
  *ptr++ = scopesettings.triggermode;
  *ptr++ = scopesettings.triggeredge;
  *ptr++ = scopesettings.triggerchannel;
  *ptr++ = scopesettings.triggerlevel;
  *ptr++ = scopesettings.triggerhorizontalposition;
  *ptr++ = scopesettings.triggerverticalposition;
  
  //Point to the other settings
  ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];
  
  //Save the other settings
  *ptr++ = scopesettings.movespeed;
  *ptr++ = scopesettings.confirmationmode;
  *ptr++ = scopesettings.screenbrightness;
  *ptr++ = scopesettings.gridbrightness;
  *ptr++ = scopesettings.alwaystrigger50;
  *ptr++ = scopesettings.xymodedisplay;

  //Point to the cursor settings
  ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];
  
  //Save the time cursor settings
  *ptr++ = scopesettings.timecursorsenable;
  *ptr++ = scopesettings.timecursor1position;
  *ptr++ = scopesettings.timecursor2position;

  //Save the volt cursor settings
  *ptr++ = scopesettings.voltcursorsenable;
  *ptr++ = scopesettings.voltcursor1position;
  *ptr++ = scopesettings.voltcursor2position;

  //Point to the first measurement enable setting
  ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

  //Save the measurement slots states
  for(index=0;index<6;index++)
  {
    //Copy the current measurement channel and index
    *ptr++ = scopesettings.measurementitems[index].channel;
    *ptr++ = scopesettings.measurementitems[index].index;
  }
  
  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

  //Copy the working set values to the saved values
  for(index=0;index<6;index++,ptr++)
  {
    //Copy the data for both channels
    ptr[0] = scopesettings.channel1.dc_calibration_offset[index];
    ptr[6] = scopesettings.channel2.dc_calibration_offset[index];
  }

  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];
  
  //Save the ADC compensation values
  *ptr++ = scopesettings.channel1.adc1compensation;
  *ptr++ = scopesettings.channel1.adc2compensation;
  *ptr++ = scopesettings.channel2.adc1compensation;
  *ptr++ = scopesettings.channel2.adc2compensation;
  
  //Calculate a checksum over the settings data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Save the checksum
  settingsworkbuffer[0] = checksum >> 16;
  settingsworkbuffer[1] = checksum;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_restore_config_data(void)
{
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Calculate a checksum over the loaded data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Check if the checksum is a match as well as the version number
  if((settingsworkbuffer[0] == (checksum >> 16)) && (settingsworkbuffer[1] == (checksum & 0xFFFF)) && (settingsworkbuffer[2] == SETTING_SECTOR_VERSION_HIGH) && (settingsworkbuffer[3] == SETTING_SECTOR_VERSION_LOW))
  {
    //Point to the channel 1 settings
    ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];

    //Restore the channel 1 settings
    scopesettings.channel1.enable            = *ptr++;
    scopesettings.channel1.coupling          = *ptr++;
    scopesettings.channel1.magnification     = *ptr++;
    scopesettings.channel1.displayvoltperdiv = *ptr++;
    scopesettings.channel1.samplevoltperdiv  = *ptr++;
    scopesettings.channel1.fftenable         = *ptr++;
    scopesettings.channel1.traceposition     = *ptr++;

    //Point to the channel 2 settings
    ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];

    //Restore the channel 2 settings
    scopesettings.channel2.enable            = *ptr++;
    scopesettings.channel2.coupling          = *ptr++;
    scopesettings.channel2.magnification     = *ptr++;
    scopesettings.channel2.displayvoltperdiv = *ptr++;
    scopesettings.channel2.samplevoltperdiv  = *ptr++;
    scopesettings.channel2.fftenable         = *ptr++;
    scopesettings.channel2.traceposition     = *ptr++;

    //Point to the trigger settings
    ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];

    //Restore trigger settings
    scopesettings.timeperdiv                = *ptr++;
    scopesettings.samplerate                = *ptr++;
    scopesettings.triggermode               = *ptr++;
    scopesettings.triggeredge               = *ptr++;
    scopesettings.triggerchannel            = *ptr++;
    scopesettings.triggerlevel              = *ptr++;
    scopesettings.triggerhorizontalposition = *ptr++;
    scopesettings.triggerverticalposition   = *ptr++;

    //Point to the other settings
    ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];

    //Restore the other settings
    scopesettings.movespeed        = *ptr++;
    scopesettings.confirmationmode = *ptr++;
    scopesettings.screenbrightness = *ptr++;
    scopesettings.gridbrightness   = *ptr++;
    scopesettings.alwaystrigger50  = *ptr++;
    scopesettings.xymodedisplay    = *ptr++;
    
    //Point to the cursor settings
    ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];

    //Restore the time cursor settings
    scopesettings.timecursorsenable   = *ptr++;
    scopesettings.timecursor1position = *ptr++;
    scopesettings.timecursor2position = *ptr++;

    //Restore the volt cursor settings
    scopesettings.voltcursorsenable   = *ptr++;
    scopesettings.voltcursor1position = *ptr++;
    scopesettings.voltcursor2position = *ptr++;

    //Point to the first measurement enable setting
    ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

    //Restore the measurement slots states
    for(index=0;index<6;index++)
    {
      //Copy the current measurement channel and index
      scopesettings.measurementitems[index].channel = *ptr++;
      scopesettings.measurementitems[index].index   = *ptr++;
      
      //Set the pointer to the actual channel data based on the selected channel
      if(scopesettings.measurementitems[index].channel == 0)
      {
        scopesettings.measurementitems[index].channelsettings = &scopesettings.channel1;
      }
      else
      {
        scopesettings.measurementitems[index].channelsettings = &scopesettings.channel2;
      }
    }
    
    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

    //Restore the working set values from the saved values
    for(index=0;index<6;index++,ptr++)
    {
      //Copy the data for both channels
      scopesettings.channel1.dc_calibration_offset[index] = ptr[0];
      scopesettings.channel2.dc_calibration_offset[index] = ptr[6];
    }

    //The last entry is a copy of the fore last value
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];
    
    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];

    //Restore the ADC compensation values
    scopesettings.channel1.adc1compensation = *ptr++;
    scopesettings.channel1.adc2compensation = *ptr++;
    scopesettings.channel2.adc1compensation = *ptr++;
    scopesettings.channel2.adc2compensation = *ptr++;
    
    //Update the user interface data based on the retrieved settings
    //Set the actual movement speed in the user interface data
    if(scopesettings.movespeed == 0)
    {
      //Fast speed selected means taking 10 pixel steps
      userinterfacedata.movespeed = 10;
    }
    else
    {
      //Slow speed selected means taking 1 pixel steps
      userinterfacedata.movespeed = 1;
    }
    
    //Navigation function pointers need to be set based on the cursor measurements
  }
  else
  {
    //Load a default set on failure
    scope_reset_config_data();
    
    //Save it to the SD card
    scope_save_configuration_data();
  }
}


//----------------------------------------------------------------------------------------------------------------------------------
