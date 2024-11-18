//////////////////////////////////////////////////////////////////////////////
// * File name: dma.c
// *                                                                          
// * Description:  DMA functions.
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
#include <stdio.h>
#include"ezdsp5502.h"
#include"ezdsp5502_mcbsp.h"
#include "csl_dma.h"
#include "csl_mcbsp.h"

Uint8 dmaState = 0;

//---------Global constants---------
#define N 6

//---------Global data definition---------

/* Define transmit and receive buffers */
#pragma DATA_SECTION(transmition,"dmaTest")
Int16 transmition[N];
#pragma DATA_SECTION(reception,"dmaTest")
Int16 reception[N];

Uint16 xmtEventId, rcvEventId;
extern void VECSTART(void);
Uint16 old_intm;

/* Protoype for interrupt functions */
interrupt void dmaXmtIsr(void);
interrupt void dmaRcvIsr(void);


DMA_Handle dmaHandleTx, dmaHandleRx;  // Handles para os canais de DMA de Tx e Rx

// Configuração do DMA para Transmissão (Tx)
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
    N,                                 // Número de elementos por quadro
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
        DMA_DMACCR_PRIO_LOW,            // Alta prioridade
        DMA_DMACCR_FS_DISABLE,         // Sincronização por elemento
        DMA_DMACCR_SYNC_REVT1          // Sincronizar com o evento de recepção (REVT)
    ),
    DMA_DMACICR_RMK(
        DMA_DMACICR_AERRIE_OFF,        // Sem interrupção por erro de acesso
        DMA_DMACICR_BLOCKIE_OFF,       // Sem interrupção de bloco completo
        DMA_DMACICR_LASTIE_ON,        // Sem interrupção de último quadro
        DMA_DMACICR_FRAMEIE_OFF,       // Sem interrupção de quadro completo
        DMA_DMACICR_FIRSTHALFIE_OFF,   // Sem interrupção de primeira metade
        DMA_DMACICR_DROPIE_OFF,        // Sem interrupção por evento descartado
        DMA_DMACICR_TIMEOUTIE_OFF      // Sem interrupção por tempo esgotado
    ),
    (DMA_AdrPtr)(MCBSP_ADDR(DRR11)),   // Endereço de origem (registrador DRR)
    0,                                 // Endereço superior da origem
    (DMA_AdrPtr)&reception,    // Endereço de destino (memória de áudio)
    0,                                 // Endereço superior do destino
    N,                                // Número de elementos por quadro
    1,                                 // Número de quadros
    0,                                 // Índice de quadro da origem
    0,                                 // Índice de elemento da origem
    0,                                 // Índice de quadro do destino
    0                                  // Índice de elemento do destino
};

DMA_Handle dmaReception, dmaTransmition;
volatile Uint16 transferComplete = FALSE;

#define DELAY 512                 // Tamanho do buffer de atraso (em amostras)
Int16 delayBuffer[DELAY] = {0};   // Buffer circular para o atraso
Int16 delayIndex = 0;

// Função para aplicar o efeito de reverberação
void aplicarReverb(Int16 *buffer, int tamanho) {
    Int16 i;
    for (i = 0; i < tamanho; i++) {
        // Amostra atrasada obtida do buffer circular
        Int16 delayedSample = delayBuffer[delayIndex];

        // Combina a amostra atual com a amostra atrasada
        buffer[i] += delayedSample / 1.5;  // Divisão por 2 para criar o efeito de reverb

        // Atualiza o buffer de atraso com a amostra atual
        delayBuffer[delayIndex] = buffer[i];

        // Atualiza o índice circular
        delayIndex = (delayIndex + 1) % DELAY;
    }
}

void aplicarVozFina(Int16 *buffer, int tamanho, float fatorAceleracao) {
    Int16 i;
    for (i = 0; i < tamanho; i++) {
        // Calcula o novo índice baseado no fator de aceleração
        int novoIndice = (int)(i * fatorAceleracao);

        // Se o novo índice ultrapassar o tamanho do buffer, então ignora
        if (novoIndice < tamanho) {
            buffer[i] = buffer[novoIndice];
        } else {
            buffer[i] = 0; // Preenche com 0 (silêncio) para evitar ler fora do buffer
        }
    }
}

interrupt void dmaXmtIsr(void) {
    //DMA_stop(dmaTransmition);      // Para a operação de transmissão do DMA.
    //IRQ_disable(xmtEventId);       // Desabilita o evento de interrupção da transmissão.
    //IRQ_enable(xmtEventId);
    //DMA_start(dmaTransmition);
    //printf("transmissao \n");
}

interrupt void dmaRcvIsr(void) {
     //DMA_stop(dmaReception);        // Para a operação de recepção do DMA.
    IRQ_disable(rcvEventId);       // Desabilita o evento de interrupção da recepção.

    /*
    aplicarVozFina(reception, N, 1.2);
    */

    Int16 i;
    for (i = 0; i < N; i++) { // Copia os dados processados para o buffer de transmissão
        transmition[i] = reception[i];
    }

    /*
    //DMA_start(dmaTransmition);
    */
    transferComplete = TRUE;
    DMA_start(dmaReception);
    IRQ_enable(rcvEventId);
}

/* Define a DMA_Handle object to be used with DMA_open function */
/*
 *  configAudioDma( )
 *
 *    Configure DMA for Audio
 */
void configAudioDma (void)
{
    CSL_init();

    int i = 0;
    for (i = 0; i <= N - 1; i++) {
           transmition[i] =  0;
           reception[i] = 0;

       }

    IRQ_setVecs((Uint32)(&VECSTART));


    /* Set source address to Sine_1K */
    dmaTxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)&transmition) << 1); //origem informação armazenada
    dmaTxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DXR11)) << 1); // enviado para saida mcbsp ouvir(destino)


    dmaRxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)MCBSP_ADDR(DRR11)) << 1); // (origem)
    dmaRxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)&reception) << 1); // (destino)
    //dmaRxConfig.dmacssal = (DMA_AdrPtr)(((Uint32)&reception) << 1); // informação armazenada (origem)
    //dmaRxConfig.dmacdsal = (DMA_AdrPtr)(((Uint32)&transmition) << 1); //recebe a informação (destino)

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

/*
 *  startAudioDma( )
 *
 *    Start DMA transfer for Audio
 */

Int16 testWhile = 0;
void startAudioDma (void)
{
    DMA_start(dmaReception); // Begin Transfer
    DMA_start(dmaTransmition); // Begin Transfer

    EZDSP5502_MCBSP_init( );

    /* Wait for DMA transfer to be complete */


    while (!transferComplete){
        testWhile = 2;
    }
}

/*
 *  stopAudioDma( )
 *
 *    Stop DMA transfer for Audio
 */
void stopAudioDma (void)
{
    DMA_stop(dmaTransmition);  // Stop Transfer
    DMA_stop(dmaReception);  // Stop Transfer

}

/*
 *  changeTone( )
 *
 *    Swap between 1KHz and 2KHz audio tones
 */
