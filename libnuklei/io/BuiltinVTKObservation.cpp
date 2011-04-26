// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/BuiltinVTKObservation.h>

namespace nuklei {
  
  const double BuiltinVTKObservation::TOL = 1e-5;

  
  BuiltinVTKObservation::BuiltinVTKObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorDescriptor d;
    RGBColor c(.5,.5,.5);
    d.setColor(c);
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  BuiltinVTKObservation::BuiltinVTKObservation(const kernel::r3& k) : k_(k)
  {}
    
  void BuiltinVTKObservation::setLoc(Location loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Location BuiltinVTKObservation::getLoc() const { return k_.loc_; }
  
  void BuiltinVTKObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.setWeight(weight);
    NUKLEI_TRACE_END();
  }
  weight_t BuiltinVTKObservation::getWeight() const { return k_.getWeight(); }
    
  const Color& BuiltinVTKObservation::getColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorDescriptor&>(k_.getDescriptor()).getColor();
    NUKLEI_TRACE_END();
  }
  void BuiltinVTKObservation::setColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorDescriptor&>(k_.getDescriptor()).setColor(color);
    NUKLEI_TRACE_END();
  }






}

