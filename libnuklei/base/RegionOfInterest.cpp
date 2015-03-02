// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

/**
 * @file
 */

#include <nuklei/RegionOfInterest.h>
#include <nuklei/KernelCollection.h>
#include <sstream>
#include <boost/assign/std/vector.hpp>

#define CGAL_EIGEN3_ENABLED

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/AABB_polyhedron_triangle_primitive.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/squared_distance_3.h>
#include <CGAL/Point_with_normal_3.h>



namespace nuklei
{

  typedef CGAL::Simple_cartesian<double> K;
  typedef K::FT FT;
  typedef K::Ray_3 Ray;
  typedef K::Line_3 Line;
  typedef K::Point_3 Point;
  typedef K::Plane_3 Plane;
  typedef K::Vector_3 Vector;
  typedef K::Segment_3 Segment;
  typedef K::Triangle_3 Triangle;
  typedef std::list<Triangle>::iterator Iterator;
  typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
  typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
  typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
#if CGAL_VERSION_NR >= 1040300000
  typedef boost::optional< Tree::Intersection_and_primitive_id<Plane>::Type > Plane_intersection;
#else
  typedef boost::optional< Tree::Object_and_primitive_id > Plane_intersection;
#endif
  
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
    Vector3 center, axis1, axis2;
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

  struct BoxFace
  {
  public:
    BoxFace(const int a,
            const int b,
            const int c,
            const int d) :
    a(a), b(b), c(c), d(d)
    {
      
    }
    
    int a, b, c, d;
  };
  
  BoxFace make_boxface(const int a,
                       const int b,
                       const int c,
                       const int d)
  {
    return BoxFace(a, b, c, d);
  }
  
  void BoxROI::pushTriangles_(std::list<boost::any>& triangles) const
  {
    KernelCollection corners;
    {
      kernel::r3 k;
      k.loc_ = Vector3(edgeLengths_.X()/2 , + edgeLengths_.Y()/2 , + edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(edgeLengths_.X()/2 , + edgeLengths_.Y()/2 , - edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(edgeLengths_.X()/2 , - edgeLengths_.Y()/2 , + edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(edgeLengths_.X()/2 , - edgeLengths_.Y()/2 , - edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(- edgeLengths_.X()/2 , + edgeLengths_.Y()/2 , + edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(- edgeLengths_.X()/2 , + edgeLengths_.Y()/2 , - edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(- edgeLengths_.X()/2 , - edgeLengths_.Y()/2 , + edgeLengths_.Z()/2);
      corners.add(k);
    }
    {
      kernel::r3 k;
      k.loc_ = Vector3(- edgeLengths_.X()/2 , - edgeLengths_.Y()/2 , - edgeLengths_.Z()/2);
      corners.add(k);
    }
    
    corners.transformWith(centerLoc_, centerOri_);
    
    std::vector<BoxFace> faces;
    {
      using namespace boost::assign;
      faces += make_boxface(0, 1, 3, 2),
      make_boxface(2, 3, 7, 6),
      make_boxface(6, 7, 5, 4),
      make_boxface(4, 5, 1, 0),
      make_boxface(0, 1, 3, 2),
      make_boxface(0, 2, 6, 4),
      make_boxface(1, 3, 7, 5);
    }
    
    for (std::vector<BoxFace>::const_iterator i = faces.begin();
         i != faces.end(); ++i)
    {
      const Vector3& aa = corners.at(i->a).getLoc();
      const Vector3& bb = corners.at(i->b).getLoc();
      const Vector3& cc = corners.at(i->c).getLoc();
      const Vector3& dd = corners.at(i->d).getLoc();
      
      Point a(aa.X(), aa.Y(), aa.Z());
      Point b(bb.X(), bb.Y(), bb.Z());
      Point c(cc.X(), cc.Y(), cc.Z());
      Point d(dd.X(), dd.Y(), dd.Z());
      
      triangles.push_back(Triangle(a,b,c));
      triangles.push_back(Triangle(a,d,c));
    }
  }

  
  void BoxROI::buildAABBTree_()
  {
    std::list<Triangle> triangles;
    std::list<boost::any> t;
    pushTriangles(t);
    for (std::list<boost::any>::const_iterator i = t.begin(); i != t.end(); ++i)
    {
      triangles.push_back(boost::any_cast<Triangle>(*i));
    }
    
    // constructs AABB tree
    boost::shared_ptr<Tree> tree(new Tree(triangles.begin(),triangles.end()));
    tree_ = tree;
  }
  
  bool intersects(Plane p, const boost::any& t)
  {
    NUKLEI_ASSERT(!p.is_degenerate());
    NUKLEI_ASSERT(!t.empty());
    boost::shared_ptr<Tree> tree = boost::any_cast< boost::shared_ptr<Tree> >(t);
    Plane_intersection plane_intersection = tree->any_intersection(p);
    return plane_intersection;
  }
  
  bool BoxROI::intersectsPlane(const Vector3& p, const Vector3& q, const Vector3& r) const
  {
    Point pp(p.X(), p.Y(), p.Z());
    Point qq(q.X(), q.Y(), q.Z());
    Point rr(r.X(), r.Y(), r.Z());
    Plane plane_query(pp, qq, rr);
    return intersects(plane_query, tree_);
  }
  
  bool BoxROI::intersectsPlane(const Vector3& p, const Vector3& v) const
  {
    Vector vec(v.X(), v.Y(), v.Z());
    Point a(p.X(), p.Y(), p.Z());
    Plane plane_query(a,vec);
    return intersects(plane_query, tree_);
  }

}
