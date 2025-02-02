
#include <stdio.h>
#include <math.h>
#include"ezdsp5502.h"
#include"ezdsp5502_mcbsp.h"
#include "csl_dma.h"
#include "csl_mcbsp.h"
#include "tistdtypes.h"
#include "configs.h"

extern Int16 apply_reverb(Int16 z, Int16 size);
extern void pitchTeste(Int16* X, Int16 p, float pitch_factor, Int16* aux);
extern void checkSwitch();
extern Int16 applyChorus(Int16 j, Int16 size);

Int16 sizeBufferTransmition = 4096;
/* Define transmit and receive buffers */
#pragma DATA_SECTION(transmition,"dmaTest")
#pragma DATA_ALIGN(transmition, 4096)
Int16 transmition[sizeBuffer];

#pragma DATA_SECTION(reception,"dmaTest")
#pragma DATA_ALIGN(reception, 4096);
Int16 reception[sizeBuffer];

#pragma DATA_SECTION(d_buffer, "scratch_buf");
#pragma DATA_ALIGN(d_buffer, 4096);
Int16  d_buffer[sizeBuffer];


Int16 isActive = 0;
Int16 firstTime = 1;
Int16 transfer = 0;

Uint16 xmtEventId, rcvEventId;

volatile Int16 bufferPitch;
extern void VECSTART(void);


Uint16 old_intm;

/* Protoype for interrupt functions */
interrupt void dmaXmtIsr(void);
interrupt void dmaRcvIsr(void);

extern volatile state currentEffect;  // Efeito atual (modificado dinamicamente)


DMA_Handle dmaHandleTx, dmaHandleRx;  // Handles para os canais de DMA de Tx e Rx

DMA_Config dmaTxConfig = {
    DMA_DMACSDP_RMK(
        DMA_DMACSDP_DSTBEN_NOBURST,    // Sem burst de destino
        DMA_DMACSDP_DSTPACK_OFF,       // Sem empacotamento de destino
        DMA_DMACSDP_DST_PERIPH,        // Destino é um periférico (MCBSP)
        DMA_DMACSDP_SRCBEN_NOBURST,    // Sem burst de origem
        DMA_DMACSDP_SRCPACK_OFF,       // Sem empacotamento de origem
        DMA_DMACSDP_SRC_DARAMPORT0,        // Origem é a memória
        DMA_DMACSDP_DATATYPE_16BIT     // Tipo de dado: 16 bits
    ),
    DMA_DMACCR_RMK(
        DMA_DMACCR_DSTAMODE_CONST,   // Incremento de endereço de destino
        DMA_DMACCR_SRCAMODE_POSTINC,     // Endereço de origem constante
        DMA_DMACCR_ENDPROG_OFF,        // Bit de fim de programação
        DMA_DMACCR_WP_DEFAULT,         // Sem proteção de escrita
        DMA_DMACCR_REPEAT_ALWAYS,     // Sem repetição automática
        DMA_DMACCR_AUTOINIT_ON,        // Auto inicialização ligada
        DMA_DMACCR_EN_STOP,            // Desabilitado inicialmente
        DMA_DMACCR_PRIO_LOW,            // Alta prioridade
        DMA_DMACCR_FS_DISABLE,         // Sincronização por elemento
        DMA_DMACCR_SYNC_XEVT1          // Sincronizar com o evento de transmissão (XEVT)
    ),
    DMA_DMACICR_RMK(
        DMA_DMACICR_AERRIE_OFF,        // Sem interrupção por erro de acesso
        DMA_DMACICR_BLOCKIE_OFF,       // Sem interrupção de bloco completo
        DMA_DMACICR_LASTIE_OFF,        // Sem interrupção de último quadro
        DMA_DMACICR_FRAMEIE_OFF,       // Sem interrupção de quadro completo
        DMA_DMACICR_FIRSTHALFIE_OFF,   // Sem interrupção de primeira metade
        DMA_DMACICR_DROPIE_OFF,        // Sem interrupção por evento descartado
        DMA_DMACICR_TIMEOUTIE_OFF      // Sem interrupção por tempo esgotado
    ),
    (DMA_AdrPtr)&transmition,         // Endereço de origem (memória de áudio)
    0,                                 // Endereço superior da origem
    (DMA_AdrPtr)(MCBSP_ADDR(DXR11)),   // Endereço de destino (registrador DXR)
    0,                                 // Endereço superior do destino
    sizeBuffer,                        // Número de elementos por quadro
    1,                                 // Número de quadros
    0,                                 // Índice de quadro da origem
    0,                                 // Índice de elemento da origem
    0,                                 // Índice de quadro do destino
    0                                  // Índice de elemento do destino
};

