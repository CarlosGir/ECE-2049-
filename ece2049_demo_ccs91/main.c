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
#include "midi.h"

//Function prototypes
void buttonConfig(void);
int getButtons(void);
void configUserLED(char val);
int getPitch(int n);
void BuzzerOnPitch(int pitch);
void initTimer_A(void);
void setupLEDS(void);
void setLEDS(int leds);
void correctLight(void);
void incorrectLight(void);
void usrLightsOff(void);

//Global vars

int delay = 0;
int timer = 0;
int timeoutReached = 0;

int main(void) {
    int leds = 0;
    int noteIndex = 0;
    int pitch; //for debugging
    int btnPressed=0;
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    setupLEDS();
    buttonConfig();

    //Set up user leds
    P1SEL = P1SEL & ~BIT0;
    P1DIR |= BIT0;
    P4SEL = P4SEL & ~BIT7;
    P4DIR |= BIT7;

    //Set up first delay
    delay = notes[0][1];

    //notes[][0] = note number
    //notes[][1] = delay
    //this is easier than using a struct

    //timer Stuff
    _enable_interrupt();
    _BIS_SR(GIE);
    initTimer_A();





    //Main loop, terminate when out of notes
    while(noteIndex < 1240){

            //get delay of note (regular speed is note/2)
            delay = notes[noteIndex][1];

            //if correct button pressed
            if (getButtons() == leds && getButtons() != 0 && btnPressed == 0){
                btnPressed = 1;
                correctLight();
                leds = 0;
                setLEDS(leds);
            }

            //if incorrect button pressed
            else if(getButtons() != leds && getButtons() != 0 && btnPressed == 0){
                btnPressed = 1;
                incorrectLight();
            }

            //TODO, make a case where the button is not pressed in time




            if (timer >= delay){
                //get the next note
                pitch = getPitch(notes[noteIndex][0]);
                BuzzerOnPitch(pitch);

                //Figure out which led to light up
                int bShift = notes[noteIndex][0] % 4;
                if(notes[noteIndex][0] == 0){
                    leds = 0;
                }
                else{
                    leds = 1 << bShift;
                }

                usrLightsOff();
                btnPressed = 0;
                setLEDS(leds);
                timer = 0;
                noteIndex++;

            }
        }
        return 0;
    }

void initTimer_A(void)
    {
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 163; // 163+1 = 164 ACLK tics = ~0.005 seconds
    //need to fix timer so that its within required constraints
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
    }

    //Timer ISR
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void)
    {
        timer++;
    }


void buttonConfig(){
    //S1: P7.0
    //S2: P3.6
    //S3: P2.2
    //S4: P7.4

    P7SEL = P7SEL &~(BIT0|BIT4);
    P7DIR |= (BIT0|BIT4);
    P7REN |= (BIT0|BIT4);
    P7OUT |= (BIT0|BIT4);

    P3SEL = P3SEL & ~BIT6;
    P3DIR |= BIT6;
    P3REN |= BIT6;
    P3OUT |= BIT6;

    P2SEL = P4SEL & ~BIT2;
    P2DIR |= BIT2;
    P2REN |= BIT2;
    P2OUT |= BIT2;


}
int getButtons(){
    //P7
    int S1 = !(P7IN & BIT0);
    int S2 = !(P3IN & BIT6) << 1;
    int S3 = !(P2IN & BIT2) << 2;
    int S4 = !(P7IN & BIT4) << 3;
    return( 0|S1|S2|S3|S4);


}

void incorrectLight(void){
    P1OUT |= BIT0;
}
void correctLight(void){
    P4OUT |= BIT7;
}
void usrLightsOff(void){
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;
}

void configUserLED(char val){
    //LED1: P1.0
    //LED2: P4.7

    //Setup
    P1SEL = P1SEL & ~BIT0;
    P1DIR |= BIT0;
    P4SEL = P4SEL & ~BIT7;
    P4DIR |= BIT7;

    int out = 0;
    if('0' <= val && val <= '9'){
        out += val - '0';

    }
    else if('A' <= val <= 'F'){
        out+= val - 'A' + 10;
    }
    else{
        out+= val - 'a' + 10;
    }
    P1OUT |= (out & BIT0);
    P4OUT |= ( (out & BIT1) << 6);

}

void setLEDS(int leds){
    leds <<= 1;
    //Input: S4 S3 S2 S1
    int D1 = (leds & BIT1) << 1;
    int D2 = (leds & BIT2) >> 1;
    int D3 = leds & BIT3;
    int D4 = leds & BIT4;
    P6OUT = (D4|D3|D1|D2);

}

void setupLEDS(){
    //D1: P6.2
    //D2: P6.1
    //D3: P6.3
    //D4: P6.4
    int bits = BIT1|BIT2|BIT3|BIT4;
    P6SEL = P1SEL & ~bits;
    P6DIR |= bits;

}


int getPitch(int n){
    if(n == 0){
        return 0;
    }
    float fn = (float) n;

    float pitch = powf(2,(fn-69)/12)*440;
    return (int) pitch;

}

void BuzzerOnPitch(int pitch)
{
    // Initialize PWM output on P3.5, which corresponds to TB0.5
    P3SEL |= BIT5; // Select peripheral output mode for P3.5
    P3DIR |= BIT5;

    TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
    TB0CTL  &= ~TBIE;                       // Explicitly Disable timer interrupts for safety

    // Now configure the timer period, which controls the PWM period
    // Doing this with a hard coded values is NOT the best method
    // We do it here only as an example. You will fix this in Lab 2.
    TB0CCR0   = 32768/pitch;                    // Set the PWM period in ACLK ticks
    TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

    // Configure CC register 5, which is connected to our PWM pin TB0.5
    TB0CCTL5  = OUTMOD_7;                   // Set/reset mode for PWM
    TB0CCTL5 &= ~CCIE;                      // Disable capture/compare interrupts
    TB0CCR5   = TB0CCR0/2;                  // Configure a 50% duty cycle
}

//Ways to store notes,pitch, and duration

//Opt1: struct
//Opt2: int with different bits correspondance: 0000000000000000

//First Bit: Always 0 for bitshift logic
//Next 5 Bits: Note
//Rest of bits: duration

//A2 :0
//A2#:1
//B2 :2
//C2 :3
//C2#:4
//And so on


//To get frequency, just execute (note * 2^(1/12) + 110)
//In order to get led assossiation, opperation note % 4 is done.


