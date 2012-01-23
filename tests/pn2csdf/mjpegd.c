// #include <stdlib.h>
#include "mjpegd_func.h"

int main(const int argc, const char **argv)
{
	int i,j;
	SubHeader1 sh1;
	SubHeader2 sh2;
	FValue MCU[10];
	PBlock PB[10];
	FBlock FB;
	ColorBuffer CB;

// 	  init_header_vld(NULL, NULL, NULL);

	while (1) {
	    // process input image with size 32 x 24 (12 MCUs)
	    for (i = 0; i < 12; i++) {
				    
    // 		header_vld(&MCU, &sh1, &sh2);
		init_header(&sh1, &sh2);

    // 		for (i = 0; i < sh1.n_comp; i++) {
		// images with 3 color components
		for (j = 0; j < 3; j++) {
		    init_vld(&MCU[i]);
		    iqzz(&MCU[i], &FB);
		    idct(&FB, &PB[i]);
		    cc(&sh1, &PB[i], &CB);
		    raster(&sh2, &CB);
		}
	    }
	}
	
	return 0;
}
