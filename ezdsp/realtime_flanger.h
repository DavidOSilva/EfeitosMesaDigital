/*
* realtime_flanger.h
*
*  Created on: Oct 28, 2012
*      Author: BLEE
*
*  Description: This C header file for flanger experiment
*
*  For the book "Real Time Digital Signal Processing:
*                Fundamentals, Implementation and Application, 3rd Ed"
*                By Sen M. Kuo, Bob H. Lee, and Wenshun Tian
*                Publisher: John Wiley and Sons, Ltd
*
*/

#ifndef __FLANGER_H__
#define __FLANGER_H__
 
#define LFO_F 0.5


typedef struct {
	float A_maxSwing;
	float G_depth;
	float var_delay;
	Int16 delay;
	Int32 n;
} flangerRT;

Int16 Flanger_process(Int16 xinp, flangerRT *t);
void  Flanger_sweep(flangerRT *t);

#endif
