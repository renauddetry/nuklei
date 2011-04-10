// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/NukleiObservation.h>

namespace nuklei {
    
  
  NukleiObservation::NukleiObservation()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  NukleiObservation::NukleiObservation(const kernel::base& k)
  {
    NUKLEI_TRACE_BEGIN();
    setKernel(k);
    NUKLEI_TRACE_END();
  }
  
}

