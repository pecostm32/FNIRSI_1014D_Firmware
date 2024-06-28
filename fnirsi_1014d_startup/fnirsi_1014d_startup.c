//----------------------------------------------------------------------------------------------------------------------------------

#include "arm32.h"
#include "ccu_control.h"
#include "dram_control.h"
#include "display_control.h"
#include "spi_control.h"
#include "bl_display_lib.h"
#include "bl-uart.h"
#include "bl_fpga_control.h"
#include "bl_sd_card_interface.h"
#include "bl_variables.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------
// This code now works but needs a debug flag to disable the displaying of information
//
// For the 1014D a new version that only loads the main code without showing messages and does not wait for the FPGA
// can be distilled and tested.
//
// There is a problem with the SD card code though when used with a specific 4GB card. Could be a speed issue
//----------------------------------------------------------------------------------------------------------------------------------
// The bootloader itself starts on sector 16, so in the package the firmware should be copied in from address 0x8000
//  to allow for 32KB boot loader
//----------------------------------------------------------------------------------------------------------------------------------

#define PROGRAM_START_SECTOR      80

//----------------------------------------------------------------------------------------------------------------------------------

//Buffer for reading header from SD card
unsigned char buffer[512];

//----------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
  //Set the address to run the loaded main program
  unsigned int address = 0x80000000; 
  unsigned int length;
  unsigned int blocks;
  int choice = 0;
  int i,j, retval;
  
  //Initialize the clock system
  sys_clock_init();
  
  //Initialize the internal DRAM
  sys_dram_init();

  //Instead of full memory management just the caches enabled
  arm32_icache_enable();
  arm32_dcache_enable();

  //Initialize display (PORT D + DEBE)
  sys_init_display(SCREEN_WIDTH, SCREEN_HEIGHT, (uint16 *)maindisplaybuffer);
  
  //Setup the display lib
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_set_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);

  //Setup for error message display
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_0);
  
  //Initialize UART1 for communication with the user interface microcontroller
  uart1_init();
  
  //Initialize FPGA (PORT E)
  fpga_init();
  
  //Got some time to spare
  for(j=0;j<1000;j++)
  {
    //At 600MHz CPU_CLK 1000 = ~200uS
    for(i=0;i<1000;i++);
  }
  
  //Wait and make sure FPGA is ready
  fpga_check_ready();

  //Turn of the display brightness
  fpga_set_backlight_brightness(0x78);
  
  //When any additional button is pressed while powering up 0 instead of 49 is returned
  if(uart1_receive_data() == 0)
  {
    //Setup the choices texts
    display_text(610,  28, "Start new PECO firmware");
    display_text(554, 110, "Start original FNIRSI firmware");
    display_text(682, 192, "Start FEL mode");
    
    choice = -1;
    
    //Wait for a valid choice
    while(choice == -1)
    {
      //Wait for some valid input
      while((retval = uart1_receive_data()) == 0);
      
      //Check if a valid choice is made
      switch(retval)
      {
        case UIC_BUTTON_F1:
          choice = 0;
          break;
          
        case UIC_BUTTON_F2:
          choice = 1;
          break;
          
        case UIC_BUTTON_F3:
          choice = 2;
          break;
      }
    }
  }

  //Load the chosen program
  if(choice == 0)
  {
    //Initialize the SD card
    if(sd_card_init() != SD_OK)
    {
      display_set_fg_color(0x00FF0000);
      display_text(10, 10, "SD card init failed");

      //On error just frees
      while(1);
    }

    //Load the first program sector from the SD card
    if(sd_card_read(PROGRAM_START_SECTOR, 1, buffer) != SD_OK)
    {
      display_set_fg_color(0x00FF0000);
      display_text(10, 10, "SD card first read failed");

      //On error just frees
      while(1);
    }

    //Check if there is a brom header there
    if(memcmp(&buffer[4], "eGON.EXE", 8) != 0)
    {
      display_set_fg_color(0x00FF0000);
      display_text(10, 10, "Not an executable");

      //On error just frees
      while(1);
    }

    //Get the length from the header
    length = ((buffer[19] << 24) | (buffer[18] << 16) | (buffer[17] << 8) | buffer[16]);

    //Calculate the number of sectors to read
    blocks = (length + 511) / 512;

    //Copy the first program bytes to DRAM (So the eGON header is skipped)
    memcpy((void *)0x80000000, &buffer[32], 480);

    //Check if more data needs to be read
    if(blocks > 1)
    {
      //Already read the first block
      blocks--;

      //Load the remainder of the program from the SD card
      if((retval = sd_card_read(PROGRAM_START_SECTOR + 1, blocks, (void *)0x800001E0)) != SD_OK)
      {
        display_set_fg_color(0x00FF0000);
        display_text(10, 10, "Failed reading program sector");
        display_text(10, 26, "Error: ");
        display_text(10, 42, "Sector: ");
        display_decimal(150, 26, retval);
        display_decimal(150, 42, blocks);

        //On error just frees
        while(1);
      }
    }

  }
  else if(choice == 1)
  {
    //Initialize SPI for flash (PORT C + SPI0)
    sys_spi_flash_init();
  
    //Load the main program from flash
    //Get the header first
    sys_spi_flash_read(0x27000, buffer, 32);

    //Original boot loader checks on eGON.EXE file identifier

    //Get the length from the header and take of the header
    length = ((buffer[19] << 24) | (buffer[18] << 16) | (buffer[17] << 8) | buffer[16]) - 32;

    //Read the main program into DRAM. 
    sys_spi_flash_read(0x27020, (unsigned char *)0x80000000, length);
  }
  else
  {
    //Clear the screen
    display_set_fg_color(0x00000000);
    display_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    //Show the scope is in FEL mode
    display_set_fg_color(0x00FFFFFF);
    display_text(360, 230, "Running FEL mode");
    
    //Set the address to run the FEL code
    address = 0xFFFF0020;
  }

  //Start the chosen code
  __asm__ __volatile__ ("mov pc, %0\n" :"=r"(address):"0"(address));
}
