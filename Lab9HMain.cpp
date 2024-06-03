// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Nathan DeLaRosa
// Thompson Truong
// Last Modified: 5/29/2024

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC.h"
#include "../inc/SPI.h"
#include "SmallFont.h"
#include "Switch.h"
#include "images/images.h"
#include "integer.h"
#include "diskio.h"
#include "ff.h"
#include "../inc/DAC.cpp"
#include "../inc/FIFO2.h"
#include "Arrows.h"
#include "arrow_list.h"
#include "common.h"


extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);


static FATFS g_sFatFs;
FIL Handle,Handle2;
FRESULT MountFresult;
FRESULT Fresult;
DRESULT Result;

uint32_t IdleTime=0;
void diskError(char *errtype, int32_t code, int32_t block){

  while(1){};
}

#define BUFSIZE8 512
uint8_t Buf[BUFSIZE8];
uint8_t Buf2[BUFSIZE8];
uint32_t Count8;
uint8_t *front8; // buffer being output to DAC
uint8_t *back8;  // buffer being loaded from SDC
int flag8; // 1 means need data into back
#define NUMBUF8 (1004563/BUFSIZE8)
uint32_t BufCount8; // 0 to NUMBUF8-1
void SysTick_IntArm(uint32_t period, uint32_t priority){
  SysTick->CTRL  = 0x00;      // disable during initialization
  SysTick->LOAD  = period-1;  // set reload register
  //The ARM Cortex-M0+ only implements the most significant 2 bits of each 8-bit priority field (giving the 4 priority levels).
  SCB->SHP[1]    = (SCB->SHP[1]&(~0xC0000000))|priority<<30;    // set priority (bits 31,30)
  SysTick->VAL   = 0;         // clear count, cause reload
  SysTick->CTRL  = 0x07;      // Enable SysTick IRQ and SysTick Timer
}
extern "C" void SysTick_Handler(void);
void SysTick_Handler(void){
  GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27
  uint32_t data = front8[Count8]<<4;
//  if(data>4095) data = 4095;
  DAC_Out(data); // 8 to 12 bit
  Count8++;
  if(Count8 == BUFSIZE8){
    Count8 = 0;
    uint8_t *pt = front8;
    front8 = back8;
    back8 = pt; // swap buffers
    flag8 = 1;  // need more data
  }
}


<<<<<<< HEAD
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
  inputs &= 0x01F; // 1111 0000 0000 0000 0000 0000 0000
  ///inputs = inputs >> 24;
  return inputs;
}



// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
=======

>>>>>>> 188de40 (created seperate files for arrow class, arrow_list struct, and switch inputs. moved adjust_arrow function into Lab9HMain.cpp and added simple class methods to access private data members)
void PLL_Init(void){ // set phase lock loop (PLL)
  //Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}


uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


SlidePot Sensor(1500,0); // copy calibration from Lab 7

bool remove_arrow = false;

arrow_list_t active_arrows;

Arrows right_arrow(96, right, 32, 38, 8);
Arrows left_arrow(5,left,  29 , 36, 1);
Arrows up_arrow(66, up, 29, 38, 4);
Arrows down_arrow(35, down, 29, 37, 2);
Arrows directions[4] = {right_arrow, left_arrow, up_arrow, down_arrow};



uint32_t time_counter = 0;
uint8_t rndm_arrow_cntr = 0;
uint32_t score = 0;


void adjust_arrows() {
    int cnt = 0;
        int add = 0;
        for (int i = 0; i < active_arrows.num_arrows; i++) {
            if (!active_arrows.arr[i].get_remove_state()) {
                active_arrows.temp[cnt] = active_arrows.arr[i];
                cnt++;
            }
            else {
                add += active_arrows.arr[i].get_score();
            }
        }
        active_arrows.num_arrows = cnt;
        score += add * 10;

        for (int k = 0; k < active_arrows.num_arrows; k++) {
            active_arrows.arr[k] = active_arrows.temp[k];
        }
}


// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)temp arr[12];t
    //READ SWITCH INPUTS
    for (int i = 0; i < active_arrows.num_arrows; i++) {
        active_arrows.arr[i].pressed();
        }
    }
    //MOVE SPRITES
    for (int j = 0; j < active_arrows.num_arrows; j++) {
        active_arrows.arr[j].move();
    }

    //ADJUST LIST OF ARROWS
    adjust_arrows();

