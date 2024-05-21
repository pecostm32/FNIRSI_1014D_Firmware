//----------------------------------------------------------------------------------------------------------------------------------
//The clock generator on the scope is not connected to a true I2C interface of the processor, so bit banging is used
//
//The connections are:
//  PA0:  SDA
//  PA1:  SCL
//
//  CLK0 is used for the generator and ranges from 200MHz down to ??
//  CLK1 is used for the sampling system and needs to be fixed at 50MHz
//
//----------------------------------------------------------------------------------------------------------------------------------

#include "gpio_control.h"
#include "ccu_control.h"
#include "clock_synthesizer.h"

//----------------------------------------------------------------------------------------------------------------------------------
//ClockGen could use register file from ClockBuilder Pro, but currently there are only few registers set, so setting them separately

void clock_synthesizer_setup(void)
{
  //Make sure SCL and SDA are high before enabling the pins
  *PORTA_DATA_REG |= 0x00000003;

  //Setup the used pins to output
  *PORTA_CFG0_REG &= 0xFFFFFF11;

  // 0x03 3 Output Enable Control -- Disable all clock outputs
  i2c_send_data(CS_CLK_DIS,0xFF);

  // Power down clocks - we will rewrite these shortly.
  i2c_send_data(CS_CLK0_CTRL, 0x80);
  i2c_send_data(CS_CLK1_CTRL, 0x80);
  i2c_send_data(CS_CLK2_CTRL, 0x80);

  // PLL setup -- 200MHz && 50MHz config

  // 0x0F 15 PLL Input Source left on default

  // 0x10 16 CLK0 Control -- MS0_INT=Integer |  PLLA | CLK0_SRC=MultiSynth0 | CLK0_IDRV=8mA
  i2c_send_data(CS_CLK0_CTRL, 0x4F);

  // 0x11 17 CLK1 Control -- PLLA | CLK1_SRC=MultiSynth1 | CLK1_IDRV=8mA
  i2c_send_data(CS_CLK1_CTRL, 0x0F);


  // MultiSync PLLA Divider

  // 0x1B 27 MSNA_P3[7:0] -- No zero denominator
  i2c_send_data(CS_MSNA_P3, 0x01);

  // 0x1d 29 MSNA_P1[15:8] -- ( Integer part - 4 ) * 128
  i2c_send_data(CS_MSNA_P1B, 0x0E);  // 800 MHz max

  // MultiSynth0 -- 200MHz configuration
  // 0x2B 43 MS0_P3[7:0] -- No zero denominator
  i2c_send_data(CS_MS0_P3, 0x01);

  // 0x2C 44 R0_DIV[2:0] -- MS0_DIVBY4 for 200MHz clock
  i2c_send_data(CS_MS0_DIV, 0x0C);

  // MultiSynth1 -- 50MHz original
  // 0x33 51 MS1_P3[7:0] -- No zero denominator
  i2c_send_data(CS_MS1_P3, 0x01);

  // 0x35 53 MS1_P1[15:8] -- Integer part * 128 - 4
//  0x05; // 66MHz
//  0x04; // 75MHz
//  0x03; // 80MHz
  i2c_send_data(CS_MS1_P1B, 0x06);   // 50MHz

  // 0xB1 177 PLL_RST
  i2c_send_data(CS_PLL_RST,0xAC);

  // Enable clock outputs
  i2c_send_data(CS_CLK_DIS, 0xFC);
}

//----------------------------------------------------------------------------------------------------------------------------------
//I2C functions

void i2c_send_data(uint8 reg_addr, uint8 data)
{
  //Start a communication sequence
  i2c_send_start();

  //Send the device address for writing
  i2c_send_byte(CS_DEVICE_ADDR_WRITE);

  //Send the register address
  i2c_send_byte(reg_addr);

  //Send a byte
  i2c_send_byte(data);

  //Stop the communication sequence
  i2c_send_stop();
}

//----------------------------------------------------------------------------------------------------------------------------------
//Since no read is done the port pins can remain as outputs and is there no need to set them again in each function

void i2c_send_byte(uint8 data)
{
  int i;

  //Send 8 bits
  for(i=0;i<8;i++)
  {
    //Check if bit to send is high or low
    if(data & 0x80)
    {
      //Make SDA high
      *PORTA_DATA_REG |= 0x00000001;
    }
    else
    {
      //Make SDA low
      *PORTA_DATA_REG &= 0x00000002;
    }

    //Wait for a while
    i2c_delay(CS_DATA_DELAY);

    //Make SCL high
    *PORTA_DATA_REG |= 0x00000002;

    //Wait for a while
    i2c_delay(CS_CLOCK_DELAY);

    //Make SCL low
    *PORTA_DATA_REG &= 0x00000001;

    //Wait for a while
    i2c_delay(CS_DATA_DELAY);

    //Select the next bit to send
    data <<= 1;
  }

  //Clock the ack bit
  i2c_clock_ack();
}

//----------------------------------------------------------------------------------------------------------------------------------

void i2c_send_start(void)
{
  //Make SDA high
  *PORTA_DATA_REG |= 0x00000001;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);

  //Make SCL high
  *PORTA_DATA_REG |= 0x00000002;

  //Wait for a while
  i2c_delay(CS_CLOCK_DELAY);

  //Make SDA low
  *PORTA_DATA_REG &= 0x0000000E;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);

  //Make SCL low
  *PORTA_DATA_REG &= 0x0000000D;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void i2c_send_stop(void)
{
  //Make SDA low
  *PORTA_DATA_REG &= 0x0000000E;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);

  //Make SCL high
  *PORTA_DATA_REG |= 0x00000002;

  //Wait for a while
  i2c_delay(CS_CLOCK_DELAY);

  //Make SDA high
  *PORTA_DATA_REG |= 0x00000001;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void i2c_clock_ack(void)
{
  //Make SDA low
  *PORTA_DATA_REG &= 0x00000002;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);

  //Make SCL high
  *PORTA_DATA_REG |= 0x00000002;

  //Wait for a while
  i2c_delay(CS_CLOCK_DELAY);

  //Make SCL low
  *PORTA_DATA_REG &= 0x00000001;

  //Wait for a while
  i2c_delay(CS_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------
//A count of 4 is approximately 3uS when running on 600MHz with cache enabled

void i2c_delay(uint32 usec)
{
  //Lower then 64 does not work properly, because the panel fails to hold the new configuration when coming from the original code
  unsigned int loops = usec * 90;

  __asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"  "bne 1b":"=r"(loops):"0"(loops));
}

//----------------------------------------------------------------------------------------------------------------------------------

