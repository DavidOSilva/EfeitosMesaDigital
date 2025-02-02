#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tistdtypes.h"
#include "configs.h"



void pitchTeste(Int16* X, Int16 p, float pitch_factor, Int16* aux) {

    float new_index = p * pitch_factor;
    int i = (int)new_index;
    float frac = new_index - i;
    if(i < sizeBuffer){
           aux[p] = X[i] + ((X[i + 1] - X[i]) * frac) * (0.5f + 0.5f * frac);

       }
}
