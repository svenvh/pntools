// 2 arguments with non-uniform dep. along diff. directions

#define M 8

void producer(int* in, int* in1);

void consumer(const int* out, const int* out1);



int main(){
  int t,i,j;
  int x[100*100], y[100*100];
    
  for (i = 1 ; i<=M; i++){
    producer(&(x[i]), &(y[i]));
  }
  
  
  for (i = 1 ; i<=M/2; i++){
    consumer(&(x[2*i-1]), &(y[i]));
  }
    
  return 0;
}