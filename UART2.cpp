/* UART2.cpp
 * Nathan DeLaRosa nad2333 Thompson Truong TT27224
 * Chiou
 * Data:
 * PA22 UART2 Rx from other microcontroller PA8 IR output<br>
 */


#include <ti/devices/msp/msp.h>
#include "UART2.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/FIFO2.h"

uint32_t LostData;
Queue FIFO2;

// power Domain PD0
// for 80MHz bus clock, UART2 clock is ULPCLK 40MHz
// initialize UART2 for 2375 baud rate
// no transmit, interrupt on receive timeout CHECK
void UART2_Init(void){
  UART2->GPRCM.RSTCTL = 0xB1000003;
  UART2->GPRCM.PWREN = 0x26000001;
  Clock_Delay(24); // time for uart to power up
  //IOMUX->SECCFG.PINCM[PA8INDEX]  = 0x00000082;
  IOMUX->SECCFG.PINCM[PA22INDEX]  = 0x00040082;
  UART2->CLKSEL = 0x08; // bus clock
  UART2->CLKDIV = 0x00; // no divide
  UART2->CTL0 &= ~0x09; // disable UART2
  UART2->CTL0 = 0x00020018;
 // assumes an 80 MHz bus clock
  //40,000,000/16 = 2,500,000
  //2,500,000/2375 = 1052.6315
  UART2->IBRD = 1052;//   divider = 1052+40/64 = 1052.625  CHECK
  UART2->FBRD = 40; // CHECK
  UART2->LCRH = 0x00000030;
  UART2->CPU_INT.IMASK = 0x0C01;
  UART2->IFLS = 0x422;
  NVIC->ICPR[0] = 1 << 14;
  NVIC->ISER[0] = 1<< 14;
  NVIC->IP[3] = (NVIC ->IP[3]&(~0xFF000000)) | (2 << 22);
  UART2->CTL0 |= 0x09;
}

//------------UART2_InChar------------
// Get new serial port receive data from FIFO2
// Input: none
// Output: Return 0 if the FIFO2 is empty
//         Return nonzero data from the FIFO1 if available
char UART2_InChar(void){
  char out;
  while (1) {
      if (FIFO2.Get(&out)) {
          //\\if (out ^ 0xFFFFFF00) {  //bit 7
              return out;

      }
  }
}

extern "C" void UART2_IRQHandler(void);
void UART2_IRQHandler(void){ 
// acknowledge, clear RTOUT
    int RxCounter;
    int status;
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
// read all data, putting in FIFO
    while ((UART2->STAT & 0x04) == 0) {   //"as long as the RXFE (bit 2) in the UART2->STAT is zero" while loop or if statement?
        FIFO2.Put(UART2->RXDATA & 0xFF);  //putting data (first byte of RXDATA)
    //"The message will be interpreted in the main program, wat does that mean lol
    }
    RxCounter++;
    status = UART2->CPU_INT.IIDX; //acknowledge interrupt COME BACK TO THIS
// finish writing this
    GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
    return;
}
