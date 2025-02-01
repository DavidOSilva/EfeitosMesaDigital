
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

//---------Glob constants---------
//#define N 4096
Int16 retorno = 0;
#define FIXED_SCALE 32768  // Escala de ponto fixo para Q15 (2^15)
#define PITCH_SHIFT_FACTOR 0.5  // Exemplo de fator de pitch shift
//---------Global data definition---------

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

volatile state currentEffect;  // Efeito atual (modificado dinamicamente)




DMA_Handle dmaHandleTx, dmaHandleRx;  // Handles para os canais de DMA de Tx e Rx

// Configura��o do DMA para Transmiss�o (Tx)
DMA_Config dmaTxConfig = {
    DMA_DMACSDP_RMK(
        DMA_DMACSDP_DSTBEN_NOBURST,    // Sem burst de destino
        DMA_DMACSDP_DSTPACK_OFF,       // Sem empacotamento de destino
        DMA_DMACSDP_DST_PERIPH,        // Destino � um perif�rico (MCBSP)
        DMA_DMACSDP_SRCBEN_NOBURST,    // Sem burst de origem
        DMA_DMACSDP_SRCPACK_OFF,       // Sem empacotamento de origem
        DMA_DMACSDP_SRC_DARAMPORT0,        // Origem � a mem�ria
        DMA_DMACSDP_DATATYPE_16BIT     // Tipo de dado: 16 bits
    ),
    DMA_DMACCR_RMK(
        DMA_DMACCR_DSTAMODE_CONST,   // Incremento de endere�o de destino
        DMA_DMACCR_SRCAMODE_POSTINC,     // Endere�o de origem constante
        DMA_DMACCR_ENDPROG_OFF,        // Bit de fim de programa��o
        DMA_DMACCR_WP_DEFAULT,         // Sem prote��o de escrita
        DMA_DMACCR_REPEAT_ALWAYS,     // Sem repeti��o autom�tica
        DMA_DMACCR_AUTOINIT_ON,        // Auto inicializa��o ligada
        DMA_DMACCR_EN_STOP,            // Desabilitado inicialmente
        DMA_DMACCR_PRIO_LOW,            // Alta prioridade
        DMA_DMACCR_FS_DISABLE,         // Sincroniza��o por elemento
        DMA_DMACCR_SYNC_XEVT1          // Sincronizar com o evento de transmiss�o (XEVT)
    ),
    DMA_DMACICR_RMK(
        DMA_DMACICR_AERRIE_OFF,        // Sem interrup��o por erro de acesso
        DMA_DMACICR_BLOCKIE_OFF,       // Sem interrup��o de bloco completo
        DMA_DMACICR_LASTIE_OFF,        // Sem interrup��o de �ltimo quadro
        DMA_DMACICR_FRAMEIE_OFF,       // Sem interrup��o de quadro completo
        DMA_DMACICR_FIRSTHALFIE_OFF,   // Sem interrup��o de primeira metade
        DMA_DMACICR_DROPIE_OFF,        // Sem interrup��o por evento descartado
        DMA_DMACICR_TIMEOUTIE_OFF      // Sem interrup��o por tempo esgotado
    ),
    (DMA_AdrPtr)&transmition,         // Endere�o de origem (mem�ria de �udio)
    0,                                 // Endere�o superior da origem
    (DMA_AdrPtr)(MCBSP_ADDR(DXR11)),   // Endere�o de destino (registrador DXR)
    0,                                 // Endere�o superior do destino
    sizeBuffer,                        // N�mero de elementos por quadro
    1,                                 // N�mero de quadros
    0,                                 // �ndice de quadro da origem
    0,                                 // �ndice de elemento da origem
    0,                                 // �ndice de quadro do destino
    0                                  // �ndice de elemento do destino
};

