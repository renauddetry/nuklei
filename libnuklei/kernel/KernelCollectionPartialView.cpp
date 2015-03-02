// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifdef NUKLEI_HAS_PARTIAL_VIEW

#define CGAL_EIGEN3_ENABLED

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/AABB_polyhedron_triangle_primitive.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/squared_distance_3.h>
#include <CGAL/Point_with_normal_3.h>

#endif

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

#include <boost/shared_ptr.hpp>

#include <nuklei/KernelCollection.h>

#ifdef NUKLEI_HAS_PARTIAL_VIEW

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point;
typedef K::Plane_3 Plane;
typedef K::Vector_3 Vector;
typedef K::Segment_3 Segment;
typedef K::Line_3 Line;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef CGAL::AABB_polyhedron_triangle_primitive<K,Polyhedron> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef Tree::Object_and_primitive_id Object_and_primitive_id;
typedef Tree::Primitive_id Primitive_id;

typedef CGAL::Point_with_normal_3<K> Point_with_normal;
typedef std::vector<Point_with_normal> PointList;

#endif

namespace nuklei {

  typedef std::vector< std::pair< Vector3, std::vector<int> > > viewcache_t;

#ifdef NUKLEI_HAS_PARTIAL_VIEW
  
  
  inline bool isVisible(const Vector3& wtarget,
                        const Vector3& wnormal,
                        const Vector3& wcamera,
                        const Polyhedron& poly,
                        const Tree& tree,
                        const coord_t& tolerance)
  {
    if (std::fabs(wnormal.SquaredLength()-1) < FLOATTOL)
    {
      Vector3 ctot = la::normalized(wtarget-wcamera);
      double dot = wnormal.Dot(ctot);
      if (std::acos(std::fabs(dot)) > (80./180*M_PI))
      {
        return false;
      }
    }
    
    Point camera(wcamera.X(), wcamera.Y(), wcamera.Z());
    Point target(wtarget.X(), wtarget.Y(), wtarget.Z());
    
    //double d = dot();
    Segment segment_query(camera,target);
    bool visible = true;
    
    std::list<Object_and_primitive_id> intersections;
    tree.all_intersections(segment_query, std::back_inserter(intersections));
    
    int self_intersect = 0;
    for (std::list<Object_and_primitive_id>::const_iterator intersection = intersections.begin();
         intersection != intersections.end(); ++intersection)
    {
      Object_and_primitive_id op = *intersection;
      CGAL::Object object = op.first;
      Point intersectionPoint;
      if(CGAL::assign(intersectionPoint,object))
      {
        double sq2 = squared_distance(intersectionPoint, target);
        if (sq2 < tolerance*tolerance) self_intersect++;
      }
      else
      {
        // This is very unlikely (p<<0.001)
        //NUKLEI_THROW("intersection is not a point!");
      }
    }
    
    int num_inter = intersections.size();
    if (self_intersect > 1)
      NUKLEI_LOG("Number of self-intersections (" << self_intersect << ") greater than 1");
    
    if (num_inter == 0)
      visible = true;
    else
    {
      if (num_inter - self_intersect > 0)
        visible = false;
      else
        visible = true;
    }
    if (num_inter > 1 && visible)
      NUKLEI_INFO("Strange visibility at" << target);
    
    return visible;
    
  }
  
  inline bool isVisible(const Vector3& wtarget,
                        const Vector3& wcamera,
                        const Polyhedron& poly,
                        const Tree& tree,
                        const coord_t& tolerance)
  {
    return isVisible(wtarget,
                     Vector3::ZERO,
                     wcamera,
                     poly,
                     tree,
                     tolerance);
  }

#endif
  
