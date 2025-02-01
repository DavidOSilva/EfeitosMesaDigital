
#include <stdio.h>
#include "ezdsp5502.h"
#include "ezdsp5502_i2cgpio.h"
#include "csl_gpio.h"
#include <math.h>
#include "configs.h"

extern void lcdPage0( );
extern void lcdPage1( );
extern void disableAic3204();
extern void inputAic3204Config();
extern void initAic3204();
extern void configAudioDma();
extern void startAudioDma();
extern void stopAudioDma();
extern void initPLL(void);

void checkSwitch(void);

state currentStateEffect = effect3;
stateSelection currentStateSelection = notSelected;

void executeStateSelection(){

    startAudioDma(currentStateEffect);
}
void transitionSelection(){
    if( currentStateSelection == selected){
        currentStateSelection = notSelected;
        disableAic3204(); //Desabilita o codec a caputrar o áudio (evita ruido)
        stopAudioDma();
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

void checkSwitch(){

    if(!EZDSP5502_I2CGPIO_readLine(SW0)){
           transitionEffect();

    }else if(!EZDSP5502_I2CGPIO_readLine(SW1)){
           transitionSelection();

    }
}

void main( void )
{

    EZDSP5502_init( );

    EZDSP5502_I2CGPIO_configLine(  SW0, IN );
    EZDSP5502_I2CGPIO_configLine(  SW1, IN );

    initPLL();         // Initialize PLL

    lcdPage0(3); //começa com efeito 3 no LCD
    lcdPage1(currentStateSelection); //começa not selected no lcd

    inputAic3204Config(48000); //configuração inicial do codec 3204
    configAudioDma();

    while (1){

        checkSwitch(); //checa se algum botão foi pressionado

        if(currentStateSelection == selected){
           executeStateSelection(); //executa a captura do áudio com amostragem específica

        }

    }

}


