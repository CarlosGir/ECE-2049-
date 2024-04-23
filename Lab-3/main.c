//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P6.2
//
//  Description; Toggle red LED by xor'ing P6.2 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430F5529
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P6.2|-->LED
//
//  smj --15 Jan 2016
//***************************************************************************************

#include <msp430.h>
#include <math.h>
#include "peripherals.h"
#include <stdio.h>

//states
#define RUNMODE 0
#define EDITMODE 1

//Function prototypes

void initTimer_A(void);
void displayTime(long unsigned int inTime);
void displayTemp(float inAvgTempC);
void displayWheel(int wheelIn); //for debug
void wheelSetup(void);
void pollWheel(void);
float getTemp(void);
void buttonConfig(void);
int getLeftButton(void);
int getRightButton(void);
void stoptimerA2(void);



//Global vars

int currentState = RUNMODE;

unsigned int in_value = 0;
unsigned int prev_in_value = 0;
int startMonth = 3;
int startDay = 17;
long unsigned int timer = 0;
int currentMonth = 3;
int monthOffset[12] = {1,-1,1,0,1,0,1,1,0,1,0,1};
int tempStore[30];

int main(void) {


    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    configDisplay();


    //timer Stuff
    timer = startMonth * 2592000 + startDay * 86400;
    _enable_interrupt();
    _BIS_SR(GIE);
    initTimer_A();
    wheelSetup();
    int buttonReleased = 0;

    //Define underline location for when editing time
    //{x1,x2}
    int monthCords[2] = {3,18};
    int dayCords[2] = {26,38};
    int hourCords[2] = {46,58};
    int minCords[2] = {62,74};
    int secCords[2] = {80,92};
    int (*coords[5])[2] = {monthCords,dayCords,hourCords,minCords,secCords};

    //Define time increase values
    unsigned long int times[5] = {2592000,86400,3600,60,1};
    //unsigned long int times[5] = {2505600,86400,3600,60,1};



    int clicks=0;
    long unsigned int timeAdd = 0;
    while(1){

        //Regular clock runtime
        if(currentState == RUNMODE){
            if(getLeftButton() == 1){
                currentState = EDITMODE;
                stoptimerA2();
                clicks = 0;
                timeAdd=times[0];
                Graphics_drawLineH(&g_sContext, (*coords[clicks])[0],(*coords[clicks])[1], 20);
                Graphics_flushBuffer(&g_sContext);
                pollWheel(); //get an initial previous value
                prev_in_value = in_value;
            }
        }



        //Go into edit mode with scroll wheel
        else {
            if(getLeftButton() == 1 && buttonReleased == 1){
                buttonReleased = 0;
                clicks++;
                if(clicks>4){
                    clicks = 0;
                }
                timeAdd = times[clicks];

                //clear line and draw new line
                Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_drawLineH(&g_sContext, (*coords[0])[0],(*coords[4])[1], 20);
                Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_BLACK);
                Graphics_drawLineH(&g_sContext, (*coords[clicks])[0],(*coords[clicks])[1], 20);
                Graphics_flushBuffer(&g_sContext);
            }
            else if(getLeftButton() == 0){
                buttonReleased = 1;
            }
            //Use to detect differences in scroll value


            pollWheel(); //blocking function



            //Modify day, will later implement for other time aspects

            if(in_value> 4082 || prev_in_value+80<in_value){
                if(clicks == 0){
                    timeAdd= times[0] + monthOffset[currentMonth]*times[1];
                }
                timer += timeAdd;
                displayTime(timer);
                prev_in_value = in_value;
            }
            else if(prev_in_value-80>in_value){
                if(clicks == 0){
                    timeAdd= times[0] + monthOffset[currentMonth-1]*times[1];
                }
                timer -= timeAdd;
                displayTime(timer);
                prev_in_value = in_value;
            }


            //Exit edit mode
            if(getRightButton() == 1){
                currentState = RUNMODE;
                initTimer_A();

                //clear underline
                Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_drawLineH(&g_sContext, (*coords[0])[0],(*coords[4])[1], 20);
                Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_BLACK);

                displayTime(timer);
            }

        }

      }

    return 0;
   }

void initTimer_A(void)
    {
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 32700; // 32700+1 = 32800 ACLK tics = ~1 seconds
    //need to fix timer so that its within required constraints
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
    }

    //Timer ISR
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void)
    {
        timer++;
        displayTemp(getTemp()); //Get new temp reading every second
        displayTime(timer);
    }

void stoptimerA2(){

    // This function stops Timer A2
    TA2CTL = MC_0; // stop timer
    TA2CCTL0 &= ~CCIE; // TA2CCR0 interrupt disabled

}



//Config launchpad buttons
void buttonConfig(){
    //Left: P2.1
    //Right: P1.1

    P2SEL = P2SEL &~(BIT1);
    P2DIR |= (BIT1);
    P2REN |= (BIT1);
    P2OUT |= (BIT1);

    P1SEL = P1SEL & ~BIT1;
    P1DIR |= BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;


}

int getLeftButton(){
    return !(P2IN & BIT1);
}

int getRightButton(){
    return !(P1IN & BIT1);
}

