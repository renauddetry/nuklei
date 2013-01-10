// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifdef NUKLEI_USE_CGAL

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

#ifdef NUKLEI_USE_CGAL

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
  
#ifdef NUKLEI_USE_CGAL

  inline bool isVisible(const Point& target, const Point& camera,
                        const Polyhedron& poly,
                        const Tree& tree,
                        const coord_t& tolerance)
  {
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
  
#endif
  
  bool KernelCollection::isVisibleFrom(const Vector3& p, const Vector3& viewpoint,
                                       const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    Point camera(viewpoint.X(), viewpoint.Y(), viewpoint.Z());
    Point target(p.X(), p.Y(), p.Z());
    
    if (!deco_.has_key(MESH_KEY))
      NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
    if (!deco_.has_key(AABBTREE_KEY))
      NUKLEI_THROW("Undefined AABB tree. Call buildMesh() first.");
    
    return isVisible(target, camera,
                     *deco_.get< boost::shared_ptr<Polyhedron> >(MESH_KEY),
                     *deco_.get< boost::shared_ptr<Tree> >(AABBTREE_KEY),
                     tolerance);
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  template<typename C>
  C KernelCollection::partialView(const Vector3& viewpoint,
                                  const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    if (!deco_.has_key(MESH_KEY))
      NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
    if (!deco_.has_key(AABBTREE_KEY))
      NUKLEI_THROW("Undefined AABB tree. Call buildMesh() first.");
    
    const Polyhedron& poly = *deco_.get< boost::shared_ptr<Polyhedron> >(MESH_KEY);
    const Tree& tree = *deco_.get< boost::shared_ptr<Tree> >(AABBTREE_KEY);
    Point camera(viewpoint.X(), viewpoint.Y(), viewpoint.Z());
    C index_collection;
    
    for (const_iterator v = begin(); v != end(); ++v)
    {
      Vector3 p = v->getLoc();
      Point target(p.X(), p.Y(), p.Z());
      if (isVisible(target, camera, poly, tree, tolerance))
        index_collection.push_back(std::distance(begin(), v));
    }
    return index_collection;
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  
  std::vector<int> KernelCollection::partialView(const Vector3& viewpoint,
                                                 const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    return partialView< std::vector<int> >(viewpoint, tolerance);
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  
  KernelCollection::const_partialview_iterator KernelCollection::partialViewBegin(const Vector3& viewpoint,
                                                                                  const coord_t& tolerance) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    
    typedef const_partialview_iterator::index_container
    index_container;
    typedef const_partialview_iterator::index_container_ptr
    index_container_ptr;
    typedef const_partialview_iterator::index_t
    index_t;
    
    index_container_ptr index_collection(new index_container);
    *index_collection = partialView< index_container >(viewpoint, tolerance);
    
    return const_partialview_iterator(begin(), index_collection);
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
}

