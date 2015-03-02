// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/IisObservation.h>

namespace nuklei {
    
  
  IisObservation::IisObservation()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  IisObservation::IisObservation(const kernel::base& k)
  {
    NUKLEI_TRACE_BEGIN();
    setKernel(k);
    NUKLEI_TRACE_END();
  }
  
}

