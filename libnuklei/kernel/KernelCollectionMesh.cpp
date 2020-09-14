// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifdef NUKLEI_HAS_PARTIAL_VIEW


#ifndef CGAL_EIGEN3_ENABLED
#  define CGAL_EIGEN3_ENABLED 1
#endif

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/pca_estimate_normals.h>
#include <CGAL/jet_estimate_normals.h>
#include <CGAL/mst_orient_normals.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>

#include <CGAL/trace.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/output_surface_facets_to_polyhedron.h>
#include <CGAL/Poisson_reconstruction_function.h>
#include <CGAL/Point_with_normal_3.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/compute_average_spacing.h>


#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Polyhedron_3.h>
#if CGAL_VERSION_NR >= 1040300000
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#else
#include <CGAL/AABB_polyhedron_triangle_primitive.h>
#endif
#include <CGAL/property_map.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/squared_distance_3.h>
#include <CGAL/Point_with_normal_3.h>

#endif

#include <vector>
#include <fstream>
#include <utility> // defines std::pair
#include <list>
#include <boost/filesystem.hpp>
#ifdef NUKLEI_HAS_PARTIAL_VIEW
#include <trimesh/TriMesh.h>
#endif

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

#ifdef NUKLEI_HAS_PARTIAL_VIEW


namespace meshing_types {
  // Types
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  
  typedef Kernel::FT FT;
  typedef Kernel::Point_3 Point;
  typedef CGAL::Point_with_normal_3<Kernel> Point_with_normal;
  typedef Kernel::Sphere_3 Sphere;
  typedef std::vector<Point_with_normal> PointList;
  typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
  typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;
  typedef CGAL::Surface_mesh_default_triangulation_3 STr;
  typedef CGAL::Surface_mesh_complex_2_in_triangulation_3<STr> C2t3;
  typedef CGAL::Implicit_surface_3<Kernel, Poisson_reconstruction_function> Surface_3;
  
  // Types
  
  typedef Kernel::Vector_3 Vector;
  
  // Point with normal vector stored in a std::pair.
  typedef std::pair<Point, Vector> PointVectorPair;
}

typedef CGAL::Polyhedron_3< CGAL::Simple_cartesian<double> > SimplePolyhedron;

namespace view_types {
  typedef CGAL::Simple_cartesian<double> K;
  typedef K::Point_3 Point;
  typedef K::Plane_3 Plane;
  typedef K::Vector_3 Vector;
  typedef K::Segment_3 Segment;
  typedef K::Line_3 Line;
  typedef SimplePolyhedron Polyhedron;
#if CGAL_VERSION_NR >= 1040300000
  typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron> Primitive;
#else
  typedef CGAL::AABB_polyhedron_triangle_primitive<K,Polyhedron> Primitive;
#endif
  typedef CGAL::AABB_traits<K, Primitive> Traits;
  typedef CGAL::AABB_tree<Traits> Tree;
  typedef Tree::Object_and_primitive_id Object_and_primitive_id;
  typedef Tree::Primitive_id Primitive_id;
  
  
  typedef CGAL::Point_with_normal_3<K> Point_with_normal;
  typedef std::vector<Point_with_normal> PointList;
}

#endif

namespace nuklei {
  
#ifdef NUKLEI_HAS_PARTIAL_VIEW
  static void buildAABBTree(decoration<int>& deco, const int aabbKey,
                            SimplePolyhedron& poly)
  {
    using namespace view_types;
    
    boost::shared_ptr<Tree> tree(new Tree);
    
    // constructs AABB tree
#if CGAL_VERSION_NR >= 1040300000
    tree.reset(new Tree(faces(poly).first, faces(poly).second, poly));
#else
    tree.reset(new Tree(poly.facets_begin(),poly.facets_end()));
#endif
    tree->accelerate_distance_queries();
    
    if (deco.has_key(aabbKey)) deco.erase(aabbKey);
    deco.insert(aabbKey, tree);
  }
#endif
  
  void KernelCollection::buildMesh()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    
    boost::shared_ptr<SimplePolyhedron> poly(new SimplePolyhedron);
    
