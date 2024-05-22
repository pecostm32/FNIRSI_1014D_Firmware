//----------------------------------------------------------------------------------------------------------------------------------

#include "statemachine.h"
#include "timer.h"
#include "uart.h"
#include "fpga_control.h"
#include "user_interface_functions.h"
#include "scope_functions.h"
#include "display_lib.h"

#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------
//Navigation action structures for switching between different functionality
//----------------------------------------------------------------------------------------------------------------------------------

NAVIGATIONFUNCTIONS lefttimecursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  0,                                      //up;
  0,                                      //down;
  0,                                      //ok;
  sm_move_left_time_cursor_position       //dial;
};

NAVIGATIONFUNCTIONS righttimecursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  0,                                      //up;
  0,                                      //down;
  0,                                      //ok;
  sm_move_right_time_cursor_position      //dial;
};

NAVIGATIONFUNCTIONS topvoltcursor =
{
  0,                                      //left;
  0,                                      //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_top_volt_cursor_position        //dial;
};

NAVIGATIONFUNCTIONS bottomvoltcursor =
{
  0,                                      //left;
  0,                                      //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_bottom_volt_cursor_position     //dial;
};

NAVIGATIONFUNCTIONS lefttimevoltcursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_left_time_cursor_position       //dial;
};

NAVIGATIONFUNCTIONS righttimevoltcursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_right_time_cursor_position      //dial;
};

NAVIGATIONFUNCTIONS topvolttimecursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_top_volt_cursor_position        //dial;
};

NAVIGATIONFUNCTIONS bottomvolttimecursor =
{
  sm_select_left_time_cursor,             //left;
  sm_select_right_time_cursor,            //right;
  sm_select_top_volt_cursor,              //up;
  sm_select_bottom_volt_cursor,           //down;
  0,                                      //ok;
  sm_move_bottom_volt_cursor_position     //dial;
};

//----------------------------------------------------------------------------------------------------------------------------------

NAVIGATIONFUNCTIONS mainmenuactions[] =
{
  {                                         //Picture browsing
    sm_close_menu,                          //left;
    sm_open_picture_view_screen,            //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    sm_open_picture_view_screen,            //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Wave browsing
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Output browsing
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Capture output
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Screen brightness
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Scale brightness
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Automatic 50%
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //X-Y mode curve
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Base calibration
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //USB export
    sm_close_menu,                          //left;
    sm_start_usb_export,                    //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    sm_start_usb_export,                    //ok;
    sm_select_main_menu_item                //dial;
  },
  {                                         //Factory settings
    0,                                      //left;
    0,                                      //right;
    sm_select_main_menu_item,               //up;
    sm_select_main_menu_item,               //down;
    0,                                      //ok;
    sm_select_main_menu_item                //dial;
  },
};


