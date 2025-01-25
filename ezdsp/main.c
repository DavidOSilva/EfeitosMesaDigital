
#include <stdio.h>
#include "ezdsp5502.h"
#include "ezdsp5502_i2cgpio.h"
#include "csl_gpio.h"


extern void lcdPage0( );
extern void lcdPage1( );

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

void checkSwitch(void);
void executeStateSelection(){

    startAudioDma();
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


    lcdPage0(3); //começa com efeito 3 no LCD
    lcdPage1(currentStateSelection); //começa not selected no lcd

    inputAic3204Config(); //configuração inicial do codec 3204

    configAudioDma();  // Configure DMA for Audio tone



    while (1){

        checkSwitch(); //checa se algum botão foi pressionado

        if(currentStateSelection == selected){
           executeStateSelection(); //executa a captura do áudio com amostragem específica
        }

    }

}
