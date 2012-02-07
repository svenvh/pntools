#include "dwt_func.h"

//int Nrw=100; // image Height/2
//#pragma parameter Nrw 40 256
#define Nrw 20
//int Ncl=50; // image Width/2
//#pragma parameter Ncl 40 256
#define Ncl 20

int main( void ) {

     int i,j;

     static int image[500][500];

     static int LL[500][500];
     static int HL[500][500];
     static int LH[500][500];
     static int HH[500][500];

     static int Hf[1000];
     static int Lf[1000];
     static int oldHf[1000];

     int tmpLine;
     int tmp;
//----------------------------------------------------------------------------------
	 for( i=0; i<2*Nrw; i++)
	   for( j=0; j<2*Ncl; j++)
		   Init( &image[i][j] );
//----------------------------------------------------------------------------------
	 for( i=0; i<Nrw; i++ ) {

	      // DWT by columns at pixel level (3 elements per column with subsampling)
		  for( j=0; j<2*Ncl; j++ ) { // Ncol

			   if( i==Nrw-1 ) {
			 	 tmpLine = ( image[2*i][j]);
			  } else {
				 tmpLine = ( image[2*i+2][j]);
			  }

			  // compute high frequency coeff.
			  my_high_flt_vert( &image[2*i][j], &image[2*i+1][j], &tmpLine,  &Hf[j]); // High Pass Filter

			  if (i == 0)
				tmp = Hf[j];
			  else
				tmp = oldHf[j];

			  // compute low frequency coeff.
			  my_low_flt_vert( &tmp, &image[2*i][j], &Hf[j], &oldHf[j], &Lf[j] ); // Low Pass Filter
		   }

           // DWT by rows at pixel level (Low Pass Filter with subsampling) ---------------------------------
		   for( j=0; j<Ncl; j++ ) {

			  if( j==Ncl-1 ) { 
				 tmp = ( Lf[2*j] );
			  } else {
				 tmp = ( Lf[2*j+2] );
			  }

			  my_high_flt_hor( &Lf[2*j], &Lf[2*j+1], &tmp, &HL[i][j]); // High Pass Filter

			  if( j==0 ) { 
				 tmp = ( HL[i][j] ); 
			  } else {
				 tmp = ( HL[i][j-1] ); 
			  }

			  my_low_flt_hor( &tmp, &Lf[2*j], &HL[i][j], &LL[i][j]); // Low Pass Filter
		   }
 
           // DWT rows row at pixel level (High Pass Filter with subsampling) ---------------------------------
		   for( j=0; j<Ncl; j++ ) {

				 if( j==Ncl-1 ) {
				   tmp = ( Hf[2*j] );
				 } else {
				   tmp = ( Hf[2*j+2] );
				 }

				 my_high_flt_hor( &Hf[2*j], &Hf[2*j+1], &tmp, &HH[i][j]); // High Pass Filter

				 if( j==0 ) {
				   tmp = ( HH[i][j] ); 
				 } else {
				   tmp = ( HH[i][j-1] ); 
				 }

				 my_low_flt_hor( &tmp, &Hf[2*j], &HH[i][j], &LH[i][j]); // Low Pass Filter
		   } 
		  } // for( i=0; i<Nrw; i++ )
//-----------------------------------------------------------------------------------------
	      // The Sink
		  for (i = 0; i < Nrw; i++) {
			for (j = 0; j < Ncl; j++) {

				Sink( &LL[i][j] );
				Sink( &HL[i][j] );
				Sink( &LH[i][j] );
				Sink( &HH[i][j] );
		  }}
//----------------------------------------------------------------------------------------------
     return(0);
} // main
