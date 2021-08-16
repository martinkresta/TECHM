// Universal implementation of circular buffer
// Author : Martin Kresta, inspired by (https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc) 
// Date   : 25.10.2019
// Project: Robodof/Universal

#include "circbuf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
//#include "canopen.h"



struct s_CB
{
	uint8_t* buff;   // all data are stored as an array of bytes 
	uint8_t sizeOfElement;
	uint16_t length;
	uint16_t head;
	uint16_t tail;
	uint8_t full;
	uint8_t lock;  // prevents corruption of data by preempting get/put commands 
};


 // s_CanTxMsg Msg;

	

CB_handle CB_Create(uint8_t elementsize, uint16_t length)
{
	CB_handle cb = malloc(sizeof(s_CB)); 
	cb->buff = malloc (length * elementsize);  // TBD : check pointer
	cb->length = length;
	cb->head = 0;
	cb->tail = 0;
	cb->full = 0;
	cb->sizeOfElement = elementsize;
	
	
	return cb;
}


uint8_t CB_Destroy(CB_handle cb)
{
	if (cb != NULL)
	{
		if(!cb->lock)
		{
			free(cb->buff);
			free(cb);
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
}



uint16_t CB_NumOfItems(CB_handle cb)
{
	uint16_t noi = 0;
	
	if (cb != NULL)
	{
		noi = cb->length;
		if (!cb->full)
		{
			if(cb->head >=cb->tail)
			{
				noi = cb->head - cb->tail;
			}
			else
			{
				noi = cb->length + cb->head - cb->tail;
			}
		}
		
	}
	else
	{
		// TBD: assert
	}
	return noi;
	
}

uint8_t CB_IsEmpty(CB_handle cb)
{
	if (cb!= NULL)
	{
		return (!cb->full && (cb->head == cb->tail));
	}
	return 1; 
}


// Gets the item from the buffer, but do not remove it from the buffer
// To remove the intem, call CB_Probe() + CB_Remove(), or use CB_Get() to get and remove in one step 

uint8_t CB_Probe(CB_handle cb, uint8_t * item)
{
	if (cb!= NULL)
	{

		
		__disable_irq();
			if(!CB_IsEmpty(cb))
			{
				
				memcpy(item, &cb->buff[cb->tail * cb->sizeOfElement], cb->sizeOfElement);
				//cb->tail = (cb->tail + 1) % cb->length;
				//cb->full = 0;
				__enable_irq();
				return 0;

			}
			else  // empty buffer
			{
				__enable_irq();
				return 1;
			}
		}

	// TBD assert
	return 1;

}


// Removes the item from the buffer, use together with CB_Probe()
uint8_t CB_Remove(CB_handle cb)
{
	if (cb!= NULL)
	{
		
		__disable_irq();
			if(!CB_IsEmpty(cb))
			{
				cb->tail = (cb->tail + 1) % cb->length;
				cb->full = 0;
				__enable_irq();
				return 0;

			}
			else  // empty buffer
			{
				__enable_irq();
				return 1;
			}
		}

	// TBD assert
	return 1;

}


uint8_t CB_Get(CB_handle cb, uint8_t * item)
{
	if (cb!= NULL)
	{
		
		__disable_irq();
			if(!CB_IsEmpty(cb))
			{
				
				memcpy(item, &cb->buff[cb->tail * cb->sizeOfElement], cb->sizeOfElement);
				cb->tail = (cb->tail + 1) % cb->length;
				cb->full = 0;
				__enable_irq();
				return 0;

			}
			else  // empty buffer
			{
				__enable_irq();
				return 1;
			}
		}

	// TBD assert
	return 1;

}


uint8_t CB_Put(CB_handle cb, uint8_t* item)
{
	
	if (cb != NULL)
	{	
		memcpy(&cb->buff[cb->head * cb->sizeOfElement],item, cb->sizeOfElement);
		if(cb->full)
		{
			cb->tail = (cb->tail + 1) % cb->length;		
		}
		cb->head = (cb->head + 1) % cb->length;
		cb->full = (cb->head == cb->tail? 1 : 0);

		return 0;
	}
	else
	{
		return 1;
	}
}
