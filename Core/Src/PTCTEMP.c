// Module: Temperature measurement by analog PTC sensor connected via amplification network

/* Dependencies */
#include "PTCTEMP.h"
#include "main.h"
#include "VARS.h"
#include "ADC.h"
#include "APP.h"

#include <math.h>



/* Private variables */

int16_t mPtcTemp;



/* Public variables */

/* Private methods */

/* Public methods */


void PTCTEMP_Convert(void)
{
	uint16_t PtcRaw = ADC_GetValue(ADC_CHANNEL_PTC);  // raw ADC result
	double Ptc_mV = ADC_VREF_MV/4096.0 * PtcRaw;  // convert to milivolts

	Ptc_mV += 60;  // compensation of offset error ?   TODO checked in full scale!

	double V2 = (double)ADC_VREF_MV * R2/(R1+R2);   // Opamp inputs in miliVolts,
	double Vptc = (V2*(R3+R4) - Ptc_mV*R3)/R4;
	double Rpt = (Vptc*R5)/(ADC_VREF_MV - Vptc);   // resistance of PT1000 in ohms
	double temp = (Rpt - 1000) * 2.61;		// 10ths of degree C
	mPtcTemp = (int16_t)temp;

	if(PtcRaw != 0 && PtcRaw != 4096)
	{
		VAR_SetVariable(VAR_TEMP_BOILER_EXHAUST, mPtcTemp, 1);
	}
	else
	{
		VAR_SetVariable(VAR_TEMP_BOILER_EXHAUST, mPtcTemp, 0);
		// TBD LOG ERR
	}

}



