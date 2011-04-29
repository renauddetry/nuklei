// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */


#ifndef NUKLEI_KERNEL_LOGISTIC_REGRESSION_H
#define NUKLEI_KERNEL_LOGISTIC_REGRESSION_H


#include <nuklei/KernelCollection.h>

namespace nuklei
{
  
  struct KernelLogisticRegression
  {
    KernelLogisticRegression() {}
    KernelLogisticRegression(const KernelCollection &data,
                             const std::vector<int>& labels);    
    KernelLogisticRegression(const KernelCollection &data,
                             const GMatrix& gramMatrix,
                             const std::vector<int>& labels);    
    void setData(const KernelCollection &data,
                 const std::vector<int>& labels);
    
    void train(const double delta = 0.0001, const unsigned itrNewton = 5);
    
    bool isTrained()
    {
      return vklr_;
    };
    
    Vector2 test(const kernel::base &t) const;
    GMatrix test(const KernelCollection &testSet) const;
        
    const GMatrix& vklr() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_ASSERT(vklr_);
      return *vklr_;
      NUKLEI_TRACE_END();
    }
  private:
    void computeGramMatrix();
    
    KernelCollection trainSet_;
    GMatrix gramMatrix_;
    std::vector<int> labels_;
    boost::optional<GMatrix> vklr_;
  };
}


#endif