<<<<<<< HEAD
    right_arrow.Adjust_List();
    for (int k = 0; k < active_arrows.num_arrows; k++) {
        active_arrows.arr[k] = active_arrows.temp[k];
    }
    count++;
    count2 += 3;
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
=======
    //INCREMENT COUNTERS
    time_counter++;
    rndm_arrow_cntr += 3;
>>>>>>> 188de40 (created seperate files for arrow class, arrow_list struct, and switch inputs. moved adjust_arrow function into Lab9HMain.cpp and added simple class methods to access private data members)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
}



void random_arrow() {
    uint8_t idx = active_arrows.num_arrows;
    int num = Random(rndm_arrow_cntr) % 4;
    active_arrows.arr[idx] = directions[num];
    active_arrows.num_arrows += 1;
    time_counter++;
}




<<<<<<< HEAD

=======
>>>>>>> 188de40 (created seperate files for arrow class, arrow_list struct, and switch inputs. moved adjust_arrow function into Lab9HMain.cpp and added simple class methods to access private data members)
int test_switches(void){ // main3
    uint32_t last=0,now;
    Clock_Init80MHz(0);
    LaunchPad_Init();
    Key_Init(); // your Lab 5 initialization


    while(1){
      now = Key_In(); // Your Lab5 input
      if(now != last){ // change
          ST7735_OutString((char *)"Switch= 0x"); ST7735_OutUHex2(now,123);ST7735_OutString((char *)"\n\r");
      }
      last = now;
      Clock_Delay(800000); // 10ms, to debounce switch
    }
}

