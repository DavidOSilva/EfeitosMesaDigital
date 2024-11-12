//////////////////////////////////////////////////////////////////////////////
// * File name: aic3204_loop_linein.c
// *                                                                          
// * Description:  AIC3204 Loop test.
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
#include "ezdsp5502_mcbsp.h"
#include "csl_mcbsp.h"
#define AIC3204_I2C_ADDR 0x18
#include "ezdsp5502_i2c.h"
#include "ezdsp5502_i2cgpio.h"

extern Int16 AIC3204_rset( Uint16 regnum, Uint16 regval);

/*
 *
 *  AIC3204 Loop
 *      Loops audio from LINE IN to LINE OUT
 *
*/
      // Taxa de amostragem (em Hz)
#include <stdint.h>
#include <math.h>
#define SAMPLE_RATE 48000        // Taxa de amostragem
#define MAX_DELAY 40             // Máximo atraso em milissegundos
#define LFO_RATE 0.5             // Frequência da modulação (LFO) em Hz

// Configurações do buffer de atraso e LFO
Int16 leftDelayBuffer[MAX_DELAY * (SAMPLE_RATE / 1000)];
Int16 rightDelayBuffer[MAX_DELAY * (SAMPLE_RATE / 1000)];
int delayBufferSize = MAX_DELAY * (SAMPLE_RATE / 1000);
int delayIndexLeft = 20, delayIndexRight = 20;
float lfoPhaseLeft = 10.0f, lfoPhaseRight = 10.0f;
float lfoIncrement = 2.0f * 3.14 * LFO_RATE / SAMPLE_RATE;

Int16 applyChorus(Int16 inputSample, Int16 *delayBuffer, int *delayIndex, float *lfoPhase) {
    float modulatedDelay = (10.0f + 5.0f * sin(*lfoPhase)) * (SAMPLE_RATE / 1000.0f);
    int delaySamples = (int)modulatedDelay;

    delayBuffer[*delayIndex] = inputSample;
    int readIndex = (*delayIndex - delaySamples + delayBufferSize) % delayBufferSize;

    Int16 outputSample = (inputSample + delayBuffer[readIndex]);

    *delayIndex = (*delayIndex + 1) % delayBufferSize;
    *lfoPhase += lfoIncrement;
    if (*lfoPhase > 2.0f * 3.1415) *lfoPhase -= 2.0f * 3.1415;

    return outputSample;
}


// Filtra nx amostras de entrada usando um filtro FIR com nh taps

