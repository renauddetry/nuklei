// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/OsuTxtObservation.h>

namespace nuklei {
  
  const double OsuTxtObservation::TOL = 1e-5;

  
  OsuTxtObservation::OsuTxtObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorDescriptor d;
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  OsuTxtObservation::OsuTxtObservation(const kernel::r3& k) : k_(k)
  {}
    
  void OsuTxtObservation::setLoc(Location loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Location OsuTxtObservation::getLoc() const { return k_.loc_; }
  
  void OsuTxtObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.setWeight(weight);
    NUKLEI_TRACE_END();
  }
  weight_t OsuTxtObservation::getWeight() const { return k_.getWeight(); }
    
  const Color& OsuTxtObservation::getColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorDescriptor&>(k_.getDescriptor()).getColor();
    NUKLEI_TRACE_END();
  }
  void OsuTxtObservation::setColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorDescriptor&>(k_.getDescriptor()).setColor(color);
    NUKLEI_TRACE_END();
  }

}

