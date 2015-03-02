// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/Descriptor.h>

namespace nuklei
{
  
  const std::string GraspDescriptor::TypeNames[] = {
    "physical_success", "physical_failure", "physical_unstable",
    "virtual_success", "virtual_failure", "pe_error", "kinematic_error",
    "unknown" }; 
  
  double ColorDescriptor::distanceTo(const Descriptor &d) const
  {
    NUKLEI_TRACE_BEGIN();
    const ColorDescriptor* cd = dynamic_cast<const ColorDescriptor*>(&d);
    if (cd != NULL)
    {
      HSVConeColor c1(getColor());
      HSVConeColor c2(cd->getColor());
      return c1.distanceTo(c2);
    }
    const ColorPairDescriptor* cpd = dynamic_cast<const ColorPairDescriptor*>(&d);
    if (cpd != NULL)
    {
      HSVConeColor c1(getColor());
      HSVConeColor c2(cpd->getLeftColor());
      HSVConeColor c3(cpd->getRightColor());
      return std::min(c1.distanceTo(c2),
                      c1.distanceTo(c3));
    }
    NUKLEI_THROW("Not implemented.");
    NUKLEI_TRACE_END();
  }
  
  
  double ColorPairDescriptor::distanceTo(const Descriptor &d) const
  {
    NUKLEI_TRACE_BEGIN();
    const ColorDescriptor* cd = dynamic_cast<const ColorDescriptor*>(&d);
    if (cd != NULL)
    {
      HSVConeColor c1(getLeftColor());
      HSVConeColor c2(getRightColor());
      HSVConeColor c3(cd->getColor());
      return std::min(c1.distanceTo(c3),
                      c2.distanceTo(c3));
    }
    NUKLEI_THROW("Not implemented.");
    NUKLEI_TRACE_END();
  }
  
  
}

#if BOOST_VERSION < 104100

BOOST_CLASS_EXPORT_GUID(nuklei::ColorPairDescriptor, "mdfh_Descriptor_ColorPair")
BOOST_CLASS_EXPORT_GUID(nuklei::ColorDescriptor, "mdfh_Descriptor_Color")
BOOST_CLASS_EXPORT_GUID(nuklei::GraspDescriptor, "mdfh_Descriptor_Grasp")
BOOST_CLASS_EXPORT_GUID(nuklei::TwoFingerDescriptor, "mdfh_Descriptor_TwoFinger")
BOOST_CLASS_EXPORT_GUID(nuklei::PlaneDescriptor, "mdfh_Descriptor_Plane")
BOOST_CLASS_EXPORT_GUID(nuklei::RGBColor, "mdfh_Color_RGB")
BOOST_CLASS_EXPORT_GUID(nuklei::HSVColor, "mdfh_Color_HSV")
BOOST_CLASS_EXPORT_GUID(nuklei::HSVConeColor, "mdfh_Color_HSVCone")

#else

BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::ColorPairDescriptor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::ColorDescriptor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::GraspDescriptor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::TwoFingerDescriptor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::PlaneDescriptor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::RGBColor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::HSVColor)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::HSVConeColor)

#endif // BOOST_VERSION
