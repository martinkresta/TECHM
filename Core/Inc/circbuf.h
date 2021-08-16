// Universal implementation of circular buffer
// Author : Martin Kresta, inspired by (https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc) 
// Date   : 25.10.2019
// Project: Robodof/Universal

#ifndef CIRCBUF_H
#define CIRCBUF_H

#include <stdint.h>

/*struct s_CB
{
	uint8_t* buff;   // all data are stored as an array of bytes 
	uint8_t sizeOfElement;
	uint16_t length;
	uint16_t head;
	uint16_t tail;
	uint8_t full;
};*/



typedef struct s_CB s_CB;

typedef s_CB* CB_handle; 

CB_handle CB_Create(uint8_t elementsize, uint16_t length);

uint8_t CB_Destroy(CB_handle cb);

uint16_t CB_NumOfItems(CB_handle cb);


uint8_t CB_Put(CB_handle cb, uint8_t* item);

uint8_t CB_Get(CB_handle cb, uint8_t* item);

uint8_t CB_Probe(CB_handle cb, uint8_t * item);

// Removes the item from the buffer, use together with CB_Probe()
uint8_t CB_Remove(CB_handle cb);

uint8_t CB_IsEmpty(CB_handle cb);

#endif //  CIRBUF_H

