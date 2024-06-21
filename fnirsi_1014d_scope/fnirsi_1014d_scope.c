//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

#include "ccu_control.h"
#include "spi_control.h"
#include "timer.h"
#include "interrupt.h"
#include "display_control.h"
#include "uart.h"
#include "fpga_control.h"
#include "clock_synthesizer.h"

#include "fnirsi_1014d_scope.h"
#include "display_lib.h"
#include "scope_functions.h"
#include "user_interface_functions.h"
#include "statemachine.h"

#include "sd_card_interface.h"
#include "ff.h"

#include "usb_interface.h"

#include "arm32.h"

#include "variables.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

#define USE_SD_CARD

//----------------------------------------------------------------------------------------------------------------------------------

extern IRQHANDLERFUNCION interrupthandlers[];

//----------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
  //Initialize data in BSS section
  memset(&BSS_START, 0, &BSS_END - &BSS_START);

  //Initialize the clock system
  sys_clock_init();
  
  //Setup the external clock synthesizer for generating the needed FPGA clocks
  clock_synthesizer_setup();

  //Instead of full memory management just the caches enabled
  arm32_icache_enable();
  arm32_dcache_enable();

  //Clear the interrupt variables
  memset(interrupthandlers, 0, 256);

  //Setup timer interrupt
  timer0_setup();

  //Enable interrupts only once. In the original code it is done on more then one location.
  arm32_interrupt_enable();

  //Initialize SPI for flash (PORT C + SPI0)
  sys_spi_flash_init();
  
  //Initialize UART1 for communication with the user interface microcontroller
  uart1_init();

  //Initialize FPGA (PORT E)
  fpga_init();

  //Turn off the display brightness
  fpga_set_backlight_brightness(0x00);

  //Initialize display (PORT D + DEBE)
  sys_init_display(SCREEN_WIDTH, SCREEN_HEIGHT, (uint16 *)maindisplaybuffer);

  //Setup the display library for the scope hardware
  ui_setup_display_lib();

#ifdef USE_SD_CARD
  //Setup and check SD card on file system being present
  if(f_mount(&fs, "0", 1))
  {
    //Show SD card error message on failure
    //Set max screen brightness
    fpga_set_backlight_brightness(0x78);

    //Clear the display
    display_set_fg_color(COLOR_BLACK);
    display_fill_rect(0, 0, 800, 480);

    //Display the message in red
    display_set_fg_color(COLOR_RED);
    display_set_font(&font_2);
    display_text(30, 50, "SD ERROR");

    //On error just hang
    while(1);
  }
#endif
  
  //Setup the USB interface
  usb_device_init();

  //Load configuration data from SD card
  scope_load_configuration_data();

  //Enable or disable the channels based on the scope loaded settings
  fpga_set_channel_enable(&scopesettings.channel1);
  fpga_set_channel_enable(&scopesettings.channel2);

  //Set the volts per div for each channel based on the loaded scope settings
  fpga_set_channel_voltperdiv(&scopesettings.channel1);
  fpga_set_channel_voltperdiv(&scopesettings.channel2);

  //Set the channels AC or DC coupling based on the loaded scope settings
  fpga_set_channel_coupling(&scopesettings.channel1);
  fpga_set_channel_coupling(&scopesettings.channel2);

  //Enable something in the FPGA
  fpga_enable_system();

  //Setup the trigger system in the FPGA based on the loaded scope settings
  fpga_set_sample_rate(scopesettings.samplerate);
  fpga_set_time_base(scopesettings.timeperdiv);
  fpga_set_trigger_channel();
  fpga_set_trigger_edge();
  fpga_set_trigger_level();
  fpga_set_trigger_mode();

  //Set the trace offsets in the FPGA
  fpga_set_channel_offset(&scopesettings.channel1);
  fpga_set_channel_offset(&scopesettings.channel2);

#if 0
  //Some initialization of the FPGA??. Data written with command 0x3C
  fpga_set_battery_level();      //Only called here and in hardware check
#endif

  //Setup the main parts of the screen
  ui_setup_main_screen();
  
  //Clear the sample memory
  memset(channel1tracebuffer, 128, sizeof(channel1tracebuffer));
  memset(channel2tracebuffer, 128, sizeof(channel2tracebuffer));
  
  //Show initial trace data. When in NORMAL or SINGLE mode the display needs to be drawn because otherwise if there is no signal it remains black
  scope_display_trace_data();

  //Set screen brightness
  fpga_set_translated_brightness();

  //Discard the first response from the user interface controller
  uart1_receive_data();
  
  //Initialize the state machine
  sm_init();
  
  //Process and display trace data and handle user input until power is switched off
  while(1)
  {
    //Only do sampling when enabled. Gets disabled when a menu is open or when file viewing
    if(enablesampling)
    {
      //Go through the trace data and make it ready for displaying
      scope_acquire_trace_data();
    }
    
    //Check if the user provided input and handle it
    sm_handle_user_input();

    //Only display trace data when enabled. Gets disabled when a menu is open or when file viewing, except when viewing a wave file
    if(enabletracedisplay)
    {
      //Display the trace data and the other enabled screen items
      scope_display_trace_data();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
