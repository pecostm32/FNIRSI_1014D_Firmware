//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "display_lib.h"
#include "variables.h"
#include "uart.h"
#include "usb_interface.h"
#include "user_interface_functions.h"
#include "scope_functions.h"

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
  ui_draw_outline();
  ui_draw_grid();
  ui_display_logo();
  ui_display_run_stop_text();
  ui_display_trigger_settings();
  ui_display_move_speed();
  ui_display_channel_settings(&scopesettings.channel1);
  ui_display_channel_settings(&scopesettings.channel2);
  ui_display_time_per_division();
  ui_display_waiting_triggered_text();
  ui_display_measurements();
  
  //Show version information
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_2);
  display_text(VERSION_STRING_XPOS, VERSION_STRING_YPOS, VERSION_STRING);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_setup_usb_screen(void)
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

  //Start the USB interface
  usb_device_enable();

  //Wait for the user to push a button or rotate a dial on the front panel of the scope
  while(1)
  {
    //Get the latest command to be processed
    if(uart1_get_data() != 0)
    {
      //User gave input so quit the wait loop
      break;
    }
  }

  //Stop the USB interface
  usb_device_disable();

  //Clear the whole screen
  display_set_fg_color(0x00000000);
  display_fill_rect(0, 0, 800, 480);
  
  //Redraw the screen
  ui_setup_main_screen();
  scope_display_trace_data();
  
  //Re-sync the system files
  //scope_sync_thumbnail_files();
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
  int xs1 = 50;
  int xe1 = 110;
  int xs2 = 227;
  int xe2 = 492;
  int xs3 = 222;
  int xe3 = 622;
  int x,y,yt,yb;
  
  const uint32 *color = shade_colors;
  
  //Set the color for drawing the signal display shade
  display_set_fg_color(0x00202020);

  //Draw the outer edge
  display_draw_rect(4, 57, 703, 403);
  
  //Set the color for drawing the signal display outline
  display_set_fg_color(0x00646464);

  //Draw the inner edge
  display_draw_rect(5, 58, 701, 401);
  
  //Draw top side shades with starting color 0x00343434 down going with 0x00040404 down to 0x00000000)
  for(yt=57,yb=459;yt>(57 - (sizeof(shade_colors)/sizeof(uint32)));yt--,yb++)
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
  display_set_fg_color(0x00000000);
  
  //A range of cutouts with an 10 pixel interval
  for(x=252;x<595;x+=10)
  {
    display_draw_rect(x, 464, 2, 9);
  }
  
  //Draw the top and bottom edges of the measurement sections
  display_set_fg_color(0x00303430);
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
    display_set_fg_color(0x00303430);
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
  display_set_fg_color(0x00F8FCF8);
  display_set_bg_color(0x00000000);
  
  //Select the icon based on the selected edge
  if(scopesettings.runstate == 0)
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