// Configura��o do DMA para Recep��o (Rx)
DMA_Config dmaRxConfig = {
    DMA_DMACSDP_RMK(
        DMA_DMACSDP_DSTBEN_NOBURST,    // Sem burst de destino
        DMA_DMACSDP_DSTPACK_OFF,       // Sem empacotamento de destino
        DMA_DMACSDP_DST_DARAMPORT0,        // Destino � a mem�ria
        DMA_DMACSDP_SRCBEN_NOBURST,    // Sem burst de origem
        DMA_DMACSDP_SRCPACK_OFF,       // Sem empacotamento de origem
        DMA_DMACSDP_SRC_PERIPH,        // Origem � um perif�rico (MCBSP)
        DMA_DMACSDP_DATATYPE_16BIT     // Tipo de dado: 16 bits
    ),
    DMA_DMACCR_RMK(
        DMA_DMACCR_DSTAMODE_POSTINC,   // Incremento de endere�o de destino
        DMA_DMACCR_SRCAMODE_CONST,     // Endere�o de origem constante
        DMA_DMACCR_ENDPROG_OFF,        // Bit de fim de programa��o
        DMA_DMACCR_WP_DEFAULT,         // Sem prote��o de escrita
        DMA_DMACCR_REPEAT_ALWAYS,     // Sem repeti��o autom�tica
        DMA_DMACCR_AUTOINIT_ON,        // Auto inicializa��o ligada
        DMA_DMACCR_EN_STOP,            // Desabilitado inicialmente
        DMA_DMACCR_PRIO_HI,            // Alta prioridade
        DMA_DMACCR_FS_DISABLE,         // Sincroniza��o por elemento
        DMA_DMACCR_SYNC_REVT1             // Sincronizar com o evento de recep��o (REVT)
    ),
    DMA_DMACICR_RMK(
        DMA_DMACICR_AERRIE_OFF,        // Sem interrup��o por erro de acesso
        DMA_DMACICR_BLOCKIE_OFF,       // Sem interrup��o de bloco completo
        DMA_DMACICR_LASTIE_OFF,        // Sem interrup��o de �ltimo quadro
        DMA_DMACICR_FRAMEIE_ON,       // Sem interrup��o de quadro completo
        DMA_DMACICR_FIRSTHALFIE_OFF,   // Sem interrup��o de primeira metade
        DMA_DMACICR_DROPIE_OFF,        // Sem interrup��o por evento descartado
        DMA_DMACICR_TIMEOUTIE_OFF      // Sem interrup��o por tempo esgotado
    ),
    (DMA_AdrPtr)(MCBSP_ADDR(DRR11)),   // Endere�o de origem (registrador DRR)
    0,                                 // Endere�o superior da origem
    (DMA_AdrPtr)&reception,    // Endere�o de destino (mem�ria de �udio)
    0,                                 // Endere�o superior do destino
    1,                                // N�mero de elementos por quadro
    sizeBuffer,                                 // N�mero de quadros
    0,                                 // �ndice de quadro da origem
    0,                                 // �ndice de elemento da origem
    0,                                 // �ndice de quadro do destino
    0                                  // �ndice de elemento do destino
};

DMA_Handle dmaReception, dmaTransmition;
volatile Uint16 transferComplete = FALSE;
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
                    bufferPitch = 4096;
                  transmition[x] =  apply_reverb(reception[x], 1024);
                  x++;
                    }else{
                        x=0;
                    }
                }else if(currentEffect == effect6 ){
                    if(x < sizeBuffer){
                        bufferPitch = 4096;
                        transmition[x] =  apply_reverb(reception[x],2048);

                         x++;
                      }else{
                          x=0;
                    }
                }else if(currentEffect == effect9 ){
                     if(x < sizeBuffer){
                         bufferPitch = 4096;
                      transmition[x] =  apply_reverb(reception[x],4096);

                     x++;
                    }else{
                      x=0;
                    }
                  }else if(currentEffect == effect24 ){
                    if(x < sizeBuffer){
                        bufferPitch = 4096;
                       transmition[x] = reception[x];

                         x++;
                      }else{
                          x=0;
                    }
                }else if(currentEffect == effect21 ){
                    if(x < sizeBuffer){


                       // d_buffer[x] =  apply_reverb(reception[x],1024);
                        //dmaTxConfig.dmacen = 2729;
                        pitchTeste(reception,x, 1.5, transmition);

                        //transmition[x] =  d_buffer[x];
                       x++;
                    }else{
                       x=0;
                    }
                }

    IRQ_enable(rcvEventId);


}

void configAudioDma (void)
{
    CSL_init();

    int i = 0;
    for (i = 0; i < sizeBuffer; i++) {
           transmition[i] =  0;
           reception[i] = 0;
           d_buffer[i] = 0;
       }

    IRQ_setVecs((Uint32)(&VECSTART));


    dmaTxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)&transmition) << 1); //origem informa��o armazenada
    dmaTxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DXR11)) << 1); // enviado para saida mcbsp ouvir(destino)


    dmaRxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DRR11)) << 1); // (origem)
    dmaRxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)&reception) << 1); // (destino)


    dmaTransmition = DMA_open(DMA_CHA2, DMA_OPEN_RESET);  // Open DMA Channel 0
    DMA_config(dmaTransmition, &dmaTxConfig);   // Configure Channel

    dmaReception = DMA_open(DMA_CHA1, DMA_OPEN_RESET);  // Open DMA Channel 1
    DMA_config(dmaReception, &dmaRxConfig);   // Configure Channel

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


void startAudioDma (int effect)
{

    if(!isActive){
        DMA_start(dmaReception); // Begin Transfer
        DMA_start(dmaTransmition); // Begin Transfer
        EZDSP5502_MCBSP_init( );
        isActive = 1;
        currentEffect = effect;

    }


}

void stopAudioDma (void)
{
    DMA_stop(dmaTransmition);  // Stop Transfer
    DMA_stop(dmaReception);  // Stop Transfer
    isActive = 0;

}
