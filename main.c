//////////////////////////////////////////////////////////////////////////////
// * File name: main.c
// *                                                                          
// * Description:  Main function.
// *                                                                          
// * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
// * Copyright (C) 2011 Spectrum Digital, Incorporated
// *                                                                          
// *                                                                          
// *  Redistribution and use in source and binary forms, with or without      
// *  modification, are permitted provided that the following conditions      
// *  are met:                                                                
// *                                                                          
// *    Redistributions of source code must retain the above copyright        
// *    notice, this list of conditions and the following disclaimer.         
// *                                                                          
// *    Redistributions in binary form must reproduce the above copyright     
// *    notice, this list of conditions and the following disclaimer in the   
// *    documentation and/or other materials provided with the                
// *    distribution.                                                         
// *                                                                          
// *    Neither the name of Texas Instruments Incorporated nor the names of   
// *    its contributors may be used to endorse or promote products derived   
// *    from this software without specific prior written permission.         
// *                                                                          
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     
// *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       
// *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   
// *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    
// *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   
// *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        
// *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
// *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   
// *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     
// *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   
// *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    
// *                                                                          
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5502.h"
#include "ezdsp5502_i2cgpio.h"
#include "csl_gpio.h"
//#include "aic3204.h"

extern Int16 aic3204_test( );

extern void lcdPage0( );
extern void lcdPage1( );
extern void startTimer0();
extern void initTimer0();
int  TestFail    = (int)-1;
extern Uint16 timerFlag;
extern void initPLL(void);
extern void configAic3204();
extern void aic3204_tone_headphone();
extern void aic3204_loop_linein( );
extern void disableAic3204();
extern void inputAic3204Config();
extern void initAic3204();
extern void configAudioDma();
extern void startAudioDma();
extern void stopAudioDma();
typedef enum{
    effect3 = 3,
    effect6 = 6,
    effect9 = 9,
    effect12 = 12,
    effect15 = 15,
    effect18 = 18,
    effect21 = 21,
    effect24 = 24

}state;

typedef enum{
    notSelected = 0,
    selected = 1
}stateSelection;

state currentStateEffect = effect3;
stateSelection currentStateSelection = notSelected;

void checkTimer(void);
void checkSwitch(void);
void executeStateSelection(){

    aic3204_loop_linein();
}
void transitionSelection(){
    if( currentStateSelection == selected){
        currentStateSelection = notSelected;
        disableAic3204(); //Desabilita o codec a caputrar o áudio (evita ruido)
       // stopAudioDma();
    }else{
         initAic3204(); //Habilita o codec a capturar o áudio (evita ruido)
         currentStateSelection++;
    }
    lcdPage1(currentStateSelection);

}

void executeStateEffect(){

    lcdPage0(currentStateEffect);

}

void transitionEffect(){
    if (currentStateSelection == notSelected) {
        if( currentStateEffect == effect24){
            currentStateEffect = effect3;
        }else{
            currentStateEffect = currentStateEffect + 3;
        }
        executeStateEffect();

    }
}

void checkTimer(void)
{
    if(timerFlag == 1)
    {
        checkSwitch();
        printf("timer0 \n");
    }
}

void checkSwitch(){

    if(!EZDSP5502_I2CGPIO_readLine(SW0)){
           transitionEffect();

    }else if(!EZDSP5502_I2CGPIO_readLine(SW1)){
           transitionSelection();

    }
}
void main( void )
{

    /* Initialize BSL */
    //initPLL( );
    EZDSP5502_init( );
    //initTimer0( );
    //  startTimer0();
    EZDSP5502_I2CGPIO_configLine(  SW0, IN );
    EZDSP5502_I2CGPIO_configLine(  SW1, IN );



    lcdPage0(3); //começa com efeito 3 no LCD
    lcdPage1(currentStateSelection); //começa not selected no lcd

    inputAic3204Config(); //configuração inicial do codec 3204

    configAudioDma( );  // Configure DMA for Audio tone
    startAudioDma ();

      /* Start Demo */
              // Start DMA to service McBSP


    while (1){

        checkSwitch(); //checa se algum botão foi pressionado
        //checkTimer();

        if(currentStateSelection == selected){
           // executeStateSelection(); //executa a captura do áudio com amostragem específica
        }
    }

}
