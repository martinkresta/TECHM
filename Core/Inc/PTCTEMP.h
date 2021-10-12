/*
 * PTCTEMP.h
 *  Module: Temperature measurement by analog PTC sensor connected via amplification network
 *  Created on: 12. 10. 2021
 *      Author: Martin
 */

#ifndef INC_PTCTEMP_H_
#define INC_PTCTEMP_H_

#include "main.h"

// PTC Gain network

#define				R5			1000
#define 			R1			20000
#define 			R2			27000
#define				R3			10000
#define				R4			47000


void PTCTEMP_Convert(void);

#endif /* INC_PTCTEMP_H_ */
