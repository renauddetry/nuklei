// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/RIFObservation.h>

namespace nuklei {
  
  const double RIFObservation::TOL = 1e-5;

  
  RIFObservation::RIFObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorDescriptor d;
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  RIFObservation::RIFObservation(const kernel::r3& k) : k_(k)
  {}
    
  void RIFObservation::setLoc(Location loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Location RIFObservation::getLoc() const { return k_.loc_; }
  
  void RIFObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.w_ = weight;
    NUKLEI_TRACE_END();
  }
  weight_t RIFObservation::getWeight() const { return k_.w_; }
    
  const Color& RIFObservation::getColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorDescriptor&>(k_.getDescriptor()).getColor();
    NUKLEI_TRACE_END();
  }
  void RIFObservation::setColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorDescriptor&>(k_.getDescriptor()).setColor(color);
    NUKLEI_TRACE_END();
  }

}

