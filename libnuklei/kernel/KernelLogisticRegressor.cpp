// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/KernelLogisticRegressor.h>
#include <nuklei/ObservationIO.h>
#include <klr_train.h>

namespace nuklei
{
  
  
  KernelLogisticRegressor::KernelLogisticRegressor
  (const KernelCollection &data,
   const std::vector<int>& labels) :
  trainSet_(data), labels_(labels)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(trainSet_.size() == labels_.size());
    computeGramMatrix();
    NUKLEI_TRACE_END();
  }
  
  KernelLogisticRegressor::KernelLogisticRegressor
  (const KernelCollection &data,
   const GMatrix& gramMatrix,
   const std::vector<int>& labels) :
  trainSet_(data), gramMatrix_(gramMatrix), labels_(labels)
  {
    NUKLEI_ASSERT(trainSet_.size() == labels_.size());
    NUKLEI_ASSERT(gramMatrix_.GetRows() == trainSet_.size());
    NUKLEI_ASSERT(gramMatrix_.GetColumns() == trainSet_.size());
  }
  
  
  void KernelLogisticRegressor::setData(const KernelCollection &data,
                                         const std::vector<int>& labels)
  {
    trainSet_ = data;
    labels_ = labels;
    computeGramMatrix();
    vklr_ = boost::optional<GMatrix>();
  }
  
  void KernelLogisticRegressor::computeGramMatrix()
  {
    NUKLEI_TRACE_BEGIN();
    gramMatrix_ = GMatrix(trainSet_.size(), trainSet_.size());

    for (unsigned i = 0; i < trainSet_.size(); ++i)
    {
      for (unsigned j = 0; j < trainSet_.size(); ++j)
      {
        gramMatrix_(i, j) = trainSet_.at(i).polyEval(trainSet_.at(j));
      }
    }
    NUKLEI_TRACE_END();
  }
  
  void KernelLogisticRegressor::train(const double delta,
                                       const unsigned itrNewton)
  {
    NUKLEI_TRACE_BEGIN();
    
    NUKLEI_ASSERT(trainSet_.size() == labels_.size());
    NUKLEI_ASSERT(trainSet_.size() != 0);
    
    GMatrix vklr(trainSet_.size(), 2);
    
    klr_train(gramMatrix_, gramMatrix_.GetRows(), gramMatrix_.GetColumns(),
              &labels_.front(), labels_.size(),
              vklr, vklr.GetRows(), vklr.GetColumns(),
              delta, itrNewton);
    
    vklr_ = vklr.Transpose();
    NUKLEI_TRACE_END();
  }
  
    
  Vector2 KernelLogisticRegressor::test(const kernel::base &t) const
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(vklr_);
    Vector2 pr;
    
    GMatrix Ktest(trainSet_.size(), 1);
    for (unsigned i = 0; i < trainSet_.size(); ++i)
    {
      Ktest(i, 0) = trainSet_.at(i).polyEval(t);
    }
    
    GMatrix f = *vklr_ * Ktest;
    double m = std::max(f(0,0), f(1,0));
    
    double e1 = std::exp(f(0,0)-m), e2 = std::exp(f(1,0)-m);
    pr.X() = e1/(e1+e2);
    pr.Y() = e2/(e1+e2);
    
    return pr;
    NUKLEI_TRACE_END();
  }
  
  
  GMatrix KernelLogisticRegressor::test(const KernelCollection &testSet) const
  {
    NUKLEI_TRACE_BEGIN();
    
    GMatrix pr(2, testSet.size());
    
    for (KernelCollection::const_iterator ti = testSet.begin();
         ti != testSet.end(); ++ti)
    {
      Vector2 p = test(*ti);
      pr(0,std::distance(testSet.begin(), ti)) = p.X();
      pr(1,std::distance(testSet.begin(), ti)) = p.Y();
    }
    return pr;
    NUKLEI_TRACE_END();
  }
  
  
}