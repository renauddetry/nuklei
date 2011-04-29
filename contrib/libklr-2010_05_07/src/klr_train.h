#ifndef LIBKLR_KLR_TRAIN_H
#define LIBKLR_KLR_TRAIN_H

void klr_train(const double *inTrain,
               const int inTrainR,
               const int inTrainC,
               const int *inLabel,
               const int inLabelN,
               double *outVexp,
               const int outVexpR,
               const int outVexpC,
               const double inDelta = -1,
               const int inItrNewton = -1);

#endif
