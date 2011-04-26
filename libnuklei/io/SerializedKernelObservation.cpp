// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

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