// Configuração do DMA para Recepção (Rx)
DMA_Config dmaRxConfig = {
    DMA_DMACSDP_RMK(
        DMA_DMACSDP_DSTBEN_NOBURST,    // Sem burst de destino
        DMA_DMACSDP_DSTPACK_OFF,       // Sem empacotamento de destino
        DMA_DMACSDP_DST_DARAMPORT0,        // Destino é a memória
        DMA_DMACSDP_SRCBEN_NOBURST,    // Sem burst de origem
        DMA_DMACSDP_SRCPACK_OFF,       // Sem empacotamento de origem
        DMA_DMACSDP_SRC_PERIPH,        // Origem é um periférico (MCBSP)
        DMA_DMACSDP_DATATYPE_16BIT     // Tipo de dado: 16 bits
    ),
    DMA_DMACCR_RMK(
        DMA_DMACCR_DSTAMODE_POSTINC,   // Incremento de endereço de destino
        DMA_DMACCR_SRCAMODE_CONST,     // Endereço de origem constante
        DMA_DMACCR_ENDPROG_OFF,        // Bit de fim de programação
        DMA_DMACCR_WP_DEFAULT,         // Sem proteção de escrita
        DMA_DMACCR_REPEAT_ALWAYS,     // Sem repetição automática
        DMA_DMACCR_AUTOINIT_ON,        // Auto inicialização ligada
        DMA_DMACCR_EN_STOP,            // Desabilitado inicialmente
        DMA_DMACCR_PRIO_HI,            // Alta prioridade
        DMA_DMACCR_FS_DISABLE,         // Sincronização por elemento
        DMA_DMACCR_SYNC_REVT1             // Sincronizar com o evento de recepção (REVT)
    ),
    DMA_DMACICR_RMK(
        DMA_DMACICR_AERRIE_OFF,        // Sem interrupção por erro de acesso
        DMA_DMACICR_BLOCKIE_OFF,       // Sem interrupção de bloco completo
        DMA_DMACICR_LASTIE_OFF,        // Sem interrupção de último quadro
        DMA_DMACICR_FRAMEIE_ON,       // Sem interrupção de quadro completo
        DMA_DMACICR_FIRSTHALFIE_OFF,   // Sem interrupção de primeira metade
        DMA_DMACICR_DROPIE_OFF,        // Sem interrupção por evento descartado
        DMA_DMACICR_TIMEOUTIE_OFF      // Sem interrupção por tempo esgotado
    ),
    (DMA_AdrPtr)(MCBSP_ADDR(DRR11)),   // Endereço de origem (registrador DRR)
    0,                                 // Endereço superior da origem
    (DMA_AdrPtr)&reception,    // Endereço de destino (memória de áudio)
    0,                                 // Endereço superior do destino
    1,                                // Número de elementos por quadro
    sizeBuffer,                                 // Número de quadros
    0,                                 // Índice de quadro da origem
    0,                                 // Índice de elemento da origem
    0,                                 // Índice de quadro do destino
    0                                  // Índice de elemento do destino
};

DMA_Handle dmaReception, dmaTransmition;
volatile Int32 counter1 = 0;
volatile Int32 counter2 = 0;
volatile int i=0;
volatile Int16 x = 0;

interrupt void dmaXmtIsr(void) {
    IRQ_disable(xmtEventId);
    //d_buffer[x] = reception[x];
    IRQ_enable(xmtEventId);

}
#define PI 3.1415926

Int16 clamp(Int16 value) {
    if (value > 32767) return 32767;
    if (value < -32768) return -32768;
    return (Int16)value;
}


