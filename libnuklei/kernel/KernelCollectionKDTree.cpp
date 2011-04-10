// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include "KernelCollectionTypes.h"

#include <nuklei/KernelCollection.h>

namespace nuklei {
  
  void KernelCollection::buildNeighborSearchTree()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_ENABLE_QPL
    using namespace cgal_userdef_neighbor_search_types;
    
    boost::shared_ptr<Tree> tree(new Tree);
    for (const_iterator i = as_const(*this).begin(); i != as_const(*this).end(); ++i)
    {
      Vector3 loc = i->getLoc();
      tree->insert(FlexiblePoint(loc.X(), loc.Y(), loc.Z(),
                                 std::distance(as_const(*this).begin(), i)));
    }
    if (deco_.has_key(NSTREE_KEY)) deco_.erase(NSTREE_KEY);
    deco_.insert(NSTREE_KEY, tree);
    
#else
    NUKLEI_THROW("This function requires QPL-licensed code.");
#endif
    NUKLEI_TRACE_END();
  }

  void KernelCollection::buildKdTree()
  {
    using namespace libkdtree_types;
  
    boost::shared_ptr<Tree> tree(new Tree);
    for (const_iterator i = as_const(*this).begin(); i != as_const(*this).end(); ++i)
    {
      Vector3 loc = i->getLoc();
      tree->insert(FlexiblePoint(loc.X(), loc.Y(), loc.Z(),
                                 std::distance(as_const(*this).begin(), i)));
    }
    tree->optimise();

    if (deco_.has_key(KDTREE_KEY)) deco_.erase(KDTREE_KEY);
    deco_.insert(KDTREE_KEY, tree);
  }
  
  template<class KernelType>
  weight_t KernelCollection::staticEvaluationAt(const kernel::base &k,
                                                const EvaluationStrategy strategy) const
  {
    NUKLEI_TRACE_BEGIN();
    
    NUKLEI_ASSERT(KernelType().type() == kernelType_);
    NUKLEI_ASSERT(kernelType_ == k.polyType());
    
    coord_t value = 0;
    if (KDTREE_DENSITY_EVAL && size() > 1000)
    {
      using namespace libkdtree_types;
      NUKLEI_ASSERT(deco_.has_key(KDTREE_KEY));

      const KernelType &evalPoint = static_cast<const KernelType&>(k);
      std::vector<FlexiblePoint> in_range;
      
      
      boost::shared_ptr<Tree> tree(deco_.get< boost::shared_ptr<Tree> >(KDTREE_KEY));
      FlexiblePoint s(evalPoint.loc_.X(), evalPoint.loc_.Y(), evalPoint.loc_.Z(), -1);
      
      coord_t range = maxLocCutPoint();
      
      as_const(*tree).find_within_range(s, range, std::back_inserter(in_range));

      for (std::vector<FlexiblePoint>::const_iterator i = in_range.begin(); i != in_range.end(); i++)
      {
        coord_t cvalue = 0;
        const KernelType &densityPoint = static_cast<const KernelType&>(at(i->idx()));
        cvalue = densityPoint.eval(evalPoint);
        if (strategy == MAX_EVAL) value = std::max(value, cvalue);
        else if (strategy == SUM_EVAL) value += cvalue;
        else if (strategy == WEIGHTED_SUM_EVAL) value += cvalue * densityPoint.w_;
        else NUKLEI_ASSERT(false);
      }
    }
    else
    {
      const KernelType &evalPoint = static_cast<const KernelType&>(k);
      for (const_iterator i = begin(); i != end(); i++)
      {
        coord_t cvalue = 0;
        const KernelType &densityPoint = static_cast<const KernelType&>(*i);
        cvalue = densityPoint.eval(evalPoint);
        if (strategy == MAX_EVAL) value = std::max(value, cvalue);
        else if (strategy == SUM_EVAL) value += cvalue;
        else if (strategy == WEIGHTED_SUM_EVAL) value += cvalue * densityPoint.w_;
        else NUKLEI_ASSERT(false);
      }
    }
        
    return value;
    NUKLEI_TRACE_END();
  }

  weight_t KernelCollection::evaluationAt(const kernel::base &k,
                                          const EvaluationStrategy strategy) const
  {
    NUKLEI_TRACE_BEGIN();
    if (empty()) return 0;
    NUKLEI_ASSERT(kernelType_ == k.polyType());
    
    coord_t value = 0;
    switch (*kernelType_)
    {
      case kernel::base::R3:
      {
        value = staticEvaluationAt<kernel::r3>(k, strategy);
        break;
      }
      case kernel::base::R3XS2:
      {
        value = staticEvaluationAt<kernel::r3xs2>(k, strategy);
        break;
      }
      case kernel::base::R3XS2P:
      {
        value = staticEvaluationAt<kernel::r3xs2p>(k, strategy);
        break;
      }
      case kernel::base::SE3:
      {
        value = staticEvaluationAt<kernel::se3>(k, strategy);
        break;
      }
      default:
      {
        // One could implement a fallback polymorphic eval here (polyEval), but
        // it's so slow that it's not really useful.
        NUKLEI_THROW("Unknow kernel type.");
        break;
      }
    }
    
    return value;
    NUKLEI_TRACE_END();
  }
  
}

