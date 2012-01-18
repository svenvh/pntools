#define M 10

void src(int* in);
void F1(const int* in);
void F2(const int* in);

int main(){
    int t,i,j,k;
    int x[M][M], y[M][M];
    int x3[M][M][M];
    
    
    for (i = 1 ; i<=M; i++){
	for (j = 1; j<=M; j++){
	    for (k = 1; k<=M; k++){
		src(&(x3[i][j][k]));
	    }
	}
    }
    
    
    for (i = 1 ; i<=M; i++){
	for (j = 1; j<=M; j++){
	    for (k = 1; k<=M; k++){
		if (k % 2 == 0){
		    F(&(x3[i][j][k]));
		} else {
		    F13(&(x3[i][j][k]));
		}
	    }
	}
    }
    

       
    
    return 0;
}