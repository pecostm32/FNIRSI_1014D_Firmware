//----------------------------------------------------------------------------------------------------------------------------------

#ifndef UART_H
#define UART_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define UART1_RX_REG           ((volatile uint32 *)(0x01C25400))
#define UART1_TX_REG           ((volatile uint32 *)(0x01C25400))
#define UART1_DLL_REG          ((volatile uint32 *)(0x01C25400))
#define UART1_DLM_REG          ((volatile uint32 *)(0x01C25404))
#define UART1_IE_REG           ((volatile uint32 *)(0x01C25404))
#define UART1_FC_REG           ((volatile uint32 *)(0x01C25408))
#define UART1_LC_REG           ((volatile uint32 *)(0x01C2540C))
#define UART1_MC_REG           ((volatile uint32 *)(0x01C25410))
#define UART1_LS_REG           ((volatile uint32 *)(0x01C25414))
#define UART1_MS_REG           ((volatile uint32 *)(0x01C25418))
#define UART1_S_REG            ((volatile uint32 *)(0x01C2547C))
#define UART1_DBG_DLL_REG      ((volatile uint32 *)(0x01C254B0))

//----------------------------------------------------------------------------------------------------------------------------------

#define UART_IER_RDI           0x00000001

//----------------------------------------------------------------------------------------------------------------------------------

#define UART_LSR_DR            0x00000001
#define UART_LSR_OE            0x00000002
#define UART_LSR_PE            0x00000004
#define UART_LSR_FE            0x00000008
#define UART_LSR_BI            0x00000010
#define UART_LSR_THRE          0x00000020
#define UART_LSR_TEMT          0x00000040
#define UART_LSR_FIFOERR       0x00000080

//----------------------------------------------------------------------------------------------------------------------------------

#define UART_SR_BUSY           0x00000001
#define UART_SR_TFNF           0x00000002
#define UART_SR_TFE            0x00000004
#define UART_SR_RFNE           0x00000008
#define UART_SR_RFF            0x00000010

//----------------------------------------------------------------------------------------------------------------------------------

#define UART_FCR_ENABLE_FIFO   0x00000001
#define UART_FCR_CLEAR_RCVR    0x00000002
#define UART_FCR_CLEAR_XMIT    0x00000004
#define UART_FCR_T_TRIG_11     0x00000030

//----------------------------------------------------------------------------------------------------------------------------------

#define UART_LCR_DLAB          0x00000080

#define UART_LCR_WLEN8         0x00000003

//----------------------------------------------------------------------------------------------------------------------------------

void uart1_init(void);

//Requests data from the user interface controller and returns it to the caller
uint8 uart1_receive_data(void);

//Requests data from the user interface controller when no previous command is set and sets it in the toprocesscommand variable
uint8 uart1_get_user_input(void);

//Waits for user input and sets the received command in the lastreceivedcommand variable
void uart1_wait_for_user_input(void); 


//----------------------------------------------------------------------------------------------------------------------------------

#endif /* UART_H */

//----------------------------------------------------------------------------------------------------------------------------------

