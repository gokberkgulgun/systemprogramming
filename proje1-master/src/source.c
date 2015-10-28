#include<stdio.h>
#include<stdlib.h>

void print_menu();
int make_choice(char, int*, int*,int*, int);
void print_square_matrix(int* in_matrix, int size);

/*---------------Assembly Functions---------------------- */
int sum(int* in_matrix, int in_size);
int _add(int* matrix1, int* matrix2,int* result, int size);
void scale(int* in_matrix,int value,int* out_result,int in_size);
void mult(int* matrix1, int* matrix2,int* result, int size);
void square(int* matrix1,int* result,int size);
void itu(int* matrix1,int size);



int main(){

	FILE* input_A = fopen("src/matrixA.txt", "r");
	FILE* input_B = fopen("src/matrixB.txt", "r");

	if (input_A == NULL || input_B == NULL) {
		printf("Error! Files cannot be read.\n");
		return 1;
	}

	int sizeA, sizeB;
	fscanf(input_A, "%d",&sizeA);
	fscanf(input_B, "%d",&sizeB);
	
	if( sizeA != sizeB ){
		printf("Error! Sizes don't match.\n");
		return 1;
	}
	
	int *matrixA, *matrixB, *result;
	matrixA = (int*)malloc(sizeA * sizeA * sizeof(int));
	matrixB = (int*)malloc(sizeB * sizeB * sizeof(int));
	result =  (int*)malloc(sizeB * sizeB * sizeof(int));
	

	int i, j;
	for(i = 0; i < sizeA; i++ ){
		for(j = 0; j < sizeA; j++){
			fscanf(input_A, "%d", matrixA + i * sizeA + j);
			fscanf(input_B, "%d", matrixB + i * sizeA + j);
		}
		fgetc(input_A); //reading newline character for first file
		fgetc(input_B); //reading newline character for second file
		
	}
	
	char choice;
	do{
		print_menu();
		scanf("%c", &choice);
		getc(stdin); //reading newline character
	}while (make_choice(choice, matrixA, matrixB, result, sizeA));
	
	free(matrixA);
	free(matrixB);
	free(result);
	fclose(input_A);
	fclose(input_B);
	return 0;
}

void print_menu(){
	printf("Matrix Application\n"       );
	printf("------------------------\n" );
	printf("A: Add Matrices\n"          );
	printf("S: Sum of Matrix\n"         );
	printf("M: Multiply Matrices\n"     );
	printf("C: Scale Matrix\n"          );
	printf("Q: Square of Matrix\n"      );
	printf("B: ITU\n"                   );
	printf("E: Exit Program\n"          );
	printf("Enter letter to perform:\n" );

}

int make_choice(char choice, int* matrixA, int* matrixB, int* result , int size){
	int scale_factor = 0;
	switch (choice)
	{
	case 'A': case 'a':
		_add(matrixA,matrixB,result,size);
		print_square_matrix(result, size);
		break;
	case 'S': case 's':
		printf("Result of Matrix1's Sum = %d", sum(matrixA, size));
		break;
	case 'M': case 'm':
		mult(matrixA,matrixB,result,size);
		print_square_matrix(result,size);
		break;
	case 'C': case 'c':
	    printf("Enter scale factor: "); 
	    scanf("%d", &scale_factor); getc(stdin);
	    printf("\n");
		scale(matrixA, scale_factor, result, size);
		print_square_matrix(result, size);
		break;
	case 'Q': case 'q':
		square(matrixA,result,size);
		print_square_matrix(result,size);
		
		break;
	case 'B': case 'b':
		if(size<11 || size > 80){
		printf("Size is not between 11 and 80");
	}
		else{
		itu(matrixA,size);
		print_square_matrix(matrixA,size);
	}
		break;
	case 'E': case 'e':
		return 0;
	default:
		break;
	}
	printf("\n");
	return 1;
}

void print_square_matrix(int* in_matrix, int size){
	int i,j;
	for(i = 0; i < size; i++){
		for(j = 0; j < size; j++)
			printf("%6d ", in_matrix[i*size + j]);
		printf("\n");
	}
}
