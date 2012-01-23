#ifndef _JPEG_ACTORS_H_INCLUDED
#define _JPEG_ACTORS_H_INCLUDED

// #include "./structures.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

/* block of pixel-space values */
typedef union {
	unsigned char block[8][8];
	unsigned char linear[64];
} PBlock;

/* block of frequency-space values */
typedef union {
	int block[8][8];
	int linear[64];
} FBlock;

/* block of frequency-space values before IQZZ */
typedef struct {
	unsigned long linear[64];
	PBlock qtable;
} FValue;

/* pixel value array */
typedef struct {
	unsigned char data[3072];
} ColorBuffer;

/* component descriptor */
typedef struct {
	unsigned char CID, IDX;
	unsigned char HS, VS;
	unsigned char HDIV, VDIV;

	char QT, DC_HT, AC_HT;
	int PRED;
} cd_t;

/* settings for color conversion */
typedef struct {
	int MCU_sx, MCU_sy;
	int n_comp;
	cd_t comp[3];
} SubHeader1;

/* settings for rasterization */
typedef struct {
	int MCU_sx;
	int n_comp;
	int x_size, y_size;
	int goodrows, goodcolumns, offset;
	int leftover;
	int MCU_column;
} SubHeader2;


	void init_header_vld(FValue * mcu_after_vld, SubHeader1 * SH1, SubHeader2 * SH2);
	void header_vld(FValue * mcu_after_vld, SubHeader1 * SH1, SubHeader2 * SH2);
	void init_header(SubHeader1 * SH1, SubHeader2 * SH2);
	void init_vld(FValue * mcu_after_vld);
	void iqzz(const FValue * V, FBlock * B);
	void idct(const FBlock * input, PBlock * output);
	void cc(const SubHeader1 * SH1, const PBlock * PB, ColorBuffer * CB);
	void raster(const SubHeader2 * SH2, const ColorBuffer * CB);
// #ifdef __cplusplus
// }
// #endif


#endif							/* _JPEG_ACTORS_H_INCLUDED */