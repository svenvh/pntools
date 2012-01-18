#define M 10

void src(int* in);
void F1(const int* in, int* out);
void F2(const int* in, int* out);
void sink(const int* out);

int main(){
    int t,i,j,k;
    int x[M][M], y[M][M];
    int x3[M][M][M];
    
    
    for (i = 1 ; i<=M; i++){
	for (j = 1; j<=M; j++){
	    for (k = 1; k<=3; k++){
		src(&(x3[i][j][k]));
	    }
	}
    }
    
    for (i = 1 ; i<=M; i++){
	for (j = 1; j<=M; j++){
	    for (k = 1; k<=3; k++){
		if (k <= 2){
		    F1(&(x3[i][j][k]), &(x3[i][j][k]));
		} else {
		    F2(&(x3[i][j][k]), &(x3[i][j][k]));
		}
	    }
	}
    }
    
    for (i = 1 ; i<=M; i++){
	for (j = 1; j<=M; j++){
	    for (k = 1; k<=3; k++){
		sink(&(x3[i][j][k]));
	    }
	}
    }
       
    
    return 0;
}