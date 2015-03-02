// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/SerializedKernelObservation.h>


namespace nuklei {
  
  const double SerializedKernelObservation::TOL = 1e-5;

  
  SerializedKernelObservation::SerializedKernelObservation()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  SerializedKernelObservation::SerializedKernelObservation(const kernel::base& k) : k_(k)
  {}

}

