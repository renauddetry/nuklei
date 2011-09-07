#include "libklr.h"

LIBKLR_API int nlibklr=0;

LIBKLR_API int fnlibklr(void)
{
	return 42;
}

Clibklr::Clibklr(int c, int n)
{
  class_num  = c;
  sample_num = n;
  
  delta   = 0.00001; 
  itrCG   = n;       
  itrNewton  = 1;    
  itrKLR0 = 50;      
  tparam  = 0.001;   
  
  weight = (double*)malloc(sizeof(double)*sample_num);
  Vparam = (double*)malloc(sizeof(double)*c*sample_num);
  
  for(int i = 0; i < sample_num; i++){
    weight[i] = 1.0/(double)sample_num;
  }
  return;
}


double* Clibklr::test(double *Ktest, int n_test, double *V, int c, int n_train){
  
  double *prob;
  
  prob = malloc_double(c*n_test);
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n_test, n_train, 1.0, V, c,Ktest, n_train, 0.0, prob, c);
  mlogistic(prob,c,n_test);    
  
  return prob;
}

void Clibklr::train(double *Ktrain, int *label){
  
  int i,j,k;
  int c = class_num;
  int n = sample_num;
  double val,mval;
	
  double *Gamma;
  double *prob;
  double *Vtemp;
  double *Vnew;
  double *dV;
  double *L;
  double *Ltemp;
  double *Kones;
  double *mKtrain;
  double *Ytemp;
  double alpha0;
  double kmax;
  double kmax2;
  double *Y;
  
  Vtemp = malloc_double(c*n);
  Vnew  = malloc_double(c*n);
  dV    = malloc_double(c*n);
  L     = malloc_double(c*n);
  Ltemp = malloc_double(c*n);
  Gamma = malloc_double(c*c);
  Kones = malloc_double(c*c);
  prob  = malloc_double(c*n);
  mKtrain = malloc_double(n);
  Ytemp = malloc_double(c*n);
  Y     = malloc_double(c*n); 
  
  yconst(label,Y,c,n);
  
  for(i = 0; i < c*c; i++){
    Gamma[i] = 0.0;
  }
  
  /* Gamma = delta/n*Y*Y'; */
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, c, c, n, delta/(double)n, Y, c, Y, c, 0.0, Gamma, c);
  
  mval = -1.0;
  for(i = 0; i < n; i++){
    val = 0.0;
    for(j = 0; j < n; j++){
      val += abs(MAT(Ktrain,n,i,j));
    }
    if(val > mval){
      mval = val;
    }
  }
  
  alpha0 = 1/mval;
	
  /**  V and prob Initialization  **/
  for (i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(Vparam,c,i,j) = alpha0*(MAT(Y,c,i,j)*weight[j] - 1.0/(double)c);
      MAT(Vtemp,c,i,j) = 0.0;
      MAT(prob,c,i,j) = 0.0;
    }
  }
  
  /** V Estimation by Gradient Method **/
  for(k = 0; k < itrKLR0; k++){
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, Vparam, c,Ktrain, n, 0.0, prob, c);
    mlogistic(prob,c,n);
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c,Vparam, c, 0.0, Vtemp, c);
    
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(Vparam,c,i,j) = MAT(Vparam,c,i,j) - alpha0*((MAT(prob,c,i,j) - MAT(Y,c,i,j))*weight[j] + MAT(Vtemp,c,i,j));
      }
    }
  }
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, Vparam, c, Ktrain, n, 0.0, prob, c);
	
  mlogistic(prob,c,n);
  
  /** V estimation using Conjugate Gradient (CG) method **/
  for(i = 0; i < c; i++){
    for(j =0; j < c; j++){
      MAT(Kones,c,i,j) = 1.0;
    }
  }
  
  for (i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(Ltemp,c,i,j) = MAT(prob,c,i,j)*MAT(Y,c,i,j);
    }
  }
  
  for(i = 0; i < n; i++){
    mKtrain[i] = 0.0;
    for(j = 0; j < n; j++){
      mKtrain[i] += abs(MAT(Ktrain,n,i,j));
    }
  }
  
  kmax = mKtrain[0];
  for(i = 1; i < n; i++){
    if(kmax < mKtrain[i]){
      kmax = mKtrain[i];
    }
  }
  
  kmax2 = pow(kmax,2);
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Kones, c,Ltemp, c, 0.0, L, c);
  
  for (i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(L,c,i,j) = -2.0/kmax2*(MAT(Y,c,i,j) - 1.0/(double)c)/MAT(L,c,i,j);
    }
  }
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, L, c,
              Ktrain, n, 0.0, dV, c);
  
  /*Conjugate Gradient estimation;*/
  for(k = 0; k < itrNewton; k++){
    CG(prob,Ktrain,Y,Gamma,c,n,dV,itrCG);	
    
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(Vparam,c,i,j) = MAT(Vparam,c,i,j) - 1.0*MAT(dV,c,i,j);
      }
    }
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, Vparam, c, Ktrain, n, 0.0, prob, c);
    mlogistic(prob,c,n);
  }
  
  
  free(Gamma);
  free(prob);
  free(Vtemp);
  free(Vnew);
  free(dV);
  free(L);
  free(Ltemp);
  free(Kones);
  free(mKtrain);
}

