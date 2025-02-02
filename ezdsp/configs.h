/*
 * configs.h
 *
 *  Created on: 1 de fev de 2025
 *      Author: caios
 */

#ifndef CONFIGS_H_
#define CONFIGS_H_


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tistdtypes.h"


#define sizeBuffer 4096
#define M_PI 3.14159

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


#endif /* CONFIGS_H_ */
