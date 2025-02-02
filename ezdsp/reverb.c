#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tistdtypes.h"
#include "configs.h"

Int16 delayIndex = 0;
volatile int writeIndex = 0;
#define DECAY_FACTOR 0.5f
#define maxBufferSizeReverb 4096
#define  DELAY_SAMPLES 0
#pragma DATA_SECTION(delayBuffer,"data_br_buf")
#pragma DATA_ALIGN(delayBuffer, 2048);

#define PI 3.14159265358979323846
Int16 delayBuffer[maxBufferSizeReverb];

void initReverb(){
    int i;

    for(i=0; i<maxBufferSizeReverb;i++){
        delayBuffer[i] = 0;
    }
}
Int16 readIndex = 0;

Int16 apply_reverb(Int16 input,Int16 bufferSizeReverb) {


    readIndex = (writeIndex - DELAY_SAMPLES  + bufferSizeReverb) % bufferSizeReverb;

    Int16 delayedSample = delayBuffer[readIndex];


    Int16 reverbSample = input + (Int16)(delayedSample * DECAY_FACTOR)
                                  + (Int16)(delayBuffer[(readIndex + 103) % bufferSizeReverb] * (DECAY_FACTOR * 0.5))
                                  + (Int16)(delayBuffer[(readIndex + 233) % bufferSizeReverb] * (DECAY_FACTOR * 0.25));

    delayBuffer[writeIndex] = (Int16)(0.7 * delayBuffer[writeIndex]  + (1 - 0.7) * reverbSample );

    writeIndex = (writeIndex + 1) % bufferSizeReverb;

    return reverbSample;
}

Int16 apply_reverb2(Int16 input,Int16 bufferSizeReverb) {


    readIndex = (writeIndex - DELAY_SAMPLES  + bufferSizeReverb) % bufferSizeReverb;

    Int16 delayedSample = delayBuffer[readIndex];

    Int16 reverbSample = input + (Int16)(delayedSample * DECAY_FACTOR);

    delayBuffer[writeIndex] = reverbSample;

    writeIndex = (writeIndex + 1) % bufferSizeReverb;

    return reverbSample;
}





