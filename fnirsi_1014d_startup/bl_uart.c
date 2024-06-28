//----------------------------------------------------------------------------------------------------------------------------------

#include "ccu_control.h"
#include "gpio_control.h"
#include "bl-uart.h"

//----------------------------------------------------------------------------------------------------------------------------------
//The connections for the UART are on:
//  PA2:  UART1_RX
//  PA3:  UART1_TX
//----------------------------------------------------------------------------------------------------------------------------------

void uart1_init(void)
{
  //PORTA is also used for the communication with the clock synthesizer IC and is setup elsewhere too, so no direct write here
  //After reset the pins are disabled and set to 7. Anding with 5 sets it to the needed value of 5
  *PORTA_CFG0_REG &= 0xFFFF55FF;
//  *PORTA_CFG0_REG |= 0x00005500;

  //Enable UART1 clock
  *CCU_BUS_CLK_GATE2 |= CCU_BCRG2_UART1_EN;

  //De-assert the reset on UART1
  *CCU_BUS_SOFT_RST2 |= CCU_BSRR2_UART1_RST;

  //No modem control used
  *UART1_MC_REG = 0;

  //Enable access to the divisor latch register. Probably no need to or here, just set the bit
  *UART1_LC_REG |= UART_LCR_DLAB;

  //set divisor latch LSB 24/25 * 0x51
  *UART1_DLL_REG = 0x4E;

  //divisor latch MSB left at default 0
  *UART1_DLM_REG = 0;

  //Setup the UART for working with 8 bit data, 1 stop bit and no parity
  *UART1_LC_REG = UART_LCR_WLEN8;
}

//----------------------------------------------------------------------------------------------------------------------------------

uint8 uart1_receive_data(void)
{
  //Wait for the UART to be ready to transmit new data
  while((*UART1_LS_REG & UART_LSR_TEMT) == 0);

  //Send the poll request byte to the target
  *UART1_TX_REG = 0xFF;

  //Wait for the response from the target
  while((*UART1_LS_REG & UART_LSR_DR) == 0);

  //Return the received data
  return(*UART1_RX_REG);
}

//----------------------------------------------------------------------------------------------------------------------------------
