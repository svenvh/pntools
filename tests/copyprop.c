// Test case demonstrating copy propagation combined with a function call.
#define M 8

void source(int *x, int *y);
void negate(int s, int *d);
void trans(int s1, int s2, int *d);
void sink(int v);

int main() {
  int a[M],b[M];
  int v[M],w[M];
  int x[M],y[M];
  int z[M];
  int i;

  for (i = 0; i < M; i++) {
    source( &(v[i]), &(w[i]) );
    source( &(x[i]), &(y[i]) );
  }

  for (i = 0; i < M; i++) {
    if (i < M/2) {
      a[i] = v[i];
      b[i] = x[i];
    }
    else {
      a[i] = w[i];
      negate( y[i], &b[i] );
    }
    // a = v[i] or w[i]     propagation works fine
    // b = x[i] or -y[i]    propagation breaks, caused by the negate function call
    trans( a[i], b[i], &(z[i]) );
  }

  for (i = 0; i < M; i++) {
    sink( z[i] );
  }

  return 0;
}