int start_screen_test(void){ // start screen test
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  //Sound_Init();  // initialize sound
  Key_Init();
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();
  bool isEnglish;
  int wait = 1;
  while(1){
      while(wait) {
          ST7735_DrawBitmap(0,90, DDRlogo, 123, 33);
          ST7735_DrawBitmap(30, 125, language, 62, 26);
          uint32_t sensorVal = Sensor.In();

          if (sensorVal >= 2048) { //ENGLISH
              ST7735_FillRect(10,117,18,17,ST7735_BLACK);
              ST7735_DrawBitmap(10, 116, languageselect, 18, 17);
              isEnglish = true;

          }
          else { //SPANISH
              ST7735_FillRect(10,99,18,11,ST7735_BLACK);
              ST7735_DrawBitmap(10,126, languageselect, 18, 17);
              isEnglish = false;
          }
          if (Key_In()) {
              wait = 0;
          }
      }
      if (isEnglish) {
          ST7735_DrawBitmap(96,70, right, 29, 32); //70 is OOB


      }
      else {
          ST7735_DrawBitmap(3, 60, left, 30, 28); //40 ish is great
      }


    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}


int music(void){ // final main
  UINT successfulreads, successfulwrites;
  __disable_irq();
  ///music stuff
  front8 = Buf2; // buffer being output to DAC
  back8 = Buf;   // buffer being written to from SDC
  Count8 = 0;
  flag8 = 1; // 1 means need data into back
  BufCount8 = 0;
  DAC_Init(); // 12bit DAC on PA15
  SysTick_IntArm(80000000/11025,0); // 11.025kHz FOR MUSIC
  // initialize all data structures
  __enable_irq();


  //Result = disk_initialize(0); // initialize disk

  //if(Result) diskError("disk_initialize", Result, 0);
  MountFresult = f_mount(&g_sFatFs, "", 0);
  if(MountFresult){
      //ST7735_DrawString(0, 0, "f_mount error", ST7735_Color565(0, 0, 255));
      while(1){};
  }
  // open the file to be read
  Fresult = f_open(&Handle, "death.bin", FA_READ);
  if(Fresult){
      //ST7735_DrawString(0, 6, "open error", ST7735_Color565(255, 0, 0));

      while(1){
          //ST7735_FillScreen(ST7735_RED);
      }
  }
  while(1){
      //ST7735_FillScreen(ST7735_GREEN);
      if(flag8){ // 1 means need data
          flag8 = 0;
          GPIOB->DOUTSET31_0 = RED; // set PB26
          // 1.5ms to 1.6ms to read 512 bytes
          Fresult = f_read(&Handle, back8, BUFSIZE8, &successfulreads);
          GPIOB->DOUTCLR31_0 = RED; // clear PB26
          if(Fresult){
              //ST7735_DrawString(0, 6, "read error", ST7735_Color565(255, 0, 0));
              while(1){};
          }
          BufCount8++;
          if(BufCount8 == NUMBUF8){ // could have seeked
              Fresult = f_close(&Handle);
              Fresult = f_open(&Handle, "death.bin", FA_READ);
              BufCount8 = 0;
          }
      }
  }


  return 1;
}


int main(void) {
    __disable_irq();
    PLL_Init(); // set bus speed
    LaunchPad_Init();
    ST7735_InitPrintf();
    ST7735_FillScreen(ST7735_BLACK);
    Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
    Key_Init();  // initialize switches
    int i = 0;
    __enable_irq();
    bool isEnglish;
    //START SCREEN
    //SELECT LANGUAGE
    while(!Key_In()){

            ST7735_DrawBitmap(0,90, DDRlogo, 123, 33);
            ST7735_DrawBitmap(30, 125, language, 62, 26);
            uint32_t sensorVal = Sensor.In();

            if (sensorVal >= 2048) { //ENGLISH
                ST7735_FillRect(10,117,18,17,ST7735_BLACK);
                ST7735_DrawBitmap(10, 116, languageselect, 18, 17);
                isEnglish = true;

            }
            else { //SPANISH
                ST7735_FillRect(10,99,18,11,ST7735_BLACK);
                ST7735_DrawBitmap(10,126, languageselect, 18, 17);
                isEnglish = false;
            }
    }
    __disable_irq();
    // initialize interrupts on TimerG12 at 30 Hz
    TimerG12_IntArm(80000000/30,2);
    __enable_irq();
    ST7735_FillScreen(0x0000);
    //BEGIN GAMEPLAY
    while(1) {
        //COVER ANY LEFTOVER SPRITE
        if (remove_arrow) {
            ST7735_FillRect(10,40,128,60,ST7735_BLACK);
            remove_arrow = false;
        }
        ST7735_DrawBitmap(0, 45, background2, 128, 48);

        //DRAW ARROW BITMAPS
        for (i = 0; i < active_arrows.num_arrows; i++) {
            active_arrows.arr[i].draw();
        }
        //DISPLAY SCORE
        ST7735_SetCursor(7, 14);
        if (isEnglish) {
            ST7735_OutString((char *)"Score: ");
        }
        else {
            ST7735_OutString((char *)"Puntaje: ");
        }
        ST7735_OutUDec(score);
        //SEND OUT ARROW
        if ((time_counter > 50) && (time_counter  % 10 == 0)) {
            random_arrow();
        }
        //FINISH
        if (time_counter > 2750) {
          break;
        }

    }
    ST7735_FillScreen(0x0000);
    ST7735_SetCursor(5, 1);
    if (isEnglish) {
        ST7735_OutString((char *)"Score: ");
    }
    else {
        ST7735_OutString((char *)"Puntaje: ");
    }
    ST7735_OutUDec(score);
    ST7735_DrawBitmap(10, 120, endkirb, 110, 82);


    return 1;
}

int main5(void) {  //THIS FUNCTION IS PLAYED ON THE OTHER MICROCONTROLLER
    __disable_irq();
    PLL_Init();
    LaunchPad_Init();
    Key_Init();
    __enable_irq();
    while (Key_In() != 0x02000000) {}
    music();
    return 1;

}
