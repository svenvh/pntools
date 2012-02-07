/** Sobel filter ( 256 x 256 ) for the PNgen compiler
 *
 */

#define N 25
#define M 25

void gradient( const int *a1, const int *a2, const int *a3, const int *a4, const int *a5, const int *a6, int *output );

void absVal( const int *x, const int *y, int *output );

void readPixel( int *output );

void  writePixel( const int *pp );

int main(void)
{
   int i, j;

   static int image[1000][1000];
   static int Jx[1000][1000];
   static int Jy[1000][1000];
   static int av[1000][1000];
   

   for (j=1; j <= M; j++) {
      for (i=1; i <= N; i++) {
         readPixel(&image[j][i]);
      }
   }

   for (j=2; j <= M-1; j++) {
      for (i=2; i <= N-1; i++) {
           gradient( &image[j-1][i-1], &image[j][i-1], &image[j+1][i-1], &image[j-1][i+1], &image[j][i+1], &image[j+1][i+1], &Jx[j][i] );
      }
   }

   for (j=2; j <= M-1; j++) {
      for (i=2; i <= N-1; i++) {
          gradient( &image[j-1][i-1], &image[j-1][i], &image[j-1][i+1], &image[j+1][i-1], &image[j+1][i], &image[j+1][i+1], &Jy[j][i] );
      }
   }

   for (j=2; j <= M-1; j++) {
      for (i=2; i <= N-1; i++) {
           absVal( &Jx[j][i], &Jy[j][i], &av[j][i] );
      }
   }

   for (j=2; j <= M-1; j++) {
      for (i=2; i <= N-1; i++) {
          writePixel( &av[j][i] );
      }
   }

   return (0);
}

