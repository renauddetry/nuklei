#include "libklr.h"
#include <stdexcept>

void klr_train(const double *inTrain,
               const int inTrainR,
               const int inTrainC,
               const int *inLabel,
               const int inLabelN,
               double *outVexp,
               const int outVexpR,
               const int outVexpC,
               const double inDelta = -1,
               const int inItrNewton = -1)
{
  double const *Ktrain;
  double *Ktrain_temp;
  int const *label;
  int *label_temp;
  int n,n_test,m,c;
  //int wn,wm;
  //double const *delta;
  //double *itrNewton;
  //double *inweight;
  double *weight_temp;
  
  Ktrain = inTrain;
  // I didn't check which is which
  n      = inTrainR;
  m      = inTrainC;
  
  if ( n != m )
  {
    throw std::runtime_error("Error: Gram matrix should be symmetric.\n");
  }
  
  if ( n == 1 || m == 1)
  {
    throw std::runtime_error("Error: Gram matrix size should be greater than 2.\n");
  }
      
  Ktrain_temp = (double*)malloc(sizeof(double)*n*m);
  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      Ktrain_temp[(i)+(j)*n] = Ktrain[(j) + i*n];
    }
  }
  
  label = inLabel;
  n_test = inLabelN;
  
  if (n != n_test)
  {
    throw std::runtime_error("Error: Numbers of Training and label samples are not same!\n"); 
  }
  
  c = 0;
  label_temp = (int*)malloc(sizeof(int)*n);
  for(int i = 0; i < n_test; i++){
    if(c < label[i]){
      c = label[i];
    }
    label_temp[i] = label[i];
  }
  
  if (outVexpR != inLabelN || outVexpC != c)
  {
    throw std::runtime_error("Error: Output matrix has incorrect size.\n");
  }
  
  Clibklr klr(c,n);
  
  //KLR setting
  weight_temp = (double *)malloc(sizeof(double)*n);
  for(int i = 0; i < n; i++){
    weight_temp[i] = 1.0/(double)n;
  }
  
  klr.set_weight(weight_temp);
  klr.set_delta(0.01); 
  klr.set_itrCG(500);        
  klr.set_itrNewton(3);
  klr.set_itrKLR0(10);
  klr.set_tparam(0.001); 
  

  if (inDelta >= 0)
    klr.set_delta(inDelta);
  
  if (inItrNewton >= 0)
    klr.set_itrNewton(inItrNewton);
  
  //if(Noperand == 3){
  //  delta   = mxGetPr(operand[2]);
  //  klr.set_delta(delta[0]);
  //}
  
  //if(Noperand == 4){
  //  delta   = mxGetPr(operand[2]);
  //  itrNewton  = mxGetPr(operand[3]);
  //  klr.set_delta(delta[0]);
  //  klr.set_itrNewton((int)itrNewton[0]);
  //}
  
  //if(Noperand >= 5){
  //  delta   = mxGetPr(operand[2]);
  //  itrNewton  = mxGetPr(operand[3]);
  //  inweight  = mxGetPr(operand[4]);
  //  wn      = mxGetN(operand[4]);
  //  wm      = mxGetM(operand[4]);
  //  
  //  if((wn == n_test && wm == 1) || (wn == 1 && wm == n_test)){    
  //    for(int i = 0; i < n; i++){
  //      weight_temp[i] = inweight[i];
  //    }
  //    klr.set_weight(weight_temp);
  //  }else{
  //    printf("Warning: Numbers of importance weight is wrong. We set weight = ones(%d, 1).\n",n); 
  //  }
  //  
  //  klr.set_delta(delta[0]); 
  //  klr.set_itrNewton((int)itrNewton[0]);
  //}
  
  klr.train(Ktrain_temp, label_temp);
  klr.get_V(outVexp);    
  free(Ktrain_temp);
  free(label_temp);
  free(weight_temp);
}
