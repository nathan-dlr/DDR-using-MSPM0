/* IRxmt.cpp
 * Nathan DeLaRosa nad2333 Thompson Truong TT27224
 * Chiou
 * Date: 4/11/24
 * PA8 GPIO output to IR LED, to other microcontroller IR sensor, PA22 UART2 Rx
 */

#include <ti/devices/msp/msp.h>
#include "IRxmt.h"

#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N
#define IR (1<<8)
#define BAUD 2375  // bps
#define PulsePerBit (38000/2375) // 16 pulses each bit
// to get a 38kHz wave, look at output on scope and
// adjust the delay constant to get 38kHz output on PA8


// Initialize GPIO on PA8
// Baud rate=2375 evenly divides into 38,000
// 38 kHz is 26.315us period
// when pulsing, 38 kHz wave is on for 13.158us, off for 13.158us
void IRxmt_Init(void){
    // initialize PA8 as output
    IOMUX->SECCFG.PINCM[PA8INDEX] = 0x81;
    GPIOA->DOE31_0 |= IR;
    // make PA8 low (IR off)
    GPIOA->DOUTCLR31_0 = IR;
}

extern "C" void Delay(void);
void Delay(uint32_t time){
#ifdef __GNUC__
    __asm(".syntax unified");
#endif
    __asm volatile(
"Delay_Loop: SUBS  R0, R0, #1; \n"
"            BNE   Delay_Loop; \n"
    );
}

// baud rate = 2375 bps
// bit time = 1/2375 = 421.05us
// 16 pulses per bit (receiver needs at least 10 pulses to decode IR signal)
// each pulse is 421.06us/16 = 26.315us
// negative logic: 38KHz IR pulses occur with bit=0
// if bit=0 the PA8 pulses, 16 times at on for 13.158us, off for 13.158us
// if bit=1 no PA8 pulses, 16 times at off for 13.158us, off for 13.158us
void IRxmt_SendBit(int bit){
  if (bit) {
      for (int i = 0; i < 32; i++) {
          Delay(334);
      }
  }
  else {
      for (int i = 0; i < 32; i++) {
          GPIOA->DOUTTGL31_0 = IR;
          Delay(334);
      }
  }
}

// output ASCII character to IR transmitter
// blind cycle synchronization
// in Lab 8 this will wait until transmission is done
// start=0,bit0,bit1,bit2,bit2,bit4,bit5,bit6,bit7,stop=1
// 0 means 38 kHz pulse for 1 bit time
// 1 means no pulses for 1 bit time
// should take 10*421.06us = 4.2106ms
void IRxmt_OutChar(char data){
  IRxmt_SendBit(0); //start
  uint8_t bit = 1;  //the bit we're sending
  for (int i = 0; i < 8; i++) {
      IRxmt_SendBit(data & bit);
      bit = bit<<1;
  }
  IRxmt_SendBit(1); //stop
}