void Clibklr::CG(double *prob,double *Ktrain, double *Y,double *Gamma,int c,int n,double *dV,int itrCG){
  
  int i,j,k;
  double *R;
  double *Rtemp;
  double *Q;
  double *Qtemp;
  double *W;
  double *RR;
  double *QQ;
  double *means;
  double normRR  = 0.0;
  double normRRtmp = 0.0;
  double normQQ   = 0.0;
  double alpha  = 0.0;
  double beta   = 0.0;
  double normR  = 0.0;
  double tparam_temp = 0.0;
  int  itrcount = 0;
	
  R     = malloc_double(c*n);
  Rtemp = malloc_double(c*n);
  means = malloc_double( n );
  Q     = malloc_double(c*n);
  Qtemp = malloc_double(c*n);
  W     = malloc_double(c*n);
  RR   = malloc_double(c*n);
  QQ    = malloc_double(c*n);
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c,Vparam, c, 0.0, R, c);
	
  for(i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(R,c,i,j) = weight[j]*(MAT(prob,c,i,j) - MAT(Y,c,i,j)) + MAT(R,c,i,j);
    }
  }
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, dV, c,Ktrain, n, 0.0, W, c);
  
  for(i = 0; i < n; i++){
    means[i] = 0.0;
    for(j = 0; j < c; j++){
      means[i] += MAT(prob,c,j,i)*MAT(W,c,j,i);
    }
  }
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c,dV, c, 0.0, Rtemp, c);
  
  for(i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(R,c,i,j) = MAT(R,c,i,j) - weight[j]*(MAT(W,c,i,j) - means[j])*MAT(prob,c,i,j) - MAT(Rtemp,c,i,j);
    }
  }
  
  for(i = 0; i < n; i++){
    means[i] = 0.0;
    for(j = 0; j < c; j++){
      means[i] += MAT(prob,c,j,i)*MAT(R,c,j,i);
    }
  }
  
  for(i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(W,c,i,j) = weight[j]*(MAT(R,c,i,j) - means[j])*MAT(prob,c,i,j);
    }
  }
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, W, c,Ktrain, n, 0.0, Q, c);
  
  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c,R, c, 0.0, Qtemp, c);
  
  normRR = 0.0;
  for(i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      MAT(Q,c,i,j) = MAT(Q,c,i,j) + MAT(Qtemp,c,i,j);
      normRR += pow(MAT(Q,c,i,j),2);
    }
  }
  
  normRR = sqrt(normRR);
  
	
  itrcount = 0;
  for(k = 0; k < itrCG; k++){
    normR = norm(R,c,n);
    itrcount++;
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, Q, c, Ktrain, n, 0.0, W, c);
    
    for(i = 0; i < n; i++){
      means[i] = 0.0;
      for(j = 0; j < c; j++){
        means[i] += MAT(prob,c,j,i)*MAT(W,c,j,i);
      }
    }
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c, Q, c, 0.0, Qtemp, c);
    
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(QQ,c,i,j) = weight[j]*(MAT(W,c,i,j) - means[j])*MAT(prob,c,i,j) + MAT(Qtemp,c,i,j);
      }
    }
    
    normQQ = norm(QQ,c,n);
    
    alpha = pow(normRR/normQQ,2);
    
    tparam_temp = 0.0;
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(dV,c,i,j) = MAT(dV,c,i,j) + alpha*MAT(Q,c,i,j);
        MAT(R,c,i,j)  = MAT(R,c,i,j) - alpha*MAT(QQ,c,i,j);
        tparam_temp += pow(alpha*MAT(Q,c,i,j),2);
      }
    }
    
    /* If there is no update of dV, we finish the iteration. */
    tparam_temp = sqrt(tparam_temp);
    if(tparam_temp < tparam){
      break;
    }
    
    for(i = 0; i < n; i++){
      means[i] = 0.0;
      for(j = 0; j < c; j++){
        means[i] += MAT(prob,c,j,i)*MAT(R,c,j,i);
      }
    }
    
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(W,c,i,j) = weight[j]*(MAT(R,c,i,j) - means[j])*MAT(prob,c,i,j);
      }
    }
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, n, 1.0, W, c,Ktrain, n, 0.0, RR, c);
    
    
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, c, n, c, 1.0, Gamma, c,R, c, 0.0, Qtemp, c);
    
    normRRtmp = 0.0;
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(RR,c,i,j) = MAT(RR,c,i,j) + MAT(Qtemp,c,i,j);
        normRRtmp += pow(MAT(RR,c,i,j),2);
      }
    }
    normRRtmp = sqrt(normRRtmp);
    
    beta = pow(normRRtmp/normRR,2);
    normRR = normRRtmp;
    
    for(i = 0; i < c; i++){
      for(j = 0; j < n; j++){
        MAT(Q,c,i,j)  = MAT(RR,c,i,j) + beta*MAT(Q,c,i,j);
      }
    }
  }
	
  free(R);
  free(Rtemp);
  free(Q);
  free(Qtemp);
  free(W);
  free(RR);
  free(QQ);
  free(means);
}


