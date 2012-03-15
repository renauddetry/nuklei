// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_KERNEL_COLLECTION_TYPES_H
#define NUKLEI_KERNEL_COLLECTION_TYPES_H

#ifdef NUKLEI_ENABLE_QPL
// CGAL search tree
#include <CGAL/basic.h>
#include <CGAL/Search_traits.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_d.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Cartesian_d.h>

// CGAL hull
#include <CGAL/Homogeneous.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Convex_hull_d_traits_3.h>
// NOTE: the choice of double here for a number type may cause problems
//       for degenerate point sets
#  include <CGAL/double.h>
//#include <CGAL/Gmpz.h>

// CGAL PCA
#include <CGAL/Cartesian.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#endif


// libkdtree++
#include "kdtree++/kdtree.hpp"

#include "KernelCollectionFlexiblePoint.h"

#ifdef NUKLEI_ENABLE_QPL

namespace CGAL {
  template <>
  struct Kernel_traits< ::nuklei::FlexiblePoint > {
    struct Kernel {
      typedef ::nuklei::FlexiblePoint::value_type FT;
      typedef ::nuklei::FlexiblePoint::value_type RT;
    };
  };
}

#endif

namespace nuklei
{
  namespace libkdtree_types
  {
    typedef KDTree::KDTree< 3, FlexiblePoint, FlexiblePoint::Accessor > Tree;
  }

#ifdef NUKLEI_ENABLE_QPL
  namespace cgal_convex_hull_types
  {
    //typedef CGAL::Gmpz RT;
    typedef double RT;

    typedef CGAL::Homogeneous<RT>                  K;
    typedef K::Point_3                             Point_3;

    typedef CGAL::Convex_hull_d_traits_3<K>        Hull_traits_3;
    typedef CGAL::Convex_hull_d< Hull_traits_3 >   Convex_hull_3;
  }

  namespace cgal_neighbor_search_types
  {
    typedef CGAL::Simple_cartesian<coord_t> K;
    typedef K::Point_3 Point_d;
    typedef CGAL::Search_traits_3<K> Traits;
    typedef CGAL::Orthogonal_k_neighbor_search<Traits> K_neighbor_search;
    typedef K_neighbor_search::Tree Tree;
  }

  namespace cgal_userdef_neighbor_search_types
  {
    typedef CGAL::Search_traits<
      FlexiblePoint::value_type,
      FlexiblePoint,
      const FlexiblePoint::value_type*,
      FlexiblePoint::Construct_coord_iterator
    > Traits;
    typedef CGAL::Orthogonal_k_neighbor_search<
      Traits,
      FlexiblePoint::Distance
    > K_neighbor_search;
    typedef K_neighbor_search::Tree Tree;
  }

  namespace cgal_pca_types
  {
    typedef coord_t              FT;
    typedef CGAL::Cartesian<FT>  K;
    typedef K::Plane_3           Plane_3;
    typedef K::Line_3            Line_3;
    typedef K::Point_3           Point_3;
  }
#endif

}



#endif