//----------------------------------------------------------------------------------------------------------------------------------
//State machine handling functions
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_user_input(void)
{
  //Get the latest command to be processed
  if(uart1_get_data() == 0)
  {
    //No active command so skip the rest
    return;
  }
  
  //Set the action value for the add or subtract commands
  switch(userinterfacedata.command)
  {
    //For all the addition actions the set and speed value need to be positive;
    case UIC_BUTTON_NAV_UP:
    case UIC_ROTARY_SEL_ADD:
    case UIC_ROTARY_SCALE_CH1_ADD:
    case UIC_ROTARY_SCALE_CH2_ADD:
    case UIC_ROTARY_TIME_ADD:
    case UIC_ROTARY_CH1_POS_ADD:
    case UIC_ROTARY_CH2_POS_ADD:
    case UIC_ROTARY_TRIG_POS_ADD:
    case UIC_ROTARY_TRIG_LEVEL_ADD:
      userinterfacedata.setvalue = 1;
      userinterfacedata.speedvalue = userinterfacedata.movespeed;
      break;
      
    //For all the subtraction actions the set and speed value need to be negative;
    case UIC_BUTTON_NAV_DOWN:
    case UIC_ROTARY_SEL_SUB:
    case UIC_ROTARY_SCALE_CH1_SUB:
    case UIC_ROTARY_SCALE_CH2_SUB:
    case UIC_ROTARY_TIME_SUB:
    case UIC_ROTARY_CH1_POS_SUB:
    case UIC_ROTARY_CH2_POS_SUB:
    case UIC_ROTARY_TRIG_POS_SUB:
    case UIC_ROTARY_TRIG_LEVEL_SUB:
      userinterfacedata.setvalue = -1;
      userinterfacedata.speedvalue = userinterfacedata.movespeed * -1;
      break;
  }
  
  //Check if navigation is activated
  if(userinterfacedata.navigationfunctions)
  {
    //Check on navigation input first
    switch(userinterfacedata.command)
    {
      case UIC_BUTTON_NAV_LEFT:
        if(userinterfacedata.navigationfunctions->left)
        {
          userinterfacedata.navigationfunctions->left();
        }
        break;

      case UIC_BUTTON_NAV_UP:
        if(userinterfacedata.navigationfunctions->up)
        {
          userinterfacedata.navigationfunctions->up();
        }
        break;

      case UIC_BUTTON_NAV_OK:
        if(userinterfacedata.navigationfunctions->ok)
        {
          userinterfacedata.navigationfunctions->ok();
        }
        break;

      case UIC_BUTTON_NAV_DOWN:
        if(userinterfacedata.navigationfunctions->down)
        {
          userinterfacedata.navigationfunctions->down();
        }
        break;

      case UIC_BUTTON_NAV_RIGHT:
        if(userinterfacedata.navigationfunctions->right)
        {
          userinterfacedata.navigationfunctions->right();
        }
        break;

      case UIC_ROTARY_SEL_ADD:
      case UIC_ROTARY_SEL_SUB:
        if(userinterfacedata.navigationfunctions->dial)
        {
          userinterfacedata.navigationfunctions->dial();
        }
        break;

      default:
        //Need handling of picture and wave file functions at this point
//viewactive can be checked for handling things        
        
        //When a basic command is received check if a menu is open
        if(scopesettings.menustate)
        {
          //If so close the menu and restore the previous functionality
          sm_close_menu();
          
          //No further handling needed
          return;
        }
        break;
    }  
  }
  
  //Handle the received command
  switch(userinterfacedata.command)
  {
    case UIC_BUTTON_RUN_STOP:
      //Toggle the run state
      scopesettings.runstate ^= 1;
      
      //Display the new state on the screen
      ui_display_run_stop_text();
      break;

    case UIC_BUTTON_MENU:
      //Signal the menu is opened
      scopesettings.menustate = 1;
      
      //First item on the list is highlighted
      userinterfacedata.menuitem = 0;
      
      //Hook in the navigation actions
      userinterfacedata.navigationfunctions = &mainmenuactions[userinterfacedata.menuitem];
      
      //Display the main menu on the screen
      ui_display_main_menu();
      break;
      
    case UIC_BUTTON_SAVE_PICTURE:
      //Save the screen as bitmap on the SD card
      ui_save_view_item_file(VIEW_TYPE_PICTURE);
      break;

    case UIC_BUTTON_SAVE_WAVE:
      //Save the screen as bitmap on the SD card
      ui_save_view_item_file(VIEW_TYPE_WAVEFORM);
      break;

    case UIC_BUTTON_H_CUR:
      //Toggle the horizontal cursor state
      scopesettings.timecursorsenable ^= 1;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.timecursorsenable)
      {
        //Select the left cursor
        userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
      
        //Set the needed navigation functions based on the enabled cursors
        if(scopesettings.voltcursorsenable)
        {
          //Both cursor set enabled so functions for both cursors and moving the left time cursor to start with
          userinterfacedata.navigationfunctions = &lefttimevoltcursor;
        }
        else
        {
          //Single cursor set enabled so only functions for the left time cursor to start with
          userinterfacedata.navigationfunctions = &lefttimecursor;
        }
      }
      else
      {
        //When the time cursor gets disabled check if the voltage cursor is enabled
        if(scopesettings.voltcursorsenable)
        {
          //Set the needed action set based on the selected cursor
          if(userinterfacedata.selectedcursor == CURSOR_VOLT_BOTTOM)
          {
            //Just set the navigation functions for the bottom voltage cursor
            userinterfacedata.navigationfunctions = &bottomvoltcursor;
          }
          else
          {
            //Select the cursor and set the navigation functions for just the top voltage cursor
            userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
            userinterfacedata.navigationfunctions = &topvoltcursor;
          }
        }
        else
        {
          //No action connected to the navigation part
          userinterfacedata.navigationfunctions = 0;
        }
      }
      break;

    case UIC_BUTTON_V_CUR:
      //Toggle the vertical cursor state
      scopesettings.voltcursorsenable ^= 1;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.voltcursorsenable)
      {
        //Select the left cursor
        userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
      
        //Set the needed navigation functions based on the enabled cursors
        if(scopesettings.timecursorsenable)
        {
          //Both cursor set enabled so functions for both cursors and moving the top volt cursor to start with
          userinterfacedata.navigationfunctions = &topvolttimecursor;
        }
        else
        {
          //Single cursor set enabled so only functions for the top volt cursor to start with
          userinterfacedata.navigationfunctions = &topvoltcursor;
        }
      }
      else
      {
        //When the volt cursor gets disabled check if the time cursor is enabled
        if(scopesettings.timecursorsenable)
        {
          //Set the needed action set based on the selected cursor
          if(userinterfacedata.selectedcursor == CURSOR_TIME_RIGHT)
          {
            //Just set the navigation functions for the bottom voltage cursor
            userinterfacedata.navigationfunctions = &righttimecursor;
          }
          else
          {
            //Select the cursor and set the navigation functions for just the top voltage cursor
            userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
            userinterfacedata.navigationfunctions = &lefttimecursor;
          }
        }
        else
        {
          //No action connected to the navigation part
          userinterfacedata.navigationfunctions = 0;
        }
      }
      break;
      
    case UIC_BUTTON_MOVE_SPEED:
      //Toggle the move speed
      scopesettings.movespeed ^= 1;
      
      //Display the new speed on the screen
      ui_display_move_speed();
      
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
      break;

    case UIC_BUTTON_CH1_ENABLE:
      scopesettings.channel1.enable ^= 1;
      
      //Update the information part to show the channel is either disabled or enabled
      ui_display_channel_settings(&scopesettings.channel1);
      break;

    case UIC_BUTTON_CH1_CONF:
      break;
      
    case UIC_BUTTON_CH2_ENABLE:
      scopesettings.channel2.enable ^= 1;
      
      //Update the information part to show the channel is either disabled or enabled
      ui_display_channel_settings(&scopesettings.channel2);
      break;
      
    case UIC_BUTTON_CH2_CONF:
      break;
      
    case UIC_BUTTON_TRIG_ORIG:
      //Reset the trigger position and level to center positions
      scopesettings.triggerhorizontalposition = 342;
      
      //Setting is done based on the selected trigger channel
      sm_do_50_percent_trigger_setup();
      break;
      
    case UIC_BUTTON_TRIG_MODE:
      //Step through the trigger modes
      scopesettings.triggermode++;
      scopesettings.triggermode %= 3;
      
      //Display the new mode on the screen
      ui_display_trigger_mode();
      break;
      
    case UIC_BUTTON_TRIG_EDGE:
      //Toggle the trigger edge
      scopesettings.triggeredge ^= 1;
      
      //Display the new edge on the screen
      ui_display_trigger_edge();
      break;
      
    case UIC_BUTTON_TRIG_CHX:
      //Toggle the trigger edge
      scopesettings.triggerchannel ^= 1;
      
      //Display the new edge on the screen
      ui_display_trigger_channel();
      break;

    case UIC_BUTTON_TRIG_50_PERCENT:
      //Setting is done based on the selected trigger channel
      sm_do_50_percent_trigger_setup();
      break;
      
    case UIC_ROTARY_CH1_POS_ADD:
    case UIC_ROTARY_CH1_POS_SUB:
      sm_set_channel_position(&scopesettings.channel1);
      break;
      
    case UIC_ROTARY_CH2_POS_ADD:
    case UIC_ROTARY_CH2_POS_SUB:
      sm_set_channel_position(&scopesettings.channel2);
      break;
      
    case UIC_ROTARY_TRIG_POS_ADD:
    case UIC_ROTARY_TRIG_POS_SUB:
      sm_set_trigger_position();
      break;
      
    case UIC_ROTARY_TRIG_LEVEL_ADD:
    case UIC_ROTARY_TRIG_LEVEL_SUB:
      sm_set_trigger_level();
      break;
      
    case UIC_ROTARY_SCALE_CH1_ADD:
    case UIC_ROTARY_SCALE_CH1_SUB:
      sm_set_channel_sensitivity(&scopesettings.channel1);
      break;
      
    case UIC_ROTARY_SCALE_CH2_ADD:
    case UIC_ROTARY_SCALE_CH2_SUB:
      sm_set_channel_sensitivity(&scopesettings.channel2);
      break;

    case UIC_ROTARY_TIME_ADD:
    case UIC_ROTARY_TIME_SUB:
      sm_set_time_base();
      break;

