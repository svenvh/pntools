// 2 SCCs
// Sven van Haastregt
#define N 10

void source(int *d);
void trans1(int s, int *d);
void trans2(int s, int *d);
void trans3(int s, int *d);
void sink(int s);

int main() {
  int a[1];
  int b[1];
  int d;
  int i,j,k;

  source(&d);

  for (i = 0; i < N; i++) {
    trans1( d, &d );
    trans2( d, &d );
  }

  for (i = 0; i < N; i++) {
    trans1( d, &d );
    trans2( d, &d );
    trans3( d, &d );
  }

  sink(d);

  return 0;
}
