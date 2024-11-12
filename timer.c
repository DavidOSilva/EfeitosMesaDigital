//////////////////////////////////////////////////////////////////////////////
// * File name: timer.c
// *
// * Description:  Timer functions.
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
#include "ezdsp5502.h"
#include "csl_gpt.h"
#include "csl_irq.h"
#include "timer.h"

/*  Reference the start of the interrupt vector table
 *  This symbol is defined in file vectors.asm   */
//extern Uint32 vecs;

Uint8 timerState = 0;  // Indicates Timer's period state
Uint16 timerFlag = 0;  // Flag indicating a timer interrupt
Uint16 gpt12Evt_Id;

GPT_Handle myhGpt;

/* Timer configuration structure */
GPT_Config myGptCfg =
{
    0,
    GPT_GPTGPINT_RMK(
        GPT_GPTGPINT_TIN1INV_DEFAULT,
        GPT_GPTGPINT_TIN1INT_DEFAULT
    ),
    GPT_GPTGPEN_RMK(
        GPT_GPTGPEN_TOUT1EN_DEFAULT,
        GPT_GPTGPEN_TIN1EN_DEFAULT
    ),
    GPT_GPTGPDIR_RMK(
        GPT_GPTGPDIR_TOUT1DIR_DEFAULT,
        GPT_GPTGPDIR_TIN1DIR_DEFAULT
    ),
    GPT_GPTGPDAT_RMK(
        GPT_GPTGPDAT_TOUT1DAT_DEFAULT,
        GPT_GPTGPDAT_TIN1DAT_DEFAULT
    ),
    0x68C0, //PRD1
    0x0478, //PRD2
    0xFFFF, //PRD3
    0xFFFF, //PRD4
    GPT_GPTCTL1_RMK(
        GPT_GPTCTL1_TIEN_DEFAULT,
        GPT_GPTCTL1_CLKSRC_DEFAULT,
        GPT_GPTCTL1_ENAMODE_CONTINUOUS,
        GPT_GPTCTL1_PWID_DEFAULT,
        GPT_GPTCTL1_CP_DEFAULT,
        GPT_GPTCTL1_INVIN_DEFAULT,
        GPT_GPTCTL1_INVOUT_DEFAULT
    ),
    GPT_GPTCTL2_RMK(
        GPT_GPTCTL2_TIEN_DEFAULT,
        GPT_GPTCTL2_CLKSRC_DEFAULT,
        GPT_GPTCTL2_ENAMODE_ONCE,
        GPT_GPTCTL2_PWID_DEFAULT,
        GPT_GPTCTL2_CP_DEFAULT,
        GPT_GPTCTL2_INVIN_DEFAULT,
        GPT_GPTCTL2_INVOUT_DEFAULT
    ),
    GPT_GPTGCTL1_RMK(
        GPT_GPTGCTL1_TDDR34_DEFAULT,
        GPT_GPTGCTL1_PSC34_DEFAULT,
        GPT_GPTGCTL1_TIMMODE_32BIT_DUAL,
        GPT_GPTGCTL1_TIM34RS_IN_RESET,
        GPT_GPTGCTL1_TIM12RS_IN_RESET
    )
};

/*
 *  initTimer0( )
 *
 *    Configure Timer and setup timer interrupt
 */
void initTimer0(void)
{
    IRQ_setVecs(0x7F00);    // Point to interrupt vector table
    IRQ_globalDisable();           //Disable interrupts globally

    /* Open GPT0 */
    myhGpt = GPT_open(GPT_DEV0, GPT_OPEN_RESET);

    gpt12Evt_Id = GPT_getEventId(myhGpt);  // Get GPTx EventId
    IRQ_clear(gpt12Evt_Id);                // Clear interrupt
    IRQ_plug(gpt12Evt_Id, &gpt0Isr);       // Add ISR to vector table
    IRQ_enable(gpt12Evt_Id);               // Enable timer interrupt
    IRQ_globalEnable();                    // Enable interrupts globally
    GPT_config(myhGpt, &myGptCfg);         // Configure timer with 1 sec period
}

/*
 *  startTimer0( )
 *
 *    Start Timer0
 */
void startTimer0(void)
{
    GPT_start12(myhGpt);
}

/*
 *  gpt0Isr( )
 *
 *    Timer0 Interrupt Service Routine
 */
interrupt void gpt0Isr(void)
{
    timerFlag=1;
}

/*
 *  changeTimer( )
 *
 *    Swap between a 1 sec period and a 250ms period
 */
void changeTimer(void)
{
    if(timerState == 0)
    {
        GPT_stop12(myhGpt);                 // Stop Timer0
        GPT_RSETH(myhGpt,GPTPRD1, 0x1A30);  //
        GPT_RSETH(myhGpt,GPTPRD2, 0x0019);  // Reset period to 25ms
        GPT_RSETH(myhGpt,GPTCNT1, 0x0000);  //
        GPT_RSETH(myhGpt,GPTCNT2, 0x0000);  // Reset counter to 0
        GPT_start12(myhGpt);                // Start Timer
        timerState = 1;                     // Change state
    }
    else
    {
        GPT_stop12(myhGpt);                 // Stop Timer0
        GPT_RSETH(myhGpt,GPTPRD1, 0x68C0);  //
        GPT_RSETH(myhGpt,GPTPRD2, 0x0019);  // Reset period to 250ms
        GPT_RSETH(myhGpt,GPTCNT1, 0x0000);  //
        GPT_RSETH(myhGpt,GPTCNT2, 0x0000);  // Reset counter to 0
        GPT_start12(myhGpt);               // Start Timer
        timerState = 0;                     // Change state
    }
}