void ui_display_waiting_triggered_text(void)
{
  const uint8 *icon;
  uint32       xpos;
  
  //Using an icon might be a simpler and faster option
  display_set_fg_color(0x00F8FCF8);
  display_set_bg_color(0x00000000);
  
  //Select the icon based on the trigger state
  if(scopesettings.triggerstate == 0)
  {
    //Waiting for trigger
    icon = waiting_text_icon;
    xpos = 654;
  }
  else
  {
    //Triggered
    icon = triggered_text_icon;
    xpos = 652;
  }
  
  //Triggered text needs to be placed a bit to the right, which might be fixed by tweaking the icon
  //Have to see how many pixels it needs to be offset.
  
  //Display the text icon with infill of the background since the other text icon needs to be overwritten
  display_copy_icon_use_colors(icon, xpos, 464, 54, 14);
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
    display_draw_horz_line(258,  6, 704);
    display_draw_vert_line(355, 59, 457);

    //Draw the ticks on the x line
    for(i=10;i<705;i+=5)
    {
      display_draw_vert_line(i, 256, 260);
    }

    //Draw the ticks on the y line
    for(i=63;i<457;i+=5)
    {
      display_draw_horz_line(i, 353, 357);
    }

    //Draw the horizontal dots
    for(i=108;i<457;i+=50)
    {
      display_draw_horz_dots(i, 10, 704, 5);
    }

    //Draw the vertical dots
    for(i=55;i<705;i+=50)
    {
      display_draw_vert_dots(i, 63, 457, 5);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//1014d   display_copy_rect_to_screen(5, 58, 701, 401);  5 - 706  58 - 459
//1013d   display_copy_rect_to_screen(2, 46, 728, 434);  2 - 730  46 - 480


void ui_draw_pointers(void)
{
  uint32 position;

  //Text is displayed in black and with font_0
  display_set_bg_color(0x00000000);
  display_set_font(&font_0);
  
  //Draw channel 1 pointer when it is enabled
  if(scopesettings.channel1.enable)
  {
    //Set the color for drawing the pointer
    display_set_fg_color(CHANNEL1_COLOR);

    //Check if in normal or x-y display mode
    if(scopesettings.xymodedisplay == 0)
    {
      //y position for the channel 1 trace center pointer.
      position = 458 - scopesettings.channel1.traceposition;

      //Limit on the top of the displayable region
      if(position < 59)
      {
        position = 59;
      }
      //Limit on the bottom of the displayable region (pointer is 15 pixels high)
      else if(position > 443)
      {
        position = 443;
      }

      //Draw the pointer
      display_left_pointer(6, position, '1');
    }
    else
    {
      //y position for the channel 1 trace center pointer.
      position = 157 + scopesettings.channel1.traceposition;

      //Limit on the left of the active range
      if(position < 166)
      {
        position = 166;
      }
      //Limit on the right of the active range
      else if(position > 548)
      {
        position = 548;
      }

      //Draw the pointer
      display_top_pointer(position, 59, '1');
    }
  }

  //Draw channel 2 pointer when it is enabled
  if(scopesettings.channel2.enable)
  {
    //Set the color for drawing the pointer
    display_set_fg_color(CHANNEL2_COLOR);

    //y position for the channel 2 trace center pointer
    position = 458 - scopesettings.channel2.traceposition;

    //Limit on the top of the displayable region
    if(position < 59)
    {
      position = 59;
    }
    //Limit on the bottom of the displayable region
    else if(position > 443)
    {
      position = 443;
    }

    //Draw the pointer
    display_left_pointer(6, position, '2');
  }

  //Need to think about trigger position in 200mS - 20mS/div settings. Not sure if they work or need to be done in software
  //The original scope does not show them for 50mS and 20mS/div

  //Draw trigger position and level pointer when in normal display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //x position for the trigger position pointer
    position = scopesettings.triggerhorizontalposition + 6;

    //Limit on the right of the displayable region
    if(position > 690)
    {
      position = 690;
    }

    //Set the color for drawing the pointer
    display_set_fg_color(TRIGGER_COLOR);

    //Draw the pointer
    display_top_pointer(position, 59, 'H');

    //y position for the trigger level pointer
    position = 458 - scopesettings.triggerverticalposition;

    //Limit on the top of the displayable region
    if(position < 59)
    {
      position = 59;
    }
    //Limit on the bottom of the displayable region
    else if(position > 443)
    {
      position = 443;
    }

    //Need to reset the fore ground color because the display_top_pointer copies the background color over it
    display_set_fg_color(TRIGGER_COLOR);
    
    //The trigger level id uses a different font
    display_set_font(&font_3);

    //Draw the pointer
    display_right_pointer(684, position, 'T');
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_time_cursors(void)
{
  //Only draw the lines when enabled
  if(scopesettings.timecursorsenable)
  {
    //Set the color for the dashed lines
    display_set_fg_color(CURSORS_COLOR);

    //Draw the lines
    display_draw_vert_dashes(scopesettings.timecursor1position, 59, 459, 3, 2);
    display_draw_vert_dashes(scopesettings.timecursor2position, 59, 459, 3, 2);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_draw_volt_cursors(void)
{
  //Only draw the lines when enabled
  if(scopesettings.voltcursorsenable)
  {
    //Set the color for the dashed lines
    display_set_fg_color(CURSORS_COLOR);

    //Draw the lines
    display_draw_horz_dashes(scopesettings.voltcursor1position, 5, 706, 3, 2);
    display_draw_horz_dashes(scopesettings.voltcursor2position, 5, 706, 3, 2);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_selected_text(void)
{
  int32 x,y;
  
  //Only display the text when a cursor is enabled
  if((scopesettings.timecursorsenable) || (scopesettings.voltcursorsenable) || (scopesettings.capturecursorsenable))
  {
    //Determine the positions for displaying the text based on the selected cursor and it's position
    //X and Y are fixed based on horizontal or vertical cursor lines
    switch(userinterfacedata.selectedcursor)
    {
      case CURSOR_TIME_LEFT:
      case CURSOR_TIME_RIGHT:
      case CURSOR_CAPTURE_LEFT:
      case CURSOR_CAPTURE_RIGHT:
        y = 441;
        break;

      case CURSOR_VOLT_TOP:
      case CURSOR_VOLT_BOTTOM:
        x = 655;
        break;
    }

    //Determine the positions for displaying the text based on the selected cursor and it's position
    switch(userinterfacedata.selectedcursor)
    {
      case CURSOR_TIME_LEFT:
        //Position the text to the left of the cursor
        x = scopesettings.timecursor1position - 51;
        
        //Check if beyond the left border
        if(x < 6)
        {
          //If so move the text to the right side of the cursor
          x = scopesettings.timecursor1position + 3;
        }
        break;

      case CURSOR_TIME_RIGHT:
        //Position the text to the right of the cursor
        x = scopesettings.timecursor2position + 3;
        
        //Check if beyond the right border
        if(x > 656)
        {
          //If so move the text to the left side of the cursor
          x = scopesettings.timecursor2position - 51;
        }
        break;

      case CURSOR_VOLT_TOP:
        //Position the text above the cursor
        y = scopesettings.voltcursor1position - 15;
        
        //Check if above the top of the screen
        if(y < 58)
        {
          //If so position it below the cursor
          y = scopesettings.voltcursor1position + 3;
        }
        break;

      case CURSOR_VOLT_BOTTOM:
        //Position the text below the cursor
        y = scopesettings.voltcursor2position + 3;
        
        //Check if below the bottom of the screen
        if(y > 445)
        {
          //If so position it above the cursor
          y = scopesettings.voltcursor2position - 15;
        }
        break;

      case CURSOR_CAPTURE_LEFT:
        //Position the text to the left of the cursor
        x = scopesettings.capturecursor1position - 51;
        
        //Check if beyond the left border
        if(x < 6)
        {
          //If so move the text to the right side of the cursor
          x = scopesettings.capturecursor1position + 3;
        }
        break;

      case CURSOR_CAPTURE_RIGHT:
        //Position the text to the right of the cursor
        x = scopesettings.capturecursor2position + 3;
        
        //Check if beyond the right border
        if(x > 656)
        {
          //If so move the text to the left side of the cursor
          x = scopesettings.capturecursor2position - 51;
        }
        break;
    }
    
    //The selected text is displayed in white and with basic font
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_1);
    display_text(x, y, "Selected");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_top_box =
{
  20,
  14,
  { 0x0000FC00, 0x0000A800, 0x00005400 },
  0x00002000
};

TEXTDATA trigger_top_box_text =
{
  7,
  0,
  0x00F8FCF8,
  &font_1,
  "T"
};

SHADEDRECTDATA trigger_bottom_box =
{
  21,
  14,
  { 0x0000FC00, 0x0000A800, 0x00005400 },
  0x00002000
};

TEXTDATA trigger_bottom_box_text =
{
  7,
  0,
  0x00F8FCF8,
  &font_1,
  "H"
};

SHADEDRECTDATA trigger_x_pos_box =
{
  94,
  14,
  { 0x00404440, 0x00282828, 0x00101410 },
  0x00000000
};

TEXTDATA trigger_x_pos_box_text =
{
  5,
  0,
  0x00F8FCF8,
  &font_1,
  "POS :"
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_settings(void)
{
  //Mark it as the trigger settings area
  display_draw_shaded_rect(127, 6, &trigger_top_box, &trigger_top_box_text);
  
  //The fixed texts are drawn in a grey shade and basic font
  display_set_fg_color(0x00888888);
  display_set_font(&font_1);
  
  //Modified character 0x3A (:) for having it one pixel higher then normal to
  //match the original firmware where it is printed separately to do the same
  //Selected trigger channel text
  display_text(127, 23, "CHS :");

  //Trigger y (level) position text
  display_text(127, 37, "POS :");

  //Time per division text in the bottom information section
  display_text(125, 465, "DIV :");
  
  //Setup the bottom information section
  display_draw_shaded_rect(3, 465, &trigger_bottom_box, &trigger_bottom_box_text);
  display_draw_shaded_rect(26, 465, &trigger_x_pos_box, &trigger_x_pos_box_text);

  //Fill in the variable sections
  ui_display_trigger_mode();
  ui_display_trigger_channel();
  ui_display_trigger_edge();
  ui_display_trigger_y_position();
  ui_display_trigger_x_position();
  ui_display_time_per_division();
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_mode_box =
{
  57,
  14,
  { 0x00404440, 0x00282828, 0x00101410 },
  0x00000000
};

TEXTDATA trigger_mode_texts[] =
{
  { 15, 0, 0x00F8FCF8, &font_1, "Auto" },
  { 12, 0, 0x00F8FCF8, &font_1, "Single" },
  {  9, 0, 0x00F8FCF8, &font_1, "Normal" },
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_mode(void)
{
  //Check which trigger mode is set and if it is a valid setting
  if(scopesettings.triggermode < 3)
  {
    //Display the shaded rectangle with the selected text
    display_draw_shaded_rect(149, 6, &trigger_mode_box, &trigger_mode_texts[scopesettings.triggermode]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA trigger_channel_boxes[] =
{
  { 27, 12, { 0x00F8FC00, 0x00D8DC00, 0x00B8BC00 }, 0x00A8A800 },
  { 27, 12, { 0x0000FCF8, 0x0000DCD8, 0x0000BCB8 }, 0x0000A8A8 },
};

TEXTDATA trigger_channel_texts[] =
{
  { 3, -1, 0x00000000, &font_1, "CH1" },
  { 3, -1, 0x00000000, &font_1, "CH2" },
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_channel(void)
{
  //Check which trigger channel is selected and if it is a valid setting
  if(scopesettings.triggerchannel < 2)
  {
    //Display the shaded rectangle with the selected text
    display_draw_shaded_rect(162, 24, &trigger_channel_boxes[scopesettings.triggerchannel], &trigger_channel_texts[scopesettings.triggerchannel]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_edge(void)
{
  const uint8 *icon;
  
  //Using an icon might be a simpler and faster option
  display_set_fg_color(0x00F8FCF8);
  display_set_bg_color(0x00000000);
  
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
  display_copy_icon_use_colors(icon, 198, 24, 7, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_x_position(void)
{
  //Have to clear the section first
  //Needs interpretation of the position against a set time level????
  
  //Reading is based on center line of the grid being 0s and the channels time per division setting ??
  //is taken into account to calculate the current position of the pointer.
  
  //When not on screen show *********
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_trigger_y_position(void)
{
  //Have to clear the section first
  //Needs interpretation of the position against a set voltage level????
  
  //Reading is based on center line of the grid being 0V and the channels voltage per division setting
  //is taken into account to calculate the current position of the pointer. Not very useful and should
  //be changed to showing the actual trigger level in relation to the signal
  
  //When not on screen show *********
  
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_time_per_division(void)
{
  //Clear the old text before printing the new text
  display_set_fg_color(0x00000000);
  display_fill_rect(153, 465, 65, 14);
  
  //Text is RGB 565 based white
  display_set_fg_color(0x00F8FCF8);
  display_set_font(&font_1);
  
  //Only display the text when in the setting is range of the text array
  if(scopesettings.timeperdiv < (sizeof(time_div_texts) / sizeof(int8 *)))
  {
    //Display the in the bottom information section using the table
    display_text(155, 465, (char *)time_div_texts[scopesettings.timeperdiv]);
  }
  
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_move_speed(void)
{
  const uint8 *icon;
  
  //Text is slightly off white
  display_set_fg_color(0x00F8FCF8);
  display_set_bg_color(0x00000000);
  
  //Draw the icon for this section
  display_copy_icon_fg_color(move_speed_icon, 330, 13, 16, 16);
  
  //Set the needed text based on the selected speed
  if(scopesettings.movespeed == 0)
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

SHADEDRECTDATA channel_disabled_box =
{
  20,
  14,
  { 0x00404440, 0x00282828, 0x00101410 },
  0x00000000
};

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_1_box =
{
  20,
  14,
  { 0x00F8FC00, 0x00888800, 0x00404400 },
  0x00202000
};

TEXTDATA channel_1_box_text =
{
  7,
  0,
  0x00F8FCF8,
  &font_1,
  "1"
};

//----------------------------------------------------------------------------------------------------------------------------------

SHADEDRECTDATA channel_2_box =
{
  20,
  14,
  { 0x0000FCF8, 0x00008888, 0x00004440 },
  0x00002020
};

TEXTDATA channel_2_box_text =
{
  7,
  0,
  0x00F8FCF8,
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
  display_set_fg_color(0x00888888);
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
  { 0x00404440, 0x00282828, 0x00101410 },
  0x00000000
};

TEXTDATA channel_probe_texts[] =
{
  { 12, 0, 0x00F8FCF8, &font_1,   "1 : 1" },
  {  8, 0, 0x00F8FCF8, &font_1,  "10 : 1" },
  {  5, 0, 0x00F8FCF8, &font_1, "100 : 1" },
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
  int x = settings->infoxpos + 70;
  int y = settings->infoypos;
  
  //Clear the old text first
  display_set_fg_color(0x00000000);
  display_fill_rect(x - 1, y + 2, 17, 9);

  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used
    display_set_fg_color(settings->color);
  }
  else
  {
    //When disabled the disabled color is used
    display_set_fg_color(0x00888888);
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
  int x = settings->infoxpos + 43;
  int y = settings->infoypos + 17;
  
  //Clear the old text first
  display_set_fg_color(0x00000000);
  display_fill_rect(x - 1, y + 2, 44, 9);
  
  //Colors are different when disabled or enabled
  if(settings->enable)
  {
    //For the enabled channel the channel color is used
    display_set_fg_color(settings->color);
  }
  else
  {
    //When disabled the disabled color is used
    display_set_fg_color(0x00888888);
  }
  
  //The text is drawn a basic font
  display_set_font(&font_1);
  
  //Display the selected sensitivity
  display_text(x, y, volt_div_texts[settings->magnification][settings->displayvoltperdiv]);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_position(PCHANNELSETTINGS settings)
{
  //Have to clear the section first
  //Needs interpretation of the position against a set voltage level????
  
  //Reading is based on center line of the grid being 0V and the channels voltage per division setting
  //is taken into account to calculate the current position of the pointer.
  
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
  int i,y;
  
  //Process the data for the available measurement slots
  for(i=0;i<(sizeof(scopesettings.measurementitems)/sizeof(MEASUREMENTINFO));i++)
  {
    //Get the channel information for displaying the box in the channel color
    settings = scopesettings.measurementitems[i].channelsettings;
    
    //Setup the base position
    y = MEASUREMENT_INFO_Y + (i * MEASUREMENT_Y_DISPLACEMENT);
    
    //Mark it as the channel settings area
    display_draw_shaded_rect(MEASUREMENT_CHANNEL_BOX_X, y, settings->boxdata, settings->boxtext);

    //The fixed texts are drawn in a grey shade and basic font
    display_set_fg_color(0x00888888);
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
  
  //Process the data for the available measurement slots
  for(i=0;i<(sizeof(scopesettings.measurementitems)/sizeof(MEASUREMENTINFO));i++)
  {
    //Get the channel information for displaying the value
    settings = scopesettings.measurementitems[i].channelsettings;
    
    //Setup the base position
    y = MEASUREMENT_INFO_Y + (i * MEASUREMENT_Y_DISPLACEMENT) + 21;
    
    //Call the set function for displaying the actual value and
    //pass the information for this measurement to the function for displaying it
    measurement_functions[scopesettings.measurementitems[i].index](y, settings);
  }
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
  display_set_fg_color(0x00FFFFFF);
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
  char   *buffer = measurementtext;
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
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_1);
  
  //Display it on the screen
  display_text(x + 2, ypos + 5, measurementtext);
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

    //Check if decimal point needs to be placed
    if(i == 10 - decimals)
    {
      //Move back to where the next dot needs to be placed
      x -= 6;

      //If so display it at the right height
      display_copy_icon_full_color(measurement_dot_icon, x, ypos + 12, 4, 4);

      //Create space before the dot
      x -= 2;
    }

    //Move back to the next digit location
    x -= 13;

    //Take of the current digit
    value /= 10;
  }
  
  //Return the most right x position used
  return(r);
}

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA main_menu_highlight_box =
{
  175,
  26,
  { 0x0000FF00, 0x00009900, 0x00005500, 0x00002200 },
  0x00000000
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_main_menu(void)
{
  int i,y;
  
  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(0x00303430);
  display_draw_rect(5, 114, 183, 345);
  
  //Fill the lighter background of the menu area
  display_set_fg_color(0x00101010);
  display_fill_rect(6, 115, 180, 342);

  //Calculate the y position for the highlight box based on the selected menu item
  y = 118 + (userinterfacedata.menuitem * 31);
  
  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(9, y, &main_menu_highlight_box);
  
  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);
  
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
  y = 118 + (userinterfacedata.menuitem * 31);
  
  //Fill the section with the lighter background of the menu area
  display_set_fg_color(0x00101010);
  display_fill_rect(9, y, 174, 25);
  
  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);
  
  //Text line sits 4 pixels lower
  y += 4;
  
  //Draw the icon and belonging text for the selected item
  display_copy_icon_fg_color(main_menu_icons[userinterfacedata.menuitem], 14, y, 166, 18);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_highlight_main_menu_item(void)
{
  int y;
  
  //Clear the previous selection first
  
  //Calculate the y position for the highlight box based on the selected menu item
  y = 118 + (userinterfacedata.menuitem * 31);
  
  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(9, y, &main_menu_highlight_box);
  
  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);
  
  //Text line sits 4 pixels lower
  y += 4;
  
  //Draw the icon and belonging text for the selected item
  display_copy_icon_fg_color(main_menu_icons[userinterfacedata.menuitem], 14, y, 166, 18);
}

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA channel_1_highlight_box =
{
  183,
  24,
  { 0x00FFFF00, 0x00999900, 0x00555500, 0x00222200 },
  0x00000000
};

//----------------------------------------------------------------------------------------------------------------------------------

HIGHLIGHTRECTDATA channel_2_highlight_box =
{
  183,
  24,
  { 0x0000FFFF, 0x00009999, 0x00005555, 0x00002222 },
  0x00000000
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu(PCHANNELSETTINGS settings)
{
  int i,y;
  
  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(0x00303430);
  display_draw_rect(260, 213, 191, 91);
  
  //Fill the lighter background of the menu area
  display_set_fg_color(0x00101010);
  display_fill_rect(261, 214, 188, 88);

  //Need to calculate the y position for the highlight box
  y = 275; //217 + (selected * 29);
  
  //Draw the menu high lighter box for the selected item
  display_draw_highlight_rect(264, y, settings->highlightboxdata);

  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);
  
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
  
  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);

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
  display_set_fg_color(0x00000000);
  display_copy_icon_fg_color(channel_menu_magnification_icons[settings->magnification], channel_menu_magnification_x_positions[settings->magnification], 224, channel_menu_magnification_widths[settings->magnification], 10);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu_coupling_select(PCHANNELSETTINGS settings)
{
  if(settings->coupling == 0)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(367, 250, 29, 15);
    
    //Display the selected text in black
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Display the AC text
  display_copy_icon_fg_color(channel_menu_AC_icon, 373, 252, 19, 12);

  if(settings->coupling == 1)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(407, 250, 29, 15);
    
    //Display the selected text in black
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Display the DC text
  display_copy_icon_fg_color(channel_menu_DC_icon, 413, 252, 19, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_channel_menu_fft_on_off_select(PCHANNELSETTINGS settings)
{
  if(settings->fftenable == 1)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(367, 279, 29, 15);
    
    //Display the selected text in black
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Display the AC text
  display_copy_icon_fg_color(channel_menu_ON_icon, 372, 281, 21, 12);

  if(settings->fftenable == 0)
  {
    //Selected item highlight box is set in channel color
    display_set_fg_color(settings->color);

    //Highlight the selected item
    display_fill_rect(407, 279, 29, 15);
    
    //Display the selected text in black
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Not selected text is displayed in white
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Display the DC text
  display_copy_icon_fg_color(channel_menu_OFF_icon, 409, 281, 26, 12);
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_cursor_measurements(void)
{
  uint32 height = 5;
  uint32 ch1ypos = 64;
  uint32 ch2ypos = 64;
  uint32 delta;
  char   displaytext[10];

  //Check if need to do anything here
  if(scopesettings.timecursorsenable || (scopesettings.voltcursorsenable && (scopesettings.channel1.enable || scopesettings.channel2.enable)))
  {
    //Check if time cursor is enabled
    if(scopesettings.timecursorsenable)
    {
      //Add height for two text lines
      height += 32;

      //Shift the voltage text positions down
      ch1ypos += 32;
      ch2ypos += 32;
    }

    //Check if volt cursor is enabled
    if(scopesettings.voltcursorsenable)
    {
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

    //Set gray background for the cursor measurements
    display_set_fg_color(0x00404040);

    //Draw rectangle for the texts depending on what is enabled.
    display_fill_rect(6, 59, 101, height - 1);
    
    //Make it a on one corner rounded thing
    display_draw_horz_line(height + 59, 6, 106);
    display_draw_horz_line(height + 60, 6, 106);
    display_draw_horz_line(height + 61, 6, 105);
    display_draw_horz_line(height + 62, 6, 103);

    //Use white text and font_0
    display_set_fg_color(0x00FFFFFF);
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
      ui_cursor_print_value(displaytext, delta, tcd->time_scale, "T = ", "s");
      display_text(13, 64, displaytext);

      //Calculate the frequency for this time. Need to adjust for it to stay within 32 bits
      delta /= 10;
      delta = 1000000000 / delta;

      //Format the frequency for displaying
      ui_cursor_print_value(displaytext, delta, tcd->freq_scale, "F = ", "Hz");
      display_text(13, 80, displaytext);
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
        ui_cursor_print_value(displaytext, volts, vcd->volt_scale, "V1 = ", "V");
        display_text(13, ch1ypos, displaytext);
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
        ui_cursor_print_value(displaytext, volts, vcd->volt_scale, "V2 = ", "V");
        display_text(13, ch2ypos, displaytext);
      }
    }
  }
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

HIGHLIGHTRECTDATA measurement_menu_highlight_box =
{
  141,
  24,
  { 0x0000FF00, 0x00009900, 0x00005500, 0x00002200 },
  0x00000000
};

//----------------------------------------------------------------------------------------------------------------------------------

void ui_display_measurements_menu(void)
{
  int y;
  
  //Fill the lighter background of the menu area
  display_set_fg_color(0x00101010);
  display_fill_rect(380, 114, 313, 332);

  //Draw the menu outline slightly lighter then the background
  display_set_fg_color(0x00303430);
  display_draw_rect(379, 113, 316, 335);

  //Draw the lines to separate the header and the two channels
  display_draw_horz_line(140, 380, 694);
  display_draw_vert_line(537, 114, 139);                 //x +158
  
  
  
  //Need to calculate the y position for the highlight box
  //Need two x values for the selection box
  //One for the channel 1 list and one for the channel 2 list
  y = 142; // + (selected * 31);
  
  //Draw the menu high lighter box for the selected item (Original code used three rectangles)
  display_draw_highlight_rect(383, y, &measurement_menu_highlight_box);
  
  //Text is displayed in white
  display_set_fg_color(0x00FFFFFF);
  
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
  display_set_fg_color(0x00FFFFFF);
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
    display_right_aligned_text(x, y, measurementtext);
    
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
    ui_msm_print_value(measurementtext, settings->frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "Hz");
  }
  else
  {
    strcpy(measurementtext, "xxxHz");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_cycle(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(measurementtext, (((uint64)settings->periodtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_time_plus(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(measurementtext, (((uint64)settings->hightime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void ui_msm_display_time_min(uint32 xpos, uint32 ypos, PCHANNELSETTINGS settings)
{
  //Only when the frequency is valid calculate the time
  if(settings->frequencyvalid)
  {
    //Format the time for displaying
    ui_msm_print_value(measurementtext, (((uint64)settings->lowtime * time_calc_data[scopesettings.samplerate].mul_factor) >> 20), time_calc_data[scopesettings.samplerate].time_scale, "s");
  }
  else
  {
    strcpy(measurementtext, "xxxs");
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
    buffer = ui_msm_print_decimal(measurementtext, (((uint64)settings->hightime * 1000) / settings->periodtime), 1, 0);
    
    //Add the duty cycle sign
    strcpy(buffer, "%");
    
  }
  else
  {
    strcpy(measurementtext, "xx%");
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
    buffer = ui_msm_print_decimal(measurementtext, (((uint64)settings->lowtime * 1000) / settings->periodtime), 1, 0);
    
    //Add the duty cycle sign
    strcpy(buffer, "%");
    
  }
  else
  {
    strcpy(measurementtext, "xx%");
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
  ui_msm_print_value(measurementtext, volts, vcd->volt_scale, "V");
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

//----------------------------------------------------------------------------------------------------------------------------------
