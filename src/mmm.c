/*****************************************************************
 *
 * The following program computes a matrix multiply operation:
 *   C = X*Y,
 ******************************************************************/



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>



#include <math.h>


#define page 64

//#define COLD_CACHE
// ============ VCA: code for measuring performance counters ==================//

//These are compilation flags
//#define PCM
//#define COLD_CACHE


void _ini1(double * m, size_t row, size_t col)
{
  size_t i;
  for (i = 0; i < row*col; ++i)  m[i] = (double)1.1;
}

void _iniMatrix(double * m, size_t row, size_t col, double * copy)
{
  size_t i;
  for (i = 0; i < row*col; ++i)  m[i] = copy[i];
}



#ifdef PCM
#define REP 20
#include <stdbool.h>
#include "measuring_core.h"


#endif

// ============ End VCA: code for measuring performance counters ==================//






static __attribute__((noinline)) void
mmm (int n, double *A, double *B, double *C) 
{
  int i;
  int j;
  int k;
  double c;
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
	{
	  c = C[i * n + j];
	  for (k = 0; k < n; k++)
	    {
//	      C[i * n + j] += A[i * n + k] * B[k * n + j];
		c += A[i * n + k] * B[k * n + j];	  
  }
	  C[i * n + j] = c;
	}
    }

}






int main(int argc, char *argv[])
{
   int n;
   	int i, j;
 static double *tmp, *A, *B, *C;


  //  Make sure a matrix size is specified
   if (argv[1] == NULL)
   {
      printf("USAGE: %s [side of matrix] [optional|size of block]\n", argv[0]);
      exit(1);
   }

   if ((n = atoi(argv[1])) < 0){                                                       
      exit(1);                                               
}



// Initialize a tmp matrix vector to force the matrices being
// in consecutive memory regions and facilitate the 
// data-memory mapping. In theory, malloc should extend
// the heap, so every data/structure allocated with
// malloc should be in consecutive memory positions,
// but this does not happen from the first element
// allocated with malloc to the second element :S   
 
//  	tmp = (double *) malloc (3* n * n * sizeof (double));

	/*tmp = malloc_aligned (128, 3* n * n * sizeof (double));
  	A = tmp;
	B=tmp+(n*n);
	C=B+(n*n);

	printf("%p\n",tmp);
  printf("%p\n",A);
  printf("%p\n",B);
  printf("%p\n",C);
*/
  A = (double *) _mm_malloc (n * n * sizeof (double), page);
  B = (double *) _mm_malloc (n * n * sizeof (double), page);
  C = (double *) _mm_malloc (n * n * sizeof (double), page);
  

  
	for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
	{
	  A[i * n + j] = 1;
	  B[i * n + j] = 1;
	  C[i * n + j] = 0;
	}

    }


 
#ifdef PCM
  int r;
  int dataSetSize =3*(n*n);
  printf("Measuring core\n");

long events[] = {	/* double Scalar */ 0x10, 0x80, /* double packed */ 0x10, 0x10, /*double AVX*/ 0x11, 0x02};

long events_stalls[] = {	/* RESOURCES_STALLS.LB */ 0xA2, 0x02, /* RESOURCES_STALLS.RS */ 0xA2, 0x04, 
/*RESOURCES_STALLS.SB*/ 0xA2, 0x08,/*RESOURCES_STALLS.SB*/ 0xA2, 0x10};

long events_hits[] = {/* L1_HIT */ 0xD1, 0x01, /* L2_HIT */ 0xD1, 0x02, /* LLC_HIT*/ 0xD1, 0x04};


  measurement_init(events, 0, 0);
  
  unsigned long runs = 1; //start of with a single run for sample
  unsigned long multiplier;
  do{
    
    measurement_start();
    for(i = 0; i <= runs; i++)
    {
       mmm(n,A,B,C);
    }
    measurement_stop(runs);
    
    multiplier = measurement_run_multiplier(1000000000);
    runs = runs * multiplier;
    
  }while (multiplier > 2);
  
  
  measurement_emptyLists(true); //don't clear the vector of runs
  
  long size_per_run = dataSetSize;
  size_per_run = (dataSetSize) * sizeof(double);
  
  if(runs * size_per_run < (100 * 1024 * 1024))
    runs = ceil((100 * 1024 * 1024)/size_per_run);
  
  printf("nruns %lu\n", runs);
  

#ifdef COLD_CACHE
  
  long numberofshifts = (100 * 1024 * 1024 / (dataSetSize* sizeof(double)));
  
  if (numberofshifts < 2) numberofshifts = 2;


//__asm cpuid;
  
  double ** A_array = (double **) CreateBuffers((n*n)* sizeof(double),numberofshifts);
  double ** B_array = (double **) CreateBuffers((n*n)* sizeof(double),numberofshifts);
  double ** C_array = (double **) CreateBuffers((n*n)* sizeof(double),numberofshifts);

  for(i = 0; i < numberofshifts; i++){
    _ini1(A_array[i],n , n);
    _ini1(B_array[i],n , n);
    _ini1(C_array[i],n , n);	
  }
  for(r = 0; r < REP; r++){
    measurement_start();
    for(i = 0; i < runs; i++){
	mmm(n,A_array[i%numberofshifts],B_array[i%numberofshifts], C_array[i%numberofshifts]);
    }
    measurement_stop(runs);
  }

//__asm cpuid;

  measurement_end();
  
  _mm_free(A);
  _mm_free(B);
  _mm_free(C);
  DestroyBuffers( (void **) A_array, numberofshifts);
  DestroyBuffers( (void **) B_array, numberofshifts);
   DestroyBuffers( (void **) C_array, numberofshifts);

#else
  for (r = 0; r < REP; r++) {
    measurement_start();
    for(i = 0; i < runs; i++){
     mmm(n,A,B,C);
      
    }
    measurement_stop(runs);
  }
  measurement_end();
  
  _mm_free(A);
  _mm_free(B);
  _mm_free(C);
#endif
#else
#ifndef COLD_CACHE
for(i=0; i<2 ; i++)
#endif
  mmm(n,A,B,C);
 

  printf("C[0] %f\n", C[0]);
 
   _mm_free(A);
   _mm_free(B); 
   _mm_free(C);
#endif



  return 0;
}
