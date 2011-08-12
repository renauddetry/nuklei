// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_IIS_OBSERVATION_H
#define NUKLEI_IIS_OBSERVATION_H


#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include <nuklei/Definitions.h>
#include <nuklei/Color.h>
#include <nuklei/LinearAlgebra.h>
#include <nuklei/SerializedKernelObservation.h>

namespace nuklei {

  class IisObservation : public SerializedKernelObservation
  {
  public:
    
    Type type() const { return IIS; }
    
    IisObservation();
    IisObservation(const kernel::base& k);
    ~IisObservation() {};
  };
  
  
}

#endif
