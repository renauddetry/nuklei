// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/OffObservation.h>


namespace nuklei {
  
  const double OffObservation::TOL = 1e-5;

  
  OffObservation::OffObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorDescriptor d;
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  OffObservation::OffObservation(const kernel::r3& k) : k_(k)
  {}
    
  void OffObservation::setLoc(Vector3 loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Vector3 OffObservation::getLoc() const { return k_.loc_; }
  
  void OffObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.setWeight(weight);
    NUKLEI_TRACE_END();
  }
  weight_t OffObservation::getWeight() const { return k_.getWeight(); }

  const Color& OffObservation::getColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorDescriptor&>(k_.getDescriptor()).getColor();
    NUKLEI_TRACE_END();
  }
  void OffObservation::setColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorDescriptor&>(k_.getDescriptor()).setColor(color);
    NUKLEI_TRACE_END();
  }
  

  
}

