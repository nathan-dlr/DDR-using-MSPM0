// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Nathan DeLaRosa
// Thompson Truong
// Last Modified: 1/1/2024

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC.h"
#include "../inc/DAC5.h"
#include "../inc/SPI.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
//#include "ST7735_SDC.h"
#include "integer.h"
#include "diskio.h"
#include "ff.h"
#include "../inc/DAC.cpp"
#include "../inc/FIFO2.h"
#include "IRxmt.h"
#include "UART2.h"




extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);

//#define PA24INDEX 53
//#define PA25INDEX 54
//#define PA26INDEX 58
//#define PA27INDEX 59
//#define PA28INDEX 2

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

class Arrows {
private:
    uint8_t x_coord;
    uint8_t y_coord;
    const uint16_t *image; //pointer to bitmap array
    uint8_t h, w;          //height/width
    uint8_t button;        //corresponding input value
    uint8_t target;        //out of bounds = 0, bad = 1, good = 2, great = 3
    bool remove;

public:
    Arrows() {
        x_coord = 0;
        y_coord = 0;
        image = nullptr;
        h = 0;
        w = 0;
        button = 0;
        target = 0;
        remove = false;
    }
    Arrows(uint8_t x_coord, const uint16_t *image, uint8_t w, uint8_t h, uint8_t button) {
        this->x_coord = x_coord;
        this->image = image;
        this->h = h;
        this->w = w;
        this->button = button;
        y_coord = 160;
        target = 0;  //0 = out of bounds, 1 = okay, 2= good, 3 = great
        remove = false;
    }
    Arrows(Arrows &other) {
        x_coord = other.x_coord;
        y_coord = other.y_coord;
        image = other.image;
        h = other.h;
        w = other.w;
        button = other.button;
        target = other.target;
        remove = other.remove;
    }
    Arrows& operator=(const Arrows& other) {
        if (this != &other) {
            this->x_coord = other.x_coord;
            this->image = other.image;
            this->h = other.h;
            this->w = other.w;
            this->button = other.button;
            this->y_coord = other.y_coord;
            this->target = other.target;
            this->remove = other.remove;
        }
        return *this;
    }

    bool move(){ //returns true if arrows is completely off screen, and false if still on screen
        if (!remove) {
            y_coord -= 5;
            if (y_coord < 6) {
                remove = true;
                return false;
            }
            else {
                target_zone();
                return true;
            }
        }
        return false;
    }
    void target_zone() {
        if ((y_coord >= 65) || (y_coord < 15)) {
            target = 0;
        }
        else if ((55 < y_coord) && (y_coord <= 65) || ((15 < y_coord) && (y_coord <= 20))) {
            target = 1;
        }
        else if ((40 < y_coord) && (y_coord <= 55) || ((20 < y_coord) && (y_coord <= 35))) {
            target = 2;
        }
        else if ((35 < y_coord) && (y_coord <= 40)) {
            target = 3;
        }
   }
   bool pressed () {
       if ((Key_In() & button) && (target > 0)) {
           remove = true;
           return true;
       }
       else {
           return false;
       }
   }
   void draw() {
       ST7735_DrawBitmap(x_coord, y_coord, image, w, h);
   }
   int Adjust_List();
};
Arrows right_arrow(96, right, 32, 38, 8);
Arrows left_arrow(5,left,  29 , 36, 1);
Arrows up_arrow(66, up, 29, 38, 4);
Arrows down_arrow(35, down, 29, 37, 2);
Arrows directions[4] = {right_arrow, left_arrow, up_arrow, down_arrow};

uint32_t count = 0;
uint8_t count2 = 0;
uint32_t score = 0;

struct arrow_list {
    Arrows arr[12];
    Arrows temp[12];
    uint8_t num_arrows = 0;
};
typedef struct arrow_list arrow_list_t;

arrow_list_t active_arrows;

int Arrows::Adjust_List(){
    int cnt = 0;
    int add = 0;
    for (int i = 0; i < active_arrows.num_arrows; i++) {
        if (!active_arrows.arr[i].remove) {
            active_arrows.temp[cnt] = active_arrows.arr[i];
            cnt++;
        }
        else {
            add += active_arrows.arr[i].target;
        }
    }
    active_arrows.num_arrows = cnt;
    score += add * 10;
    return target;
}

// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)temp arr[12];t
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    for (int i = 0; i < active_arrows.num_arrows; i++) {
        active_arrows.arr[i].pressed();
        }
    }
    // 3) move sprites
    for (int j = 0; j < active_arrows.num_arrows; j++) {
        active_arrows.arr[j].move();
    }

    right_arrow.Adjust_List();
    for (int k = 0; k < active_arrows.num_arrows; k++) {
        active_arrows.arr[k] = active_arrows.temp[k];
    }
    //active_arrows.arr = Temp;
    count++;
    count2 += 3;
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
}
/*
void TIMG0_IRQHandler(void) {
    if ((TIMG0->CPU_INT.IIDX) == 1) {
        count1++;
    }
}
*/
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}
void random_arrow() {
    uint8_t idx = active_arrows.num_arrows;
    int num = Random(count2) % 4;
    active_arrows.arr[idx] = directions[num];
    active_arrows.num_arrows += 1;
    count++;
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};



// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(int myPhrase=0; myPhrase<= 2; myPhrase++){
    for(int myL=0; myL<= 3; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

// use main to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(0, 45, background2, 128, 39);
  ST7735_DrawBitmap(96,70, right, 29, 32); //70 is OOB
  ST7735_DrawBitmap(3, 60, left, 30, 28); //40 ish is great




    //Clock_Delay1ms(50);              // delay 50 msec

  //ST7735_FillScreen(0x0000);   // set screen to black
  //ST7735_SetCursor(1, 1);
  //ST7735_OutString((char *)"GAME OVER");
  //ST7735_SetCursor(1, 2);
  //ST7735_OutString((char *)"Nice try,");
  //ST7735_SetCursor(1, 3);
  //ST7735_OutString((char *)"Earthling!");
  //ST7735_SetCursor(2, 4);
  //ST7735_OutUDec(1234);
  while(1){

  }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
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

int main30(void){ // start screen test
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  //Sound_Init();  // initialize sound
  Key_Init();
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
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

// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  //Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      //Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      //Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      //Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      //Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}
int music(void){ // final main
  UINT successfulreads, successfulwrites;
  __disable_irq();
 // set bus speed
  //ST7735_InitPrintf();
  //ST7735_FillScreen(ST7735_BLACK);
  //Sensor.Init(); // PB18 = ADC1 channel 5, slidepot // initialize switches
  //LED_Init();    // initialize LED
  //Sound_Init();  // initialize sound
  //TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
  ///music stuff
  front8 = Buf2; // buffer being output to DAC
  back8 = Buf;   // buffer being written to from SDC
  Count8 = 0;
  flag8 = 1; // 1 means need data into back
  BufCount8 = 0;
  DAC_Init(); // 12bit DAC on PA15
  //////////
    // initialize interrupts on TimerG12 at 30 Hz
  //TimerG12_IntArm(80000000/30,2);
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

      //ST7735_DrawBitmap(0, 45, background2, 128, 39);
      //right_arrow.draw();
      }


    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  return 1;
}


int main(void) {
    __disable_irq();
    PLL_Init(); // set bus speed
    LaunchPad_Init();
    ST7735_InitPrintf();
    ST7735_FillScreen(ST7735_BLACK);
    Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
    Switch_Init(); // initialize switches
    LED_Init();    // initialize LED
    //Sound_Init();  // initialize sound
    Key_Init();
    int i = 0;
    //TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
      // initialize interrupts on TimerG12 at 30 Hz


    // initialize all data structures
    //TimerG0_IntArm(1000, 40, 1); //25ns*1000*40=1ms
    __enable_irq();
    bool isEnglish;
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
    TimerG12_IntArm(80000000/30,2);
    __enable_irq();
    ST7735_FillScreen(0x0000);
    while(1) {
        ST7735_DrawBitmap(0, 45, background2, 128, 48);

        for (i = 0; i < active_arrows.num_arrows; i++) {
            active_arrows.arr[i].draw();
        }
        ST7735_SetCursor(5, 5);
        if (isEnglish) {
            ST7735_OutString((char *)"Score: ");
        }
        else {
            ST7735_OutString((char *)"Puntaje: ");
        }
        ST7735_OutUDec(score);
        if ((count > 100) && (count  % 10 == 0)) {
            random_arrow();
        }
        if (count > 3000) {
          break;
        }

    }
    ST7735_FillScreen(0x0000);
    ST7735_SetCursor(7, 0);
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
    Switch_Init();
    Key_Init();
    __enable_irq();
    while (Key_In() != 0x02000000) {}
    music();
    return 1;

}
