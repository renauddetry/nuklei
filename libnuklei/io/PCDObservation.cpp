// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/PCDObservation.h>

namespace nuklei {
    
  PCDObservation::PCDObservation()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  PCDObservation::PCDObservation(const kernel::base& k) : SerializedKernelObservation(k)
  {}

}

