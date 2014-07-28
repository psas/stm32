#include "lwip/def.h"
#include "utils_general.h"

static void swap(int length, const void * in, void * out){
	switch(length){
	 case 1:
		 *(uint8_t *)out = *(uint8_t *)in;
		 break;
	 case 2:
		 *(uint16_t *) out = htons(*(const uint16_t *) in);
		 break;
	 case 4:
		 *(uint32_t *) out = htonl(*(const uint32_t *) in);
		 break;
	 }
}


int len_swapped(const struct swap *swaps){
	int len = 0;
	while(swaps->length){
		len += swaps->length;
		++swaps;
	}
	return len;
}

void write_swapped(const struct swap *swaps, const void *data, uint8_t *buffer){
	while(swaps->length){
		const char *current = (const char *) data + swaps->offset;
		swap(swaps->length, current, buffer);
		buffer += swaps->length;
		++swaps;
	}
}

void read_swapped(const struct swap *swaps, void *data, const uint8_t *buffer){
	while(swaps->length){
		char *current = (char *) data + swaps->offset;
		swap(swaps->length, buffer, current);
		buffer += swaps->length;
		++swaps;
	}
}


