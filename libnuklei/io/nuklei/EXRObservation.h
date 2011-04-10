// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifdef NUKLEI_USE_EXR_LIB

#ifndef NUKLEI_EXROBSERVATION_H
#define NUKLEI_EXROBSERVATION_H


#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include <nuklei/Definitions.h>
#include <nuklei/Color.h>
#include <nuklei/LinearAlgebra.h>
#include <nuklei/Observation.h>

namespace nuklei {

  /**
   * EXR depth maps reader.
   * @author Damien Teney <damien.teney@ulg.ac.be>
   */
  class EXRObservation : public Observation
    {
    public:
  
      Type type() const { return EXR; }

      static const double TOL;
 
      std::auto_ptr<kernel::base> getKernel() const
      {
        return k_.clone();
      }
 
      void setKernel(const kernel::base& k)
      {
        NUKLEI_TRACE_BEGIN();
        k_ = dynamic_cast<const kernel::r3&>(k);
        NUKLEI_TRACE_END();
      }

      EXRObservation();
      EXRObservation(const kernel::r3& k);
      ~EXRObservation() {};
    
      void setLoc(Location loc);
      Location getLoc() const;
  
      void setWeight(weight_t weight);
      weight_t getWeight() const;
  
      void setColor(const Color& color);
      const Color& getColor() const;
    
    private:
      kernel::r3 k_;
    };

}

#endif

#endif

