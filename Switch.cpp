
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Key_Init(void){
// Assumes LaunchPad_Init has been called
// I.e., PortB has already been reset and activated (do not reset PortB here again)
    uint32_t Input = 0x40081;
    IOMUX->SECCFG.PINCM[PB0INDEX] = Input;
    IOMUX->SECCFG.PINCM[PB1INDEX] = Input;
    IOMUX->SECCFG.PINCM[PB2INDEX] = Input;
    IOMUX->SECCFG.PINCM[PB3INDEX] = Input;
}

uint32_t Key_In(void){
    uint32_t inputs = GPIOB->DIN31_0;
    inputs &= 0x01F;
    return inputs;
}
