// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include "KernelCollectionTypes.h"

#ifdef NUKLEI_ENABLE_QPL
// CGAL jet fitting
#include <CGAL/Monge_via_jet_fitting.h>
#include <CGAL/Cartesian.h>
#endif

#include <nuklei/KernelCollection.h>

namespace nuklei
{
  
#ifdef NUKLEI_ENABLE_QPL
  namespace cgal_jet_fitting_types
  {
    typedef double                   DFT;
    typedef CGAL::Cartesian<DFT>     Data_Kernel;
    typedef Data_Kernel::Point_3     DPoint;
    typedef CGAL::Monge_via_jet_fitting<Data_Kernel> Monge_via_jet_fitting;
    typedef Monge_via_jet_fitting::Monge_form     Monge_form;
  }
#endif
  
  boost::tuple<Matrix3, Vector3, coord_t>
  KernelCollection::localLocationDifferential(const Vector3& loc) const
  {
    NUKLEI_TRACE_BEGIN();

#ifdef NUKLEI_ENABLE_QPL

    using namespace cgal_userdef_neighbor_search_types;
    using namespace cgal_jet_fitting_types;
    
    if (!deco_.has_key(NSTREE_KEY))
      NUKLEI_THROW("Undefined neighbor search tree. Call buildNeighborSearchTree() first.");

    boost::shared_ptr<Tree> tree(deco_.get< boost::shared_ptr<Tree> >(NSTREE_KEY));
      

    std::vector<DPoint> in_points;
    K_neighbor_search search(*tree,
                             FlexiblePoint(loc.X(), loc.Y(), loc.Z()),
                             16+1);
    NUKLEI_ASSERT(search.begin() != search.end());
    for (K_neighbor_search::iterator i = search.begin(); i != search.end(); ++i)
      in_points.push_back(DPoint(i->first.x(), i->first.y(), i->first.z()));

    size_t d_fitting = 4;
    size_t d_monge = 4;

    if (in_points.size() != 16+1)
      return boost::tuple<Matrix3, Vector3, coord_t>();
    
    Monge_form monge_form;
    Monge_via_jet_fitting monge_fit;
    monge_form = monge_fit(in_points.begin(), in_points.end(), d_fitting, d_monge);

//    NUKLEI_LOG("vertex : " << in_points[0] << std::endl
//        << "number of points used : " << in_points.size() << std::endl
//        << monge_form << "condition_number : " << monge_fit.condition_number() << std::endl
//         << "pca_eigen_vals and associated pca_eigen_vecs :"  << std::endl <<
//            "first eval: " << monge_fit.pca_basis(0).first << "; first evec: " << monge_fit.pca_basis(0).second)

    
    Matrix3 eigenVectors;
    Vector3 eigenValues;
    coord_t conditionNumber;
    
    for (int i = 0; i < 3; i++)
    {
      Vector3 v(monge_fit.pca_basis(i).second[0],
                monge_fit.pca_basis(i).second[1],
                monge_fit.pca_basis(i).second[2]);
      eigenVectors.SetColumn(i, v);
      eigenValues[i] = monge_fit.pca_basis(i).first;
    }
        
    conditionNumber = monge_fit.condition_number();
    NUKLEI_ASSERT(conditionNumber != 0);
    
    return boost::make_tuple(eigenVectors, eigenValues, conditionNumber);
    
#else
    NUKLEI_THROW("This function requires QPL-licensed code.");
#endif
    
    NUKLEI_TRACE_END();
  }

}


