// Module: Temperature measurement module
// Target: STM32F4xx family, utilizing STM CUBE platform
// Author: Martin Kresta
// Date: 05.07.2016

// Brief: The module implements 1-Wire interface to read
// temperatures from multiple sensors DS18B20

/* Dependencies */
#include "main.h"

/* Macros */


#define NUM_OF_ALL_SENSORS    32
#define NUM_OF_ASSIGNED_SENSORS	 13



// temp sensors aliases (see TempSensIDs.xlsx)
#define      T1            0
#define      T2            1
#define      T3            2
#define      T4            3
#define      T5            4
#define      T6            5
#define      T7            6
#define      T8            7
#define      T9            8
#define      T10           9
#define      T101          10
#define      T102          11
#define      T103          12
#define      T104          13
#define      T105          14
#define      T106          15
#define      T107          16
#define      T108          17
#define      T109          18
#define      T110          19
#define      T301          20
#define      T302          21
#define      T303          22
#define      T304          23
#define      T305          24
#define      T306          25
#define      T307          26
#define      T308          27
#define      T309          28
#define      T310          29
#define      T_IOBOARD     30
#define      T_TECHM       31


// PTC Gain network

#define				R5			1000
#define 			R1			20000
#define 			R2			27250
#define				R3			10000
#define				R4			47000
#define				V0			3313   // mV

/* Private variables */

/* Public variables */

/* Private methods */

/* Public methods */

void TEMP_Init(void);
void TEMP_Update100ms(void);


