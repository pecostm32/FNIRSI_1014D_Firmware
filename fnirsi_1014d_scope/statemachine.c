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

NAVIGATIONFUNCTION mainmenustartactions[] =
{
  sm_open_picture_file_viewing,              //Picture browsing
  sm_open_waveform_file_viewing,             //Wave browsing
  0,                                         //Output browsing
  0,                                         //Capture output
  sm_open_brightness_setting,                //Screen brightness
  sm_open_brightness_setting,                //Scale (grid) brightness
  0,                                         //Automatic 50%
  0,                                         //X-Y mode curve
  sm_do_base_calibration,                    //Base calibration
  sm_start_usb_export,                       //USB export
  0                                          //Factory settings
};

//----------------------------------------------------------------------------------------------------------------------------------

void sm_init(void)
{
  //On startup the scope is in normal working state, so sampling and displaying enabled
  enablesampling     = SAMPLING_ENABLED;
  enabletracedisplay = TRACE_DISPLAY_ENABLED;
  
  //For the user input only the basic scope control buttons and rotary dials are active
  navigationstate = NAV_NO_ACTION;
  fileviewstate   = FILE_VIEW_NO_ACTION;
  buttondialstate = BUTTON_DIAL_NORMAL_HANDLING;
}

//----------------------------------------------------------------------------------------------------------------------------------
//State machine handling functions
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_user_input(void)
{
  //Get the latest command to be processed
  if(uart1_get_user_input() == 0)
  {
    //No active command so skip the rest
    return;
  }
  
  //Set the action value for the add or subtract commands
  switch(toprocesscommand)
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
  
  //Handle the navigation state first
  switch(navigationstate)
  {
    //Time and volt cursor handling
    case NAV_TIME_VOLT_CURSOR_HANDLING:
      sm_handle_time_volt_cursor();
      break;
      
    //Main menu handling
    case NAV_MAIN_MENU_HANDLING:
      sm_handle_main_menu_actions();
      break;
      
    //File view handling
    case NAV_FILE_VIEW_HANDLING:
      sm_handle_file_view_actions();
      break;
      
    //File view select state handling
    case NAV_FILE_VIEW_SELECT_HANDLING:
      sm_handle_file_view_select_actions();
      break;
      
    //Picture view handling
    case NAV_ITEM_VIEW_HANDLING:
      sm_handle_item_view_actions();
      break;
      
    //Slider adjust handling
    case NAV_SLIDER_HANDLING:
      sm_handle_slider_actions();
      break;
  }
  
  //Handle the file view actions second
  switch(fileviewstate)
  {
    case FILE_VIEW_DEFAULT_CONTROL:
      sm_handle_file_view_control();
      break;
      
    case FILE_VIEW_SELECT_CONTROL:
      sm_handle_file_view_select_control();
      break;
      
    case FILE_VIEW_ITEM_CONTROL:
      sm_handle_item_view_control();
      break;
  }

  //Handle the button and dial actions last
  switch(buttondialstate)
  {
    case BUTTON_DIAL_NORMAL_HANDLING:
      sm_button_dial_normal_handling();
      break;

    case BUTTON_DIAL_MENU_HANDLING:
      sm_button_dial_menu_handling();
      break;

    case BUTTON_DIAL_FILE_VIEW_HANDLING:
      sm_button_dial_file_view_handling();
      break;

    case BUTTON_DIAL_PICTURE_VIEW_HANDLING:
      sm_button_dial_picture_view_handling();
      break;

    case BUTTON_DIAL_WAVE_VIEW_HANDLING:
      sm_button_dial_wave_view_handling();
      break;
  }
  
  //Signal the active command has been processed
  toprocesscommand = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------
//Navigation handling functions
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_time_volt_cursor(void)
{
  //For the left time cursor navigation only the right button and the rotary dial are active
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NAV_LEFT:
      //Select the left time cursor if enabled
      if(scopesettings.timecursorsenable)
      {
        userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
      }
      break;
      
    case UIC_BUTTON_NAV_RIGHT:
      //Select the right time cursor if enabled
      if(scopesettings.timecursorsenable)
      {
        userinterfacedata.selectedcursor = CURSOR_TIME_RIGHT;
      }
      break;
      
    case UIC_BUTTON_NAV_UP:
      //Select the top volt cursor if enabled
      if(scopesettings.voltcursorsenable)
      {
        userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
      }
      break;

    case UIC_BUTTON_NAV_DOWN:
      //Select the bottom volt cursor if enabled
      if(scopesettings.voltcursorsenable)
      {
        userinterfacedata.selectedcursor = CURSOR_VOLT_BOTTOM;
      }
      break;
      
    case UIC_ROTARY_SEL_ADD:
    case UIC_ROTARY_SEL_SUB:
      switch(userinterfacedata.selectedcursor)
      {
        case CURSOR_TIME_LEFT:
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
          break;
          
        case CURSOR_TIME_RIGHT:
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
          break;
          
        case CURSOR_VOLT_TOP:
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
          break;
          
        case CURSOR_VOLT_BOTTOM:
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
         break;
      }
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_main_menu_actions(void)
{
  //With the navigation actions the menu list can be traversed and the active option can be started
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NAV_LEFT:
      sm_close_menu();
      break;

    case UIC_BUTTON_NAV_OK:
    case UIC_BUTTON_NAV_RIGHT:
      //If there is a start action handler set for this menu option, execute it
      if(mainmenustartactions[userinterfacedata.menuitem])
        mainmenustartactions[userinterfacedata.menuitem]();
      break;

    case UIC_BUTTON_NAV_UP:
    case UIC_BUTTON_NAV_DOWN:
    case UIC_ROTARY_SEL_ADD:
    case UIC_ROTARY_SEL_SUB:
      sm_select_main_menu_item();
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_file_view_actions(void)
{
  //With the navigation actions the item list can be traversed and the active item can be opened or selected
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NAV_OK:
      sm_open_file_view_item();
      break;
      
    case UIC_ROTARY_SEL_SUB:
    case UIC_BUTTON_NAV_LEFT:
      sm_file_view_goto_previous_item();
      break;

    case UIC_ROTARY_SEL_ADD:
    case UIC_BUTTON_NAV_RIGHT:
      sm_file_view_goto_next_item();
      break;

    case UIC_BUTTON_NAV_UP:
      sm_file_view_goto_previous_row();
      break;
      
    case UIC_BUTTON_NAV_DOWN:
      sm_file_view_goto_next_row();
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_file_view_select_actions(void)
{
  //With the navigation actions the item list can be traversed and the active item can be opened or selected
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NAV_OK:
      //Toggle the selected state for this item
      viewitemselected[viewcurrentindex % VIEW_ITEMS_PER_PAGE] ^= 1;
      
      //Go and show the item as selected
      ui_display_thumbnails();
      break;
      
    case UIC_ROTARY_SEL_SUB:
    case UIC_BUTTON_NAV_LEFT:
      sm_file_view_goto_previous_item_on_page();
      break;

    case UIC_ROTARY_SEL_ADD:
    case UIC_BUTTON_NAV_RIGHT:
      sm_file_view_goto_next_item_on_page();
      break;

    case UIC_BUTTON_NAV_UP:
      sm_file_view_goto_previous_row_on_page();
      break;
      
    case UIC_BUTTON_NAV_DOWN:
      sm_file_view_goto_next_row_on_page();
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_item_view_actions(void)
{
  //With the navigation actions the item list can be traversed, opening them in series
  switch(toprocesscommand)
  {
    case UIC_ROTARY_SEL_SUB:
    case UIC_BUTTON_NAV_LEFT:
      sm_item_view_goto_previous_item();
      break;

    case UIC_ROTARY_SEL_ADD:
    case UIC_BUTTON_NAV_RIGHT:
      sm_item_view_goto_next_item();
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_slider_actions(void)
{
  //With the navigation actions the slider can be closed or adjusted
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NAV_OK:
    case UIC_BUTTON_NAV_LEFT:
      sm_slider_close();
      break;

    case UIC_ROTARY_SEL_ADD:
    case UIC_ROTARY_SEL_SUB:
      sm_slider_adjust();
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//File view handling functions
//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_file_view_control(void)
{
  //Check the buttons for the file view actions and handle them accordingly
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NEXT:
      //Select the next page
      viewcurrentindex += VIEW_ITEMS_PER_PAGE;
      viewpage++;

      //Check if on the last page and beyond the last item on that page
      if((viewpage == viewpages) && (viewcurrentindex >= viewavailableitems))
      {
        //If so use the last item on the page
        viewcurrentindex = viewavailableitems - 1;
      }
      //Else check if page rolls over
      else if(viewpage > viewpages)
      {
        //Fall back to the first page
        viewcurrentindex = viewcurrentindex % VIEW_ITEMS_PER_PAGE;
        viewpage = 0;
      }

      //Go and highlight the indicated item
      ui_display_thumbnails();
      break;

    case UIC_BUTTON_PREVIOUS:
      //Select the previous page
      viewcurrentindex -= VIEW_ITEMS_PER_PAGE;
      viewpage--;

      //Check if underflow through to last page
      if(viewpage < 0)
      {
        //If so select item on the last page
        viewcurrentindex = viewavailableitems - 1;
        viewpage = viewpages;
      }

      //Go and highlight the indicated item
      ui_display_thumbnails();
      break;

    case UIC_BUTTON_DELETE:
      sm_file_view_delete_current();
      break;
      
    case UIC_BUTTON_SELECT_ALL:
      sm_file_view_process_select(1);
      break;

    case UIC_BUTTON_SELECT:
      sm_file_view_process_select(0);
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_file_view_select_control(void)
{
  //Check the buttons for the file view actions and handle them accordingly
  switch(toprocesscommand)
  {
    case UIC_BUTTON_DELETE:
      sm_file_view_delete_selected();
      break;

    case UIC_BUTTON_SELECT_ALL:
      sm_file_view_process_select(1);
      break;

    case UIC_BUTTON_SELECT:
      sm_file_view_process_select(0);
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_handle_item_view_control(void)
{
  //Check the buttons for the file view actions and handle them accordingly
  switch(toprocesscommand)
  {
    case UIC_BUTTON_NEXT:
      sm_item_view_goto_next_item();
      break;

    case UIC_BUTTON_PREVIOUS:
      sm_item_view_goto_previous_item();
      break;

    case UIC_BUTTON_DELETE:
      sm_item_view_delete_current();
      
      //Check if last item got deleted
      if(viewavailableitems == 0)
      {
        //If so return to the file view state to show the empty screen
        navigationstate = NAV_FILE_VIEW_HANDLING;
        fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
        buttondialstate = BUTTON_DIAL_FILE_VIEW_HANDLING;

        //Display the empty thumbnail page
        ui_display_thumbnails();
      }
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Button and rotary dial handling functions
//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_normal_handling(void)
{
  //Handle the received command
  switch(toprocesscommand)
  {
    case UIC_BUTTON_RUN_STOP:
      //Toggle the run state
      scopesettings.runstate ^= 1;
      
      //Display the new state on the screen
      ui_display_run_stop_text();
      break;

    case UIC_BUTTON_MENU:
      //First item on the list is highlighted
      userinterfacedata.menuitem = 0;
      
      //Switch to the menu handling navigation state
      navigationstate = NAV_MAIN_MENU_HANDLING;
      
      //Switch to button and dial state for menu handling
      buttondialstate = BUTTON_DIAL_MENU_HANDLING;
      
      //Disable sampling and trace displaying
      enablesampling = SAMPLING_NOT_ENABLED;
      enabletracedisplay = TRACE_DISPLAY_NOT_ENABLED;
      
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

      //Enable the navigation state for the cursor handling
      navigationstate = NAV_TIME_VOLT_CURSOR_HANDLING;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.timecursorsenable)
      {
        //Select the left cursor to start with
        userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
      }
      else
      {
        //When the time cursor gets disabled check if the voltage cursor is enabled
        if(scopesettings.voltcursorsenable)
        {
          //Select the top volt cursor if not on the bottom volt cursor 
          if(userinterfacedata.selectedcursor != CURSOR_VOLT_BOTTOM)
          {
            //Select the top volt cursor
            userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
          }
        }
        else
        {
          //No more cursor enabled so no more action in the navigation part
          navigationstate = NAV_NO_ACTION;
        }
      }
      break;

    case UIC_BUTTON_V_CUR:
      //Toggle the vertical cursor state
      scopesettings.voltcursorsenable ^= 1;

      //Enable the navigation state for the cursor handling
      navigationstate = NAV_TIME_VOLT_CURSOR_HANDLING;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.voltcursorsenable)
      {
        //Select the top volt cursor to start with
        userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
      }
      else
      {
        //When the volt cursor gets disabled check if the time cursor is enabled
        if(scopesettings.timecursorsenable)
        {
          //Select the left time cursor if not on the right time cursor 
          if(userinterfacedata.selectedcursor != CURSOR_TIME_RIGHT)
          {
            //Select the left time cursor
            userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
          }
        }
        else
        {
          //No more cursor enabled so no more action in the navigation part
          navigationstate = NAV_NO_ACTION;
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
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_menu_handling(void)
{
  //This depends on the fact that the navigation buttons are in an undivided sequential range to filter them out and respond the same to all other given user input except for the navigation dial commands
  if(((toprocesscommand < UIC_BUTTON_NAV_RIGHT) || (toprocesscommand > UIC_BUTTON_NAV_LEFT)) && (toprocesscommand != UIC_ROTARY_SEL_ADD) && (toprocesscommand != UIC_ROTARY_SEL_SUB))
  {
    //When in a menu state only the navigation keys and rotary dial have dedicated actions. All the others close the menu and return to normal operation
    sm_close_menu();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_file_view_handling(void)
{
  //Return to the previous mode when the menu button is pressed
  if(toprocesscommand == UIC_BUTTON_MENU)
  {
    sm_close_view_screen();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_picture_view_handling(void)
{
  //Return to the previous mode when the menu button is pressed
  if(toprocesscommand == UIC_BUTTON_MENU)
  {
    //Set the file viewing states
    navigationstate = NAV_FILE_VIEW_HANDLING;
    fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
    buttondialstate = BUTTON_DIAL_FILE_VIEW_HANDLING;
    
    //Display the thumbnail page with the current view item selected
    ui_display_thumbnails();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_button_dial_wave_view_handling(void)
{
  //Process the user input as far as is allowed for wave file viewing
  switch(toprocesscommand)
  {
    case UIC_BUTTON_MENU:
      //When the menu button is pressed return to the previous viewing state
      navigationstate = NAV_FILE_VIEW_HANDLING;
      fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
      buttondialstate = BUTTON_DIAL_FILE_VIEW_HANDLING;

      //Disable the trace displaying
      enabletracedisplay = TRACE_DISPLAY_NOT_ENABLED;
      
      //Display the thumbnail page with the current view item selected
      ui_display_thumbnails();
      break;
      
    case UIC_BUTTON_SAVE_PICTURE:
      //Save the screen as bitmap on the SD card
      ui_save_view_item_file(VIEW_TYPE_PICTURE);
      break;

    case UIC_BUTTON_H_CUR:
      //Toggle the horizontal cursor state
      scopesettings.timecursorsenable ^= 1;

      //Enable the navigation state for the cursor handling
      navigationstate = NAV_TIME_VOLT_CURSOR_HANDLING;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.timecursorsenable)
      {
        //Select the left cursor to start with
        userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
      }
      else
      {
        //When the time cursor gets disabled check if the voltage cursor is enabled
        if(scopesettings.voltcursorsenable)
        {
          //Select the top volt cursor if not on the bottom volt cursor 
          if(userinterfacedata.selectedcursor != CURSOR_VOLT_BOTTOM)
          {
            //Select the top volt cursor
            userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
          }
        }
        else
        {
          //No more cursor enabled so no more action in the navigation part
          navigationstate = NAV_NO_ACTION;
        }
      }
      break;

    case UIC_BUTTON_V_CUR:
      //Toggle the vertical cursor state
      scopesettings.voltcursorsenable ^= 1;

      //Enable the navigation state for the cursor handling
      navigationstate = NAV_TIME_VOLT_CURSOR_HANDLING;
      
      //Take needed actions when the cursor is enabled
      if(scopesettings.voltcursorsenable)
      {
        //Select the top volt cursor to start with
        userinterfacedata.selectedcursor = CURSOR_VOLT_TOP;
      }
      else
      {
        //When the volt cursor gets disabled check if the time cursor is enabled
        if(scopesettings.timecursorsenable)
        {
          //Select the left time cursor if not on the right time cursor 
          if(userinterfacedata.selectedcursor != CURSOR_TIME_RIGHT)
          {
            //Select the left time cursor
            userinterfacedata.selectedcursor = CURSOR_TIME_LEFT;
          }
        }
        else
        {
          //No more cursor enabled so no more action in the navigation part
          navigationstate = NAV_NO_ACTION;
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

    case UIC_BUTTON_CH2_ENABLE:
      scopesettings.channel2.enable ^= 1;
      
      //Update the information part to show the channel is either disabled or enabled
      ui_display_channel_settings(&scopesettings.channel2);
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
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Functions to handle specific tasks
//----------------------------------------------------------------------------------------------------------------------------------

void sm_close_menu(void)
{
  //Enable sampling and display tracing
  enablesampling = SAMPLING_ENABLED;
  enabletracedisplay = TRACE_DISPLAY_ENABLED;

  //Switch back to normal button and dial handling
  buttondialstate = BUTTON_DIAL_NORMAL_HANDLING;
  
  //Disable file view handling
  fileviewstate = FILE_VIEW_NO_ACTION;

  //Set the navigation state based on enabled cursors
  sm_restore_navigation_handling();

  //Redraw the outline to ensure proper screen after having menu open
  ui_draw_outline();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_restore_navigation_handling(void)
{
  //Set the navigation state based on enabled cursors
  if((scopesettings.timecursorsenable) || (scopesettings.voltcursorsenable))
  {
    //At least one cursor is enabled so allow handling them
    navigationstate = NAV_TIME_VOLT_CURSOR_HANDLING;
  }
  else
  {
    //No cursor enabled so no navigation handling needed
    navigationstate = NAV_NO_ACTION;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------------------------

void sm_select_main_menu_item(void)
{
  //Reset the highlight on the previous selected item
  ui_unhighlight_main_menu_item();
  
  //Adjust the setting based on the set value
  userinterfacedata.menuitem -= userinterfacedata.setvalue;
  
  //Keep it in range of the item list. Fixed on 11 items including 0 for now. Should make it a define.
  //Or based on the function pointer array
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

  //Set the highlight on the current selected item
  ui_highlight_main_menu_item();
}

//----------------------------------------------------------------------------------------------------------------------------------
//The next functions are for opening and closing the file viewing page
//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_file_view(void)
{

  //Start with the first file highlighted
  viewcurrentindex = 0;
  viewpage = 0;

  //Go and setup everything to view the available items
  ui_setup_view_screen();
  
  //Set specific handling for the general scope control buttons and dials
  buttondialstate = BUTTON_DIAL_FILE_VIEW_HANDLING;

  //Check if there are available items to look through
  if(viewavailableitems)
  {
    //If so, set the proper states for it
    navigationstate = NAV_FILE_VIEW_HANDLING;
    fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
  }
  else
  {
    //If not, only allowing returning to the main operational mode makes sense
    navigationstate = NAV_NO_ACTION;
    fileviewstate   = FILE_VIEW_NO_ACTION;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_close_view_screen(void)
{
  //Enable sampling and display tracing
  enablesampling = SAMPLING_ENABLED;
  enabletracedisplay = TRACE_DISPLAY_ENABLED;

  //Switch back to normal button and dial handling
  buttondialstate = BUTTON_DIAL_NORMAL_HANDLING;
  
  //Disable file view handling
  fileviewstate = FILE_VIEW_NO_ACTION;

  //Set the navigation state based on enabled cursors
  sm_restore_navigation_handling();
  
  //Restore the normal scope screen
  ui_close_view_screen();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_file_view_item(void)
{
  //When the OK button has been pressed try to open the file based on its type
  //On failure an error message will be displayed and the scope remains in the file view state it was in
  switch(viewtype)
  {
    case VIEW_TYPE_PICTURE:
      //Try to load the bitmap
      if(ui_load_bitmap_data() == VIEW_BITMAP_LOAD_OK)
      {
        //If all went well set the handling states for picture viewing
        navigationstate = NAV_ITEM_VIEW_HANDLING;
        fileviewstate   = FILE_VIEW_ITEM_CONTROL;
        buttondialstate = BUTTON_DIAL_PICTURE_VIEW_HANDLING;
      }
      else
      {
        //Redraw the thumbnails since the current one should not be there
        ui_initialize_and_display_thumbnails();
      }
      break;
      
    case VIEW_TYPE_WAVEFORM:
      //try to load the wave file
      if(ui_load_trace_data() == VIEW_TRACE_LOAD_OK)
      {
        //Set the handling states for picture viewing
        navigationstate = NAV_ITEM_VIEW_HANDLING;
        fileviewstate   = FILE_VIEW_ITEM_CONTROL;
        buttondialstate = BUTTON_DIAL_WAVE_VIEW_HANDLING;
      }
      else
      {
        //Redraw the thumbnails since the current one should not be there
        ui_initialize_and_display_thumbnails();
      }
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//The next set of functions browse through the view items across all the available pages
//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_next_item(void)
{
  //Select the next item
  viewcurrentindex++;
  
  //Check if in range of the available items
  if(viewcurrentindex >= viewavailableitems)
  {
    //Fall back to the first item
    viewcurrentindex = 0;
    viewpage = 0;
  }
  //Check if overflow to next page
  else if(viewcurrentindex >= ((viewpage * VIEW_ITEMS_PER_PAGE) + viewitemsonpage))
  {
    //Jump to the next page if so
    viewpage++;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_previous_item(void)
{
  //Select the previous item
  viewcurrentindex--;
  
  //Check if it underflows
  if(viewcurrentindex < 0)
  {
    //If so roll over to the last item
    viewcurrentindex = viewavailableitems - 1;
    viewpage = viewpages;
  }
  //Check if underflow to previous page
  else if(viewcurrentindex < (viewpage * VIEW_ITEMS_PER_PAGE))
  {
    //Jump to the previous page if so
    viewpage--;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_next_row(void)
{
  //Select the next row
  viewcurrentindex += VIEW_ITEMS_PER_ROW;
  
  //Check if in range of the available items
  if(viewcurrentindex >= viewavailableitems)
  {
    //Fall back to the first item in the active row
    viewcurrentindex = viewcurrentindex % VIEW_ITEMS_PER_ROW;
    viewpage = 0;
  }
  //Check if overflow to next page
  else if(viewcurrentindex >= ((viewpage * VIEW_ITEMS_PER_PAGE) + viewitemsonpage))
  {
    //Jump to the next page if so
    viewpage++;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_previous_row(void)
{
  int16 activerow;
  int16 newrow;
  
  //Get the row number of the current highlighted item
  activerow = (viewcurrentindex % VIEW_ITEMS_PER_ROW);
  
  //Select the previous row
  viewcurrentindex -= VIEW_ITEMS_PER_ROW;
  
  //Check if in range of the available items
  if(viewcurrentindex < 0)
  {
    //Roll over to the last item in the active row. Remainder of a negative value is negative
    viewcurrentindex = viewavailableitems - 1;
    viewpage = viewpages;
    
    //Get the row number of the new item
    newrow = viewcurrentindex % VIEW_ITEMS_PER_ROW;
    
    //Check if the new row is beyond the active row
    if(newrow > activerow)
    {
      //If so take of the difference between the two to get into the right row
      viewcurrentindex -= (newrow - activerow);
    }
    //If not check if it is before the active row
    else if(newrow < activerow)
    {
      //If so skip to the required one by taking of a number based on the delta
      viewcurrentindex -= (4 - (activerow - newrow));
      
      //Get the index of the first item on the last page to see if the new index is on the previous page
      if(viewcurrentindex < (viewpage * VIEW_ITEMS_PER_PAGE))
      {
        //if so jump to the previous page
        viewpage--;
      }
    }
  }
  //Check if underflow to previous page  
  else if(viewcurrentindex < (viewpage * VIEW_ITEMS_PER_PAGE))
  {
    //Jump to the previous page if so
    viewpage--;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_delete_current(void)
{
  //Ask the user if the current item should be deleted
  if(ui_handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
  {
    //User opted for delete so do this for the current item
    ui_remove_item_from_thumbnails(1);

    //Save the thumbnail file
    ui_save_thumbnail_file();

    //Go and highlight the next item
    ui_display_thumbnails();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//The next set of functions browse through the view items but stay on the current page
//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_next_item_on_page(void)
{
  int16 firstitemonpage = viewpage * VIEW_ITEMS_PER_PAGE;
  int16 lastitemonpage = firstitemonpage + 16;
  
  //Last item needs to be limited to the available items
  if(lastitemonpage > viewavailableitems)
  {
    lastitemonpage = viewavailableitems;
  }
    
  //Select the next item
  viewcurrentindex++;
  
  //Check if in range of the available items on the page
  if(viewcurrentindex >= lastitemonpage)
  {
    //Fall back to the first item
    viewcurrentindex = firstitemonpage;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_previous_item_on_page(void)
{
  int16 firstitemonpage = viewpage * VIEW_ITEMS_PER_PAGE;
  int16 lastitemonpage = firstitemonpage + 16;
  
  //Last item needs to be limited to the available items
  if(lastitemonpage > viewavailableitems)
  {
    lastitemonpage = viewavailableitems;
  }
    
  //Select the next item
  viewcurrentindex--;
  
  //Check if in range of the available items on the page
  if(viewcurrentindex < firstitemonpage)
  {
    //Fall back to the last item
    viewcurrentindex = lastitemonpage - 1;
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_next_row_on_page(void)
{
  int16 firstitemonpage = viewpage * VIEW_ITEMS_PER_PAGE;
  int16 lastitemonpage = firstitemonpage + 16;
  
  //Last item needs to be limited to the available items
  if(lastitemonpage > viewavailableitems)
  {
    lastitemonpage = viewavailableitems;
  }

  //Select the next row
  viewcurrentindex += VIEW_ITEMS_PER_ROW;
  
  //Check if in range of the available items
  if(viewcurrentindex >= lastitemonpage)
  {
    //Fall back to the first item in the active row
    viewcurrentindex = firstitemonpage + (viewcurrentindex % VIEW_ITEMS_PER_ROW);
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_goto_previous_row_on_page(void)
{
  int16 activerow;
  int16 newrow;
  
  int16 firstitemonpage = viewpage * VIEW_ITEMS_PER_PAGE;
  int16 lastitemonpage = firstitemonpage + 16;
  
  //Last item needs to be limited to the available items
  if(lastitemonpage > viewavailableitems)
  {
    lastitemonpage = viewavailableitems;
  }

  //Get the row number of the current highlighted item
  activerow = (viewcurrentindex % VIEW_ITEMS_PER_ROW);
  
  //Select the previous row
  viewcurrentindex -= VIEW_ITEMS_PER_ROW;
  
  //Check if in range of the available items
  if(viewcurrentindex < firstitemonpage)
  {
    //Roll over to the last item in the active row. Remainder of a negative value is negative
    viewcurrentindex = lastitemonpage - 1;
    
    //Get the row number of the new item
    newrow = viewcurrentindex % VIEW_ITEMS_PER_ROW;
    
    //Check if the new row is beyond the active row
    if(newrow > activerow)
    {
      //If so take of the difference between the two to get into the right row
      viewcurrentindex -= (newrow - activerow);
    }
    //If not check if it is before the active row
    else if(newrow < activerow)
    {
      //If so skip to the required one by taking of a number based on the delta
      viewcurrentindex -= (4 - (activerow - newrow));
    }
  }
  
  //Go and highlight the indicated item
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_process_select(uint32 selectall)
{
  //Depending on the current state take action
  if((viewselectmode == VIEW_SELECT_NONE) || ((viewselectmode == VIEW_SELECT_INDIVIDUAL) && selectall))
  {
    //Check if select all needed
    if(selectall)
    {
      //If so, set all items as selected
      memset(viewitemselected, VIEW_ITEM_SELECTED, viewitemsonpage);

      //Switch to all items select mode
      viewselectmode = VIEW_SELECT_ALL;
    }
    else
    {
      //Switch to individual item select mode
      viewselectmode = VIEW_SELECT_INDIVIDUAL;
    }
    
    //Switch to handling the select state
    navigationstate = NAV_FILE_VIEW_SELECT_HANDLING;
    fileviewstate   = FILE_VIEW_SELECT_CONTROL;
  }
  else
  {
    //Switch to normal mode
    viewselectmode = VIEW_SELECT_NONE;

    //Not in a selected mode any more so clear the selected items
    memset(viewitemselected, VIEW_ITEM_NOT_SELECTED, viewitemsonpage);
    
    //Switch back to handling the basic view state
    navigationstate = NAV_FILE_VIEW_HANDLING;
    fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
  }
      
  //Update the page to show this
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_file_view_delete_selected(void)
{
  int32  index;
  uint32 found;
  
  //Need to see if there are items selected for delete
  if(viewselectmode)
  {
    //A select mode is active so check the list to see if there are items selected
    //This is not done in the original code. Activate the single select mode and without selecting an item press delete. The confirm menu is shown
    for(index=0,found=0;index<viewitemsonpage;index++)
    {
      //Check if the current item is selected
      if(viewitemselected[index] == VIEW_ITEM_SELECTED)
      {
        //Signal there is at least one item to delete
        found = 1;
        break;
      }
    }

    //Check if there is an item to delete
    if(found)
    {
      //Ask the user if the items should be deleted
      if(ui_handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
      {
        //User opted for delete so do this for the selected items
        //Start with the last item on the page to avoid problems with the file number list being modified
        for(index=viewitemsonpage-1;index>=0;index--)
        {
          //Check if the current item is selected
          if(viewitemselected[index] == VIEW_ITEM_SELECTED)
          {
            //Set the current index for this file
            viewcurrentindex = index + (viewpage * VIEW_ITEMS_PER_PAGE);

            //Remove the current item from the thumbnails and delete the item from disk
            ui_remove_item_from_thumbnails(1);
          }
        }

        //Save the thumbnail file
        ui_save_thumbnail_file();

        //Clear the select flags
        memset(viewitemselected, VIEW_ITEM_NOT_SELECTED, viewitemsonpage);

        //Clear the select state and the button highlights
        viewselectmode = VIEW_SELECT_NONE;

        //Redisplay the thumbnails
        ui_initialize_and_display_thumbnails();
        
        //Switch back to normal view mode
        viewselectmode = VIEW_SELECT_NONE;

        //Switch back to handling the basic view state
        navigationstate = NAV_FILE_VIEW_HANDLING;
        fileviewstate   = FILE_VIEW_DEFAULT_CONTROL;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Next functions are for browsing through the pictures one by one instead of on the overview pages
//----------------------------------------------------------------------------------------------------------------------------------

void sm_item_view_delete_current(void)
{
  //Ask the user if the current item should be deleted
  if(ui_handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
  {
    //User opted for delete so do this for the current item
    ui_remove_item_from_thumbnails(1);

    //Save the thumbnail file
    ui_save_thumbnail_file();

    //Need to decrement the index first because the next function increments it
    viewcurrentindex--;

    //The next function handles missing files so used here to show the next available one
    sm_item_view_goto_next_item();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_item_view_goto_next_item(void)
{
  uint32 retval = 1;
  
  //Try opening until successful or out of items. Success is based on a return value of zero
  //Out of items can happen when the last file has been deleted with he sm_item_view_delete_current function
  do
  {
    //Select the next picture
    viewcurrentindex++;

    //Check if in range of the available items
    if(viewcurrentindex >= viewavailableitems)
    {
      //Fall back to the first item
      viewcurrentindex = 0;
      viewpage = 0;
    }
    //Check if overflow to next page
    else if(viewcurrentindex >= ((viewpage * VIEW_ITEMS_PER_PAGE) + viewitemsonpage))
    {
      //Jump to the next page if so
      viewpage++;
    }

    //Make sure there is an item available
    if(viewavailableitems)
    {
      //Try to open the next item
      switch(viewtype)
      {
        case VIEW_TYPE_PICTURE:
          retval = ui_load_bitmap_data();
          break;

        case VIEW_TYPE_WAVEFORM:
          retval = ui_load_trace_data();
          break;
      }
    }
  }
  while(retval && viewavailableitems);
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_item_view_goto_previous_item(void)
{
  uint32 retval = 1;
  
  //Try opening until successful or out of items. Success is based on a return value of zero
  //Out of items cannot happen here, but still checking on it to be safe
  do
  {
    //Select the previous picture
    viewcurrentindex--;

    //Check if it underflows
    if(viewcurrentindex < 0)
    {
      //If so roll over to the last item
      viewcurrentindex = viewavailableitems - 1;
      viewpage = viewpages;
    }
    //Check if underflow to previous page
    else if(viewcurrentindex < (viewpage * VIEW_ITEMS_PER_PAGE))
    {
      //Jump to the previous page if so
      viewpage--;
    }

    //Make sure there is an item available
    if(viewavailableitems)
    {
      //Open the next item
      switch(viewtype)
      {
        case VIEW_TYPE_PICTURE:
          retval = ui_load_bitmap_data();
          break;

        case VIEW_TYPE_WAVEFORM:
          retval = ui_load_trace_data();
          break;
      }
    }
  }
  while(retval && viewavailableitems);
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_slider_close(void)
{
  //Start with the assumption that the screen brightness option is selected
  uint16 y = SLIDER_SCREEN_YPOS;
  
  //Switch back to menu navigation state
  navigationstate = NAV_MAIN_MENU_HANDLING;
  
  //Signal no more slider data to be updated
  sliderdata = 0;

  //Check if the scale (grid) brightness option is selected
  if(userinterfacedata.menuitem == MAIN_MENU_GRID_BRIGHTNESS)
  {
    //If so use the Y position for that one
    y = SLIDER_GRID_YPOS;
  }
  
  //Close the slider panel
  ui_close_slider(SLIDER_XPOS, y);
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_slider_adjust(void)
{
  //Start with the assumption that the screen brightness option is selected
  uint16 y = SLIDER_SCREEN_YPOS;
  
  //Slider data needs to be set for the execution of this code
  if(sliderdata)
  {
    //Adjust the slider data for the current action
    *sliderdata += userinterfacedata.setvalue;

    //Limit with the allowable range
    if(*sliderdata < 0)
    {
      *sliderdata = 0;
    }
    else if(*sliderdata > 100)
    {
      *sliderdata = 100;
    }

    //Check if the scale (grid) brightness option is selected
    if(userinterfacedata.menuitem == MAIN_MENU_GRID_BRIGHTNESS)
    {
      //If so use the Y position for that one
      y = SLIDER_GRID_YPOS;
    }

    //Check if the screen brightness option is selected
    if(userinterfacedata.menuitem == MAIN_MENU_SCREEN_BRIGHTNESS)
    {
      //Show the new setting of the slider
      ui_display_slider(SLIDER_XPOS, y);

      //If so write the new value to the FPGA
      fpga_set_translated_brightness();
    }
    else
    {
      //For the grid brightness showing the adjusted setting directly in the background the screen has to be redrawn
      scope_display_trace_data();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Next functions are for executing main menu items
//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_picture_file_viewing(void)
{
  //Signal viewing of pictures
  viewtype = VIEW_TYPE_PICTURE;

  //Open the file viewing screen
  sm_open_file_view();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_waveform_file_viewing(void)
{
  //Signal viewing of pictures
  viewtype = VIEW_TYPE_WAVEFORM;
  
  //Open the file viewing screen
  sm_open_file_view();
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_open_brightness_setting(void)
{
  uint16 y;
  
  //Switch to the slider handling navigation state
  navigationstate = NAV_SLIDER_HANDLING;
  
  //Check if the screen brightness option is selected
  if(userinterfacedata.menuitem == MAIN_MENU_SCREEN_BRIGHTNESS)
  {
    //If so set the screen brightness variable to be adjusted
    sliderdata = &scopesettings.screenbrightness;
    
    //Set the y position for opening the slider next to this menu item
    y = SLIDER_SCREEN_YPOS;
  }
  else
  {
    //Only other option with the slider is the grid brightness so set that variable to be adjusted
    sliderdata = &scopesettings.gridbrightness;
    
    //Set the y position for opening the slider next to this menu item
    y = SLIDER_GRID_YPOS;
  }
  
  //Show the slider with the current setting and save the background for closing
  ui_open_slider(SLIDER_XPOS, y, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_do_base_calibration(void)
{
  //On selection of this menu item show the user to disconnect the probes and press ok
  ui_show_calibration_message(CALIBRATION_STATE_START);
  
  //Wait until the user provides input
  uart1_wait_for_user_input();
  
  //Check if it is ok to proceed
  if(lastreceivedcommand == UIC_BUTTON_NAV_OK)
  {
    //Show the user the scope is busy with calibrating
    ui_show_calibration_message(CALIBRATION_STATE_BUSY);
    
    //Perform the calibration
    if(scope_do_baseline_calibration() == 1)
    {
      //Show that it completed with success
      ui_show_calibration_message(CALIBRATION_STATE_SUCCESS);
    }
    else
    {
      //Show that it failed
      ui_show_calibration_message(CALIBRATION_STATE_FAIL);
    }

    //Wait for a second
    timer0_delay(1000);
    
    //Close the main menu and return to the normal operational state
    sm_close_menu();
  }
  else
  {
    //Hide the message and call it the day
    ui_show_calibration_message(CALIBRATION_STATE_HIDE);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void sm_start_usb_export(void)
{
  //Signal open command has been processed
  toprocesscommand = 0;
  
  //Cancel navigation actions
  userinterfacedata.navigationfunctions = 0;
  
  //Reset menu state since it is no longer open
  scopesettings.menustate = 0;
  
  //Open the connection
  ui_setup_usb_screen();
  
  //Signal cancel command has been processed
  toprocesscommand = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------
