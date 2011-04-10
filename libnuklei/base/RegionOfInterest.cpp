// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

/**
 * @file
 */

#include <nuklei/RegionOfInterest.h>
#include <sstream>
namespace nuklei
{

  SphereROI::SphereROI(const std::string &centerAndRadius)
  {
    std::istringstream is(centerAndRadius);
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> center_[i]);
    NUKLEI_ASSERT(is >> radius_);
    NUKLEI_ASSERT(radius_ >= 0);
  }

  BoxROI::BoxROI(const std::string &s)
  {
    setCenterOriSize(s);
  }

  void BoxROI::setCenterAxesSize(const std::string &centerSize)
  {
    std::istringstream is(centerSize);
    Location center, axis1, axis2;
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> center[i]);
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> axis1[i]);
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> axis2[i]);
    NUKLEI_ASSERT(is >> edgeLengths_[2]);
    
    axis1 -= center; axis2 -= center;
    edgeLengths_[0] = axis1.Length()*2*1.2;
    edgeLengths_[1] = axis2.Length()*2*1.2;
    edgeLengths_[2] *= 2*1.0;
    axis1.Normalize(); axis2.Normalize();
    
    Matrix3 ori;
    ori.SetColumn(0, axis1);
    ori.SetColumn(1, axis2);
    ori.SetColumn(2, la::normalized(axis1.Cross(axis2)));
    ori.Orthonormalize();
    
    centerLoc_ = center;
    la::copyRotation(centerOri_, ori);
  }

  void BoxROI::setCenterOriSize(const std::string &centerSize)
  {
    std::istringstream is(centerSize);
    Quaternion ori;
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> centerLoc_[i]);
    for (unsigned int i = 0; i < 4; ++i)
      NUKLEI_ASSERT(is >> ori[i]);
    for (unsigned int i = 0; i < 3; ++i)
      NUKLEI_ASSERT(is >> edgeLengths_[i]);
    la::copyRotation(centerOri_, ori);
  }

  bool SphereROI::contains_(const Vector3 &v) const
  {
    return dist<groupS::r3>::d(v, center_) <= radius_;
  }
    
  bool BoxROI::contains_(const Vector3 &v) const
  {
    Vector3 projection = la::project(centerLoc_, centerOri_, v);
  
    return std::fabs(projection[0]) <= edgeLengths_[0]/2. &&
      std::fabs(projection[1]) <= edgeLengths_[1]/2. &&
      std::fabs(projection[2]) <= edgeLengths_[2]/2.;
  }


}
