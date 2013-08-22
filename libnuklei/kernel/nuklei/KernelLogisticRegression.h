// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */


#ifndef NUKLEI_KERNEL_LOGISTIC_REGRESSION_H
#define NUKLEI_KERNEL_LOGISTIC_REGRESSION_H

#include <nuklei/KernelLogisticRegressor.h>

#warning The KernelLogisticRegression class is obsolete. \
 Use KernelLogisticRegressor instead.

namespace nuklei
{
  /**
   * @brief Obsolete. See KernelLogisticRegressor instead.
   */
  struct KernelLogisticRegression : public KernelLogisticRegressor
  {
    KernelLogisticRegression() : KernelLogisticRegressor() {}

    KernelLogisticRegression(const KernelCollection &data,
                             const std::vector<int>& labels) :
    KernelLogisticRegressor(data, labels) {};

    KernelLogisticRegression(const KernelCollection &data,
                             const GMatrix& gramMatrix,
                             const std::vector<int>& labels) :
    KernelLogisticRegressor(data, gramMatrix, labels) {};
  };
}

#endif
