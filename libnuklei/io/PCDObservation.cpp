// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

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

