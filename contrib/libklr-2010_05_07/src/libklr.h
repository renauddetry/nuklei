#ifdef __GNUC__
	#define LIBKLR_API	
#else
	#ifdef LIBKLR_EXPORTS
	#define LIBKLR_API __declspec(dllexport)
	#else
	#define LIBKLR_API __declspec(dllimport)
	#endif
#endif

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef __APPLE__
#include <vecLib/vecLib.h>
#else
#include <cblas.h>
#endif

/* Matrix manipulation */
#define MAT(X, row, i, j) ((X)[(i)+(j)*row])


class LIBKLR_API Clibklr {
  int class_num;        /* Number of classes */
  int sample_num;       /* Number of tatal MFCC's */
  int itrKLR0;          /* KLR0 iteration*/
  int itrCG;            /* Conjugate Gradient iteration */
  int itrNewton;        /* Newton method iteration*/
  double tparam;        /* if the error of gradient is less than tparam, stop CG*/
  double *weight;       /* Importance Weight*/
  double delta;         /* Generalization parameter for KLR*/
  double *Vparam;       /* KLR parameter*/
public:
	Clibklr(int c, int n);

	void set_class_num(int temp){class_num = temp;}
	void set_sample_num(int temp){sample_num = temp;}
	void set_itrKLR0(int temp){itrKLR0 = temp;}
	void set_itrCG(int temp){itrCG = temp;}
	void set_itrNewton(int temp){itrNewton = temp;}
	void set_tparam(double temp){tparam = temp;}
	void set_delta(double temp){delta = temp;}
	
	int get_class_num(void){return class_num;}
	int get_sample_num(void){return sample_num;}
	int get_itrKLR0(void){return itrKLR0;}
	int get_itrCG(void){return itrCG;}
	int get_itrNewton(void){return itrNewton;}
	double get_tparam(void){return tparam;}
	double get_delta(void){return delta;}

	void set_weight(double *inweight){ 
	  for(int i = 0; i < sample_num; i++){
	    weight[i] = inweight[i];
	  }
	}

	void get_V(double* Vexp){
	  for(int i = 0; i < class_num; i++){
	    for(int j = 0; j < sample_num; j++){
	      Vexp[(i)+(j)*class_num] = Vparam[(i)+(j)*class_num];
			}
		}
	}

	/*  Train Kernel Logistic Regression */
	void train(double *Ktrain, int *label);
	/* Test KLR (vector kernel)*/
	double* test(double *Ktest, int n_test, double* V, int c, int n_train);
	/* Conjugate Gradient Method*/
	void CG(double*,double*, double*,double*,int,int,double*,int);
	/* Label matrix generation*/
	void yconst(int*, double*, int, int);
	/* Logistic Transform*/
	void mlogistic(double*, int, int);
	/* Norm computation */
	double norm(double *input, int c, int n);
	int* malloc_int(int n);
	double* malloc_double(int n);
	int display_array(double*, const int, const int);
};

extern LIBKLR_API int nlibklr;

LIBKLR_API int fnlibklr(void);