    {
      using namespace meshing_types;
      
      std::list<PointVectorPair> points;
      
      {
        for (nuklei::KernelCollection::const_iterator i = as_const(*this).begin(); i != as_const(*this).end(); ++i)
        {
          nuklei::Vector3 v = i->getLoc();
          Point p(v.X(),v.Y(),v.Z());
          points.push_back(std::make_pair(p, Vector()));
        }
      }
      
      const int nb_neighbors = 16;
#if CGAL_VERSION_NR >= 1040800000
      CGAL::jet_estimate_normals<CGAL::Sequential_tag>
      (points.begin(), points.end(),
       CGAL::First_of_pair_property_map<PointVectorPair>(),
       CGAL::Second_of_pair_property_map<PointVectorPair>(),
       nb_neighbors);
#else
      CGAL::jet_estimate_normals(points.begin(), points.end(),
                                 CGAL::First_of_pair_property_map<PointVectorPair>(),
                                 CGAL::Second_of_pair_property_map<PointVectorPair>(),
                                 nb_neighbors);
#endif
      std::list<PointVectorPair>::iterator unoriented_points_begin =
      CGAL::mst_orient_normals(points.begin(), points.end(),
                               CGAL::First_of_pair_property_map<PointVectorPair>(),
                               CGAL::Second_of_pair_property_map<PointVectorPair>(),
                               nb_neighbors);
      
      // Optional: delete points with an unoriented normal
      // if you plan to call a reconstruction algorithm that expects oriented normals.
      points.erase(unoriented_points_begin, points.end());
      
      // Optional: after erase(), use Scott Meyer's "swap trick" to trim excess capacity
      std::list<PointVectorPair>(points).swap(points);
      
      if (0)
      {
        std::ofstream stream("/tmp/xyz_points_and_normals.xyz");
        if (!stream ||
            !CGAL::write_xyz_points_and_normals(stream,
                                                points.begin(), points.end(),
                                                CGAL::First_of_pair_property_map<PointVectorPair>(),
                                                CGAL::Second_of_pair_property_map<PointVectorPair>()))
          std::cerr << "Error writing temp file" << std::endl;
      }
      
      // Poisson options
      FT sm_angle = 20.0; // Min triangle angle in degrees.
      FT sm_radius = 30; // Max triangle size w.r.t. point set average spacing.
      FT sm_distance = 0.05; // Surface Approximation error w.r.t. point set average spacing.
      
      // Reads the point set file in points[].
      // Note: read_xyz_points_and_normals() requires an iterator over points
      // + property maps to access each point's position and normal.
      // The position property map can be omitted here as we use iterators over Point_3 elements.
      PointList pl;
      
      for (std::list<PointVectorPair>::const_iterator i = points.begin(); i != points.end(); ++i)
      {
        pl.push_back(Point_with_normal(i->first, i->second));
      }
      
      // Creates implicit function from the read points using the default solver.
      // Note: this method requires an iterator over points
      // + property maps to access each point's position and normal.
      // The position property map can be omitted here as we use iterators over Point_3 elements.
      Poisson_reconstruction_function function(
                                               pl.begin(), pl.end(),
#if CGAL_VERSION_NR < 1040300000
                                               CGAL::make_normal_of_point_with_normal_pmap(pl.begin())
#elif CGAL_VERSION_NR < 1041200000
                                               CGAL::make_normal_of_point_with_normal_pmap(PointList::value_type())
#else
                                               CGAL::make_normal_of_point_with_normal_map(PointList::value_type())
#endif
                                               );
      
      // Computes the Poisson indicator function f()
      // at each vertex of the triangulation.
      if ( ! function.compute_implicit_function() )
        NUKLEI_THROW("Mesh construction error.");
      
      // Computes average spacing
#if CGAL_VERSION_NR >= 1040800000
      FT average_spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>
      (pl.begin(), pl.end(),
       6 /* knn = 1 ring */);
#else
      FT average_spacing = CGAL::compute_average_spacing(pl.begin(), pl.end(),
                                                         6 /* knn = 1 ring */);
#endif
      
      // Gets one point inside the implicit surface
      // and computes implicit function bounding sphere radius.
      Point inner_point = function.get_inner_point();
      Sphere bsphere = function.bounding_sphere();
      FT radius = std::sqrt(bsphere.squared_radius());
      
      // Defines the implicit surface: requires defining a
      // conservative bounding sphere centered at inner point.
      FT sm_sphere_radius = 5.0 * radius;
      FT sm_dichotomy_error = sm_distance*average_spacing/1000.0; // Dichotomy error must be << sm_distance
      Surface_3 surface(function,
                        Sphere(inner_point,sm_sphere_radius*sm_sphere_radius),
                        sm_dichotomy_error/sm_sphere_radius);
      
      // Defines surface mesh generation criteria
      CGAL::Surface_mesh_default_criteria_3<STr> criteria(sm_angle,  // Min triangle angle (degrees)
                                                          sm_radius*average_spacing,  // Max triangle size
                                                          sm_distance*average_spacing); // Approximation error
      
      // Generates surface mesh with manifold option
      STr tr; // 3D Delaunay triangulation for surface mesh generation
      C2t3 c2t3(tr); // 2D complex in 3D Delaunay triangulation
      CGAL::make_surface_mesh(c2t3,                                 // reconstructed mesh
                              surface,                              // implicit surface
                              criteria,                             // meshing criteria
                              CGAL::Manifold_with_boundary_tag());  // require manifold mesh
      
      if(tr.number_of_vertices() == 0)
        NUKLEI_THROW("Mesh construction error.");
      
      Polyhedron output_mesh;
      CGAL::output_surface_facets_to_polyhedron(c2t3, output_mesh);
      std::stringstream stream;
      stream << output_mesh;
      CGAL::scan_OFF(stream, *poly, true /* verbose */);
      if(!stream || !poly->is_valid() || poly->empty())
      {
        NUKLEI_THROW("Cannot convert mesh.");
      }
    }
    
