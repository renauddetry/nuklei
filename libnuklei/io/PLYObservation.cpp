// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/PLYObservation.h>

namespace nuklei {
  
  const double PLYObservation::TOL = 1e-5;

  
  PLYObservation::PLYObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorDescriptor d;
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  PLYObservation::PLYObservation(const kernel::r3& k) : k_(k)
  {}
    
  void PLYObservation::setLoc(Location loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Location PLYObservation::getLoc() const { return k_.loc_; }
  
  void PLYObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.w_ = weight;
    NUKLEI_TRACE_END();
  }
  weight_t PLYObservation::getWeight() const { return k_.w_; }
    
  const Color& PLYObservation::getColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorDescriptor&>(k_.getDescriptor()).getColor();
    NUKLEI_TRACE_END();
  }
  void PLYObservation::setColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorDescriptor&>(k_.getDescriptor()).setColor(color);
    NUKLEI_TRACE_END();
  }

}