void inputAic3204Config(){

    /* Set to McBSP1 mode */
    EZDSP5502_I2CGPIO_configLine( BSP_SEL1, OUT );
    EZDSP5502_I2CGPIO_writeLine(  BSP_SEL1, LOW );

    /* Enable McBSP1 */
    EZDSP5502_I2CGPIO_configLine( BSP_SEL1_ENn, OUT );
    EZDSP5502_I2CGPIO_writeLine(  BSP_SEL1_ENn, LOW );

    EZDSP5502_wait( 100 );

    /* ---------------------------------------------------------------- *
        *  Configure AIC3204                                               *
        * ---------------------------------------------------------------- */
       AIC3204_rset( 0, 0 );      // Select page 0
       AIC3204_rset( 1, 1 );      // Reset codec
       AIC3204_rset( 0, 1 );      // Select page 1
       AIC3204_rset( 1, 8 );      // Disable crude AVDD generation from DVDD
       AIC3204_rset( 2, 1 );      // Enable Analog Blocks, use LDO power
       AIC3204_rset( 0, 0 );

       /* PLL and Clocks config and Power Up  */
       AIC3204_rset( 27, 0x0d );  // BCLK and WCLK are set as o/p; AIC3204(Master)
       AIC3204_rset( 28, 0x00 );  // Data ofset = 0
       AIC3204_rset( 4, 3 );      // PLL setting: PLLCLK <- MCLK, CODEC_CLKIN <-PLL CLK // TESTE
           AIC3204_rset( 6, 7 );      // PLL setting: J=7
           AIC3204_rset( 7, 0x06 );   // PLL setting: HI_BYTE(D=1680)
           AIC3204_rset( 8, 0x90 );   // PLL setting: LO_BYTE(D=1680)
           AIC3204_rset( 30, 0x9C );  // For 32 bit clocks per frame in Master mode ONLY            // BCLK=DAC_CLK/N =(12288000/8) = 1.536MHz = 32*fs
           AIC3204_rset( 5, 0x91 );   //
       AIC3204_rset( 13, 0 );     // Hi_Byte(DOSR) for DOSR = 128 decimal or 0x0080 DAC oversamppling
       AIC3204_rset( 14, 0x80 );  // Lo_Byte(DOSR) for DOSR = 128 decimal or 0x0080
       AIC3204_rset( 20, 0x80 );  // AOSR for AOSR = 128 decimal or 0x0080 for decimation filters 1 to 6
       AIC3204_rset( 11, 0x82 );  // Power up NDAC and set NDAC value to 2
       AIC3204_rset( 12, 0x87 );  // Power up MDAC and set MDAC value to 7
       AIC3204_rset( 18, 0x87 );  // Power up NADC and set NADC value to 7
       AIC3204_rset( 19, 0x82 );  // Power up MADC and set MADC value to 2

       /* DAC ROUTING and Power Up */
       AIC3204_rset( 0, 1 );      // Select page 1
       AIC3204_rset( 0x0c, 8 );   // LDAC AFIR routed to HPL
       AIC3204_rset( 0x0d, 8 );   // RDAC AFIR routed to HPR
       AIC3204_rset( 0, 0 );      // Select page 0
       AIC3204_rset( 64, 2 );     // Left vol=right vol
       AIC3204_rset( 65, 0);      // Left DAC gain to 0dB VOL; Right tracks Left
       AIC3204_rset( 63, 0xd4 );  // Power up left,right data paths and set channel
       AIC3204_rset( 0, 1 );      // Select page 1
       AIC3204_rset( 9, 0x30 );   // Power up HPL,HPR
       AIC3204_rset( 0x10, 0x00 );// Unmute HPL , 0dB gain
       AIC3204_rset( 0x11, 0x00 );// Unmute HPR , 0dB gain
       AIC3204_rset( 0, 0 );      // Select page 0
       EZDSP5502_waitusec( 1000000 ); // wait

       /* ADC ROUTING and Power Up */
       AIC3204_rset( 0, 1 );      // Select page 1
       AIC3204_rset( 0x34, 0x30 );// STEREO 1 Jack
                                  // IN2_L to LADC_P through 40 kohm
       AIC3204_rset( 0x37, 0x30 );// IN2_R to RADC_P through 40 kohmm
       AIC3204_rset( 0x36, 3 );   // CM_1 (common mode) to LADC_M through 40 kohm
       AIC3204_rset( 0x39, 0xc0 );// CM_1 (common mode) to RADC_M through 40 kohm
       AIC3204_rset( 0x3b, 50 );   // MIC_PGA_L unmute
       AIC3204_rset( 0x3c, 50 );   // MIC_PGA_R unmute
       AIC3204_rset( 51, 0x48) ; // Aumente a polarização do MIC usando o LDO interno TESTEEE
       AIC3204_rset( 59, 0x3c); // Ative o MICPGA esquerdo, ganho = 30 dB TESTEEE
       AIC3204_rset( 60, 0x3c); // Ative o MICPGA direito, ganho = 30 dB TESTEEE
       AIC3204_rset( 0, 0 );      // Select page 0
       AIC3204_rset( 0x51, 0xc0 );// Powerup Left and Right ADC

       AIC3204_rset( 0x52, 0 );   // Unmute Left and Right ADC
       AIC3204_rset( 0, 0 );
       EZDSP5502_waitusec( 200 ); // Wait

}
void aic3204_loop_linein( )
{
    Int16 sec, msec;
    Int16 sample, dataLeft, dataRight;


    /* Play Loopback for 5 seconds */
    dataLeft = 0;
    dataRight = 0;
    //for ( sec = 0 ; sec < 5 ; sec++ )
   // {


        for ( msec = 0 ; msec < 1000 ; msec++ )
        {
            for ( sample = 0 ; sample < 48 ; sample++ )
            {


                       // Lê o canal esquerdo e aplica o efeito
                        EZDSP5502_MCBSP_read(&dataLeft);
                        Int16 filteredLeft = applyChorus(dataLeft, leftDelayBuffer, &delayIndexLeft, &lfoPhaseLeft);

                       EZDSP5502_MCBSP_write(filteredLeft);  // Escreve com o efeito de chorus no canal esquerdo


                       EZDSP5502_MCBSP_read(&dataRight);

                       Int16 filteredRight = applyChorus(dataRight, rightDelayBuffer, &delayIndexRight, &lfoPhaseRight);
                       EZDSP5502_MCBSP_write(filteredRight);

                }
        }
   // }
    
  //  EZDSP5502_MCBSP_close(); // Disable McBSP
   // AIC3204_rset( 1, 1 );    // Reset codec
    
}
void initAic3204(){
    /* Initialize McBSP */
    EZDSP5502_MCBSP_init( );
}
void disableAic3204(){
    EZDSP5502_MCBSP_close();// Disable McBSP
}