    if (deco_.has_key(MESH_KEY)) deco_.erase(MESH_KEY);
    deco_.insert(MESH_KEY, poly);
    
    buildAABBTree(deco_, AABBTREE_KEY, *poly);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://renaud-detry.net/nuklei/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::writeMeshToOffFile(const std::string& filename) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    if (!deco_.has_key(MESH_KEY))
      NUKLEI_THROW("Undefined mesh. Call buildMesh() first.");
    std::ofstream out(filename.c_str());
    out << *deco_.get< boost::shared_ptr<SimplePolyhedron> >(MESH_KEY);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://renaud-detry.net/nuklei/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }

  void KernelCollection::writeMeshToPlyFile(const std::string& filename) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    boost::filesystem::path offfile =
    boost::filesystem::unique_path("/tmp/nuklei-%%%%-%%%%-%%%%-%%%%.off");
    boost::filesystem::path plyfile =
    boost::filesystem::unique_path("/tmp/nuklei-%%%%-%%%%-%%%%-%%%%.ply");
    writeMeshToOffFile(offfile.native());
    boost::shared_ptr<trimesh::TriMesh> mesh(trimesh::TriMesh::read(offfile.native()));
    mesh->write(plyfile.native());
    // copy_file complicates portability between C++03 and C++11
    //boost::filesystem::copy_file(plyfile, filename);
    nuklei::copy_file(plyfile.string(), filename);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://renaud-detry.net/nuklei/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }

  void KernelCollection::readMeshFromOffFile(const std::string& filename)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    boost::shared_ptr<SimplePolyhedron> poly(new SimplePolyhedron);
    std::ifstream in(filename.c_str());
    CGAL::scan_OFF(in, *poly, true /* verbose */);
    if(!in || !poly->is_valid() || poly->empty())
    {
      NUKLEI_THROW("Cannot read mesh.");
    }
    if (deco_.has_key(MESH_KEY)) deco_.erase(MESH_KEY);
    deco_.insert(MESH_KEY, poly);
    buildAABBTree(deco_, AABBTREE_KEY, *poly);
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://renaud-detry.net/nuklei/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }

  void KernelCollection::readMeshFromPlyFile(const std::string& filename)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_HAS_PARTIAL_VIEW
    boost::filesystem::path offfile =
    boost::filesystem::unique_path("/tmp/nuklei-%%%%-%%%%-%%%%-%%%%.off");
    boost::filesystem::path plyfile =
    boost::filesystem::unique_path("/tmp/nuklei-%%%%-%%%%-%%%%-%%%%.ply");
    // copy_file complicates portability between C++03 and C++11
    //boost::filesystem::copy_file(filename, plyfile);
    nuklei::copy_file(filename, plyfile.string());
    boost::shared_ptr<trimesh::TriMesh> mesh(trimesh::TriMesh::read(plyfile.native()));
    mesh->write(offfile.native());
    readMeshFromOffFile(offfile.native());
#else
    NUKLEI_THROW("This function requires the partial view build of Nuklei. See http://renaud-detry.net/nuklei/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }

}