void displayTime(long unsigned int inTime){
//    int startMonth = 3;
//    int startDay = 17;

    if(inTime>500*86400){
        inTime = 365*86400 - (0xffffffff - inTime) -1;
        timer = inTime;
    }

    char months[12][3] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
    long unsigned int sec = inTime%60;
    long unsigned int min = inTime/60;
    long unsigned int hr = min/60;
    long unsigned int day = hr/24;
    hr = hr%24;
    min = min%60;

    int month = 0;
    while(day>30+monthOffset[month]){
        day-=(30+monthOffset[month]);
        month++;
        if(month>11){
            month = 0;
            timer-= 366 * 86400;
        }
    }
    currentMonth = month;







    char secStr[2] = "  ";
    secStr[0] = sec/10 + 0x30;
    secStr[1] = sec%10 + 0x30;

    char minStr[2] = "  ";
    minStr[0] = min/10 + 0x30;
    minStr[1] = min%10 + 0x30;

    char hrStr[2] = "  ";
    hrStr[0] = hr/10 + 0x30;
    hrStr[1] = hr%10 + 0x30;

    char dayStr[2] = "  ";
    dayStr[0] = day/10 + 0x30;
    dayStr[1] = day%10 + 0x30;



    char output[16] = "000 00 00:00:00";
    output[0]=months[currentMonth][0];
    output[1]=months[currentMonth][1];
    output[2]=months[currentMonth][2];
    output[4]=dayStr[0];
    output[5]=dayStr[1];
    output[7]=hrStr[0];
    output[8]=hrStr[1];
    output[10]=minStr[0];
    output[11]=minStr[1];
    output[13]=secStr[0];
    output[14]=secStr[1];

    Graphics_drawStringCentered(&g_sContext,output, 16, 48, 15, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);


}

void displayTemp(float inAvgTempC){
    float inAvgTempF = (inAvgTempC * (9/5)) + 32;
    char cStr[8] = "000.00 C";
    char fStr[8] = "000.00 F";
    int c = (int) inAvgTempC;
    int f = (int) inAvgTempF;

    int dc = ( (int) ((inAvgTempC - c) * 100));
    int df = ( (int) ((inAvgTempF - f) * 100));

    cStr[0] = c/100 + 0x30;
    cStr[1] = (c%100)/10 + 0x30;
    cStr[2] = c%10 +0x30;
    cStr[4] = dc/10 +0x30;
    cStr[5] = dc%10 +0x30;
    fStr[0] = f/100 + 0x30;
    fStr[1] = (f%100)/10 + 0x30;
    fStr[2] = f%10 +0x30;
    fStr[4] = df/10 +0x30;
    fStr[5] = df%10 +0x30;

    Graphics_drawStringCentered(&g_sContext,cStr, 8, 48, 25, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext,fStr, 8, 48, 35, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

void displayWheel(int wheelIn){
    char wheelStr[5]= "00000";
    wheelStr[0] = wheelIn/10000 + 0x30;
    wheelStr[1] = (wheelIn % 10000)/1000 + 0x30;
    wheelStr[2] = (wheelIn % 1000)/100 + 0x30;
    wheelStr[3] = (wheelIn % 100)/10 + 0x30;
    wheelStr[4] = (wheelIn % 10) + 0x30;
//    Graphics_drawStringCentered(&g_sContext,wheelStr, 5, 48, 45, OPAQUE_TEXT);
//    Graphics_flushBuffer(&g_sContext);

}


void wheelSetup(void){
    // Configure P8.0 as digital IO output and set it to 1
     // This supplied 3.3 volts across scroll wheel potentiometer
     // See schematic at end or MSP-EXP430F5529 board users guide
     P6SEL &= ~BIT0;


     REFCTL0 &= ~REFMSTR;                      // Reset REFMSTR to hand over control of
                                               // internal reference voltages to
                                               // ADC12_A control registers
     ADC12CTL0 = ADC12SHT0_9 | ADC12ON;

     ADC12CTL1 = ADC12SHP;                     // Enable sample timer

     // Use ADC12MEM0 register for conversion results
     ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_0;   // ADC12INCH5 = Scroll wheel = A5


     __delay_cycles(100);                      // delay to allow Ref to settle
     ADC12CTL0 |= ADC12ENC;            // Enable conversion

}


void pollWheel(void){
    ADC12CTL0 &= ~ADC12SC;      // clear the start bit
    ADC12CTL0 |= ADC12SC;       // Sampling and conversion start


    // Poll busy bit waiting for conversion to complete
    while (ADC12CTL1 & ADC12BUSY){
        __no_operation();
    }

    in_value = ADC12MEM0;               // Read results if conversion done
}

float getTemp(void){
    //TODO for Carlos
    /* Write the function so that it returns the
     * moving average of the last 36 seconds from
     * the temperture sensor on the board, as described in
     * part 4 of the lab.
     * This function should return the temp in celcius
     *
     * The lab says to use an array and use something called
     * a rolling index. Might want to look into that.
     *
     * I have initiated an array called float tempStore[30].
     *
     * This function is called during the timer interrupt.
     *
     */
    return 30; //Im setting the return value to 30 for now.
}