//    case :
//      break;
  }
  
  //Signal the active command has been processed
  userinterfacedata.command = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_set_trigger_position(void)
{
  //Adjust the setting based on the given value
  scopesettings.triggerhorizontalposition += userinterfacedata.speedvalue;

  //Check if still in allowed range
  if(scopesettings.triggerhorizontalposition < 0)
  {
    //Limit it on the minimum range if needed
    scopesettings.triggerhorizontalposition = 0;
  }
  else if(scopesettings.triggerhorizontalposition > 685)
  {
    //Limit it on maximum range if needed
    scopesettings.triggerhorizontalposition = 685;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_set_trigger_level(void)
{
  //Adjust the setting based on the given value
  scopesettings.triggerverticalposition += userinterfacedata.speedvalue;

  //Check if still in allowed range
  if(scopesettings.triggerverticalposition < 15)
  {
    //Limit it on the minimum range if needed
    scopesettings.triggerverticalposition = 15;
  }
  else if(scopesettings.triggerverticalposition > 399)
  {
    //Limit it on maximum range if needed
    scopesettings.triggerverticalposition = 399;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_set_channel_sensitivity(PCHANNELSETTINGS settings)
{
  //Adjust the setting based on the given input
  uint8 newvalue = settings->displayvoltperdiv + userinterfacedata.setvalue;
  
  //Check if not outside of the settings
  if((newvalue >= 0) && (newvalue <= 6))
  {
    //Update the setting with the new value
    settings->displayvoltperdiv = newvalue;
    
    //Show the new setting on the screen
    ui_display_channel_sensitivity(settings);

    //Only update the FPGA in run mode
    //For waveform view mode the stop state is forced and can't be changed
    if(scopesettings.runstate == 0)
    {
      //Copy the display setting to the sample setting
      settings->samplevoltperdiv = settings->displayvoltperdiv;

      //Set the volts per div for this channel
      fpga_set_channel_voltperdiv(settings);

      //Since the DC offset is influenced set that too
      fpga_set_channel_offset(settings);

      //Wait 50ms to allow the circuit to settle
      timer0_delay(50);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_set_time_base(void)
{
  //Adjust the setting based on the given input
  uint8 newvalue = scopesettings.timeperdiv + userinterfacedata.setvalue;
  
  //Check if not already on the lowest setting (10nS/div)
  if((newvalue >= 0) && (newvalue <= ((sizeof(time_div_texts) / sizeof(int8 *)) - 1)))
  {
    //Go down in time by adding one to the setting
    scopesettings.timeperdiv = newvalue;

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
    ui_display_time_per_division();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_set_channel_position(PCHANNELSETTINGS settings)
{
  //Adjust the setting based on the set speed value
  settings->traceposition += userinterfacedata.speedvalue;

  //Check if still in allowed range
  if(settings->traceposition < 15)
  {
    //Limit it on the minimum range if needed
    settings->traceposition = 15;
  }
  else if(settings->traceposition > 399)
  {
    //Limit it on maximum range if needed
    settings->traceposition = 399;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_do_50_percent_trigger_setup(void)
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

void sm_move_left_time_cursor_position(void)
{
  //Adjust the setting based on the set speed value
  scopesettings.timecursor1position += userinterfacedata.speedvalue;

  //Limit it on the trace portion of the screen and the right time cursor
  if(scopesettings.timecursor1position < 6)
  {
    //So not below the left side of the region
    scopesettings.timecursor1position = 6;
  }
  else if(scopesettings.timecursor1position >= scopesettings.timecursor2position)
  {
    //And not right of the right cursor;
    scopesettings.timecursor1position = scopesettings.timecursor2position - 1;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_move_right_time_cursor_position(void)
{
  //Adjust the setting based on the set speed value
  scopesettings.timecursor2position += userinterfacedata.speedvalue;

  //Limit it on the trace portion of the screen and the left time cursor
  if(scopesettings.timecursor2position <= scopesettings.timecursor1position)
  {
    //So not to the left of or on the left cursor
    scopesettings.timecursor2position = scopesettings.timecursor1position + 1;
  }
  else if(scopesettings.timecursor2position > 704)
  {
    //And not beyond the edge of the screen;
    scopesettings.timecursor2position = 704;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_left_time_cursor(void)
{
  //Select the left time cursor and set the movement function for it
  userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;

  //Set the needed navigation functions based on the enabled cursors
  if(scopesettings.voltcursorsenable)
  {
    //Both cursor set enabled so functions for both cursors and moving the left time cursor to start with
    userinterfacedata.navigationfunctions = &lefttimevoltcursor;
  }
  else
  {
    //Single cursor set enabled so only functions for the left time cursor to start with
    userinterfacedata.navigationfunctions = &lefttimecursor;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_right_time_cursor(void)
{
  //Select the right time cursor and set the movement function for it
  userinterfacedata.selectedcursor = CURSOR_TIME_RIGHT;

  //Set the needed navigation functions based on the enabled cursors
  if(scopesettings.voltcursorsenable)
  {
    //Both cursor set enabled so functions for both cursors and moving the left time cursor to start with
    userinterfacedata.navigationfunctions = &righttimevoltcursor;
  }
  else
  {
    //Single cursor set enabled so only functions for the left time cursor to start with
    userinterfacedata.navigationfunctions = &righttimecursor;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_move_top_volt_cursor_position(void)
{
  //Adjust the setting based on the set speed value
  scopesettings.voltcursor1position -= userinterfacedata.speedvalue;

  //Limit it on the trace portion of the screen and the bottom volt cursor
  if(scopesettings.voltcursor1position < 59)
  {
    //So not above the top side of the region
    scopesettings.voltcursor1position = 59;
  }
  else if(scopesettings.voltcursor1position >= scopesettings.voltcursor2position)
  {
    //And not below or on the bottom cursor;
    scopesettings.voltcursor1position = scopesettings.voltcursor2position - 1;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_move_bottom_volt_cursor_position(void)
{
  //Adjust the setting based on the set speed value
  scopesettings.voltcursor2position -= userinterfacedata.speedvalue;

  //Limit it on the trace portion of the screen and the top volt cursor
  if(scopesettings.voltcursor2position <= scopesettings.voltcursor1position)
  {
    //And not above or on the top cursor;
    scopesettings.voltcursor2position = scopesettings.voltcursor1position + 1;
  }
  else if(scopesettings.voltcursor2position > 457)
  {
    //So not below the bottom side of the region
    scopesettings.voltcursor2position = 457;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_top_volt_cursor(void)
{
  //Select the top volt cursor and set the movement function for it
  userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
  
  //Set the needed navigation functions based on the enabled cursors
  if(scopesettings.timecursorsenable)
  {
    //Both cursor set enabled so functions for both cursors and moving the top volt cursor to start with
    userinterfacedata.navigationfunctions = &topvolttimecursor;
  }
  else
  {
    //Single cursor set enabled so only functions for the top volt cursor to start with
    userinterfacedata.navigationfunctions = &topvoltcursor;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_bottom_volt_cursor(void)
{
  //Select the bottom volt cursor and set the movement function for it
  userinterfacedata.selectedcursor = CURSOR_VOLT_BOTTOM;
  
  //Set the needed navigation functions based on the enabled cursors
  if(scopesettings.timecursorsenable)
  {
    //Both cursor set enabled so functions for both cursors and moving the top volt cursor to start with
    userinterfacedata.navigationfunctions = &bottomvolttimecursor;
  }
  else
  {
    //Single cursor set enabled so only functions for the top volt cursor to start with
    userinterfacedata.navigationfunctions = &bottomvoltcursor;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_close_menu(void)
{
  //Menu needs to be closed so redraw the outline and the traces
  ui_draw_outline();
  scope_display_trace_data();

  //Signal the menu is closed
  scopesettings.menustate = 0;

  //Signal the active command has been processed
  userinterfacedata.command = 0;

  //Need to revert navigation functionality based on the selected cursor
  //Kill it for now
  userinterfacedata.navigationfunctions = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_main_menu_item(void)
{
  //Reset the highlight on the previous selected item
  ui_unhighlight_main_menu_item();
  
  //Adjust the setting based on the set value
  userinterfacedata.menuitem -= userinterfacedata.setvalue;
  
  //Keep it in range of the item list. Fixed on 11 items including 0 for now. Should make it a define.
  if(userinterfacedata.menuitem < 0)
  {
    //Overflow to the other end of the list
    userinterfacedata.menuitem = 10;
  }
  else if(userinterfacedata.menuitem > 10)
  {
    //Overflow to the other end of the list
    userinterfacedata.menuitem = 0;
  }

  //Set the needed navigation action functionality for the highlighted item
  userinterfacedata.navigationfunctions = &mainmenuactions[userinterfacedata.menuitem];
  
  //Set the highlight on the current selected item
  ui_highlight_main_menu_item();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_start_usb_export(void)
{
  //Signal open command has been processed
  userinterfacedata.command = 0;
  
  //Cancel navigation actions
  userinterfacedata.navigationfunctions = 0;
  
  //Reset menu state since it is no longer open
  scopesettings.menustate = 0;
  
  //Open the connection
  ui_setup_usb_screen();
  
  //Signal cancel command has been processed
  userinterfacedata.command = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_picture_view_screen(void)
{
  //Signal viewing of pictures
  viewtype = VIEW_TYPE_PICTURE;

  //Go and setup everything to view the available picture items
  ui_setup_view_screen();
}

//----------------------------------------------------------------------------------------------------------------------------------