  bool KernelCollection::isVisibleFrom(const Vector3& p, const Vector3& viewpoint,
                                       const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    if (!deco_.has_key(MESH_KEY))
      NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
    if (!deco_.has_key(AABBTREE_KEY))
      NUKLEI_THROW("Undefined AABB tree. Call buildMesh() first.");
    
    return isVisible(p, viewpoint,
                     *deco_.get< boost::shared_ptr<Polyhedron> >(MESH_KEY),
                     *deco_.get< boost::shared_ptr<Tree> >(AABBTREE_KEY),
                     tolerance);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  bool KernelCollection::isVisibleFrom(const kernel::r3xs2p& p, const Vector3& viewpoint,
                                       const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    if (!deco_.has_key(MESH_KEY))
      NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
    if (!deco_.has_key(AABBTREE_KEY))
      NUKLEI_THROW("Undefined AABB tree. Call buildMesh() first.");

    return isVisible(p.loc_, p.dir_, viewpoint,
                     *deco_.get< boost::shared_ptr<Polyhedron> >(MESH_KEY),
                     *deco_.get< boost::shared_ptr<Tree> >(AABBTREE_KEY),
                     tolerance);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }

  template<typename C>
  C KernelCollection::partialView(const Vector3& viewpoint,
                                  const coord_t& tolerance,
                                  const bool useViewcache,
                                  const bool useRayToSurfacenormalAngle) const
  {
    NUKLEI_TRACE_BEGIN();

    C index_collection;

    if (!useViewcache)
    {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
      if (!deco_.has_key(MESH_KEY))
        NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
      if (!deco_.has_key(AABBTREE_KEY))
        NUKLEI_THROW("Undefined AABB tree. Call buildMesh() first.");
      
      const Polyhedron& poly = *deco_.get< boost::shared_ptr<Polyhedron> >(MESH_KEY);
      const Tree& tree = *deco_.get< boost::shared_ptr<Tree> >(AABBTREE_KEY);
      
      for (const_iterator v = begin(); v != end(); ++v)
      {
        Vector3 p = v->getLoc();
        Vector3 normal = Vector3::ZERO;
        if (useRayToSurfacenormalAngle)
          normal = kernel::r3xs2p(*v).dir_;
        if (isVisible(p, normal, viewpoint, poly, tree, tolerance))
          index_collection.push_back(std::distance(begin(), v));
      }
#else
      NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    }
    else
    {
      if (!deco_.has_key(VIEWCACHE_KEY))
        NUKLEI_THROW("Undefined view cache. Call buildPartialViewCache() first.");

      const viewcache_t &viewIndex = *deco_.get< boost::shared_ptr<viewcache_t> >(VIEWCACHE_KEY);
      
      double minDist = 1e6;
      viewcache_t::const_iterator closest = as_const(viewIndex).begin();
      for (viewcache_t::const_iterator oo = as_const(viewIndex).begin(); oo != viewIndex.end(); ++oo)
      {
        double tmp = (viewpoint-oo->first).SquaredLength();
        if (tmp < minDist*minDist)
        {
          minDist = tmp;
          closest = oo;
        }
      }
      for (std::vector<int>::const_iterator i = closest->second.begin(); i != closest->second.end(); ++i)
        index_collection.push_back(*i);
    }
    return index_collection;
    NUKLEI_TRACE_END();
  }
  
  
  std::vector<int> KernelCollection::partialView(const Vector3& viewpoint,
                                                 const coord_t& tolerance,
                                                 const bool useViewcache,
                                                 const bool useRayToSurfacenormalAngle) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    return partialView< std::vector<int> >(viewpoint, tolerance, useViewcache, useRayToSurfacenormalAngle);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  
  KernelCollection::const_partialview_iterator KernelCollection::partialViewBegin(const Vector3& viewpoint,
                                                                                  const coord_t& tolerance,
                                                                                  const bool useViewcache,
                                                                                  const bool useRayToSurfacenormalAngle) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    
    typedef const_partialview_iterator::index_container
    index_container;
    typedef const_partialview_iterator::index_container_ptr
    index_container_ptr;
    typedef const_partialview_iterator::index_t
    index_t;
    
    index_container_ptr index_collection(new index_container);
    *index_collection = partialView< index_container >(viewpoint, tolerance, useViewcache, useRayToSurfacenormalAngle);
    
    return const_partialview_iterator(begin(), index_collection);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::buildPartialViewCache(const double meshTol, const bool useRayToSurfacenormalAngle)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    Vector3 mean = as_const(*this).moments()->getLoc();
    double stdev = as_const(*this).moments()->getLocH();
    
    std::vector< Vector3 > keys;
    for (int o = 0; o < 10000; ++o)
    {
      Vector3 key = Random::uniformDirection3d();
      keys.push_back(key);
    }
    
    {
      double minDist2 = .15;
      std::vector< Vector3 > tmp = keys;
      keys.clear();
      for (std::vector<Vector3>::const_iterator
           i = tmp.begin();
           i != tmp.end(); ++i)
      {
        bool cnt = false;
        Vector3 iLoc = (*i);
        for (std::vector<Vector3>::const_iterator
             j = keys.begin();
             j != keys.end(); ++j)
        {
          if ( (iLoc - *j).SquaredLength() < minDist2*minDist2 )
          {
            cnt = true;
            break;
          }
        }
        if (!cnt)
        {
          keys.push_back(iLoc);
        }
      }
    }
    
    boost::shared_ptr<viewcache_t> viewIndex(new viewcache_t());
    
    for (unsigned int o = 0; o < keys.size(); ++o)
    {
      Vector3 key = keys.at(o);
      Vector3 vp = mean + key*stdev*20;
      std::vector<int> vi = as_const(*this).partialView(vp, meshTol, false, useRayToSurfacenormalAngle);
#if 0
      // debug - delete when code is considered stable
      KernelCollection v;
      for (std::vector<int>::iterator i = vi.begin(); i != vi.end(); ++i)
        v.add(as_const(*this).at(*i));
      v.computeKernelStatistics();
      kernel::base::ptr k = v.randomKernel().create();
      k->setLoc(vp);
      v.add(*k);
      v.computeKernelStatistics();
#endif
      viewIndex->push_back(std::make_pair(key, vi));
    }
    
    if (deco_.has_key(VIEWCACHE_KEY)) deco_.erase(VIEWCACHE_KEY);
    deco_.insert(VIEWCACHE_KEY, viewIndex);
    
#if 0
    // debug - delete when code is considered stable
    
      for (viewcache_t::iterator o = viewIndex->begin(); o != viewIndex->end(); ++o)
      {
        double d = 1e6;
        KernelCollection near;
        
        for (viewcache_t::iterator oo = viewIndex->begin(); oo != viewIndex->end(); ++oo)
        {
          if (oo == o) continue;
          double dd = (o->first-oo->first).Length();
          if (dd < d) {
            d = dd;
            near.clear();
            for (std::vector<int>::iterator j = oo->second.begin(); j != oo->second.end(); ++j)
              near.add(as_const(*this).at(*j));
            near.computeKernelStatistics();
            if (near.size() == 0) continue;
            kernel::base::ptr k = near.randomKernel().create();
            k->setLoc(mean + oo->first*stdev*20);
            near.add(*k);
            near.computeKernelStatistics();
          }
        }
        for (KernelCollection::iterator i = near.begin(); i != near.end(); ++i)
        {
          ColorDescriptor cd;
          cd.setColor(RGBColor(1, 0, 0));
          i->setDescriptor(cd);
        }
        for (std::vector<int>::iterator j = o->second.begin(); j != o->second.end(); ++j)
        {
          near.add(as_const(*this).at(*j));
          near.back().setLoc(near.back().getLoc()+Vector3(0.001, 0, 0));
        }
        near.computeKernelStatistics();
        if (near.size() == 0) continue;
        kernel::base::ptr k = near.randomKernel().create();
        k->setLoc(mean + o->first*stdev*20);
        near.add(*k);
        near.computeKernelStatistics();
        writeObservations("/tmp/v/" + stringify(std::distance(viewIndex->begin(), o)), near, Observation::SERIAL);
      }
#endif
    
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();

  }

}

