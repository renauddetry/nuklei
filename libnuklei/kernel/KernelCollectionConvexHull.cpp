// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include "KernelCollectionTypes.h"

#include <nuklei/KernelCollection.h>

namespace nuklei {

  void KernelCollection::buildConvexHull(unsigned n)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    using namespace cgal_convex_hull_types;
    
    if (n > size()) n = size();
    
    // create instance of the class with dimension == 3
    boost::shared_ptr<Convex_hull_3> CH_p(new Convex_hull_3(3));
    
    for (const_sample_iterator i = as_const(*this).sampleBegin(n); i != i.end(); ++i)
    {
      Vector3 loc = i->getLoc();
      CH_p->insert(Point_3(loc.X(), loc.Y(), loc.Z()));
    }

    if (!CH_p->is_valid())
      NUKLEI_LOG("As it often happens, CGAL says the hull is invalid.");
    
    if (deco_.has_key(HULL_KEY)) deco_.erase(HULL_KEY);
    deco_.insert(HULL_KEY, CH_p);
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }


  bool KernelCollection::isWithinConvexHull(const kernel::base& k) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CGAL
    using namespace cgal_convex_hull_types;
    Vector3 loc = k.getLoc();
    if (!deco_.has_key(HULL_KEY))
      NUKLEI_THROW("Undefined convex hull. Call buildConvexHull() first.");
    return deco_.get< boost::shared_ptr<Convex_hull_3> >(HULL_KEY)->bounded_side
      (Point_3(loc.X(), loc.Y(), loc.Z())) != CGAL::ON_UNBOUNDED_SIDE;
#else
    NUKLEI_THROW("This function requires CGAL. See http://nuklei.sourceforge.net/doxygen/group__install.html");
#endif
    NUKLEI_TRACE_END();
  }


}

