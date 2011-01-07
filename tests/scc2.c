// SCC Test
// Sven van Haastregt
#define N 10

void source(int *d);
void trans1(int s, int *d);
void trans2(int s, int *d);
void trans3(int s, int b, int *bb, int *d);
void trans4(int s, int *d);
void trans5(int s, int *d);
void trans6(int s, int *d);
void sink(int s);

int main() {
  int a[1];
  int b;
  int d;
  int i,j,k;

  source(&d);

  for (j = 0; j < N; j++) {
    for (i = 0; i < N; i++) {
      trans1( d, &d );
      trans2( d, &d );
      trans3( b, d, &d, &b );
    }
    for (i = 0; i < N; i++) {
      trans4( b, &b );
      trans5( b, &b );
    }
  }

  return 0;
}
