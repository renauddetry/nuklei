// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include "nuklei/TxtObservation.h"


namespace nuklei {
  
  const double TxtObservation::TOL = 1e-5;

  
  TxtObservation::TxtObservation()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  TxtObservation::TxtObservation(const kernel::base& k) : k_(k)
  {}

}

