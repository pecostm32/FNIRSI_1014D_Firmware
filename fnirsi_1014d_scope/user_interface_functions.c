//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "display_lib.h"
#include "timer.h"
#include "variables.h"
#include "uart.h"
#include "usb_interface.h"
#include "user_interface_functions.h"
#include "statemachine.h"
#include "scope_functions.h"
#include "fpga_control.h"

//----------------------------------------------------------------------------------------------------------------------------------
//Simple non optimized function for string copy that returns a pointer to the terminator
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

void ui_setup_display_lib(void)
{
  //Use the main buffer for displaying the oscilloscope screen
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Set the bounding box to avoid writing outside the allocated buffers
  display_set_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_setup_main_screen(void)
{
  //Set black color for background
  display_set_fg_color(COLOR_BLACK);

  //Clear the screen
  display_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  //Fill in all the items on the screen
  ui_draw_outline();
  ui_draw_grid();
  ui_display_logo();
  ui_display_run_stop_text();
  ui_display_trigger_settings();
  ui_display_move_speed();
  ui_display_channel_settings(&scopesettings.channel1);
  ui_display_channel_settings(&scopesettings.channel2);
  ui_display_time_per_division();
  ui_display_waiting_triggered_text(0);
  ui_display_measurements();

  //Show version information
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_2);
  display_text(VERSION_STRING_XPOS, VERSION_STRING_YPOS, VERSION_STRING);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_setup_view_screen(void)
{
  //Load the thumbnail file for the current view type
  if(ui_load_thumbnail_file() != 0)
  {
    //Loading the thumbnail file failed so no sense in going on
    return;
  }

  //Switch to view mode so disallow saving of settings on power down
  viewactive = VIEW_ACTIVE;

  //Set scope run state to running to have it sample fresh data on exit
  scopesettings.runstate = RUN_STATE_RUNNING;

  //Only needed for waveform view. Picture viewing does not change the scope settings
  if(viewtype == VIEW_TYPE_WAVEFORM)
  {
    //Save the current settings
    ui_save_setup(&savedscopesettings1);
  }

  //Initialize the view mode variables
  //Used for indicating if select all or select button is active
  viewselectmode = 0;

  //Start at first page
  viewpage = 0;

  //Clear the item selected flags
  memset(viewitemselected, VIEW_ITEM_NOT_SELECTED, VIEW_ITEMS_PER_PAGE);

  //Display the available thumbnails for the current view type
  ui_initialize_and_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_close_view_screen(void)
{
  //Only needed for waveform view. Picture viewing does not change the scope settings
  if(viewtype == VIEW_TYPE_WAVEFORM)
  {
    //Restore the current settings
    ui_restore_setup(&savedscopesettings1);

    //Make sure view mode is normal
    scopesettings.waveviewmode = 0;

    //And resume with auto trigger mode
    scopesettings.triggermode = 0;

    //Need to restore the original scope data and fpga settings

    //Is also part of startup, so could be done with a function
    //Set the volts per div for each channel based on the loaded scope settings
    fpga_set_channel_voltperdiv(&scopesettings.channel1);
    fpga_set_channel_voltperdiv(&scopesettings.channel2);

    //These are not done in the original code
    //Set the channels AC or DC coupling based on the loaded scope settings
    fpga_set_channel_coupling(&scopesettings.channel1);
    fpga_set_channel_coupling(&scopesettings.channel2);

    //Setup the trigger system in the FPGA based on the loaded scope settings
    fpga_set_sample_rate(scopesettings.samplerate);
    fpga_set_time_base(scopesettings.timeperdiv);
    fpga_set_trigger_channel();
    fpga_set_trigger_edge();
    fpga_set_trigger_level();
    fpga_set_trigger_mode();

    //Set channel screen offsets
    fpga_set_channel_offset(&scopesettings.channel1);
    fpga_set_channel_offset(&scopesettings.channel2);
  }

  //Reset the screen to the normal scope screen
  ui_setup_main_screen();
  scope_display_trace_data();

  //Back to normal mode so allow saving of settings on power down
  viewactive = VIEW_NOT_ACTIVE;
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_setup_usb_screen(void)
{
  //Clear the whole screen
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  //Set the light color for the equipment borders
  display_set_fg_color(COLOR_LIGHT_GREY_A);

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
  display_set_fg_color(COLOR_DARK_BLUE);
  display_fill_rect(477, 125, 235, 163);
  display_fill_rect(88, 210, 163, 112);

  //Draw a dark border around the blue screens
  display_set_fg_color(COLOR_DARK_GREY_1);
  display_draw_rect(477, 125, 235, 163);
  display_draw_rect(88, 210, 163, 112);

  //Start the USB interface
  usb_device_enable();

  //Wait for the user to push a button or rotate a dial on the front panel of the scope
  uart1_wait_for_user_input();

  //Stop the USB interface
  usb_device_disable();

  //Redraw the screen
  ui_setup_main_screen();
  scope_display_trace_data();

  //Re-sync the system files
  ui_sync_thumbnail_files();
}

//----------------------------------------------------------------------------------------------------------------------------------

const uint32 shade_colors[] =
{
  0x00303430,
  0x00303030,
  0x00282C28,
  0x00282828,
  0x00282828,
  0x00202420,
  0x00202020,
  0x00201C20,
  0x00181C18,
  0x00181818,
  0x00181418,
  0x00101010,
  0x00101010,
  0x00080C08,
  0x00080808
};

const uint32 measurement_shade_colors[] =
{
  0x00282C28,
  0x00282428,
  0x00201C20,
  0x00181818,
  0x00101010,
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_outline(void)
{
  int xs1 = TOP_SHADE_LEFT_START;
  int xe1 = TOP_SHADE_LEFT_END;
  int xs2 = TOP_SHADE_RIGHT_START;
  int xe2 = TOP_SHADE_RIGHT_END;
  int xs3 = BOTTOM_SHADE_START;
  int xe3 = BOTTOM_SHADE_END;
  int x,y,yt,yb;

  const uint32 *color = shade_colors;

  //Set the color for drawing the signal display shade
  display_set_fg_color(COLOR_DARK_GREY_2);

  //Draw the outer edge for a bit of shading effect
  display_draw_rect(TRACE_WINDOW_BORDER_XPOS - 1, TRACE_WINDOW_BORDER_YPOS - 1, TRACE_WINDOW_BORDER_WIDTH + 2, TRACE_WINDOW_BORDER_HEIGHT + 2);

  //Set the color for drawing the signal display outline
  display_set_fg_color(COLOR_DARK_GREY_6);

  //Draw the inner edge, the actual brighter border of the trace window
  display_draw_rect(TRACE_WINDOW_BORDER_XPOS, TRACE_WINDOW_BORDER_YPOS, TRACE_WINDOW_BORDER_WIDTH, TRACE_WINDOW_BORDER_HEIGHT);

  //Set the vertical starting positions for drawing the shades
  yt=TRACE_WINDOW_BORDER_YPOS - 1;
  yb=TRACE_WINDOW_BORDER_YPOS + TRACE_WINDOW_BORDER_HEIGHT;

  //Draw top and bottom side shades
  for(;yt>((TRACE_WINDOW_BORDER_YPOS - 1) - (sizeof(shade_colors)/sizeof(uint32)));yt--,yb++)
  {
    display_set_fg_color(*color);

    //Two separate set of lines spaced apart on the top of the outline
    display_draw_horz_line(yt, xs1, xe1);
    display_draw_horz_line(yt, xs2, xe2);

    //One set of lines on the bottom of the outline
    display_draw_horz_line(yb, xs3, xe3);

    //Make a trapezium by shortening and displacing the lines
    xs1++;
    xe1--;
    xs2++;
    xe2--;
    xs3++;
    xe3--;

    //Select the next color to use
    color++;
  }

  //Create the black cutouts in the bottom shade
  display_set_fg_color(COLOR_BLACK);

  //A range of cutouts with an 10 pixel interval
  for(x=BOTTOM_SHADE_CUTOUT_START;x<BOTTOM_SHADE_CUTOUT_END;x+=BOTTOM_SHADE_CUTOUT_STEP)
  {
    display_draw_rect(x, BOTTOM_SHADE_CUTOUT_TOP, BOTTOM_SHADE_CUTOUT_WIDTH, BOTTOM_SHADE_CUTOUT_HEIGHT);
  }

  //Draw the top and bottom edges of the measurement sections
  display_set_fg_color(COLOR_DARK_GREY_3);
  display_fill_rect(711,   0, 83, 2);
  display_fill_rect(711, 477, 83, 2);

  //Next lines are shorter and shifted
  xs1 = 712;
  xe1 = 793;

  //Less lines based on the colors from this table
  color = measurement_shade_colors;

  //Draw the shortening lines with the colors from the given table
  for(yt = 3,yb = 476;yt<(3 + (sizeof(measurement_shade_colors)/sizeof(uint32)));yt++,yb--)
  {
    display_set_fg_color(*color);

    //Lines above and below the center line
    display_draw_horz_line(yt, xs1, xe1);
    display_draw_horz_line(yb, xs1, xe1);

    //Make a trapezium by shortening and displacing the lines
    xs1++;
    xe1--;

    //Select the next color to use
    color++;
  }

  //Draw the five measurement separators
  for(y=80;y<410;y+=80)
  {
    //Draw the center line with the brightest color of the set
    display_set_fg_color(COLOR_DARK_GREY_3);
    display_draw_horz_line(y, 711, 794);

    //Next lines are shorter and shifted
    xs1 = 712;
    xe1 = 793;

    //Less lines based on the colors from this table
    color = measurement_shade_colors;

    //Draw the shortening lines with the colors from the given table
    for(yt = y - 1,yb = y + 1;yb<(y + (sizeof(measurement_shade_colors)/sizeof(uint32)) + 1);yt--,yb++)
    {
      display_set_fg_color(*color);

      //Lines above and below the center line
      display_draw_horz_line(yt, xs1, xe1);
      display_draw_horz_line(yb, xs1, xe1);

      //Make a trapezium by shortening and displacing the lines
      xs1++;
      xe1--;

      //Select the next color to use
      color++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_logo(void)
{
  //Setup a darker gradient for shading the logo text
  display_set_fg_y_gradient(gradientbuffer, 8, 28, 0x009F0000, 0x006C0000);

  //Draw the darker logo shifted a single pixel in both x and y direction
  display_copy_icon_fg_color_y_gradient(peco_logo_icon, 7, 8, 76, 20);

  //Setup a gradient for the actual logo text. Much lighter then the shading
  display_set_fg_y_gradient(gradientbuffer, 7, 27, 0x00FF6060, 0x00CC0000);

  //Draw the actual logo on the available location
  display_copy_icon_fg_color_y_gradient(peco_logo_icon, 6, 7, 76, 20);
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_run_stop_text(void)
{
  const uint8 *icon;

  //Set the colors for clearing the background and filling in the text
  display_set_fg_color(COLOR_WHITE);
  display_set_bg_color(COLOR_BLACK);

  //Select the icon based on the selected edge
  if(scopesettings.runstate == RUN_STATE_RUNNING)
  {
    //Run state selected
    icon = run_text_icon;
  }
  else
  {
    //Stop state selected
    icon = stop_text_icon;
  }

  //Display the text icon with infill of the background since the other text icon needs to be overwritten
  display_copy_icon_use_colors(icon, 6, 33, 35, 13);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_waiting_triggered_text(uint32 state)
{
  const uint8 *icon;

  //Using an icon might be a simpler and faster option
  display_set_fg_color(COLOR_WHITE);
  display_set_bg_color(COLOR_BLACK);

  //Select the icon based on the trigger state
  if(state == 0)
  {
    //Waiting for trigger
    icon = waiting_text_icon;
  }
  else
  {
    //Triggered
    icon = triggered_text_icon;
  }

  //Display the text icon with infill of the background since the other text icon needs to be overwritten
  display_copy_icon_use_colors(icon, 652, 464, 54, 14);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_grid(void)
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

    //Draw the center lines
    display_draw_horz_line(TRACE_VERTICAL_CENTER, TRACE_HORIZONTAL_START, TRACE_HORIZONTAL_END - 1);
    display_draw_vert_line(TRACE_HORIZONTAL_CENTER, TRACE_VERTICAL_START, TRACE_VERTICAL_END - 1);

    //Draw the ticks on the x line
    for(i=DOT_HORIZONTAL_START;i<TRACE_HORIZONTAL_END-1;i+=DOT_SPACING)
    {
      display_draw_vert_line(i, TRACE_VERTICAL_CENTER - 2, TRACE_VERTICAL_CENTER + 2);
    }

    //Draw the ticks on the y line
    for(i=DOT_VERTICAL_START;i<TRACE_VERTICAL_END-1;i+=DOT_SPACING)
    {
      display_draw_horz_line(i, TRACE_HORIZONTAL_CENTER - 2, TRACE_HORIZONTAL_CENTER + 2);
    }

    //Draw the horizontal dots
    for(i=LINE_VERTICAL_START;i<TRACE_VERTICAL_END-1;i+=LINE_SPACING)
    {
      display_draw_horz_dots(i, DOT_HORIZONTAL_START, TRACE_HORIZONTAL_END - 1, DOT_SPACING);
    }

    //Draw the vertical dots
    for(i=LINE_HORIZONTAL_START;i<TRACE_HORIZONTAL_END-1;i+=LINE_SPACING)
    {
      display_draw_vert_dots(i, DOT_VERTICAL_START, TRACE_VERTICAL_END - 1, DOT_SPACING);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_pointers(void)
{
  int32 position;

  //Text is displayed in black and with font_0
  display_set_bg_color(COLOR_BLACK);
  display_set_font(&font_0);

  //Draw channel 1 pointer when it is enabled
  if(scopesettings.channel1.enable)
  {
    //Set the color for drawing the pointer
    display_set_fg_color(CHANNEL1_COLOR);

    //Check if in normal or x-y display mode
    if(scopesettings.tracedisplaymode == DISPLAY_MODE_NORMAL)
    {
      //y position for the channel 1 trace center pointer.
      position = TRACE_VERTICAL_END - scopesettings.channel1.traceposition;

      //Limit on the top of the displayable region
      if(position < VERTICAL_POINTER_TOP)
      {
        position = VERTICAL_POINTER_TOP;
      }
      //Limit on the bottom of the displayable region
      else if(position > VERTICAL_POINTER_BOTTOM)
      {
        position = VERTICAL_POINTER_BOTTOM;
      }

      //Draw the pointer
      display_left_pointer(VERTICAL_POINTER_LEFT, position - VERTICAL_POINTER_OFFSET, '1');
    }
    else
    {
      //y position for the channel 1 trace center pointer.
      position = VERTICAL_POINTER_XY_OFFSET + scopesettings.channel1.traceposition;

      //Limit on the left of the active range
      if(position < VERTICAL_POINTER_XY_OFFSET)
      {
        position = VERTICAL_POINTER_XY_OFFSET;
      }
      //Limit on the right of the active range
      else if(position > (VERTICAL_POINTER_XY_OFFSET + TRACE_MAX_HEIGHT))
      {
        position = (VERTICAL_POINTER_XY_OFFSET + TRACE_MAX_HEIGHT);
      }

      //Draw the pointer
      display_top_pointer(position, '1');
    }
  }

  //Draw channel 2 pointer when it is enabled
  if(scopesettings.channel2.enable)
  {
    //Set the color for drawing the pointer
    display_set_fg_color(CHANNEL2_COLOR);

    //y position for the channel 2 trace center pointer
    position = TRACE_VERTICAL_END - scopesettings.channel2.traceposition;

    //Limit on the top of the displayable region
    if(position < VERTICAL_POINTER_TOP)
    {
      position = VERTICAL_POINTER_TOP;
    }
    //Limit on the bottom of the displayable region
    else if(position > VERTICAL_POINTER_BOTTOM)
    {
      position = VERTICAL_POINTER_BOTTOM;
    }

    //Draw the pointer
    display_left_pointer(VERTICAL_POINTER_LEFT, position - VERTICAL_POINTER_OFFSET, '2');
  }

  //Draw trigger position and level pointer when in normal display mode
  if(scopesettings.tracedisplaymode == DISPLAY_MODE_NORMAL)
  {
    //x position for the trigger position pointer
    position = scopesettings.triggerhorizontalposition;
    
    //Set the color for drawing the pointer
    display_set_fg_color(TRIGGER_COLOR);

    if((position > (HORIZONTAL_POINTER_LEFT - HORIZONTAL_POINTER_WIDTH)) && (position < (HORIZONTAL_POINTER_RIGHT + HORIZONTAL_POINTER_WIDTH)))
    {
      //Draw the pointer
      display_top_pointer(position - (HORIZONTAL_POINTER_WIDTH / 2), 'H');
    }
    else if(position < HORIZONTAL_POINTER_LEFT)
    {
      //When the pointer is outside the left of the window show it to the user with an arrow pointing left
      ui_display_trigger_position_arrow(0);
    }
    else
    {
      //When the pointer is outside the right of the window show it to the user with an arrow pointing right
      ui_display_trigger_position_arrow(1);
    }
    
    //y position for the trigger level pointer
    position = TRACE_VERTICAL_END - scopesettings.triggerverticalposition;

    //Need to reset the fore ground color because the display_top_pointer copies the background color over it
    display_set_fg_color(TRIGGER_COLOR);

    //Only print when part or whole of the pointer is visible within the trace window
    if((position >= (VERTICAL_POINTER_TOP - VERTICAL_POINTER_OFFSET)) && (position < (VERTICAL_POINTER_BOTTOM + VERTICAL_POINTER_OFFSET)))
    {
      //The trigger level id uses a different font
      display_set_font(&font_3);

      //Draw the pointer
      display_right_pointer(VERTICAL_POINTER_RIGHT, position - VERTICAL_POINTER_OFFSET, 'T');
    }
    else if(position < VERTICAL_POINTER_TOP)
    {
      //When the pointer is above the top of the window show it to the user with an arrow pointing up
      ui_display_trigger_level_arrow(0);
    }
    else
    {
      //When the pointer is below the bottom of the window show it to the user with an arrow pointing down
      ui_display_trigger_level_arrow(1);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_cursors(void)
{
  uint32 height = 5;
  uint32 ch1ypos = 64;
  uint32 ch2ypos = 64;
  uint32 delta;
  int32  x,y;

  //Only display something when at least one cursor and a channel is enabled
  if((scopesettings.timecursorsenable || scopesettings.voltcursorsenable || scopesettings.capturecursorsenable) && (scopesettings.channel1.enable || scopesettings.channel2.enable))
  {
    //Only draw the time cursor lines when enabled
    if(scopesettings.timecursorsenable)
    {
      //Set the color for the dashed lines
      display_set_fg_color(CURSORS_COLOR);

      //Draw the lines
      display_draw_vert_dashes(scopesettings.timecursor1position, TRACE_VERTICAL_START, TRACE_VERTICAL_END, CURSOR_LINE_LENGTH, CURSOR_SPACE_LENGTH);
      display_draw_vert_dashes(scopesettings.timecursor2position, TRACE_VERTICAL_START, TRACE_VERTICAL_END, CURSOR_LINE_LENGTH, CURSOR_SPACE_LENGTH);

      //Setup for the measurements information
      //Add height for two text lines
      height += 32;

      //Shift the voltage text positions down
      ch1ypos += 32;
      ch2ypos += 32;
    }

    //Only draw the volt cursor lines when enabled
    if(scopesettings.voltcursorsenable)
    {
      //Set the color for the dashed lines
      display_set_fg_color(CURSORS_COLOR);

      //Draw the lines
      display_draw_horz_dashes(scopesettings.voltcursor1position, TRACE_HORIZONTAL_START, TRACE_HORIZONTAL_END, CURSOR_LINE_LENGTH, CURSOR_SPACE_LENGTH);
      display_draw_horz_dashes(scopesettings.voltcursor2position, TRACE_HORIZONTAL_START, TRACE_HORIZONTAL_END, CURSOR_LINE_LENGTH, CURSOR_SPACE_LENGTH);

      //Setup for the measurements information
      //Check if channel 1 is enabled
      if(scopesettings.channel1.enable)
      {
        //Add height for one text line
        height += 16;

        //Shift the channel 2 voltage text down
        ch2ypos += 16;
      }

      //Check if channel 2 is enabled
      if(scopesettings.channel2.enable)
      {
        //Add height for one text line
        height += 16;
      }
    }

    //Determine the positions for displaying the text based on the selected cursor and it's position
    //X and Y are fixed based on horizontal or vertical cursor lines
    switch(scopesettings.selectedcursor)
    {
      case CURSOR_TIME_LEFT:
      case CURSOR_TIME_RIGHT:
      case CURSOR_CAPTURE_LEFT:
      case CURSOR_CAPTURE_RIGHT:
        y = CURSOR_SELECTED_TEXT_Y;
        break;

      case CURSOR_VOLT_TOP:
      case CURSOR_VOLT_BOTTOM:
        x = CURSOR_SELECTED_TEXT_X;
        break;
    }

    //Determine the positions for displaying the text based on the selected cursor and it's position
    switch(scopesettings.selectedcursor)
    {
      case CURSOR_TIME_LEFT:
        //Position the text to the left of the cursor
        x = scopesettings.timecursor1position - CURSOR_SELECTED_TEXT_WIDTH;

        //Check if beyond the left border
        if(x <= TRACE_HORIZONTAL_START)
        {
          //If so move the text to the right side of the cursor
          x = scopesettings.timecursor1position + CURSOR_SELECTED_TEXT_OFFSET;
        }
        break;

      case CURSOR_TIME_RIGHT:
        //Position the text to the right of the cursor
        x = scopesettings.timecursor2position + CURSOR_SELECTED_TEXT_OFFSET;

        //Check if beyond the right border
        if(x > (TRACE_HORIZONTAL_END - CURSOR_SELECTED_TEXT_WIDTH))
        {
          //If so move the text to the left side of the cursor
          x = scopesettings.timecursor2position - CURSOR_SELECTED_TEXT_WIDTH;
        }
        break;

      case CURSOR_VOLT_TOP:
        //Position the text above the cursor
        y = scopesettings.voltcursor1position - CURSOR_SELECTED_TEXT_HEIGHT;

        //Check if above the top of the screen
        if(y < TRACE_VERTICAL_START)
        {
          //If so position it below the cursor
          y = scopesettings.voltcursor1position + CURSOR_SELECTED_TEXT_OFFSET;
        }
        break;

      case CURSOR_VOLT_BOTTOM:
        //Position the text below the cursor
        y = scopesettings.voltcursor2position + CURSOR_SELECTED_TEXT_OFFSET;

        //Check if below the bottom of the screen
        if(y > (TRACE_VERTICAL_END - CURSOR_SELECTED_TEXT_HEIGHT))
        {
          //If so position it above the cursor
          y = scopesettings.voltcursor2position - CURSOR_SELECTED_TEXT_HEIGHT;
        }
        break;

      case CURSOR_CAPTURE_LEFT:
        //Position the text to the left of the cursor
        x = scopesettings.capturecursor1position - CURSOR_SELECTED_TEXT_WIDTH;

        //Check if beyond the left border
        if(x <= TRACE_HORIZONTAL_START)
        {
          //If so move the text to the right side of the cursor
          x = scopesettings.capturecursor1position + CURSOR_SELECTED_TEXT_OFFSET;
        }
        break;

      case CURSOR_CAPTURE_RIGHT:
        //Position the text to the right of the cursor
        x = scopesettings.capturecursor2position + CURSOR_SELECTED_TEXT_OFFSET;

        //Check if beyond the right border
        if(x > (TRACE_HORIZONTAL_END - CURSOR_SELECTED_TEXT_WIDTH))
        {
          //If so move the text to the left side of the cursor
          x = scopesettings.capturecursor2position - CURSOR_SELECTED_TEXT_WIDTH;
        }
        break;
    }

    //The selected text is displayed in white and with basic font
    display_set_fg_color(COLOR_WHITE);
    display_set_font(&font_1);
    display_text(x, y, "Selected");

    //Fill in the information window
    //Set gray background for the cursor measurements
    display_set_fg_color(COLOR_DARK_GREY_4);

    //Draw rectangle for the texts depending on what is enabled.
    display_fill_rect(6, 59, 101, height - 1);

    //Make it a on one corner rounded thing
    display_draw_horz_line(height + 59, 6, 106);
    display_draw_horz_line(height + 60, 6, 106);
    display_draw_horz_line(height + 61, 6, 105);
    display_draw_horz_line(height + 62, 6, 103);

    //Use white text and font_0
    display_set_fg_color(COLOR_WHITE);
    display_set_font(&font_0);

    //Check if time cursor is enabled
    if(scopesettings.timecursorsenable)
    {
      //Time texts are always on the top two lines
      //Get the time delta based on the cursor positions
      delta = scopesettings.timecursor2position - scopesettings.timecursor1position;

      //Get the time calculation data for this time base setting.
      PSCREENTIMECALCDATA tcd = (PSCREENTIMECALCDATA)&screen_time_calc_data[scopesettings.timeperdiv];

      //For the time multiply with the scaling factor and display based on the time scale
      delta *= tcd->mul_factor;

      //Format the time for displaying
      ui_cursor_print_value(globaldisplaytext, delta, tcd->time_scale, "T = ", "s");
      display_text(13, 64, globaldisplaytext);

      //Calculate the frequency for this time. Need to adjust for it to stay within 32 bits
      delta /= 10;
      delta = 1000000000 / delta;

      //Format the frequency for displaying
      ui_cursor_print_value(globaldisplaytext, delta, tcd->freq_scale, "F = ", "Hz");
      display_text(13, 80, globaldisplaytext);
    }

    //Check if volt cursor is enabled
    if(scopesettings.voltcursorsenable)
    {
      PVOLTCALCDATA vcd;
      uint32        volts;

      //Get the volts delta based on the cursor positions
      delta = scopesettings.voltcursor2position - scopesettings.voltcursor1position;

      //Check if channel 1 is enabled
      if(scopesettings.channel1.enable)
      {
        //Calculate the voltage based on the channel 1 settings
        vcd = (PVOLTCALCDATA)&volt_calc_data[scopesettings.channel1.magnification][scopesettings.channel1.displayvoltperdiv];

        //Multiply with the scaling factor for the channel 1 settings
        volts = delta * vcd->mul_factor;

        //Channel 1 text has a variable position
        //Format the voltage for displaying
        ui_cursor_print_value(globaldisplaytext, volts, vcd->volt_scale, "V1 = ", "V");
        display_text(13, ch1ypos, globaldisplaytext);
      }

      //Check if channel 2 is enabled
      if(scopesettings.channel2.enable)
      {
        //Calculate the voltage based on the channel 2 settings
        vcd = (PVOLTCALCDATA)&volt_calc_data[scopesettings.channel2.magnification][scopesettings.channel2.displayvoltperdiv];

        //Multiply with the scaling factor for the channel 2 settings
        volts = delta * vcd->mul_factor;

        //Channel 2 text has a variable position
        //Format the voltage for displaying
        ui_cursor_print_value(globaldisplaytext, volts, vcd->volt_scale, "V2 = ", "V");
        display_text(13, ch2ypos, globaldisplaytext);
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_top_box =
{
  20,
  14,
  { COLOR_GREEN, COLOR_PHOSPHOR_GREEN, COLOR_PHARMACY_GREEN },
  COLOR_DARK_GREEN
};

TEXTDATA trigger_top_box_text =
{
  7,
  0,
  COLOR_WHITE,
  &font_1,
  "T"
};

SHADEDRECTDATA trigger_bottom_box =
{
  21,
  14,
  { COLOR_GREEN, COLOR_PHOSPHOR_GREEN, COLOR_PHARMACY_GREEN },
  COLOR_DARK_GREEN
};

TEXTDATA trigger_bottom_box_text =
{
  7,
  0,
  COLOR_WHITE,
  &font_1,
  "H"
};

SHADEDRECTDATA trigger_horizontal_pos_box =
{
  94,
  14,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

TEXTDATA trigger_horizontal_pos_box_text =
{
  5,
  0,
  COLOR_GREY,
  &font_1,
  "POS :"
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_settings(void)
{
  //Mark it as the trigger settings area
  display_draw_shaded_rect(TOP_TRIGGER_INFO_XPOS, TOP_TRIGGER_INFO_YPOS, &trigger_top_box, &trigger_top_box_text);

  //The fixed texts are drawn in a grey shade and basic font
  display_set_fg_color(COLOR_GREY);
  display_set_font(&font_1);

  //Modified character 0x3A (:) for having it one pixel higher then normal to
  //match the original firmware where it is printed separately to do the same
  //Selected trigger channel text
  display_text(TOP_TRIGGER_INFO_XPOS, TOP_TRIGGER_INFO_YPOS + 17, "CHS :");

  //Trigger vertical (level) position text
  display_text(TOP_TRIGGER_INFO_XPOS, TOP_TRIGGER_INFO_YPOS + 31, "POS :");

  //Time per division text in the bottom information section
  display_text(BOTTOM_TRIGGER_INFO_XPOS + 122, BOTTOM_TRIGGER_INFO_YPOS, "DIV :");

  //Setup the bottom information section
  display_draw_shaded_rect(BOTTOM_TRIGGER_INFO_XPOS, BOTTOM_TRIGGER_INFO_YPOS, &trigger_bottom_box, &trigger_bottom_box_text);

  //Fill in the variable sections
  ui_display_trigger_mode();
  ui_display_trigger_channel();
  ui_display_trigger_edge();
  ui_display_trigger_vertical_position();
  ui_display_trigger_horizontal_position();
  ui_display_time_per_division();
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_mode_box =
{
  57,
  14,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

TEXTDATA trigger_mode_texts[] =
{
  { 15, 0, COLOR_WHITE, &font_1, "Auto" },
  { 12, 0, COLOR_WHITE, &font_1, "Single" },
  {  9, 0, COLOR_WHITE, &font_1, "Normal" },
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_mode(void)
{
  //Check which trigger mode is set and if it is a valid setting
  if(scopesettings.triggermode < 3)
  {
    //Display the shaded rectangle with the selected text
    display_draw_shaded_rect(TOP_TRIGGER_INFO_XPOS + 22, TOP_TRIGGER_INFO_YPOS, &trigger_mode_box, &trigger_mode_texts[scopesettings.triggermode]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_channel_boxes[] =
{
  { 27, 12, { CHANNEL1_COLOR, COLOR_CHARTREUSE_YELLOW, COLOR_RIOJA_YELLOW }, COLOR_CITRUS_YELLOW },
  { 27, 12, { CHANNEL2_COLOR, COLOR_BRIGHT_TURQUOISE, COLOR_IRISH_BLUE }, COLOR_PERSIAN_GREEN },
};

TEXTDATA trigger_channel_texts[] =
{
  { 3, -1, COLOR_BLACK, &font_1, "CH1" },
  { 3, -1, COLOR_BLACK, &font_1, "CH2" },
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_channel(void)
{
  //Check which trigger channel is selected and if it is a valid setting
  if(scopesettings.triggerchannel < 2)
  {
    //Display the shaded rectangle with the selected text
    display_draw_shaded_rect(TOP_TRIGGER_INFO_XPOS + 35, TOP_TRIGGER_INFO_YPOS + 18, &trigger_channel_boxes[scopesettings.triggerchannel], &trigger_channel_texts[scopesettings.triggerchannel]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_edge(void)
{
  const uint8 *icon;

  //Using an icon might be a simpler and faster option
  display_set_fg_color(COLOR_WHITE);
  display_set_bg_color(COLOR_BLACK);

  //Select the icon based on the selected edge
  if(scopesettings.triggeredge == 0)
  {
    //rising edge
    icon = trigger_rising_edge_icon;
  }
  else
  {
    //falling edge
    icon = trigger_falling_edge_icon;
  }

  //Display the edge icon with infill of the background since the other icon needs to be overwritten
  display_copy_icon_use_colors(icon, TOP_TRIGGER_INFO_XPOS + 71, TOP_TRIGGER_INFO_YPOS + 18, 7, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_vertical_position(void)
{
  PCHANNELSETTINGS settings;
  PVOLTCALCDATA    vcd;
  int32            delta;
  int32            volts;

  //Clear the background first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(TOP_TRIGGER_INFO_XPOS + 33, TOP_TRIGGER_INFO_YPOS + 32, 49, 11);
  
  //Only in normal mode the position is shown
  if(scopesettings.tracedisplaymode == DISPLAY_MODE_NORMAL)
  {
    //Select the channel based on the current trigger channel
    if(scopesettings.triggerchannel == 0)
    {
      settings = &scopesettings.channel1;
    }
    else
    {
      settings = &scopesettings.channel2;
    }

    //Get the voltage calculation data for the given channel
    vcd = (PVOLTCALCDATA)&volt_calc_data[settings->magnification][settings->displayvoltperdiv];

    //Calculate the delta of the trigger channel position versus the trigger level position
    delta = scopesettings.triggerverticalposition - settings->traceposition;

    //Multiply with the scaling factor for the given channel to get the level expressed in volts
    volts = delta * vcd->mul_factor;

    //Format the voltage for displaying
    ui_msm_print_value(globaldisplaytext, volts, vcd->volt_scale, "V");

    //The text is drawn in white and basic font
    display_set_fg_color(COLOR_WHITE);
    display_set_font(&font_1);

    //Show it in the top of the screen trigger section
    display_right_aligned_text(TOP_TRIGGER_INFO_XPOS + 80, TOP_TRIGGER_INFO_YPOS + 31, globaldisplaytext);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_horizontal_position(void)
{
  int32 delta;
  
  //Clear the old text first by re drawing the box
  display_draw_shaded_rect(BOTTOM_TRIGGER_INFO_XPOS + 23, BOTTOM_TRIGGER_INFO_YPOS, &trigger_horizontal_pos_box, &trigger_horizontal_pos_box_text);
      
  //Only in normal mode the position is shown
  if(scopesettings.tracedisplaymode == DISPLAY_MODE_NORMAL)
  {
    //Get the time delta based on the center of the pointers position
    delta = scopesettings.triggerhorizontalposition - TRACE_HORIZONTAL_CENTER;

    //Get the time calculation data for this time base setting.
    PSCREENTIMECALCDATA tcd = (PSCREENTIMECALCDATA)&screen_time_calc_data[scopesettings.timeperdiv];

    //For the time multiply with the scaling factor and display based on the time scale
    delta *= tcd->mul_factor;

    //Format the time for displaying
    ui_msm_print_value(globaldisplaytext, delta, tcd->time_scale, "s");

    //The text is drawn in white with a basic font
    display_set_font(&font_1);
    display_set_fg_color(COLOR_WHITE);

    //Display the selected sensitivity
    display_right_aligned_text(BOTTOM_TRIGGER_INFO_XPOS + 113, BOTTOM_TRIGGER_INFO_YPOS, globaldisplaytext);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_time_per_division(void)
{
  //Clear the old text before printing the new text
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(BOTTOM_TRIGGER_INFO_XPOS + 148, BOTTOM_TRIGGER_INFO_YPOS, 50, 14);

  //Text is displayed in white and basic font
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_1);

  //Only display the text when in the setting is range of the text array
  if(scopesettings.timeperdiv < (sizeof(time_div_texts) / sizeof(int8 *)))
  {
    //Display the in the bottom information section using the table
    display_right_aligned_text(BOTTOM_TRIGGER_INFO_XPOS + 188, BOTTOM_TRIGGER_INFO_YPOS, (char *)time_div_texts[scopesettings.timeperdiv]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_move_speed(void)
{
  const uint8 *icon;

  //Text is slightly off white
  display_set_fg_color(COLOR_WHITE);
  display_set_bg_color(COLOR_BLACK);

  //Draw the icon for this section
  display_copy_icon_fg_color(move_speed_icon, 330, 13, 16, 16);

  //Set the needed text based on the selected speed
  if(scopesettings.movespeed == MOVE_SPEED_FAST)
  {
    //Fast speed selected
    icon = fast_text_icon;
  }
  else
  {
    //Slow speed selected
    icon = slow_text_icon;
  }

  //Display the speed text with infill of the background since the other text needs to be overwritten
  display_copy_icon_use_colors(icon, 351, 15, 30, 13);
  display_copy_icon_fg_color(moving_text_icon, 383, 15, 54, 17);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_show_open_slider(void)
{
  //Check if the grid brightness slider menu is open and draw it in if so
  if((navigationstate == NAV_SLIDER_HANDLING) && (sliderdata == &scopesettings.gridbrightness))
  {
    //Main menu with the slider possibly opened
    ui_display_main_menu();

    //Show the slider with the current setting without saving the background
    ui_open_slider(SLIDER_XPOS, SLIDER_GRID_YPOS, 0);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_disabled_box =
{
  20,
  14,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_1_box =
{
  20,
  14,
  { COLOR_YELLOW, COLOR_OLIVE, COLOR_DARK_YELLOW },
  COLOR_LIQUORICE
};

TEXTDATA channel_1_box_text =
{
  7,
  0,
  COLOR_WHITE,
  &font_1,
  "1"
};

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_2_box =
{
  20,
  14,
  { 0x0000FCF8, COLOR_DARK_CYAN, COLOR_SHERPA_BLUE },
  COLOR_DARK_GREEN
};

TEXTDATA channel_2_box_text =
{
  7,
  0,
  COLOR_WHITE,
  &font_1,
  "2"
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_settings(PCHANNELSETTINGS settings)
{
  PSHADEDRECTDATA boxdata;

  int x = settings->infoxpos + 3;
  int y = settings->infoypos + 17;

  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used and the texts are brighter
    boxdata = settings->boxdata;
    settings->boxtext->color = 0x00F8FCF8;
  }
  else
  {
    //When disabled the disabled colors are used
    boxdata = &channel_disabled_box;
    settings->boxtext->color = 0x00888888;
  }

  //Mark it as the channel settings area
  display_draw_shaded_rect(settings->infoxpos, settings->infoypos, boxdata, settings->boxtext);

  //The fixed texts are drawn in a grey shade and basic font
  display_set_fg_color(COLOR_GREY);
  display_set_font(&font_1);

  //Modified character 0x3A (:) for having it one pixel higher then normal to
  //match the original firmware where it is printed separately to do the same
  //Volts per division text
  display_text(x, y, "DIV :");

  //Position text is placed 13 pixels lower then the volts per division text
  y += 13;

  //Channel position text
  display_text(x, y, "POS :");

  //Fill in the variable sections
  ui_display_channel_probe(settings);
  ui_display_channel_coupling(settings);
  ui_display_channel_sensitivity(settings);
  ui_display_channel_position(settings);
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_probe_box =
{
  46,
  14,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

TEXTDATA channel_probe_texts[] =
{
  { 12, 0, COLOR_WHITE, &font_1,   "1 : 1" },
  {  8, 0, COLOR_WHITE, &font_1,  "10 : 1" },
  {  5, 0, COLOR_WHITE, &font_1, "100 : 1" },
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_probe(PCHANNELSETTINGS settings)
{
  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the text is brighter
    channel_probe_texts[settings->magnification].color = 0x00F8FCF8;
  }
  else
  {
    //When disabled the disabled color is used
    channel_probe_texts[settings->magnification].color = 0x00888888;
  }

  //Display the shaded rectangle with the selected text
  display_draw_shaded_rect(settings->infoxpos + 22, settings->infoypos, &channel_probe_box, &channel_probe_texts[settings->magnification]);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_coupling(PCHANNELSETTINGS settings)
{
  int32 x = settings->infoxpos + 70;
  int32 y = settings->infoypos;

  //Clear the old text first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(x - 1, y + 2, 18, 10);

  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used
    display_set_fg_color(settings->color);
  }
  else
  {
    //When disabled the disabled color is used
    display_set_fg_color(COLOR_GREY);
  }

  //The text is drawn a basic font
  display_set_font(&font_1);

  //Check on which coupling is set
  if(settings->coupling == 0)
  {
    //DC coupling
    display_text(x, y, "DC");
  }
  else
  {
    //AC coupling
    display_text(x, y, "AC");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_sensitivity(PCHANNELSETTINGS settings)
{
  int32 x = settings->infoxpos + 31;
  int32 y = settings->infoypos + 17;

  //Clear the old text first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(x - 1, y + 2, 57, 10);

  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used
    display_set_fg_color(settings->color);
  }
  else
  {
    //When disabled the disabled color is used
    display_set_fg_color(COLOR_GREY);
  }

  //The text is drawn with a basic font
  display_set_font(&font_1);

  //Display the selected sensitivity
  display_right_aligned_text(x + 54, y, volt_div_texts[settings->magnification][settings->displayvoltperdiv]);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_position(PCHANNELSETTINGS settings)
{
  PVOLTCALCDATA vcd;
  int32         volts;
  int32         delta = TRACE_VERTICAL_END - settings->traceposition;
  int32         x     = settings->infoxpos + 31;
  int32         y     = settings->infoypos + 30;

  //Clear the old text first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(x - 1, y + 2, 57, 10);

  //Get the voltage calculation data for the given channel
  vcd = (PVOLTCALCDATA)&volt_calc_data[settings->magnification][settings->displayvoltperdiv];

  //Get the delta between the actual position and the window center
  delta = TRACE_VERTICAL_CENTER - delta;

  //Multiply with the scaling factor for the given channel to get the level expressed in volts
  volts = delta * vcd->mul_factor;

  //Format the voltage for displaying
  ui_msm_print_value(globaldisplaytext, volts, vcd->volt_scale, "V");
  
  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used
    display_set_fg_color(settings->color);
  }
  else
  {
    //When disabled the disabled color is used
    display_set_fg_color(COLOR_GREY);
  }

  //The text is drawn with a basic font
  display_set_font(&font_1);

  //Display the selected sensitivity
  display_right_aligned_text(x + 54, y, globaldisplaytext);
}

//----------------------------------------------------------------------------------------------------------------------------------

const MEASUREMENTFUNCTION measurement_functions[] =
{
  ui_display_vmax,
  ui_display_vmin,
  ui_display_vavg,
  ui_display_vrms,
  ui_display_vpp,
  ui_display_vp,
  ui_display_freq,
  ui_display_cycle,
  ui_display_time_plus,
  ui_display_time_min,
  ui_display_duty_plus,
  ui_display_duty_min
};

//----------------------------------------------------------------------------------------------------------------------------------
//For this function it is necessary that the measurementitems in the scopesettings are properly initialized

void ui_display_measurements(void)
{
  PCHANNELSETTINGS settings;
  int i,y,dy;

  //Process the data for the available measurement slots
  for(i=0;i<(sizeof(scopesettings.measurementitems)/sizeof(MEASUREMENTINFO));i++)
  {
    dy = i * MEASUREMENT_Y_DISPLACEMENT;

    //Clear the background first
    display_set_fg_color(COLOR_BLACK);
    display_fill_rect(707, 9 + dy, 92, 61);

    //Get the channel information for displaying the box in the channel color
    settings = scopesettings.measurementitems[i].channelsettings;

    //Setup the base position
    y = MEASUREMENT_INFO_Y + dy;

    //Mark it as the channel settings area
    display_draw_shaded_rect(MEASUREMENT_CHANNEL_BOX_X, y, settings->boxdata, settings->boxtext);

    //The fixed texts are drawn in a grey shade and basic font
    display_set_fg_color(COLOR_GREY);
    display_set_font(&font_2);

    //Display the measurement label
    display_text(MEASUREMENT_LABEL_X, y, measurement_names[scopesettings.measurementitems[i].index]);

    //Set the y position for the measurement
    y += 21;

    //Call the set function for displaying the actual value and
    //pass the information for this measurement to the function for displaying it
    measurement_functions[scopesettings.measurementitems[i].index](y, settings);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_update_measurements(void)
{
  PCHANNELSETTINGS settings;
  int i,y;

  //Use the first display buffer as source to copy to the main screen buffer
  display_set_source_buffer(displaybuffer1);

  //Process the data for the available measurement slots
  for(i=0;i<(sizeof(scopesettings.measurementitems)/sizeof(MEASUREMENTINFO));i++)
  {
    //Draw the item in the first display buffer to avoid flicker on the screen
    display_set_screen_buffer(displaybuffer1);

    //Get the channel information for displaying the value
    settings = scopesettings.measurementitems[i].channelsettings;

    //Setup the base position
    y = MEASUREMENT_INFO_Y + (i * MEASUREMENT_Y_DISPLACEMENT) + 21;

    //Clear the display field first
    display_set_fg_color(COLOR_BLACK);
    display_fill_rect(MEASUREMENT_VALUE_X - 2, y - 2, 83, 20);

    //Call the set function for displaying the actual value and
    //pass the information for this measurement to the function for displaying it
    measurement_functions[scopesettings.measurementitems[i].index](y, settings);

    //Copy this item to the main screen
    display_set_screen_buffer((uint16 *)maindisplaybuffer);
    display_copy_rect_to_screen(MEASUREMENT_VALUE_X - 2, y - 2, 84, 21);
  }

  //Switch back to the separate display buffer to allow further actions on the trace display
  display_set_screen_buffer(displaybuffer1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vmax(uint32 ypos, PCHANNELSETTINGS settings)
{
  //show sign either positive or negative on x location 719 followed by the value, but only when the value
  //is above 99. If less the value starts 13 pixels to the right

  //For the maximum take of the center ADC value
  ui_display_voltage(ypos, settings, settings->max - 128, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vmin(uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the minimum take of the center ADC value
  ui_display_voltage(ypos, settings, settings->min - 128, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vavg(uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the average take of the center ADC value
  ui_display_voltage(ypos, settings, settings->average - 128, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vrms(uint32 ypos, PCHANNELSETTINGS settings)
{
  //The rms has already been centered during the summation so use it as is
  ui_display_voltage(ypos, settings, settings->rms, 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vpp(uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the peak peak just use the value as is
  ui_display_voltage(ypos, settings, settings->peakpeak, 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_vp(uint32 ypos, PCHANNELSETTINGS settings)
{
  //Determine the two absolute extremes
  int32 min = 128 - settings->min;
  int32 max = settings->max - 128;

  //Display the biggest of the two
  if(min > max)
  {
    //Use the below the center value when it is the biggest
    ui_display_voltage(ypos, settings, min, 0);
  }
  else
  {
    //Use the above the center value when it is the biggest
    ui_display_voltage(ypos, settings, max, 0);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_freq(uint32 ypos, PCHANNELSETTINGS settings)
{
  uint32 frequency = 0;

  if(settings->frequencyvalid)
  {
    frequency = settings->frequency;
  }

  //Format the frequency for displaying
  ui_print_value(ypos, frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "Hz", 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_cycle(uint32 ypos, PCHANNELSETTINGS settings)
{
  uint32 time = 0;

  if(settings->frequencyvalid)
  {
    time = (((uint64)settings->periodtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20);
  }

  //Format the cycle time for displaying
  ui_print_value(ypos, time, time_calc_data[scopesettings.samplerate].time_scale, "s", 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_time_plus(uint32 ypos, PCHANNELSETTINGS settings)
{
  uint32 time = 0;

  if(settings->frequencyvalid)
  {
    time = (((uint64)settings->hightime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20);
  }

  //Format the high part time for displaying
  ui_print_value(ypos, time, time_calc_data[scopesettings.samplerate].time_scale, "s", 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_time_min(uint32 ypos, PCHANNELSETTINGS settings)
{
  uint32 time = 0;

  if(settings->frequencyvalid)
  {
    time = (((uint64)settings->lowtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20);
  }

  //Format the low part time for displaying
  ui_print_value(ypos, time, time_calc_data[scopesettings.samplerate].time_scale, "s", 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_duty_plus(uint32 ypos, PCHANNELSETTINGS settings)
{
  //Display the high duty cycle
  ui_display_duty_cycle(ypos, settings, settings->hightime);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_duty_min(uint32 ypos, PCHANNELSETTINGS settings)
{
  //Display the low duty cycle
  ui_display_duty_cycle(ypos, settings, settings->lowtime);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_duty_cycle(uint32 ypos, PCHANNELSETTINGS settings, uint32 value)
{
  uint32 percentage = 0;

  if(settings->frequencyvalid)
  {
    percentage = (((uint64)value * 100) / settings->periodtime);
  }

  if(percentage == 0)
  {
    //Value is zero so just set 0 character
    display_copy_icon_full_color(measurement_digit_icons[0], MEASUREMENT_ZERO_X, ypos, 13, 16);
  }
  else
  {
    //Format the duty cycle for displaying
    ui_print_decimal(MEASUREMENT_VALUE_X, ypos, percentage, 0);
  }

  //The designator text is drawn in white and small font
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_1);

  //Display the designator on the screen
  display_text(MEASUREMENT_DESIGNATOR_X + 5, ypos + 5, "%");
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_voltage(uint32 ypos, PCHANNELSETTINGS settings, int32 value, uint32 signedvalue)
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
  ui_print_value(ypos, volts, vcd->volt_scale, "V", signedvalue);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_print_value(uint32 ypos, int32 value, uint32 scale, char *designator, uint32 signedvalue)
{
  char   *buffer = globaldisplaytext;
  uint32  x = MEASUREMENT_VALUE_X;
  uint32  d;

  //Only when sign is needed print it and adjust the x position for it
  if(signedvalue)
  {
    //Check if positive value
    if(value > 0)
    {
      //Display the plus sign on a fixed location
      display_copy_icon_full_color(measurement_plus_icon, x, ypos + 4, 8, 8);
    }
    //Check if negative value
    else if(value < 0)
    {
      //Negate if so and signal negative sign needed
      value = -value;

      //Display the minus sign on a fixed location
      display_copy_icon_full_color(measurement_minus_icon, x, ypos + 7, 8, 2);
    }

    //Sign takes 10 pixels
    x += 10;
  }

  //Check on zero value to avoid unneeded processing
  if(value == 0)
  {
    //Value is zero so just set 0 character
    display_copy_icon_full_color(measurement_digit_icons[0], MEASUREMENT_ZERO_X, ypos, 13, 16);

    //Set the x position for printing the designator on the screen
    x = MEASUREMENT_DESIGNATOR_X;
  }
  else
  {
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
      d = 2;
    }
    else if(value < 10000)
    {
      //More then 1000 but less then 10000 means xx.y
      value /= 10;
      d = 1;
    }
    else
    {
      //More then 10000 and less then 100000 means xxx
      value /= 100;
      d = 0;
    }

    //Draw the value on the display with the needed decimals
    x = ui_print_decimal(x, ypos, value, d);
  }

  //The non signed values have the designator a bit further to the right than the last digit
  if(signedvalue == 0)
  {
    //Set the x position for printing the designator on the screen
    x += 11;
  }

  //Make sure scale is not out of range
  if(scale > 7)
  {
    scale = 7;
  }

  //When there is a magnifier and the value is zero non signed, the text needs to shift to the left
  if((scale != 4) && (value == 0) && (signedvalue == 0))
  {
    x -= 9;
  }

  //Setup the designator by first adding the magnitude scaler
  buffer = strcpy(buffer, magnitude_scaler[scale]);

  //Add the type of measurement designator
  strcpy(buffer, designator);

  //The designator text is drawn in white and small font
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_1);

  //Display it on the screen
  display_text(x + 2, ypos + 5, globaldisplaytext);
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 ui_print_decimal(uint32 xpos, uint32 ypos, int32 value, uint32 decimals)
{
  uint32 i = 10;
  uint32 r = xpos + 39;
  uint32 x;

  //Need to calculate the needed x position based on if there is a decimal point and the number of digits are going to be printed
  //Starting point is always three digits to the right
  x = xpos + 26;

  if(decimals)
  {
    //When there is a decimal point the right end is 8 pixels further then just three digits
    x += 8;
    r += 8;
  }

  //Process the digits
  while(value)
  {
    //Set current digit to decreased index
    display_copy_icon_full_color(measurement_digit_icons[value % 10], x, ypos, 13, 16);

    //House keeping for determining dot location
    i--;

    //Take of the current digit
    value /= 10;
    
    //Check if decimal point needs to be placed
    if(i == 10 - decimals)
    {
      //Move back to where the next dot needs to be placed
      x -= 6;

      //If so display it at the right height
      display_copy_icon_full_color(measurement_dot_icon, x, ypos + 12, 4, 4);

      //Create space before the dot
      x -= 2;
      
      //In case of 0.xx value the zero before the dot needs to be added
      if(value == 0)
      {
        //Move back to the next digit location
        x -= 13;
        
        //Print it on the screen
        display_copy_icon_full_color(measurement_digit_icons[0], x, ypos, 13, 16);
      }
    }

    //Move back to the next digit location
    x -= 13;
  }

  //Return the most right x position used
  return(r);
}

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA main_menu_highlight_box =
{
  175,
  26,
  { COLOR_GREEN, COLOR_ISLAMIC_GREEN, COLOR_PHARMACY_GREEN, COLOR_DARK_GREEN },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_main_menu(void)
{
  int i,y;

  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(COLOR_DARK_GREY_3);
  display_draw_rect(5, 114, 183, 345);

  //Fill the lighter background of the menu area
  display_set_fg_color(COLOR_DARK_GREY_1);
  display_fill_rect(6, 115, 180, 342);

  //Calculate the y position for the highlight box based on the selected menu item
  y = 118 + (menuitem * 31);

  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(9, y, &main_menu_highlight_box);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //The main menu has 11 items and fixed here for now.
  for(i=0,y=122;i<11;i++)
  {
    //Draw the icon and belonging text per line
    display_copy_icon_fg_color(main_menu_icons[i], 14, y, 166, 18);

    //Next line is 31 pixels down
    y += 31;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_unhighlight_main_menu_item(void)
{
  int y;

  //Calculate the y position for clearing the box based on the selected menu item
  y = 118 + (menuitem * 31);

  //Fill the section with the lighter background of the menu area
  display_set_fg_color(COLOR_DARK_GREY_1);
  display_fill_rect(9, y, 174, 25);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //Text line sits 4 pixels lower
  y += 4;

  //Draw the icon and belonging text for the selected item
  display_copy_icon_fg_color(main_menu_icons[menuitem], 14, y, 166, 18);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_highlight_main_menu_item(void)
{
  int y;

  //Clear the previous selection first

  //Calculate the y position for the highlight box based on the selected menu item
  y = 118 + (menuitem * 31);

  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(9, y, &main_menu_highlight_box);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //Text line sits 4 pixels lower
  y += 4;

  //Draw the icon and belonging text for the selected item
  display_copy_icon_fg_color(main_menu_icons[menuitem], 14, y, 166, 18);
}

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA channel_1_highlight_box =
{
  183,
  24,
  { COLOR_YELLOW, COLOR_PEA_SOUP, COLOR_VERDUN_GREEN, COLOR_LIQUORICE },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA channel_2_highlight_box =
{
  183,
  24,
  { COLOR_CYAN, COLOR_LIGHT_BLUE, COLOR_MOSQUE, COLOR_STELLAR_EXPLORER },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu(PCHANNELSETTINGS settings)
{
  int i,y;

  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(COLOR_DARK_GREY_3);
  display_draw_rect(260, 213, 191, 91);

  //Fill the lighter background of the menu area
  display_set_fg_color(COLOR_DARK_GREY_1);
  display_fill_rect(261, 214, 188, 88);

  //Need to calculate the y position for the highlight box
  y = 217 + (menuitem * 29);

  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(264, y, settings->highlightboxdata);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //The channel menu has 3 settings and fixed here for now.
  for(i=0,y=221;i<3;i++)
  {
    //Draw the icon and belonging label per line
    display_copy_icon_fg_color(channel_menu_icon_icons[i], 269, y, 17, 16);
    display_copy_icon_fg_color(channel_menu_label_icons[i], 298, y, 57, 16);

    //Next line is 29 pixels down
    y += 29;
  }

  //Process the settings
  ui_display_channel_menu_probe_magnification_select(settings);
  ui_display_channel_menu_coupling_select(settings);
  ui_display_channel_menu_fft_on_off_select(settings);
}

//----------------------------------------------------------------------------------------------------------------------------------

const uint8 *channel_menu_magnification_icons[] =
{
  channel_menu_1X_icon,
  channel_menu_10X_icon,
  channel_menu_100X_icon
};

const uint32 channel_menu_magnification_widths[] =
{
  10,
  18,
  26
};

const uint32 channel_menu_magnification_x_positions[] =
{
  366,
  387,
  414
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu_probe_magnification_select(PCHANNELSETTINGS settings)
{
  int i;

  //Clear the background first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(364, 222, 79, 14);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //Display the magnification texts in a loop, but skip the selected one
  for(i=0;i<3;i++)
  {
    //Only show the not selected ones
    if(i != settings->magnification)
    {
      display_copy_icon_fg_color(channel_menu_magnification_icons[i], channel_menu_magnification_x_positions[i], 224, channel_menu_magnification_widths[i], 10);
    }
  }

  //Selected item highlight box is set in channel color
  display_set_fg_color(settings->color);

  //Highlight the selected item
  display_fill_rect(channel_menu_magnification_x_positions[settings->magnification] - 2, 222, channel_menu_magnification_widths[settings->magnification] + 3, 13);

  //Display the selected text in black
  display_set_fg_color(COLOR_BLACK);
  display_copy_icon_fg_color(channel_menu_magnification_icons[settings->magnification], channel_menu_magnification_x_positions[settings->magnification], 224, channel_menu_magnification_widths[settings->magnification], 10);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu_coupling_select(PCHANNELSETTINGS settings)
{
  //Clear the background first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(364, 250, 78, 15);

  if(settings->coupling == 1)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(367, 250, 29, 15);

    //Display the selected text in black
    display_set_fg_color(COLOR_BLACK);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(COLOR_WHITE);
  }

  //Display the AC text
  display_copy_icon_fg_color(channel_menu_AC_icon, 373, 252, 19, 12);

  if(settings->coupling == 0)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(407, 250, 29, 15);

    //Display the selected text in black
    display_set_fg_color(COLOR_BLACK);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(COLOR_WHITE);
  }

  //Display the DC text
  display_copy_icon_fg_color(channel_menu_DC_icon, 413, 252, 19, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu_fft_on_off_select(PCHANNELSETTINGS settings)
{
  //Clear the background first
  display_set_fg_color(COLOR_BLACK);
  display_fill_rect(364, 279, 78, 15);

  if(settings->fftenable == 1)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(367, 279, 29, 15);

    //Display the selected text in black
    display_set_fg_color(COLOR_BLACK);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(COLOR_WHITE);
  }

  //Display the ON text
  display_copy_icon_fg_color(channel_menu_ON_icon, 372, 281, 21, 12);

  if(settings->fftenable == 0)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(407, 279, 29, 15);

    //Display the selected text in black
    display_set_fg_color(COLOR_BLACK);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(COLOR_WHITE);
  }

  //Display the OFF text
  display_copy_icon_fg_color(channel_menu_OFF_icon, 409, 281, 26, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_cursor_print_value(char *buffer, int32 value, uint32 scale, char *header, char *sign)
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
    buffer = ui_msm_print_decimal(buffer, value, 2, negative);
  }
  else if(value < 10000)
  {
    //More then 1000 but less then 10000 means xx.y
    value /= 10;
    buffer = ui_msm_print_decimal(buffer, value, 1, negative);
  }
  else
  {
    //More then 10000 and less then 100000 means xxx
    value /= 100;
    buffer = ui_msm_print_decimal(buffer, value, 0, negative);
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

SHADEDRECTDATA slider_outer_box =
{
  SLIDER_OUTER_BOX_WIDTH,
  SLIDER_OUTER_BOX_HEIGHT,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

SHADEDROUNDEDRECTDATA slider_rounded_box =
{
  SLIDER_ROUNDED_BOX_WIDTH,
  SLIDER_ROUNDED_BOX_HEIGHT,
  SLIDER_ROUNDED_BOX_RADIUS,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_open_slider(uint16 xpos, uint16 ypos, uint32 savebackground)
{
  //Only save the back ground when requested
  if(savebackground)
  {
    //Save the screen under the slider
    display_set_destination_buffer(displaybuffer2);
    display_copy_rect_from_screen(xpos, ypos, SLIDER_OUTER_BOX_WIDTH, SLIDER_OUTER_BOX_HEIGHT);

    //Setup the slider menu in a separate buffer to be able to display without flicker
    display_set_screen_buffer(displaybuffer1);
  }

  //Draw the outer box
  display_draw_shaded_rect(xpos, ypos, &slider_outer_box, 0);

  //Draw the inner rounded box
  display_draw_shaded_rounded_rect(xpos + SLIDER_ROUNDED_BOX_X_OFFSET, ypos + SLIDER_ROUNDED_BOX_Y_OFFSET, &slider_rounded_box);

  //Display the actual slider
  ui_display_slider(xpos, ypos);

  //Only need to copy it onto the actual screen when the background is saved
  if(savebackground)
  {
    //Set source and target for getting it on the actual screen
    display_set_source_buffer(displaybuffer1);
    display_set_screen_buffer((uint16 *)maindisplaybuffer);

    //Show the slider box on the screen
    display_copy_rect_to_screen(xpos, ypos, SLIDER_OUTER_BOX_WIDTH, SLIDER_OUTER_BOX_HEIGHT);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_close_slider(uint16 xpos, uint16 ypos)
{
  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer2);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Remove the slider box from the screen
  display_copy_rect_to_screen(xpos, ypos, SLIDER_OUTER_BOX_WIDTH, SLIDER_OUTER_BOX_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_slider(uint16 xpos, uint16 ypos)
{
  //sliderdata needs to be set otherwise no use in running the code
  if(sliderdata)
  {
    uint16 ws = ((SLIDER_LINE_MAX_WIDTH * *sliderdata) / 100);
    uint16 xs = xpos + SLIDER_LINE_X_OFFSET;
    uint16 ys = ypos + SLIDER_LINE_Y_OFFSET;
    uint16 xt = xpos + SLIDER_TEXT_X_OFFSET;
    uint16 yt = ypos + SLIDER_TEXT_Y_OFFSET;

    //Clear the background first
    display_set_fg_color(COLOR_BLACK);

    //Start with no slider
    display_fill_rect(xs, ys, SLIDER_LINE_MAX_WIDTH - 1, SLIDER_LINE_HEIGHT - 1);

    //And remove the position text
    display_fill_rect(xt - 1, yt + 1, SLIDER_TEXT_WIDTH - 1, SLIDER_TEXT_HEIGHT - 1);

    //Check if there is a line to draw
    if(ws)
    {
      //Draw the first part of the slider bar in a green color
      //Fill rect needs a reduction of 1 in both width and height
      display_set_fg_color(0x0000FF00);
      display_fill_rect(xs, ys, ws - 1, SLIDER_LINE_HEIGHT - 1);
    }

    //Display the position with wite text
    display_set_fg_color(COLOR_WHITE);
    display_set_font(&font_2);
    display_decimal(xt, yt, *sliderdata);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA on_off_setting_box =
{
  ON_OFF_SETTING_BOX_WIDTH,
  ON_OFF_SETTING_BOX_HEIGHT,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA on_off_select_box =
{
  ON_OFF_SELECT_BOX_WIDTH,
  ON_OFF_SELECT_BOX_HEIGHT,
  { COLOR_GREEN, COLOR_ISLAMIC_GREEN, COLOR_PHARMACY_GREEN, COLOR_DARK_GREEN },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA on_off_check_box =
{
  ON_OFF_CHECK_SIZE,
  ON_OFF_CHECK_SIZE,
  { COLOR_GREEN, COLOR_GLIMMER_GREEN, COLOR_ISLAMIC_GREEN },
  COLOR_PAKISTAN_GREEN
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_open_on_off_setting(uint16 xpos, uint16 ypos, uint32 savebackground)
{
  //Only save the back ground when requested
  if(savebackground)
  {
    //Save the screen under the on off setting box
    display_set_destination_buffer(displaybuffer2);
    display_copy_rect_from_screen(xpos, ypos, ON_OFF_SETTING_BOX_WIDTH, ON_OFF_SETTING_BOX_HEIGHT);

    //Setup the setting menu in a separate buffer to be able to display without flicker
    display_set_screen_buffer(displaybuffer1);
  }

  //Display the actual menu with the settings
  ui_display_on_off_setting(xpos, ypos);

  //Only need to copy it onto the actual screen when the background is saved
  if(savebackground)
  {
    //Set source and target for getting it on the actual screen
    display_set_source_buffer(displaybuffer1);
    display_set_screen_buffer((uint16 *)maindisplaybuffer);

    //Show the on off setting menu box on the screen
    display_copy_rect_to_screen(xpos, ypos, ON_OFF_SETTING_BOX_WIDTH, ON_OFF_SETTING_BOX_HEIGHT);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_close_on_off_setting(uint16 xpos, uint16 ypos)
{
  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer2);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Remove the on off menu box from the screen
  display_copy_rect_to_screen(xpos, ypos, ON_OFF_SETTING_BOX_WIDTH, ON_OFF_SETTING_BOX_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_on_off_setting(uint16 xpos, uint16 ypos)
{
  uint16 y;

  //onoffdata needs to be set, otherwise there is no use in running the code
  if(onoffdata)
  {
    //Draw the outer box to clear the background
    display_draw_shaded_rect(xpos, ypos, &on_off_setting_box, 0);

    //Need to display the highlight box on the selected item
    if(onoffhighlighteditem == 0)
    {
      y = ypos + ON_OFF_SELECT_OFF_Y_OFFSET;
    }
    else
    {
      y = ypos + ON_OFF_SELECT_ON_Y_OFFSET;
    }

    display_draw_highlight_rect(xpos + ON_OFF_SELECT_X_OFFSET, y, &on_off_select_box);

    //Need to display the check box on the currently active item
    if(*onoffdata == 0)
    {
      y = ypos + ON_OFF_CHECK_OFF_Y_OFFSET;
    }
    else
    {
      y = ypos + ON_OFF_CHECK_ON_Y_OFFSET;
    }

    display_draw_shaded_rect(xpos + ON_OFF_CHECK_X_OFFSET, y, &on_off_check_box, 0);

    //Need to display the ON and OFF text icons
    display_set_fg_color(COLOR_WHITE);
    display_copy_icon_fg_color(setting_menu_ON_icon, xpos + ON_OFF_TEXT_X_OFFSET, ypos + ON_OFF_TEXT_ON_Y_OFFSET, 20, 14);
    display_copy_icon_fg_color(setting_menu_OFF_icon, xpos + ON_OFF_TEXT_X_OFFSET, ypos + ON_OFF_TEXT_OFF_Y_OFFSET, 25, 14);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_level_arrow(uint32 direction)
{
  int32 xs = TRACE_HORIZONTAL_END - 7;
  int32 xe = xs;
  int32 y = TRACE_VERTICAL_START;
  int32 step = 1;
  int32 i;

  //For a down arrow the Y needs to start at the bottom and go up
  if(direction)
  {
    y = TRACE_VERTICAL_END - 1;
    step = -1;
  }

  for(i=0;i<5;i++)
  {
    display_draw_horz_line(y, xs, xe);

    xs--;
    xe++;
    y += step;
  }

  //For the down arrow the rectangle needs to start higher up
  if(direction)
  {
    y -= 5;
  }

  display_fill_rect(TRACE_HORIZONTAL_END - 8, y, 2, 5);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_position_arrow(uint32 direction)
{
  int32 x  = TRACE_HORIZONTAL_START;
  int32 ys = TRACE_VERTICAL_START + 6;
  int32 ye = ys;
  int32 step = 1;
  int32 i;

  //For a right arrow the X needs to start at the right side and go left
  if(direction)
  {
    x = TRACE_HORIZONTAL_END - 1;
    step = -1;
  }

  for(i=0;i<5;i++)
  {
    display_draw_vert_line(x, ys, ye);

    ys--;
    ye++;
    x += step;
  }

  //For the right arrow the rectangle needs to start more to the left
  if(direction)
  {
    x -= 5;
  }

  display_fill_rect(x, TRACE_VERTICAL_START + 5, 5, 2);
}

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA measurement_menu_highlight_box =
{
  141,
  24,
  { COLOR_GREEN, COLOR_ISLAMIC_GREEN, COLOR_PHARMACY_GREEN, COLOR_DARK_GREEN },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

const char *measurementslotnames[] =
{
  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_measurements_menu(void)
{
  uint16 x,y;

  //Fill the lighter background of the menu area
  display_set_fg_color(COLOR_DARK_GREY_1);
  display_fill_rect(380, 114, 313, 332);

  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(COLOR_DARK_GREY_3);
  display_draw_rect(379, 113, 316, 335);

  //Draw the lines to separate the header, the two channels and the current slot
  display_draw_horz_line(140, 380, 694);
  display_draw_vert_line(512, 114, 139);
  display_draw_vert_line(562, 114, 139);

  //Show the current slot name
  display_set_fg_color(FILE_NAME_HIGHLIGHT_COLOR);
  display_set_font(&font_3);
  display_text(531, 120, measurementslotnames[measurementslot]);

  //Set the x position for the currently selected item based on the used channel
  x = 383 + (scopesettings.measurementitems[measurementslot].channel * 167);

  //Set the y position for the currently selected item
  y = 142 + (scopesettings.measurementitems[measurementslot].index * 25);

  //Draw the menu high lighter box for the selected item (Original code used three rectangles)
  display_draw_highlight_rect(x, y, &measurement_menu_highlight_box);

  //Text is displayed in white
  display_set_fg_color(COLOR_WHITE);

  //Display the channel header texts
  display_copy_icon_fg_color(channel_1_text_icon, 452, 121, 23, 13);
  display_copy_icon_fg_color(channel_2_text_icon, 607, 121, 26, 13);

  //Display the list of measurement items per channel
  ui_display_measurements_menu_items(391, &scopesettings.channel1);
  ui_display_measurements_menu_items(558, &scopesettings.channel2);
}

//----------------------------------------------------------------------------------------------------------------------------------

const MSMITEMFUNCTION measurements_menu_item_functions[] =
{
  ui_msm_display_vmax,
  ui_msm_display_vmin,
  ui_msm_display_vavg,
  ui_msm_display_vrms,
  ui_msm_display_vpp,
  ui_msm_display_vp,
  ui_msm_display_freq,
  ui_msm_display_cycle,
  ui_msm_display_time_plus,
  ui_msm_display_time_min,
  ui_msm_display_duty_plus,
  ui_msm_display_duty_min
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_measurements_menu_items(uint32 xpos, PCHANNELSETTINGS settings)
{
  int i,x,y;

  //Text is displayed in white and labels are using a bigger font
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_3);

  //Display the labels for the measurements based on font_3
  for(i=0,y=146;i<(sizeof(measurement_names) / sizeof(uint8 *));i++)
  {
    //Draw the text per line
    display_text(xpos, y, measurement_names[i]);

    //Next line is 25 pixels down
    y += 25;
  }

  //The equal sign is shifted 52 pixels to the right
  x = xpos + 52;

  //A different font is used for the equal signs
  display_set_font(&font_0);

  //Display the equal sign for the measurements based on font_0
  for(i=0,y=149;i<(sizeof(measurement_names) / sizeof(uint8 *));i++)
  {
    //Draw the sign per line
    display_text(x, y, "=");

    //Next line is 25 pixels down
    y += 25;
  }

  //The right aligned location for the value is shifted 125 pixels to the right
  x = xpos + 125;

  //Display the values for the measurements
  for(i=0,y=147;i<(sizeof(measurements_menu_item_functions) / sizeof(MSMITEMFUNCTION));i++)
  {
    //Setup the value to display
    measurements_menu_item_functions[i](x, y, settings);

    //Draw the value aligned on the right per line
    display_right_aligned_text(x, y, globaldisplaytext);

    //Next line is 25 pixels down
    y += 25;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Functions for displaying the measurement values on the measurements menu
//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vmax(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the maximum take of the center ADC value
  ui_msm_display_voltage(settings, settings->max - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vmin(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the minimum take of the center ADC value
  ui_msm_display_voltage(settings, settings->min - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vavg(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the average take of the center ADC value
  ui_msm_display_voltage(settings, settings->average - 128);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vrms(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
 //The rms has already been centered during the summation so use it as is
  ui_msm_display_voltage(settings, settings->rms);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vpp(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //For the peak peak just use the value as is
  ui_msm_display_voltage(settings, settings->peakpeak);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_vp(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Determine the two absolute extremes
  int32 min = 128 - settings->min;
  int32 max = settings->max - 128;

  //Display the biggest of the two
  if(min > max)
  {
    //Use the below the center value when it is the biggest
    ui_msm_display_voltage(settings, min);
  }
  else
  {
    //Use the above the center value when it is the biggest
    ui_msm_display_voltage(settings, max);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_freq(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  if(settings->frequencyvalid)
  {
    //Format the frequency for displaying
    ui_msm_print_value(globaldisplaytext, settings->frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "Hz");
  }
  else
  {
    strcpy(globaldisplaytext, "xxxHz");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_cycle(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(globaldisplaytext, (((uint64)settings->periodtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(globaldisplaytext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_time_plus(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(globaldisplaytext, (((uint64)settings->hightime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(globaldisplaytext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_time_min(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(globaldisplaytext, (((uint64)settings->lowtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(globaldisplaytext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_duty_plus(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  char *buffer;

  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    buffer = ui_msm_print_decimal(globaldisplaytext, (((uint64)settings->hightime * 1000) / settings->periodtime), 1, 0);

    //Add the duty cycle sign
    strcpy(buffer, "%");

  }
  else
  {
    strcpy(globaldisplaytext, "xx%");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_duty_min(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  char *buffer;

  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    buffer = ui_msm_print_decimal(globaldisplaytext, (((uint64)settings->lowtime * 1000) / settings->periodtime), 1, 0);

    //Add the duty cycle sign
    strcpy(buffer, "%");

  }
  else
  {
    strcpy(globaldisplaytext, "xx%");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_voltage(PCHANNELSETTINGS settings, int32 value)
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
  ui_msm_print_value(globaldisplaytext, volts, vcd->volt_scale, "V");
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_print_value(char *buffer, int32 value, uint32 scale, char *designator)
{
  uint32 negative = 0;

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
    buffer = ui_msm_print_decimal(buffer, value, 2, negative);
  }
  else if(value < 10000)
  {
    //More then 1000 but less then 10000 means xx.y
    value /= 10;
    buffer = ui_msm_print_decimal(buffer, value, 1, negative);
  }
  else
  {
    //More then 10000 and less then 100000 means xxx
    value /= 100;
    buffer = ui_msm_print_decimal(buffer, value, 0, negative);
  }

  //Make sure scale is not out of range
  if(scale > 7)
  {
    scale = 7;
  }

  //Add the magnitude scaler
  buffer = strcpy(buffer, magnitude_scaler[scale]);

  //Add the type of measurement sign
  strcpy(buffer, designator);
}

//----------------------------------------------------------------------------------------------------------------------------------

char *ui_msm_print_decimal(char *buffer, int32 value, uint32 decimals, uint32 negative)
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

      //Take of the current digit
      value /= 10;

      //Check if decimal point needs to be placed
      if(i == 12 - decimals)
      {
        //If so put it in
        b[--i] = '.';

        //In case of 0.xx value the zero before the dot needs to be added
        if(value == 0)
        {
          b[--i] = '0';
        }
      }
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

//----------------------------------------------------------------------------------------------------------------------------------
// Picture and wave file handling and display functions
//----------------------------------------------------------------------------------------------------------------------------------

void ui_save_setup(PSCOPESETTINGS settings)
{
  //For now just copy the settings to the given struct
  memcpy(settings, &scopesettings, sizeof(SCOPESETTINGS));
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_restore_setup(PSCOPESETTINGS settings)
{
  //For now just copy the settings from the given struct
  memcpy(&scopesettings, settings, sizeof(SCOPESETTINGS));
  
  //On a change of sample rate or time per division it is necessary to re calculate the values for determining the number of point to display
  scope_calculate_sample_range_properties();
}

//----------------------------------------------------------------------------------------------------------------------------------
//These functions are for handling the settings to and from file

void ui_prepare_setup_for_file(void)
{
  uint32 *ptr = viewfilesetupdata;
  uint32 index = 0;
  uint32 measurement;
  uint32 checksum = 0;

  //Best to clear the buffer first since not all bytes are used
  memset((uint8 *)viewfilesetupdata, 0, sizeof(viewfilesetupdata));

  //Put in a version number for the waveform view file
  ptr[1] = WAVEFORM_FILE_ID1;
  ptr[2] = WAVEFORM_FILE_ID2;
  ptr[3] = WAVEFORM_FILE_VERSION;

  //Leave space for file version and checksum data
  index = CHANNEL1_SETTING_OFFSET;

  //Copy the needed channel 1 settings and measurements
  ptr[index++] = scopesettings.channel1.enable;
  ptr[index++] = scopesettings.channel1.displayvoltperdiv;
  ptr[index++] = scopesettings.channel1.samplevoltperdiv;
  ptr[index++] = scopesettings.channel1.fftenable;
  ptr[index++] = scopesettings.channel1.coupling;
  ptr[index++] = scopesettings.channel1.magnification;
  ptr[index++] = scopesettings.channel1.traceposition;
  ptr[index++] = scopesettings.channel1.min;
  ptr[index++] = scopesettings.channel1.max;
  ptr[index++] = scopesettings.channel1.average;
  ptr[index++] = scopesettings.channel1.center;
  ptr[index++] = scopesettings.channel1.peakpeak;
  ptr[index++] = scopesettings.channel1.frequencyvalid;
  ptr[index++] = scopesettings.channel1.frequency;
  ptr[index++] = scopesettings.channel1.lowtime;
  ptr[index++] = scopesettings.channel1.hightime;
  ptr[index++] = scopesettings.channel1.periodtime;

  //Leave some space for channel 1 settings changes
  index = CHANNEL2_SETTING_OFFSET;

  //Copy the needed channel 2 settings and measurements
  ptr[index++] = scopesettings.channel2.enable;
  ptr[index++] = scopesettings.channel2.displayvoltperdiv;
  ptr[index++] = scopesettings.channel2.samplevoltperdiv;
  ptr[index++] = scopesettings.channel2.fftenable;
  ptr[index++] = scopesettings.channel2.coupling;
  ptr[index++] = scopesettings.channel2.magnification;
  ptr[index++] = scopesettings.channel2.traceposition;
  ptr[index++] = scopesettings.channel2.min;
  ptr[index++] = scopesettings.channel2.max;
  ptr[index++] = scopesettings.channel2.average;
  ptr[index++] = scopesettings.channel2.center;
  ptr[index++] = scopesettings.channel2.peakpeak;
  ptr[index++] = scopesettings.channel2.frequencyvalid;
  ptr[index++] = scopesettings.channel2.frequency;
  ptr[index++] = scopesettings.channel2.lowtime;
  ptr[index++] = scopesettings.channel2.hightime;
  ptr[index++] = scopesettings.channel2.periodtime;

  //Leave some space for channel 2 settings changes
  index = TRIGGER_SETTING_OFFSET;

  //Copy the needed scope trigger settings
  ptr[index++] = scopesettings.timeperdiv;
  ptr[index++] = scopesettings.samplerate;
  ptr[index++] = scopesettings.triggermode;
  ptr[index++] = scopesettings.triggeredge;
  ptr[index++] = scopesettings.triggerchannel;
  ptr[index++] = scopesettings.triggerlevel;
  ptr[index++] = scopesettings.triggerhorizontalposition;
  ptr[index++] = scopesettings.triggerverticalposition;
  ptr[index++] = disp_trigger_index;

  //Leave some space for trigger information changes
  index = OTHER_SETTING_OFFSET;

  //Copy the needed other scope settings
  ptr[index++] = scopesettings.movespeed;
  ptr[index++] = scopesettings.screenbrightness;
  ptr[index++] = scopesettings.gridbrightness;
  ptr[index++] = scopesettings.alwaystrigger50;
  ptr[index++] = scopesettings.tracedisplaymode;
  ptr[index++] = scopesettings.confirmationmode;

  //Leave some space for other scope settings changes
  index = CURSOR_SETTING_OFFSET;

  //Copy the cursor settings
  ptr[index++] = scopesettings.selectedcursor;
  ptr[index++] = scopesettings.timecursorsenable;
  ptr[index++] = scopesettings.voltcursorsenable;
  ptr[index++] = scopesettings.timecursor1position;
  ptr[index++] = scopesettings.timecursor2position;
  ptr[index++] = scopesettings.voltcursor1position;
  ptr[index++] = scopesettings.voltcursor2position;

  //Leave some space for other cursor settings changes
  index = MEASUREMENT_SETTING_OFFSET;

    //Save the measurement slots states
  for(measurement=0;measurement<6;measurement++)
  {
    //Copy the current measurement channel and index
    ptr[index++] = scopesettings.measurementitems[measurement].channel;
    ptr[index++] = scopesettings.measurementitems[measurement].index;
  }

  //Calculate a checksum over the settings data
  for(index=1;index<VIEW_NUMBER_OF_SETTINGS;index++)
  {
    checksum += ptr[index];
  }

  //Add the sample data too
  for(index=0;index<750;index++)
  {
    //Add both the channels
    checksum += channel1tracebuffer[index];
    checksum += channel2tracebuffer[index];
  }

  //Store the checksum at the beginning of the file
  ptr[0] = checksum;
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_restore_setup_from_file(void)
{
  uint32 *ptr = viewfilesetupdata;
  uint32 index = 0;
  uint32 measurement;

  //Leave space for file version and checksum data
  index = CHANNEL1_SETTING_OFFSET;

  //Copy the needed channel 1 settings and measurements
  scopesettings.channel1.enable            = ptr[index++];
  scopesettings.channel1.displayvoltperdiv = ptr[index++];
  scopesettings.channel1.samplevoltperdiv  = ptr[index++];
  scopesettings.channel1.fftenable         = ptr[index++];
  scopesettings.channel1.coupling          = ptr[index++];
  scopesettings.channel1.magnification     = ptr[index++];
  scopesettings.channel1.traceposition     = ptr[index++];
  scopesettings.channel1.min               = ptr[index++];
  scopesettings.channel1.max               = ptr[index++];
  scopesettings.channel1.average           = ptr[index++];
  scopesettings.channel1.center            = ptr[index++];
  scopesettings.channel1.peakpeak          = ptr[index++];
  scopesettings.channel1.frequencyvalid    = ptr[index++];
  scopesettings.channel1.frequency         = ptr[index++];
  scopesettings.channel1.lowtime           = ptr[index++];
  scopesettings.channel1.hightime          = ptr[index++];
  scopesettings.channel1.periodtime        = ptr[index++];

  //Leave some space for channel 1 settings changes
  index = CHANNEL2_SETTING_OFFSET;

  //Copy the needed channel 2 settings and measurements
  scopesettings.channel2.enable            = ptr[index++];
  scopesettings.channel2.displayvoltperdiv = ptr[index++];
  scopesettings.channel2.samplevoltperdiv  = ptr[index++];
  scopesettings.channel2.fftenable         = ptr[index++];
  scopesettings.channel2.coupling          = ptr[index++];
  scopesettings.channel2.magnification     = ptr[index++];
  scopesettings.channel2.traceposition     = ptr[index++];
  scopesettings.channel2.min               = ptr[index++];
  scopesettings.channel2.max               = ptr[index++];
  scopesettings.channel2.average           = ptr[index++];
  scopesettings.channel2.center            = ptr[index++];
  scopesettings.channel2.peakpeak          = ptr[index++];
  scopesettings.channel2.frequencyvalid    = ptr[index++];
  scopesettings.channel2.frequency         = ptr[index++];
  scopesettings.channel2.lowtime           = ptr[index++];
  scopesettings.channel2.hightime          = ptr[index++];
  scopesettings.channel2.periodtime        = ptr[index++];

  //Leave some space for channel 2 settings changes
  index = TRIGGER_SETTING_OFFSET;

  //Copy the needed scope trigger settings
  scopesettings.timeperdiv                = ptr[index++];
  scopesettings.samplerate                = ptr[index++];
  scopesettings.triggermode               = ptr[index++];
  scopesettings.triggeredge               = ptr[index++];
  scopesettings.triggerchannel            = ptr[index++];
  scopesettings.triggerlevel              = ptr[index++];
  scopesettings.triggerhorizontalposition = ptr[index++];
  scopesettings.triggerverticalposition   = ptr[index++];
  disp_trigger_index                      = ptr[index++];

  //Leave some space for trigger information changes
  index = OTHER_SETTING_OFFSET;

  //Copy the needed other scope settings
  scopesettings.movespeed        = ptr[index++];
  scopesettings.screenbrightness = ptr[index++];
  scopesettings.gridbrightness   = ptr[index++];
  scopesettings.alwaystrigger50  = ptr[index++];
  scopesettings.tracedisplaymode = ptr[index++];
  scopesettings.confirmationmode = ptr[index++];

  //Leave some space for other scope settings changes
  index = CURSOR_SETTING_OFFSET;

  //Copy the cursor settings
  scopesettings.selectedcursor      = ptr[index++];
  scopesettings.timecursorsenable   = ptr[index++];
  scopesettings.voltcursorsenable   = ptr[index++];
  scopesettings.timecursor1position = ptr[index++];
  scopesettings.timecursor2position = ptr[index++];
  scopesettings.voltcursor1position = ptr[index++];
  scopesettings.voltcursor2position = ptr[index++];

  //Leave some space for other cursor settings changes
  index = MEASUREMENT_SETTING_OFFSET;

  //Restore the measurement slots states
  for(measurement=0;measurement<6;measurement++)
  {
    //Copy the current measurement channel and index
    scopesettings.measurementitems[measurement].channel = ptr[index++];
    scopesettings.measurementitems[measurement].index   = ptr[index++];

    //Set the pointer to the actual channel data based on the selected channel
    if(scopesettings.measurementitems[measurement].channel == 0)
    {
      scopesettings.measurementitems[measurement].channelsettings = &scopesettings.channel1;
    }
    else
    {
      scopesettings.measurementitems[measurement].channelsettings = &scopesettings.channel2;
    }
  }
  
  //On a change of sample rate or time per division it is necessary to re calculate the values for determining the number of point to display
  scope_calculate_sample_range_properties();
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_check_waveform_file(void)
{
  uint32 index;
  uint32 checksum = 0;

  //Calculate a checksum over the settings data
  for(index=1;index<VIEW_NUMBER_OF_SETTINGS;index++)
  {
    checksum += viewfilesetupdata[index];
  }

  //Add the sample data too
  for(index=0;index<750;index++)
  {
    //Add both the channels
    checksum += channel1tracebuffer[index];
    checksum += channel2tracebuffer[index];
  }

  //Check if it matches the checksum in the file
  if(viewfilesetupdata[0] == checksum)
  {
    return(0);
  }

  //Something is wrong so signal it
  return(-1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_print_file_name(uint32 filenumber)
{
  char    b[12];
  uint32  i = 12;   //Start beyond the array since the index is pre decremented
  uint32  s;

  //For file number 0 no need to do the work
  if(filenumber == 0)
  {
    //Value is zero so just set a 0 character
    b[--i] = '0';
  }
  else
  {
    //Process the digits
    while(filenumber)
    {
      //Set current digit to decreased index
      b[--i] = (filenumber % 10) + '0';

      //Take of the current digit
      filenumber /= 10;
    }
  }

  //Determine the size of the decimal part
  s = 12 - i;

  //Copy the path name first
  memcpy(viewfilename, view_file_path[viewtype & VIEW_TYPE_MASK].name, view_file_path[viewtype & VIEW_TYPE_MASK].length);

  //Copy in the decimal file number
  memcpy(&viewfilename[view_file_path[viewtype & VIEW_TYPE_MASK].length], &b[i], s);

  //Add the extension
  memcpy(&viewfilename[view_file_path[viewtype & VIEW_TYPE_MASK].length + s], view_file_extension[viewtype & VIEW_TYPE_MASK], 5);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_load_thumbnail_file(void)
{
  int32  result;
  uint32 size;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, view_file_path[viewtype & VIEW_TYPE_MASK].name);

  //Check the status of the directory for this view type
  result = f_stat(viewfilename, 0);

  //See if there is an error
  if(result != FR_OK)
  {
    //If so check if the directory does not exist
    if(result == FR_NO_FILE)
    {
      //Create the directory
      result = f_mkdir(viewfilename);

      if(result != FR_OK)
      {
        //Show a message stating creating the directory failed
        ui_display_file_status_message(MESSAGE_DIRECTORY_CREATE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Set the name in the global buffer for message display
      strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

      //With the directory created it is also needed to create the thumbnail file
      result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

      if(result != FR_OK)
      {
        //Show a message stating creating the file failed
        ui_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Reset the number of available items
      viewavailableitems = 0;

      //Write the no thumbnails yet data
      result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

      //Close the file
      f_close(&viewfp);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //No items to be loaded so done
      return(0);
    }
    else
    {
      //Show a message stating the file system failed
      ui_display_file_status_message(MESSAGE_FILE_SYSTEM_FAILED, 1);

      //No sense to continue, so return with an error
      return(-1);
    }
  }

  //Clear the file number list to avoid errors when swapping between the two types
  memset(viewfilenumberdata, 0, sizeof(viewfilenumberdata));

  //Set the name in the global buffer for message display
  strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

  //Try to open the thumbnail file for this view type
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check the result
  if(result == FR_OK)
  {
    //Opened ok, so read the number of items
    result = f_read(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

    if(result != FR_OK)
    {
      //Show a message stating reading the file failed
      ui_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

      //Close the file
      f_close(&viewfp);

      //No sense to continue, so return with an error
      return(-1);
    }

    //Based on the number of available items load the rest of the data
    if(viewavailableitems)
    {
      //Calculate the number of bytes to read for the file number list
      size = viewavailableitems * sizeof(uint16);

      //Check if there is an error
      if(size > VIEW_FILE_NUMBER_DATA_SIZE)
      {
        //Show a message stating that the thumbnail file is corrupt
        ui_display_file_status_message(MESSAGE_THUMBNAIL_FILE_CORRUPT, 0);

        //Reset the number of available items
        viewavailableitems = 0;

        //Write the no thumbnails yet data
        result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

        //Close the file
        f_close(&viewfp);

        if(result != FR_OK)
        {
          //Show a message stating writing the file failed
          ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

          //No sense to continue, so return with an error
          return(-1);
        }

        //No items to be loaded any more so done
        return(0);
      }

      //Read the file number data
      result = f_read(&viewfp, viewfilenumberdata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating reading the file failed
        ui_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Calculate the number of bytes to read for the thumbnail data
      size = viewavailableitems * sizeof(THUMBNAILDATA);

      //Check if there is an error
      if(size > VIEW_THUMBNAIL_DATA_SIZE)
      {
        //Show a message stating that the thumbnail file is corrupt
        ui_display_file_status_message(MESSAGE_THUMBNAIL_FILE_CORRUPT, 0);

        //Reset the number of available items
        viewavailableitems = 0;

        //Write the no thumbnails yet data
        result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

        //Close the file
        f_close(&viewfp);

        if(result != FR_OK)
        {
          //Show a message stating writing the file failed
          ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

          //No sense to continue, so return with an error
          return(-1);
        }

        //No items to be loaded any more so done
        return(0);
      }

      //Read the thumbnail data
      result = f_read(&viewfp, viewthumbnaildata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating reading the file failed
        ui_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }
    }

    //Close the file
    f_close(&viewfp);
  }
  //Failure then check if file does not exist
  else if(result == FR_NO_FILE)
  {
    //Need the file so create it
    result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

    //Check if file is created ok
    if(result == FR_OK)
    {
      //Reset the number of available items
      viewavailableitems = 0;

      //Write the no thumbnails yet data
      result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

      //Close the file
      f_close(&viewfp);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }
    }
    else
    {
      //Show a message stating creating the file failed
      ui_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);

      //No sense to continue, so return with an error
      return(-1);
    }
  }

  //Signal all went well
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_save_thumbnail_file(void)
{
  int32  result;
  uint32 size;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

  //Try to open the thumbnail file for this view type
  result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

  //Only if the file is opened write to it
  if(result == FR_OK)
  {
    //Write the number of available items to the file
    result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

    if(result != FR_OK)
    {
      //Show a message stating writing the file failed
      ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

      //Close the file
      f_close(&viewfp);

      //No sense to continue, so return with an error
      return(-1);
    }

    //Based on the number of available items write the rest of the data
    if(viewavailableitems)
    {
      //Calculate the number of bytes to write for the file number list
      size = viewavailableitems * sizeof(uint16);

      //Write the file number list to the file
      result = f_write(&viewfp, viewfilenumberdata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Calculate the number of bytes to write for the thumbnail data
      size = viewavailableitems * sizeof(THUMBNAILDATA);

      //Write the thumbnail data to the file
      result = f_write(&viewfp, viewthumbnaildata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }
    }

    //Close the file
    f_close(&viewfp);
  }
  else
  {
    //Show a message stating the file system failed
    ui_display_file_status_message(MESSAGE_FILE_SYSTEM_FAILED, 1);

    //No sense to continue, so return with an error
    return(-1);
  }

  //Signal no problem occurred
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_save_view_item_file(int32 type)
{
  uint32  newnumber;
  uint32  result;
  uint16 *fnptr;
  uint16 *eptr;

  //Save the current view type to be able to determine if the thumbnail file need to be reloaded
  uint32 currentviewtype = viewtype;

  //Switch to the given type
  viewtype = type;

  //Load the thumbnail file for this type. Needed for finding the file name and to add the thumbnail
  if(ui_load_thumbnail_file() != 0)
  {
    //Loading the thumbnail file failed so no sense in going on
    return;
  }

  //Check if there is still room for a new item
  if(viewavailableitems >= VIEW_MAX_ITEMS)
  {
    //Show the user there is no more room for a new item
    ui_display_file_status_message(MESSAGE_THUMBNAIL_FILE_FULL, 1);

    //No sense to continue
    return;
  }

  //Set the end pointer
  eptr = &viewfilenumberdata[viewavailableitems];

  //Find the first free file number
  //Most likely a more efficient solution for this problem exists, but this beats the original code where they try if a file number is free on the SD card with f_open
  for(newnumber=1;newnumber<VIEW_MAX_ITEMS;newnumber++)
  {
    //Start at the beginning of the list
    fnptr = viewfilenumberdata;

    //Go through the list to see if the current number is in the list
    while(fnptr < eptr)
    {
      //Check if this number is in the list
      if(*fnptr == newnumber)
      {
        //Found it, so quit the loop
        break;
      }

      //Select the next number entry
      fnptr++;
    }

    //Check if not found
    if(*fnptr != newnumber)
    {
      //Can use this number since it is not in the list
      break;
    }
  }

  //Bump all the entries in the list up
  memmove(&viewfilenumberdata[1], &viewfilenumberdata[0], viewavailableitems * sizeof(uint16));

  //Fill in the new number
  viewfilenumberdata[0] = newnumber;

  //Bump the thumbnails up to make room for the new one
  memmove(&viewthumbnaildata[1], &viewthumbnaildata[0], viewavailableitems * sizeof(THUMBNAILDATA));

  //Setup the filename for in the thumbnail
  ui_print_file_name(newnumber);

  //Create the thumbnail
  ui_create_thumbnail(&viewthumbnaildata[0]);

  //One more item in the list
  viewavailableitems++;

  //save the amended thumbnail file
  ui_save_thumbnail_file();

  //Copy the filename from the thumbnail filename, since the global one got written over in the saving of the thumbnail
  //Might need a re write of the message setup
  strcpy(viewfilename, viewthumbnaildata[0].filename);

  //Open the new file. On failure signal this and quit
  result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

  //Check if file created without problems
  if(result == FR_OK)
  {
    //For pictures the bitmap header and the screen data needs to be written
    if(type == VIEW_TYPE_PICTURE)
    {
      //Write the bitmap header
      result = f_write(&viewfp, bmpheader, sizeof(bmpheader), 0);

      //Check if still ok to proceed
      if(result == FR_OK)
      {
        //Write the pixel data
        result = f_write(&viewfp, (uint8 *)maindisplaybuffer, PICTURE_DATA_SIZE, 0);
      }
    }
    else
    {
      //For the waveform the setup and the waveform data needs to be written
      //Save the settings for the trace portion of the data and write them to the file
      ui_prepare_setup_for_file();

      //Write the setup data to the file
      if((result = f_write(&viewfp, viewfilesetupdata, sizeof(viewfilesetupdata), 0)) == FR_OK)
      {
        //Write the trace data to the file
        //Save the channel 1 raw sample data
        if((result = f_write(&viewfp, (uint8 *)channel1tracebuffer, 3000, 0)) == FR_OK)
        {
          //Save the channel 2 raw sample data
          result = f_write(&viewfp, (uint8 *)channel2tracebuffer, 3000, 0);
        }
      }
    }

    //Close the file
    f_close(&viewfp);

    //Check if all went well
    if(result == FR_OK)
    {
      //Show the saved successful message
      ui_display_file_status_message(MESSAGE_SAVE_SUCCESSFUL, 0);
    }
    else
    {
      //Signal unable to write to the file
      ui_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to create the file
    ui_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);
  }

  //When a picture is saved while viewing a waveform, reload the waveform lists
  if((type == VIEW_TYPE_PICTURE) && (currentviewtype == VIEW_TYPE_WAVEFORM) && (scopesettings.waveviewmode == 1))
  {
    //Restore the previous view type
    viewtype = currentviewtype;

    //Load the thumbnail file
    ui_load_thumbnail_file();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_remove_item_from_thumbnails(uint32 delete)
{
  //Set the index to the next item
  uint32 nextindex = viewcurrentindex + 1;

  //Calculate the number of items to move
  uint32 count = (viewavailableitems - nextindex);

  //Make sure there is an item available to remove
  if(viewavailableitems)
  {
    //Only delete the file when requested
    if(delete)
    {
      //Set the name in the global buffer for message display
      strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

      //Delete the file from the SD card
      if(f_unlink(viewfilename) != FR_OK)
      {
        //Signal unable to delete the file
        ui_display_file_status_message(MESSAGE_FILE_DELETE_FAILED, 0);
      }
    }

    //Bump all the entries in the file number list down
    memmove(&viewfilenumberdata[viewcurrentindex], &viewfilenumberdata[nextindex], count * sizeof(uint16));

    //Bump the thumbnails down to erase the removed one
    memmove(&viewthumbnaildata[viewcurrentindex], &viewthumbnaildata[nextindex], count * sizeof(THUMBNAILDATA));

    //One less item available
    viewavailableitems--;

    //Clear the freed up slot
    viewfilenumberdata[viewavailableitems] = 0;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_load_trace_data(void)
{
  //Point to the file numbers
  uint16 *fnptr = (uint16 *)viewfilenumberdata;
  uint32 result;

  //Setup the file name for this view item
  ui_print_file_name(fnptr[viewcurrentindex]);

  //Try to open the file for reading
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check if file opened ok
  if(result == FR_OK)
  {
    //Checks on correct number of bytes read might be needed
    //Load the setup data to the file setup data buffer
    if((result = f_read(&viewfp, (uint8 *)viewfilesetupdata, sizeof(viewfilesetupdata), 0)) == FR_OK)
    {
      //Copy the loaded data to the settings
      ui_restore_setup_from_file();

      //Check if the version of the file is wrong
      if((viewfilesetupdata[1] != WAVEFORM_FILE_ID1) || (viewfilesetupdata[2] != WAVEFORM_FILE_ID2) || (viewfilesetupdata[3] != WAVEFORM_FILE_VERSION))
      {
        //No need to load the rest of the data
        result = WAVEFORM_FILE_ERROR;

        //Show the user the file is not correct
        ui_display_file_status_message(MESSAGE_WAV_VERSION_MISMATCH, 0);
      }
      else
      {
        //Load the channel 1 sample data
        if((result = f_read(&viewfp, (uint8 *)channel1tracebuffer, 3000, 0)) == FR_OK)
        {
          //Load the channel 2 sample data
          if((result = f_read(&viewfp, (uint8 *)channel2tracebuffer, 3000, 0)) == FR_OK)
          {
            //Do a check on file validity
            if((result = ui_check_waveform_file()) == 0)
            {
              //Switch to stopped and waveform viewing mode
              scopesettings.runstate = RUN_STATE_STOPPED;
              scopesettings.waveviewmode = 1;

              //Allow redrawing of the trace display
              enabletracedisplay = TRACE_DISPLAY_ENABLED;

              //Show the normal scope screen
              ui_setup_main_screen();

              //display the trace data
              scope_display_trace_data();
            }
            else
            {
              //Checksum error so signal that to the user
              result = WAVEFORM_FILE_ERROR;

              //Show the user the file is not correct
              ui_display_file_status_message(MESSAGE_WAV_CHECKSUM_ERROR, 0);
            }
          }
        }
      }
    }

    //Done with the file so close it
    f_close(&viewfp);

    //Check if one of the reads failed
    if((result != FR_OK) && (result != WAVEFORM_FILE_ERROR))
    {
      //Signal unable to write to the file
      ui_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to open the file
    ui_display_file_status_message(MESSAGE_FILE_OPEN_FAILED, 0);
  }

  //Check if all went well
  if(result == FR_OK)
  {
    //Tell it to the caller
    return(VIEW_TRACE_LOAD_OK);
  }

  //Remove the current item from the thumbnnails and delete the item from disk, since the file is faulty and thus no longer needed
  ui_remove_item_from_thumbnails(1);

  //Save the thumbnail file
  ui_save_thumbnail_file();

  return(VIEW_TRACE_LOAD_ERROR);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_load_bitmap_data(void)
{
  uint32 result;
  uint32 dodelete = 1;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

  //Try to open the file for reading
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check if file opened ok
  if(result == FR_OK)
  {
    //Read the bitmap header to verify if the bitmap can be displayed
    result = f_read(&viewfp, viewbitmapheader, PICTURE_HEADER_SIZE, 0);

    //Check if still ok to proceed
    if(result == FR_OK)
    {
      //Check if the header matches what it should be
      if(memcmp(viewbitmapheader, bmpheader, PICTURE_HEADER_SIZE) == 0)
      {
        //Load the bitmap data directly onto the screen
        result = f_read(&viewfp, (uint8 *)maindisplaybuffer, PICTURE_DATA_SIZE, 0);

        //Show the filename on the bottom of the picture
        display_set_fg_color(FILE_NAME_HIGHLIGHT_COLOR);
        display_set_font(&font_0);
        display_text(VIEW_FILENAME_XPOS, VIEW_FILENAME_YPOS, viewfilename);
      }
      else
      {
        //Signal a header mismatch detected
        result = PICTURE_HEADER_MISMATCH;

        //Show the user the file is not correct
        ui_display_file_status_message(MESSAGE_BMP_HEADER_MISMATCH, 0);
      }
    }

    //Done with the file so close it
    f_close(&viewfp);

    //Check if one of the reads failed
    if((result != FR_OK) && (result != PICTURE_HEADER_MISMATCH))
    {
      //Signal unable to read from the file
      ui_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to open the file
    ui_display_file_status_message(MESSAGE_FILE_OPEN_FAILED, 0);

    //No need to delete the file since it won't be there
    dodelete = 0;
  }

  //Check if all went well
  if(result == FR_OK)
  {
    //Tell it to the caller
    return(VIEW_BITMAP_LOAD_OK);
  }

  //Since there is an error with the file no need to keep it in the thumbnails
  //Remove the current item from the thumbnnails and if needed, delete the item from disk
  ui_remove_item_from_thumbnails(dodelete);

  //Save the thumbnail file
  ui_save_thumbnail_file();

  return(VIEW_BITMAP_LOAD_ERROR);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_sync_thumbnail_files(void)
{
  uint32 save = 0;

  //Handle the two types of list files
  for(viewtype=0;viewtype<VIEW_MAX_TYPES;viewtype++)
  {
    //Load the thumbnail file for this type
    if(ui_load_thumbnail_file() != 0)
    {
      //Loading the thumbnail file failed so no sense in going on for this type
      continue;
    }

    //Go through the items in the thumbnail file and check if the needed files still exist on the SD card
    for(viewcurrentindex=0;viewcurrentindex<viewavailableitems;)
    {
      //Set the name in the global buffer for message display
      strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

      //Try to open the file. On failure remove it from the lists
      if(f_open(&viewfp, viewfilename, FA_READ) == FR_NO_FILE)
      {
        //Remove the current item from the thumbnails without delete, since it is already removed from the SD card
        ui_remove_item_from_thumbnails(0);

        //Signal saving of the thumbnail file is needed
        save = 1;
      }
      else
      {
        //File exists so close it
        f_close(&viewfp);

        //Point to the next item. Only needed if item still exists, because it is removed from the list otherwise
        viewcurrentindex++;
      }
    }

    //Check if there was a change
    if(save)
    {
      //Save the thumbnail file if so
      ui_save_thumbnail_file();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_initialize_and_display_thumbnails(void)
{
  //No pages yet
  viewpages = 0;

  //Calculate the number of pages available, based on number of items per page. 0 means 1 page
  //available items starts with 1 and with 16 items it would result in pages being 1, so need to subtract 1 before dividing
  if(viewavailableitems)
  {
    viewpages = (viewavailableitems - 1) / VIEW_ITEMS_PER_PAGE;
  }

  //Check if the current item is still within range
  if(viewcurrentindex >= viewavailableitems)
  {
    //Set it to the last one if out of range
    viewcurrentindex = viewavailableitems - 1;
  }

  //Need to check if the current page is still valid
  if(viewpage > viewpages)
  {
    //Page no longer valid then use last page
    viewpage = viewpages;
  }

  //Display the thumbnails
  ui_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_thumbnails(void)
{
  PTHUMBNAILDATA thumbnaildata;

  //Determine the first index based on the current page
  uint32 index = viewpage * VIEW_ITEMS_PER_PAGE;

  //Start with first item for drawing
  uint32 xpos = VIEW_ITEM_XSTART;
  uint32 ypos = VIEW_ITEM_YSTART;

  uint32 x,y;

  char *pagetext;

  //Use a separate buffer to clear the screen so it won't flicker when previous or next item is highlighted
  display_set_screen_buffer(displaybuffer1);

  //Set black color for background
  display_set_fg_color(COLOR_BLACK);

  //Clear the screen
  display_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  //Check if there are items to display
  if(viewavailableitems)
  {
    //Determine the available items for the current page
    if(viewpage < viewpages)
    {
      //Not on the last page so full set available
      viewitemsonpage = VIEW_ITEMS_PER_PAGE;
    }
    else
    {
      //Get the remainder of items for the last page
      uint32 nofitems = viewavailableitems % VIEW_ITEMS_PER_PAGE;

      //See if a fraction of the max items per page is available
      if(nofitems)
      {
        //If so only display these
        viewitemsonpage = nofitems;
      }
      else
      {
        //If the remainder is zero there are max number of items on the last page
        viewitemsonpage = VIEW_ITEMS_PER_PAGE;
      }
    }

    //Determine the last index based on the available items on the current page
    uint32 lastindex = index + viewitemsonpage;

    //Draw the available items on the screen
    while(index < lastindex)
    {
      //Create a nicer thumbnail by drawing the top information bar and the measurements
      y = ypos + 1;

      //Fill in the top bar
      display_copy_icon_full_color(thumbnail_top_bar_icon, xpos + 2, y + 1, 169, 9);

      //Fill in the side bar
      display_copy_icon_full_color(thumbnail_side_bar_icon, xpos + 173, y, 26, 118);

      //Check on highlighted item
      if(index == viewcurrentindex)
      {
        display_set_fg_color(COLOR_DARK_GREY_7);
        display_fill_rect(xpos + 3, ypos + 12, VIEW_ITEM_WIDTH - 33, VIEW_ITEM_HEIGHT - 29);
      }

      //Draw a grid
      display_set_fg_color(COLOR_DARK_GREY_6);

      //Draw the center lines
      display_draw_horz_line(ypos + 60, xpos + 3, xpos + 169);
      display_draw_vert_line(xpos + 86, ypos + 12, ypos + VIEW_ITEM_HEIGHT - 16);

      //Point to the current thumbnail
      thumbnaildata = &viewthumbnaildata[index];

      //Display the thumbnail
      //Need to make a distinction between normal display and xy display mode
      if(thumbnaildata->tracedisplaymode == DISPLAY_MODE_NORMAL)
      {
        //Normal mode
        //To avoid errors make sure the positions are in range
        //Data is read back from file so could be modified
        if(thumbnaildata->disp_xstart < 3)
        {
          thumbnaildata->disp_xstart = 3;
        }

        if(thumbnaildata->disp_xend > 169)
        {
          thumbnaildata->disp_xend = 169;
        }

        //Set the x start position based on the given start x.
        uint32 xs = xpos + thumbnaildata->disp_xstart;
        uint32 xe = xpos + thumbnaildata->disp_xend;

        //Offset the trace data to just on the signal area border
        y = ypos + 11;

        //Check if channel 1 is enabled
        if(thumbnaildata->channel1enable)
        {
          ui_display_thumbnail_data(xs, xe, y, CHANNEL1_COLOR, thumbnaildata->channel1data);
        }

        //Check if channel 2 is enabled
        if(thumbnaildata->channel2enable)
        {
          ui_display_thumbnail_data(xs, xe, y, CHANNEL2_COLOR, thumbnaildata->channel2data);
        }
      }
      else
      {
        //xy display mode so set the trace color for it
        display_set_fg_color(XYMODE_COLOR);

        //Point to the data of the two channels
        uint8 *channel1data = thumbnaildata->channel1data;
        uint8 *channel2data = thumbnaildata->channel2data;

        //Start with first sample
        uint32 sample = 0;

        //Center the xy display
        uint32 y = ypos + 12;

        //Keep the samples in registers
        register uint32 x1, x2, y1, y2;

        //Load the first samples
        x1 = *channel1data + xpos;
        y1 = *channel2data + y;

        //Point to the next samples
        channel1data++;
        channel2data++;

        //Draw the trace
        while(sample < 172)
        {
          //Get second samples
          x2 = *channel1data + xpos;
          y2 = *channel2data + y;

          //Draw all the lines
          display_draw_line(x1, y1, x2, y2);

          //Swap the samples
          x1 = x2;
          y1 = y2;

          //Point to the next samples
          channel1data++;
          channel2data++;

          //One sample done
          sample++;
        }
      }

      //Draw the grey trace border after the traces has been drawn to mask the out of the screen pixels
      display_set_fg_color(COLOR_LIGHT_GREY_9);
      display_draw_rect(xpos + 2, ypos + 11, VIEW_ITEM_WIDTH - 30, VIEW_ITEM_HEIGHT - 26);

      //Need to make a distinction between normal display and xy display mode for displaying the pointers
      if(thumbnaildata->tracedisplaymode == DISPLAY_MODE_NORMAL)
      {
        //Channel pointers position bases
        x = xpos + 3;
        y = ypos + 12;

        //Check if channel 1 is enabled
        if(thumbnaildata->channel1enable)
        {
          //Limit the position to the extremes
          if(thumbnaildata->channel1traceposition > 92)
          {
            thumbnaildata->channel1traceposition = 92;
          }

          //If so draw its pointer
          ui_thumbnail_draw_pointer(x, y + thumbnaildata->channel1traceposition, THUMBNAIL_POINTER_RIGHT, CHANNEL1_COLOR);
        }

        //Check if channel 2 is enabled
        if(thumbnaildata->channel2enable)
        {
          //Limit the position to the extremes
          if(thumbnaildata->channel2traceposition > 92)
          {
            thumbnaildata->channel2traceposition = 92;
          }

          //If so draw its pointer
          ui_thumbnail_draw_pointer(x, y + thumbnaildata->channel2traceposition, THUMBNAIL_POINTER_RIGHT, CHANNEL2_COLOR);
        }

        //Trigger level position base
        x = xpos + 170;

        //Limit the position to the extremes
        if(thumbnaildata->triggerverticalposition > 92)
        {
          thumbnaildata->triggerverticalposition = 92;
        }

        //Draw the trigger level pointer
        ui_thumbnail_draw_pointer(x, y + thumbnaildata->triggerverticalposition, THUMBNAIL_POINTER_LEFT, TRIGGER_COLOR);

        //Limit the position to the extremes
        if(thumbnaildata->triggerhorizontalposition < 3)
        {
          thumbnaildata->triggerhorizontalposition = 3;
        }
        else if(thumbnaildata->triggerhorizontalposition > 165)
        {
          thumbnaildata->triggerhorizontalposition = 165;
        }

        //Draw the trigger position pointer
        ui_thumbnail_draw_pointer(xpos + thumbnaildata->triggerhorizontalposition, y, THUMBNAIL_POINTER_DOWN, TRIGGER_COLOR);
      }
      else
      {
//Draw the pointers here

      }

      //Check on select mode being enabled
      if(viewselectmode)
      {
        //Set the colors for displaying the selected sign. White sign on blue background
        display_set_fg_color(COLOR_WHITE);
        display_set_bg_color(COLOR_BLUE);

        //Draw an empty box to indicate select mode
        display_draw_rect(xpos + 71, ypos + 44, 32, 32);
        display_draw_rect(xpos + 72, ypos + 45, 30, 30);

        //Check if the item is selected
        if(viewitemselected[index % VIEW_ITEMS_PER_PAGE])
        {
          //Display the selected sign
          display_copy_icon_use_colors(select_sign_icon, xpos + 72, ypos + 45, 30, 30);
        }
      }

      //Highlight the filename of the selected item
      if(index == viewcurrentindex)
      {
        display_set_fg_color(FILE_NAME_HIGHLIGHT_COLOR);
      }
      else
      {
        display_set_fg_color(COLOR_WHITE);
      }

      //Display the file name in the bottom left corner
      display_set_font(&font_2);
      display_text(xpos + 7, ypos + 105, thumbnaildata->filename);

      //Set a nice color for item border
      display_set_fg_color(FILE_BORDER_COLOR);  //0x00CC8947

      //Draw the border
      display_draw_rect(xpos, ypos, VIEW_ITEM_WIDTH, VIEW_ITEM_HEIGHT);

      //Skip to next coordinates
      xpos += VIEW_ITEM_XNEXT;

      //Check if next row needs to be used
      if(xpos > VIEW_ITEM_XLAST)
      {
        //Reset x position to beginning of row
        xpos = VIEW_ITEM_XSTART;

        //Bump y position to next row
        ypos += VIEW_ITEM_YNEXT;
      }

      //Select next index
      index++;
    }

    //Display page number if more then one page available
    if(viewpages)
    {
      //Show the page text in the highlight color
      display_set_fg_color(FILE_NAME_HIGHLIGHT_COLOR);
      display_set_font(&font_3);

      //Point to after the "PAGE " text to print the page number
      pagetext = viewfilename + 5;

      //Load in the "PAGE " text
      memcpy(viewfilename, "PAGE ", 5);

      //Add the actual page number. Internally viewpage starts at 0
      pagetext = ui_print_decimal_number(pagetext, viewpage + 1);

      //Put in the separator between the page number and the number of pages
      *pagetext++ = '/';

      //Add the actual number of pages. Here also one less is used internally
      ui_print_decimal_number(pagetext, viewpages + 1);

      //Display it on the screen
      display_text(5, 2, viewfilename);
    }
  }
  else
  {
    //Show the user there are no items available
    display_set_fg_color(COLOR_WHITE);
    display_set_font(&font_4);
    display_text(254, 225, "NO ITEMS AVAILABLE");
  }

  //Copy the new screen to the actual screen buffer
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_thumbnail_data(uint32 xstart, uint32 xend, uint32 ypos, uint32 color, uint8 *buffer)
{
  register uint32 x;
  register uint32 sample1, sample2;

  //Set the trace color
  display_set_fg_color(color);

  //Get the first sample
  sample1 = *buffer++;

  if(sample1 > 94)
  {
    sample1 = 94;
  }

  //Position it within the thumbnail on screen
  sample1 += ypos;

  //Do while the samples last
  for(x=xstart;x<xend;x++)
  {
    //Get the second sample
    sample2 = *buffer++;

    if(sample2 > 94)
    {
      sample2 = 94;
    }

    //Position it within the thumbnail on screen
    sample2 += ypos;

    //Draw the line for these samples
    display_draw_line(x, sample1, x + 1, sample2);

    //Swap the samples
    sample1 = sample2;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_create_thumbnail(PTHUMBNAILDATA thumbnaildata)
{
  uint16 position;

  //Set the thumbnails filename
  strcpy(thumbnaildata->filename, viewfilename);

  //Calculate and limit pointer position for channel 1
  position = TRACE_VERTICAL_END - scopesettings.channel1.traceposition;

  //Limit on the top of the displayable region
  if(position < VERTICAL_POINTER_TOP)
  {
    position = VERTICAL_POINTER_TOP;
  }
  //Limit on the bottom of the displayable region
  else if(position > VERTICAL_POINTER_BOTTOM)
  {
    position = VERTICAL_POINTER_BOTTOM;
  }

  //Set the parameters for channel 1
  thumbnaildata->channel1enable        = scopesettings.channel1.enable;
  thumbnaildata->channel1traceposition = (uint8)(((position - VERTICAL_POINTER_TOP) * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER);

  //Calculate and limit pointer position for channel 2
  position = TRACE_VERTICAL_END - scopesettings.channel2.traceposition;

  //Limit on the top of the displayable region
  if(position < VERTICAL_POINTER_TOP)
  {
    position = VERTICAL_POINTER_TOP;
  }
  //Limit on the bottom of the displayable region
  else if(position > VERTICAL_POINTER_BOTTOM)
  {
    position = VERTICAL_POINTER_BOTTOM;
  }

  //Set the parameters for channel 2
  thumbnaildata->channel2enable        = scopesettings.channel2.enable;
  thumbnaildata->channel2traceposition = (uint8)(((position - VERTICAL_POINTER_TOP) * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER);

  //Calculate and limit pointer position for trigger level
  position = TRACE_VERTICAL_END - scopesettings.triggerverticalposition;

  //Limit on the top of the displayable region
  if(position < VERTICAL_POINTER_TOP)
  {
    position = VERTICAL_POINTER_TOP;
  }
  //Limit on the bottom of the displayable region
  else if(position > VERTICAL_POINTER_BOTTOM)
  {
    position = VERTICAL_POINTER_BOTTOM;
  }

  //Set trigger information
  thumbnaildata->triggerverticalposition   = (uint8)(((position - VERTICAL_POINTER_TOP) * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER);
  thumbnaildata->triggerhorizontalposition = (scopesettings.triggerhorizontalposition * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_X_DIVIDER;

  //Set the trace display mode
  thumbnaildata->tracedisplaymode = scopesettings.tracedisplaymode;

  //Set the display start and end x positions. Conversion to thumbnail x coordinates is done by dividing since the region is smaller
  thumbnaildata->disp_xstart = (disp_xstart * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_X_DIVIDER;
  thumbnaildata->disp_xend   = (disp_xend * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_X_DIVIDER;

  //Check which display mode is active
  if(scopesettings.tracedisplaymode == DISPLAY_MODE_NORMAL)
  {
    //Normal mode so check on channel 1 being enabled
    if(scopesettings.channel1.enable)
    {
      //Process the trace points
      ui_thumbnail_set_trace_data(&scopesettings.channel1, thumbnaildata->channel1data);
    }

    //Check on channel 2 being enabled
    if(scopesettings.channel2.enable)
    {
      //Process the trace points
      ui_thumbnail_set_trace_data(&scopesettings.channel2, thumbnaildata->channel2data);
    }
  }
  else
  {
    //Use less samples to not overwrite the second buffer
    uint32 index = disp_trigger_index - 350;
    uint32 last = index + 700;

    uint8 *buffer1 = thumbnaildata->channel1data;
    uint8 *buffer2 = thumbnaildata->channel2data;

    //Copy and scale every 4th sample for the channels
    for(;index<last;index+=4)
    {
      //Adjust the samples to fit the thumbnail screen. Channel 1 is x, channel 2 is y
      *buffer1++ = (scope_get_x_sample(&scopesettings.channel1, index) * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER;
      *buffer2++ = ((scope_get_y_sample(&scopesettings.channel2, index) - VERTICAL_POINTER_TOP) * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_thumbnail_set_trace_data(PCHANNELSETTINGS settings, uint8 *buffer)
{
  int32  index;
  int32  sample;

  //Point to the first and second trace point for easy access
  PDISPLAYPOINTS ptr1 = &settings->tracepoints[0];
  PDISPLAYPOINTS ptr2 = &settings->tracepoints[1];

  //Process the points
  for(index=1;index<settings->noftracepoints;index++)
  {
    //Fill in the blanks between the given points
    ui_thumbnail_calculate_trace_data(ptr1->x, ptr1->y, ptr2->x, ptr2->y);

    //Select the next points
    ptr1++;
    ptr2++;
  }

  //Down sample the points in to the given buffer
  //This yields a max of 176 points, which is more then is displayed on the thumbnail screen
  for(index=disp_xstart;index<=disp_xend;index+=4)
  {
    //Take of the trace screen top offset to get the actual sample value. Can be negative when the trace is outside the displayable region
    sample = (int32)thumbnailtracedata[index] - TRACE_WINDOW_BORDER_YPOS;

    //Limit the sample to the extremes
    if(sample < 0)
    {
      sample = 0;
    }
    else if(sample > TRACE_WINDOW_BORDER_HEIGHT)
    {
      //Limit to the thumbnail trace display window height
      sample = THUMBNAIL_TRACE_HEIGHT;
    }
    else
    {
      //Scale the sample for the Y direction to fit the thumbnail trace screen window
      sample = (sample * THUMBNAIL_SAMPLE_MULTIPLIER) / THUMBNAIL_Y_DIVIDER;
    }

    //Fill the buffer with the samples
    *buffer++ = (uint8)sample;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Function to fill in samples based on linear interpolation

void ui_thumbnail_calculate_trace_data(int32 xstart, int32 ystart, int32 xend, int32 yend)
{
  register int32  x, dx;
  register int32  yacc;
  register int32  ystep;

  //Calculate delta x.
  dx = xend - xstart;

  //Calculate the y segment length
  ystep = ((yend - ystart) << 16) / dx;

  //Initialize the y accumulator for broken pixel accounting
  yacc = ystart << 16;

  //Set the start and end points
  thumbnailtracedata[xstart] = ystart;
  thumbnailtracedata[xend]   = yend;

  //Check if there are points in between
  if(dx > 1)
  {
    //Handle the in between x positions
    for(x=xstart+1;x<xend;x++)
    {
      //Calculate the y point of this segment
      yacc += ystep;

      //Set it in the buffer
      thumbnailtracedata[x] = yacc >> 16;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_thumbnail_draw_pointer(uint32 xpos, uint32 ypos, uint32 direction, uint32 color)
{
  uint32 x1, y1, x2, y2, w, h;

  //Set the pointer color
  display_set_fg_color(color);

  //Setup the coordinates based on the direction
  switch(direction)
  {
    //Pointing to the right
    default:
    case THUMBNAIL_POINTER_RIGHT:
      x1 = xpos;
      y1 = ypos;
      x2 = x1 + 4;
      y2 = y1 + 1;
      w = 4;
      h = 3;
      break;

    //Pointing to the left
    case THUMBNAIL_POINTER_LEFT:
      x1 = xpos - 4;
      y1 = ypos;
      x2 = x1 - 1;
      y2 = y1 + 1;
      w = 4;
      h = 3;
      break;

    //Pointing down
    case THUMBNAIL_POINTER_DOWN:
      x1 = xpos;
      y1 = ypos;
      x2 = x1 + 1;
      y2 = y1 + 4;
      w = 3;
      h = 4;
      break;
  }

  //Draw the body
  display_fill_rect(x1, y1, w - 1, h - 1);

  //Draw the point
  display_fill_rect(x2, y2, 0, 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

char *ui_print_decimal_number(char *buffer, uint32 number)
{
  char    b[12];
  uint32  i = 12;   //Start beyond the array since the index is pre decremented
  uint32  s;

  //For file number 0 no need to do the work
  if(number == 0)
  {
    //Value is zero so just set a 0 character
    b[--i] = '0';
  }
  else
  {
    //Process the digits
    while(number)
    {
      //Set current digit to decreased index
      b[--i] = (number % 10) + '0';

      //Take of the current digit
      number /= 10;
    }
  }

  //Determine the size of the decimal part
  s = 12 - i;

  //Copy the decimal file number to the buffer
  memcpy(buffer, &b[i], s);

  //Terminate the string
  buffer[s] = 0;

  //Return the pointer to the end of the string
  return(buffer + s);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_display_picture_item(void)
{
  //Display the new item
  if(ui_load_bitmap_data() == VIEW_BITMAP_LOAD_ERROR)
  {
    //Return on an error
    return(VIEW_BITMAP_LOAD_ERROR);
  }

  //And draw the bottom menu bar with a save of the background
//  ui_setup_bottom_file_menu(VIEW_BOTTON_MENU_INIT);

  return(VIEW_BITMAP_LOAD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_file_status_message(int32 msgid, int32 alwayswait)
{
  uint32 checkconfirmation = scopesettings.confirmationmode;

  //Check if need to wait is requested
  if(alwayswait)
  {
    //If so override the setting
    checkconfirmation = 1;
  }

  //Need to save the screen buffer pointer and set it to the actual screen
  //When displaying trace data to avoid flickering data is drawn in a different screen buffer
  display_save_screen_buffer();

  //Save the screen rectangle where the message will be displayed
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(260, 210, 280, 60);

  //Draw the background in grey
  display_set_fg_color(COLOR_DARK_GREY_2);
  display_fill_rect(260, 210, 279, 59);

  //Draw the border in a lighter grey
  display_set_fg_color(COLOR_DARK_GREY_3);
  display_draw_rect(260, 210, 280, 60);

  //White color for text and use font_1
  display_set_fg_color(COLOR_WHITE);
  display_set_font(&font_1);

  switch(msgid)
  {
    case MESSAGE_SAVE_SUCCESSFUL:
      display_text(270, 220, "File saved successfully");

      //Don't wait for confirmation in case of success, unless requested
      checkconfirmation = alwayswait;
      break;

    case MESSAGE_FILE_CREATE_FAILED:
      display_text(270, 220, "Failed to create file");
      break;

    case MESSAGE_FILE_OPEN_FAILED:
      display_text(270, 220, "Failed to open file");
      break;

    case MESSAGE_FILE_WRITE_FAILED:
      display_text(270, 220, "Failed to write to file");
      break;

    case MESSAGE_FILE_READ_FAILED:
      display_text(270, 220, "Failed to read from file");
      break;

    case MESSAGE_FILE_SEEK_FAILED:
      display_text(270, 220, "Failed to seek in file");
      break;

    case MESSAGE_FILE_DELETE_FAILED:
      display_text(270, 220, "Failed to delete file");
      break;

    case MESSAGE_DIRECTORY_CREATE_FAILED:
      display_text(270, 220, "Failed to create directory");
      break;

    case MESSAGE_FILE_SYSTEM_FAILED:
      display_text(270, 220, "File system failure");
      break;

    case MESSAGE_THUMBNAIL_FILE_CORRUPT:
      display_text(270, 220, "Corrupt thumbnail file");
      break;

    case MESSAGE_THUMBNAIL_FILE_FULL:
      display_text(270, 220, "Thumbnail file is full");
      break;

    case MESSAGE_BMP_HEADER_MISMATCH:
      display_text(270, 220, "Bitmap header mismatch");
      break;

    case MESSAGE_WAV_VERSION_MISMATCH:
      display_text(270, 220, "Waveform file version mismatch");
      break;

    case MESSAGE_WAV_CHECKSUM_ERROR:
      display_text(270, 220, "Waveform file checksum error");
      break;
  }

  //Display the file name in question
  display_text(270, 245, viewfilename);

  //Maybe wait for touch to continue in case of an error message
  if(checkconfirmation)
  {
    //Wait for the user to push a button or rotate a dial on the front panel of the scope
    uart1_wait_for_user_input();
  }
  else
  {
    //Wait for half a second
    timer0_delay(500);
  }

  //Restore the original screen
  display_set_source_buffer(displaybuffer2);
  display_copy_rect_to_screen(260, 210, 280, 60);

  //Need to restore the screen buffer pointer
  display_restore_screen_buffer();
}

//----------------------------------------------------------------------------------------------------------------------------------

#define HCD_XPOS   270
#define HCD_YPOS   217
#define HCD_WIDTH  260
#define HCD_HEIGHT  40

//----------------------------------------------------------------------------------------------------------------------------------

int32 ui_handle_confirm_delete(void)
{
  int32 choice = 0;

  //Save the screen rectangle where the message will be displayed
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(HCD_XPOS, HCD_YPOS, HCD_WIDTH, HCD_HEIGHT);

  //display the confirm delete menu
  //Draw the background in some shade of red
  display_set_fg_color(CONFIRM_WINDOW_BG_COLOR);
  display_fill_rect(HCD_XPOS, HCD_YPOS, HCD_WIDTH - 1, HCD_HEIGHT - 1);

  //Draw the border in a lighter grey
  display_set_fg_color(COLOR_DARK_GREY_4);
  display_draw_rect(HCD_XPOS, HCD_YPOS, HCD_WIDTH, HCD_HEIGHT);

  //Black text and a big font
  display_set_fg_color(COLOR_BLACK);
  display_set_font(&font_4);
  display_text(HCD_XPOS + 8, HCD_YPOS + 5, "Confirm to delete?");

  //Wait for the user to push a button or rotate a dial on the front panel of the scope
  uart1_wait_for_user_input();

  //Check if the user pressed the OK button
  if(lastreceivedcommand == UIC_BUTTON_NAV_OK)
  {
    //If so, set the chosen option to yes
    choice = VIEW_CONFIRM_DELETE_YES;
  }
  else
  {
    //Else set the chosen option to no
    choice = VIEW_CONFIRM_DELETE_NO;
  }

  //Restore the original screen
  display_set_source_buffer(displaybuffer2);
  display_copy_rect_to_screen(HCD_XPOS, HCD_YPOS, HCD_WIDTH, HCD_HEIGHT);

  //return the choice
  return(choice);
}


//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA calibration_start_box =
{
  CALIBRATION_START_MSG_WIDTH,
  CALIBRATION_START_MSG_HEIGHT,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

SHADEDRECTDATA calibration_message_box =
{
  CALIBRATION_MSG_WIDTH,
  CALIBRATION_MSG_HEIGHT,
  { COLOR_DARK_GREY_4, COLOR_DARK_GREY_2, COLOR_DARK_GREY_1 },
  COLOR_BLACK
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_show_calibration_message(uint32 state)
{
  //Check on what to do
  switch(state)
  {
    case CALIBRATION_STATE_START:
      //Save the screen rectangle where the message will be displayed
      display_set_destination_buffer(displaybuffer2);
      display_copy_rect_from_screen(CALIBRATION_MSG_XPOS, CALIBRATION_MSG_YPOS, CALIBRATION_START_MSG_WIDTH, CALIBRATION_START_MSG_HEIGHT);

      //Create the box to show the start text in
      display_draw_shaded_rect(CALIBRATION_MSG_XPOS, CALIBRATION_MSG_YPOS, &calibration_start_box, 0);

      //Add the text
      display_set_fg_color(COLOR_WHITE);
      display_copy_icon_fg_color(calibration_start_text_icon, CALIBRATION_MSG_XPOS + 8, CALIBRATION_MSG_YPOS + 10, 159, 44);
      return;

    case CALIBRATION_STATE_BUSY:
    case CALIBRATION_STATE_HIDE:
      //Restore the original screen to hide the start message
      display_set_source_buffer(displaybuffer2);
      display_copy_rect_to_screen(CALIBRATION_MSG_XPOS, CALIBRATION_MSG_YPOS, CALIBRATION_START_MSG_WIDTH, CALIBRATION_START_MSG_HEIGHT);

      //When the given state is hide then nothing more to do
      if(state == CALIBRATION_STATE_HIDE)
      {
        return;
      }
      break;
  }

   //Create the box to show the message text in
  display_draw_shaded_rect(CALIBRATION_MSG_XPOS, CALIBRATION_MSG_YPOS, &calibration_message_box, 0);

  //Set the color for displaying the text
  display_set_fg_color(COLOR_WHITE);

  //Display the intended message
  switch(state)
  {
    case CALIBRATION_STATE_BUSY:
      display_copy_icon_fg_color(calibrating_text_icon, CALIBRATION_MSG_XPOS + 14, CALIBRATION_MSG_YPOS + 7, 70, 18);
      break;

    case CALIBRATION_STATE_SUCCESS:
      display_copy_icon_fg_color(succeed_text_icon, CALIBRATION_MSG_XPOS + 21, CALIBRATION_MSG_YPOS + 9, 55, 14);
      break;

    case CALIBRATION_STATE_FAIL:
      display_copy_icon_fg_color(failed_text_icon, CALIBRATION_MSG_XPOS + 28, CALIBRATION_MSG_YPOS + 9, 41, 14);
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
