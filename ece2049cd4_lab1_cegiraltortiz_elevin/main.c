/************** ECE2049 DEMO CODE ******************/
/**************  25 August 2021   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include "peripherals.h"

#define NUM_ROWS 5
#define NUM_COLS 5

//States
#define START_GAME 0
#define DRAW_ALIENS 1
#define GET_INPUT 2
#define UPDATE_POS 3
#define GAME_OVER 4

// Function Prototype
void drawAliens(int screen[NUM_ROWS][NUM_COLS]);
void welcomeScreen(int level);
int random(int lowerBound,int upperBound);
void drawAliensRow(int alienR[NUM_ROWS],int row);
void swDelay(char numLoops);
void countDown();



// Main
void main(void){
    //Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //Set Up variables
    long int loopCount = 0;
    long int timeout = 40000;
    int aliens[NUM_ROWS][NUM_COLS] = {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}}; //This is what the screen looks like
    int level = 1;

    int alienCount = 5;    //how many aliens are left to print to the screen
    int aliensTotal = 5;   //how many total aliens this level should have
    int aliensShot = 0;    // how many aliens have been shot

    int state = START_GAME; //current state

    //program configurations
    configDisplay();
    configKeypad();
    srand(5);

    //Start Game
    welcomeScreen(level);


    while(1){

        //Check if timeout reached for position update
        if(loopCount >= timeout){
            state = UPDATE_POS;
            loopCount = 0;
        }

        //State Machine
        switch(state){

        case START_GAME:
        {
            BuzzerOff();
            //if player presses *, start game
            if(getKey() == '*'){
                countDown();
                BuzzerOn(50);
                swDelay(1);
                BuzzerOn(20);
                swDelay(1);
                BuzzerOff();
                state = UPDATE_POS;
            }

            break;
        }

        case GET_INPUT:
        {
            int key = (int) getKey()-'0';

            if(key>0 && key<NUM_COLS+1){
                int i;
                for(i = NUM_ROWS-1; i >= 0; i--){  //start at bottom to find first alien shot
                    if(aliens[i][key-1] != 0){
                        aliens[i][key-1] = 0;

                        BuzzerOn(100);
                        swDelay(1);
                        BuzzerOff();

                        aliensShot++;

                        drawAliensRow(aliens[i],i); //Drawing just the row makes rendering faster
                        break;
                    }
                }

            }

            //Check win condition
            if(aliensShot == aliensTotal){
                //GET_INPUT --> START_GAME
                state = START_GAME;
                level++;
                aliensTotal++;
                alienCount = aliensTotal;
                aliensShot = 0;
                timeout = timeout/2;
                loopCount = 0;
                welcomeScreen(level);

            }

            loopCount++;
            break;
        }

        case UPDATE_POS:
        {
            int i;
            int a;
            int b;

            //Moving Aliens down 1
            for(i=NUM_ROWS-1; i > 0; i--){   //Iterate starting from bottom to prevent overwrite of data
                int j;
                for(j = 0; j < NUM_COLS; j++){
                    if(aliens[i][j] !=0 && i == NUM_ROWS-1){
                        state = GAME_OVER; //End game if there are aliens on last row
                    }
                    else{
                        aliens[i][j]=aliens[i-1][j];
                    }
                }
            }

            //Set new row to 0
            for(i = 0; i < NUM_COLS; i++){
                aliens[0][i] = 0;
            }

            //Generate new aliens
                int maxAliens = level+1;
                if(maxAliens > NUM_COLS){
                    maxAliens = NUM_COLS;
                }
                int num = random(1,maxAliens);
                for(i = 0; i < num && alienCount > 0; i++){
                    int alien = random(1,NUM_COLS);
                    //If alien is already in that pos, find new alien
                    while(aliens[0][alien-1] != 0){
                        alien = random(1,NUM_COLS);
                    }
                       aliens[0][alien-1] = alien;
                       alienCount--;
                }
                drawAliens(aliens);






           if(state != GAME_OVER){
              //UPDATE_POS --> GET_INPUT
              state = GET_INPUT;
           }

           else{
               //UPDATE_POS --> GAME_OVER
               Graphics_clearDisplay(&g_sContext); // Clear the display
               Graphics_drawStringCentered(&g_sContext, "GAME OVER", 9, 50, 15, TRANSPARENT_TEXT);
               Graphics_drawStringCentered(&g_sContext, "YOU SUCK LOL", 9, 50, 25, TRANSPARENT_TEXT);
               Graphics_flushBuffer(&g_sContext);


               //Reset
               level = 1;
               aliensTotal = 5;
               alienCount = 5;
               aliensShot = 0;
               timeout = 40000;
               loopCount = 0;
               for(a = NUM_ROWS-1; a >= 0; a--){
                   for(b = NUM_COLS-1; b >=0; b--){
                       aliens[a][b] = 0;
                   }
               }


               //Annoying Stuff
               BuzzerOn(128);
               swDelay(5);
               BuzzerOn(300);

               //Reset Screen
               Graphics_clearDisplay(&g_sContext);
               welcomeScreen(level);
               state = START_GAME;
           }
            loopCount++;
            break;
        }
        default:
             getKey();
        }
    }
}

//draw all aliens
void drawAliens(int screen[NUM_ROWS][NUM_COLS]){
    Graphics_clearDisplay(&g_sContext); // Clear the display
    int pos = 15;
    int i;

    for(i = 0; i < NUM_ROWS;i++){
        unsigned char row[11] = "           ";  //leave a space in between each alien for formatting purposes
        int j;

        for(j = 0; j < NUM_COLS; j++){
            //draw alien if not 0 in array
            if(screen[i][j]!= 0){
                row[j*2] = '0' + screen[i][j];
            }
        }
        Graphics_drawStringCentered(&g_sContext, row, 11, 50, pos, OPAQUE_TEXT);
        pos+=10;
    }
    Graphics_flushBuffer(&g_sContext);


}

//For Drawing 1 alien row (faster)
void drawAliensRow(int alienR[NUM_COLS],int row){
    int pos = 15;
    unsigned char str[11] = "           ";  //leave a space in between each alien for formatting purposes
    int i=0;
    for(i = 0; i < NUM_COLS; i++){
        //draw alien if not 0 in array
        if(alienR[i]!= 0){
            str[i*2] = '0' + alienR[i];
        }
    }
    Graphics_drawStringCentered(&g_sContext, str, 11, 50, pos+row*10, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

//Draw welcome screen
void welcomeScreen(int level){
    unsigned char lvlStr[8] = "LEVEL 00";

    //Write level in string
    lvlStr[6] = '0'+level/10;
    lvlStr[7] = '0'+level % 10;

    Graphics_clearDisplay(&g_sContext); // Clear the display
    Graphics_drawStringCentered(&g_sContext, "SPACE INVADORS", 14, 50, 15, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, lvlStr, 8, 50, 25, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Press * to play", 15, 50, 45, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

//generate random number within range (inclusive)
int random(int lowerBound,int upperBound){
    return rand() % (upperBound - lowerBound + 1) + lowerBound;
}

//SW Delay from starter code
void swDelay(char numLoops){
    volatile unsigned int i,j;
    for (j=0; j<numLoops; j++){
        i = 50000 ;
        while (i > 0)
           i--;
    }
}

void countDown(){
    //Count Down, could have been a for loop but I had to deal with CCS being dumb so I kept it simple
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
    BuzzerOn(150);
    Graphics_flushBuffer(&g_sContext);
    swDelay(2);
    Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48, 25, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
    BuzzerOn(130);
    swDelay(2);
    Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48, 35, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
    BuzzerOn(110);
    swDelay(2);
    Graphics_drawStringCentered(&g_sContext, "START", AUTO_STRING_LENGTH, 48, 45, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

