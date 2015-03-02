// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

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
    
  void PLYObservation::setLoc(Vector3 loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Vector3 PLYObservation::getLoc() const { return k_.loc_; }
  
  void PLYObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.setWeight(weight);
    NUKLEI_TRACE_END();
  }
  weight_t PLYObservation::getWeight() const { return k_.getWeight(); }
    
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

