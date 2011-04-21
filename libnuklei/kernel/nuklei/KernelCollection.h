// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */


#ifndef NUKLEI_KERNELCOLLECTION_H
#define NUKLEI_KERNELCOLLECTION_H


#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <boost/tuple/tuple.hpp>

#include <nuklei/decoration.h>
#include <nuklei/BoostSerialization.h>
#include <nuklei/Definitions.h>
#include <nuklei/Kernel.h>
#include <nuklei/nullable.h>
#include <nuklei/RegionOfInterest.h>
#include <nuklei/trsl/common.hpp>
#include <nuklei/trsl/ppfilter_iterator.hpp>
#include <nuklei/trsl/is_picked_systematic.hpp>
#include <nuklei/trsl/sort_iterator.hpp>


namespace nuklei {

  /**
   * @ingroup kernels
   *
   * Todo: Speak about helper structures
   */
  class KernelCollection
    {
    public:
      void assertConsistency() const;
      void invalidateHelperStructures();

      // Forwarded container symbols

      typedef boost::ptr_vector<kernel::base> Container;
      typedef Container::iterator iterator;
      typedef Container::const_iterator const_iterator;
      typedef Container::reverse_iterator reverse_iterator;
      typedef Container::const_reverse_iterator const_reverse_iterator;

      Container::reference at(Container::size_type n)
        { invalidateHelperStructures(); return kernels_.at(n); }
      Container::const_reference at(Container::size_type n) const
        { return kernels_.at(n); }

      Container::reference front()
        { invalidateHelperStructures(); return kernels_.front(); }
      Container::const_reference front() const
        { return kernels_.front(); }

      Container::reference back()
        { invalidateHelperStructures(); return kernels_.back(); }
      Container::const_reference back() const
        { return kernels_.back(); }

      Container::size_type size() const
        { return kernels_.size(); }

      bool empty() const
        { return kernels_.empty(); }

      Container::iterator begin()
        { invalidateHelperStructures(); return kernels_.begin(); }
      Container::const_iterator begin() const
        { return kernels_.begin(); }
      Container::iterator end()
        { invalidateHelperStructures(); return kernels_.end(); }
      Container::const_iterator end() const
        { return kernels_.end(); }

      Container::reverse_iterator rbegin()
        { invalidateHelperStructures(); return kernels_.rbegin(); }
      Container::const_reverse_iterator rbegin() const
        { return kernels_.rbegin(); }
      Container::reverse_iterator rend()
        { invalidateHelperStructures(); return kernels_.rend(); }
      Container::const_reverse_iterator rend() const
        { return kernels_.rend(); }

      // Container-related methods

      void clear();
      void add(const kernel::base &f);
      void add(const KernelCollection &kv);
      void replace(const size_t idx, const kernel::base &k);
      kernel::base::Type kernelType() const;

      // Iterators

      typedef nuklei_trsl::is_picked_systematic<
        kernel::base, weight_t, kernel::base::WeightAccessor> is_picked;
      typedef nuklei_trsl::ppfilter_iterator<
        is_picked, iterator> sample_iterator;
      typedef nuklei_trsl::ppfilter_iterator<
        is_picked, const_iterator> const_sample_iterator;
      
      sample_iterator sampleBegin(size_t sampleSize);
      const_sample_iterator sampleBegin(size_t sampleSize) const;

      typedef nuklei_trsl::reorder_iterator<iterator> sort_iterator;
      typedef nuklei_trsl::reorder_iterator<const_iterator> const_sort_iterator;
      
      sort_iterator sortBegin(size_t sortSize);
      const_sort_iterator sortBegin(size_t sortSize) const;
      
      // Particle-related methods

      void computeKernelStatistics();
      weight_t totalWeight() const;
      coord_t maxLocCutPoint() const;
      void normalizeWeights();
      void uniformizeWeights();

      // Statistical moments
      
      kernel::base::ptr mean() const;
      kernel::base::ptr moments() const;
      
      // Geometrical properties
      
      void transformWith(const kernel::se3& t);
      void transformWith(const Location &translation,
                         const Rotation &rotation);

      boost::tuple<Matrix3, Vector3, coord_t>
      localLocationDifferential(const Vector3& k) const;
      
      kernel::se3 linearLeastSquarePlaneFit() const;
      kernel::se3
      ransacPlaneFit(coord_t inlinerThreshold, unsigned nSeeds = 100) const;

      std::vector<Vector3> get3DPointCloud() const;

      void buildKdTree();
      void buildNeighborSearchTree();
      void buildConvexHull(unsigned n);
      void buildFatConvexHull(unsigned n);
      bool isWithinConvexHull(const kernel::base& k) const;
      
      // Density-related methods
            
      void resetWithSampleOf(const KernelCollection &kc,
                             int sampleSize);
      
      void resetWithHeaviestKernelsOf(const KernelCollection &kc,
                                      int sampleSize);
      
      const kernel::base& randomKernel() const;
      
      void setKernelLocH(coord_t h);
      void setKernelOriH(coord_t h);
      
      typedef enum { SUM_EVAL, MAX_EVAL, WEIGHTED_SUM_EVAL } EvaluationStrategy;
      weight_t evaluationAt(const kernel::base &f,
                            const EvaluationStrategy strategy = WEIGHTED_SUM_EVAL) const;
      
      
      // Misc
      
      void clearDescriptors();

      void setFlag(const bitfield_t flag);

    protected:
      Container kernels_;
      
      boost::optional<weight_t> totalWeight_;
      boost::optional<coord_t> maxLocCutPoint_;
      boost::optional<kernel::base::Type> kernelType_;

      decoration<int> deco_;
      const static int HULL_KEY;
      const static int KDTREE_KEY;
      const static int NSTREE_KEY;

      template<class KernelType>
      weight_t staticEvaluationAt(const kernel::base &k,
                                  const EvaluationStrategy strategy) const;
      
      kernel::base::ptr deviation(const kernel::base &center) const;
      
      friend class boost::serialization::access;
      template<class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
          ar  & BOOST_SERIALIZATION_NVP(kernels_);
          ar  & BOOST_SERIALIZATION_NVP(kernelType_);
        }

    };

}

#endif
