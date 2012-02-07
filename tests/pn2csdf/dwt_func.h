#ifndef DWT_FUNC_H_
#define DWT_FUNC_H_

void Init(int *outPxl);
void my_high_flt_vert( const int *inPxl1, const int *inPxl2, const int *inPxl3, int *outHf);
void my_low_flt_vert( const int *inPxl1, const int *inPxl2, const int *inPxl3, int *oldHf, int *Lf);
void my_high_flt_hor( const int *inPxl1, const int *inPxl2, const int *inPxl3, int *outH );
void my_low_flt_hor( const int *inPxl1, const int *inPxl2, const int *inPxl3, int *outL );
void Sink(const int *inPxl);

#endif /*DWT_FUNC_H_*/

