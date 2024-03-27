/************** ECE2049 DEMO CODE ******************/
/**************  25 August 2021   ******************/
/***************************************************/

#include <msp430.h>
#define START_GAME 0;
#define DRAW_ALIENS 1;
#define GET_INPUT 2;
#define UPDATE_POS 3;
#define GAME_OVER 4;
int state = 0;

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */

//For reference, original democode is is reference.c
#include "peripherals.h"

// Function Prototypes
void drawAliens();


// Main
void main(void){
    WDTCTL = WDTPW | WDTHOLD;


    while(1==1){
        switch(state){
        case START_GAME:
            //Start Game
            break;
        case DRAW_ALIENS:
            //Draw Aliens
            break;
        case GET_INPUT:
            //Get Input
            break;
        case UPDATE_POS:
            //update Pos
            break;
        default:
            //Game over
            break;

        }
    }
}
