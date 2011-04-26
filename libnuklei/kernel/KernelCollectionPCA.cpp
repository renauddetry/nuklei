// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include "KernelCollectionTypes.h"
#include <nuklei/KernelCollection.h>

namespace nuklei
{

  kernel::se3
  KernelCollection::linearLeastSquarePlaneFit() const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_ENABLE_QPL
    using namespace cgal_pca_types;
    std::list<Point_3> points;
    for (const_iterator i = begin(); i != end(); ++i)
    {
      Vector3 loc = i->getLoc();
      points.push_back(Point_3(loc[0], loc[1], loc[2]));
    }

    Plane_3 plane;
    Point_3 centroid;
#if CGAL_VERSION_NR >= 1030400000
    CGAL::linear_least_squares_fitting_3(points.begin(),points.end(), plane, centroid, CGAL::Dimension_tag<0>());
#else
    CGAL::linear_least_squares_fitting_3(points.begin(),points.end(), plane, centroid);
#endif
    Matrix3 m;
    m.SetColumn(0, Vector3(plane.base1()[0], plane.base1()[1], plane.base1()[2]));
    m.SetColumn(1, Vector3(plane.base2()[0], plane.base2()[1], plane.base2()[2]));
    m.SetColumn(2, Vector3(plane.orthogonal_vector()[0], plane.orthogonal_vector()[1], plane.orthogonal_vector()[2]));
    la::normalized(m);
    
    kernel::se3 k;
    k.loc_ = Vector3(centroid[0], centroid[1], centroid[2]);
    k.ori_ = la::quaternionCopy(m);
    k.loc_h_ = 0;
    for (const_iterator i = begin(); i != end(); ++i)
    {
      coord_t d = dist<groupS::r3>::d(k.loc_, i->getLoc());
      if (d > k.loc_h_) k.loc_h_ = d;
    }
    
    PlaneDescriptor pd;
    k.setDescriptor(pd);
    
    return k;
#else
    NUKLEI_THROW("This function requires QPL-licensed code.");
#endif
    NUKLEI_TRACE_END();
  }

  kernel::se3
  KernelCollection::ransacPlaneFit(coord_t inlinerThreshold, unsigned nSeeds) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_ENABLE_QPL
    using namespace cgal_pca_types;
    
    Plane_3 bestPlane(Point_3(1,0,0), Point_3(0,1,0), Point_3(0,0,0));
    
    int bestPlaneInliners = 0;
    for (unsigned r = 0; r < nSeeds; ++r)
    {
      std::vector<Point_3> randomPoints;
      for (int i = 0; i < 3; ++i)
      {
        Vector3 loc = at(Random::uniformInt(size())).getLoc();
        randomPoints.push_back(Point_3(loc[0], loc[1], loc[2]));
      }
      Plane_3 plane(randomPoints.at(0),
                    randomPoints.at(1),
                    randomPoints.at(2));
      
      int inliners = 0;
      for (const_iterator i = begin(); i != end(); ++i)
      {
        Vector3 loc = i->getLoc();
        Point_3 proj = plane.projection(Point_3(loc[0], loc[1], loc[2]));
        coord_t d = dist<groupS::r3>::d(Vector3(proj[0], proj[1], proj[2]),
                                        loc);
        if (d < inlinerThreshold) inliners++;
      }
      
      if (inliners > bestPlaneInliners)
      {
        bestPlaneInliners = inliners;
        bestPlane = plane;
      }
    }
    
    NUKLEI_ASSERT(bestPlaneInliners > 0)

    KernelCollection kc;
    for (const_iterator i = begin(); i != end(); ++i)
    {
      Vector3 loc = i->getLoc();
      Point_3 proj = bestPlane.projection(Point_3(loc[0], loc[1], loc[2]));
      coord_t d = dist<groupS::r3>::d(Vector3(proj[0], proj[1], proj[2]),
                                      loc);
      if (d < inlinerThreshold) kc.add(*i);
    }
    
    return kc.linearLeastSquarePlaneFit();
#else
    NUKLEI_THROW("This function requires QPL-licensed code.");
#endif
    NUKLEI_TRACE_END();
  }

  namespace la
  {
    Vector3 project(const Plane3& plane, const Vector3& point)
    {
      NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_ENABLE_QPL
      //fixme: this is easy to do without CGAL.
      // so do it without CGAL.
      using namespace cgal_pca_types;
      
      Plane_3 cplane(plane.Normal.X(), plane.Normal.Y(), plane.Normal.Z(), -plane.Constant);
      Point_3 proj = cplane.projection(Point_3(point[0], point[1], point[2]));
      return Vector3(proj[0], proj[1], proj[2]);
#else
      NUKLEI_THROW("This function requires QPL-licensed code.");
#endif
      NUKLEI_TRACE_END();
    }  
  }
}