interrupt void dmaRcvIsr(void) {

    IRQ_disable(rcvEventId);

            if(currentEffect == effect3 ) {
                if(x < sizeBuffer){
                  transmition[x] =  apply_reverb(reception[x], 500);
                  x++;
                    }else{
                        x=0;
                    }
                }else if(currentEffect == effect6 ){
                    if(x < sizeBuffer){
                        transmition[x] =  apply_reverb(reception[x],1024);

                         x++;
                      }else{
                          x=0;
                    }
                }else if(currentEffect == effect9 ){
                     if(x < sizeBuffer){
                      transmition[x] =  apply_reverb(reception[x],2048);

                     x++;
                    }else{
                      x=0;
                    }
                  }else if(currentEffect == effect12 ){
                      if(x < sizeBuffer){
                       transmition[x] =  apply_reverb(reception[x],3500);

                      x++;
                     }else{
                       x=0;
                     }
                   }else if(currentEffect == effect15 ){
                       if(x < sizeBuffer){
                        transmition[x] =  apply_reverb(reception[x],4096);

                       x++;
                      }else{
                        x=0;
                      }
                    }else if(currentEffect == effect18 ){
                        counter1++;
                   if(x < sizeBuffer){

                       pitchTeste(reception,x, 2.0, d_buffer);
                       transmition[x] =  apply_reverb(d_buffer[x], 2048);

                       if(counter1 == 70000){

                           checkSwitch();
                           counter1 = 0;
                       }


                       x++;
                    }else{
                       x=0;
                     }
                }else if(currentEffect == effect21 ){
                    counter1++;
                    if(x < sizeBuffer){

                        pitchTeste(reception,x, 1.5, d_buffer);
                        transmition[x] =  apply_reverb2(d_buffer[x],4000);

                        if(counter1 == 70000){
                            checkSwitch();
                            counter1=0;
                        }
                        x++;
                    }else{
                       x=0;

                    }
                }else if(currentEffect == effect24 ){
                    if(x < sizeBuffer){

                       transmition[x] = reception[x] * 0.7;

                         x++;
                      }else{
                          x=0;
                    }
                }

    IRQ_enable(rcvEventId);


}

void configInterrupts(){

        IRQ_setVecs((Uint32)(&VECSTART));


    /* Get interrupt event associated with DMA receive and transmit */
      xmtEventId = DMA_getEventId(dmaTransmition);
      rcvEventId = DMA_getEventId(dmaReception);

      /* Temporarily disable interrupts and clear any pending */
      /* interrupts for MCBSP transmit */
      old_intm = IRQ_globalDisable();

      /* Clear any pending interrupts for DMA channels */
      IRQ_clear(xmtEventId);
      IRQ_clear(rcvEventId);

      /* Enable DMA interrupt in IER register */
      IRQ_enable(xmtEventId);
      IRQ_enable(rcvEventId);

      /* Place DMA interrupt service addresses at associate vector */
      IRQ_plug(xmtEventId,&dmaXmtIsr);
      IRQ_plug(rcvEventId,&dmaRcvIsr);


      IRQ_globalEnable();

}


void configAudioDma (Uint16 size)
{
    CSL_init();

    int i = 0;
    for (i = 0; i < sizeBuffer; i++) {
           transmition[i] =  0;
           reception[i] = 0;
           d_buffer[i] = 0;
       }



    dmaTxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)&transmition) << 1); //origem informação armazenada
    dmaTxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DXR11)) << 1); // enviado para saida mcbsp ouvir(destino)


    dmaRxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DRR11)) << 1); // (origem)
    dmaRxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)&reception) << 1); // (destino)

    dmaTxConfig.dmacen = size;

    dmaTransmition = DMA_open(DMA_CHA2, 0);  // Open DMA Channel 0
    DMA_config(dmaTransmition, &dmaTxConfig);   // Configure Channel

    dmaReception = DMA_open(DMA_CHA1, 0);  // Open DMA Channel 1
    DMA_config(dmaReception, &dmaRxConfig);   // Configure Channel


        configInterrupts();
        //firstTime = 0;

}



void startAudioDma (Int16 effect)
{
    currentEffect = effect;
    if(!isActive){
        if (effect == effect3 || effect == effect6 || effect == effect9 || effect == effect12 || effect == effect15 || effect == effect24) {

            configAudioDma(4096);

        }else if(effect == effect18){

            configAudioDma(2047);
        }
        else if(effect == effect21){

            configAudioDma(2729);
         }

        x = 0;
        initReverb();
        DMA_start(dmaReception); // Begin Transfer
        DMA_start(dmaTransmition); // Begin Transfer
        EZDSP5502_MCBSP_init( );
        isActive = 1;

    }


}

void stopAudioDma (void)
{

    DMA_stop(dmaTransmition);
    DMA_stop(dmaReception);
    DMA_pause(dmaTransmition);
    DMA_pause(dmaReception);
    DMA_close(dmaTransmition);
    DMA_close(dmaReception);


    isActive = 0;

}
