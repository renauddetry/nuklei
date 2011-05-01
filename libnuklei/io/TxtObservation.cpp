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