double Clibklr::norm(double *input,int c, int n){
  
  int i,j;
  double val = 0.0;
  
  for(i = 0; i < c; i++){
    for(j = 0; j < n; j++){
      val += pow(MAT(input,c,i,j),2);
    }
  }
  
  val = sqrt(val);
  
  return val;
}

void Clibklr::mlogistic(double *prob, int c, int n){
  
  int i,j;
  double mtemp = 0.0;
  double stemp = 0.0;
  
  for(i = 0; i < n; i++){
    mtemp = MAT(prob,c,0,i);
    for(j = 1; j < c; j++){
      if(MAT(prob,c,j,i) > mtemp){
        mtemp = MAT(prob,c,j,i);
      }
    }
    
    stemp = 0.0;
    for(j = 0; j < c; j++){
      MAT(prob,c,j,i) = exp(MAT(prob,c,j,i) - mtemp);
      stemp += MAT(prob,c,j,i);
    }
	  
    for(j = 0; j < c; j++){
      MAT(prob,c,j,i) = MAT(prob,c,j,i)/stemp;
    }
  }
}

void Clibklr::yconst(int *label, double *Y, int c, int n){
  
  int i,j;
  
  for (i = 0; i < c; i++){
    for (j = 0; j < n; j++){
      if((label[j]-1) == i){
        MAT(Y,c,i,j) = 1;
      }else{
        MAT(Y,c,i,j) = 0;
      }
    }
  }
}

int* Clibklr::malloc_int(int n)
{
  int *a;
  a = (int*)malloc(sizeof(int)*n);
  return a;
}

double* Clibklr::malloc_double(int n)
{
  double *a;
  a = (double*)malloc(sizeof(double)*n);
  return a;
}


