// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

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
   * @brief This class acts as a vector-like container for kernels. It also
   * provides methods related to the density function modeled by the kernels it
   * contains.
   *
   * In Nuklei, kernel classes play the double role of kernel class and point
   * class. There is no class specifically designed for holding an @f$ SE(3) @f$
   * point, the kernel class is used for that purpose. A KernelCollection is
   * thus often used to contain a set of points that are entirely unrelated to a
   * density function.
   *
   * Some of the methods of this class can benefit from caching intermediary
   * results. For instance, the KernelCollection::evaluationAt method
   * requires a @f$k@f$d-tree of all kernel positions. @f$k@f$d-trees are
   * expensive to construct, it is thus important to avoid reconstructing them
   * in each call of KernelCollection::evaluationAt. KernelCollection
   * thus allows users to precompute intermediary results, such as
   * @f$k@f$d-trees, and it stores these structures internally. When a
   * KernelCollection is modified however, intermediary results become
   * invalid. To avoid inconsistencies, each call to KernelCollection which can
   * potentially allow one to modify the contained kernels (for instance,
   * KernelCollection::add) automatically destroys all intermediary
   * results.
   *
   * The functions responsible for computing intermediary results are:
   * - KernelCollection::computeKernelStatistics which computes the sum of all
   *   kernel weights (total weight), and the maximum kernel cut point (todo:
   *   explain cutpoint).
   * - KernelCollection::buildKdTree
   * - KernelCollection::buildNeighborSearchTree
   * - KernelCollection::buildConvexHull
   *
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

      /**
       * @brief Resets the class to its initial state.
       */
      void clear();
      /**
       * @brief Adds a copy of @p f.
       */
      void add(const kernel::base &f);
      /**
       * @brief Adds a copy of the kernels contained in @p kv.
       */
      void add(const KernelCollection &kv);
      /**
       * @brief Replaces the @p idx'th kernel with a copy of @p k.
       */
      void replace(const size_t idx, const kernel::base &k);
      kernel::base::Type kernelType() const;

      // Iterators

      /**
       * @brief Used internally.
       */
      typedef nuklei_trsl::is_picked_systematic<
        kernel::base, weight_t, kernel::base::WeightAccessor> is_picked;
      /**
       * @brief Sample Iterator type.
       *
       * See KernelCollection::sampleBegin.
       */
      typedef nuklei_trsl::ppfilter_iterator<
        is_picked, iterator> sample_iterator;
      /**
       * @brief Sample Iterator type.
       *
       * See KernelCollection::sampleBegin.
       */
      typedef nuklei_trsl::ppfilter_iterator<
        is_picked, const_iterator> const_sample_iterator;
      
      /**
       * @brief Returns an iterator that iterates through @p sampleSize kernels.
       *
       * The probability that a given kernel is chosen is proportional
       * to its weight. <b>This iterator does not return a sample of
       * the density, it returns a sample of the kernels. To get
       * samples from the density, the returned kernels need to be
       * sampled exactly once each.</b>
       */
      sample_iterator sampleBegin(size_t sampleSize);
      /**
       * @brief Returns an iterator that iterates through @p sampleSize kernels.
       *
       * The probability that a given kernel is chosen is proportional
       * to its weight. <b>This iterator does not return a sample of
       * the density, it returns a sample of the kernels. To get
       * samples from the density, the returned kernels need to be
       * sampled exactly once each.</b>
       */
      const_sample_iterator sampleBegin(size_t sampleSize) const;

      /**
       * @brief Sort Iterator type.
       *
       * See KernelCollection::sortBegin.
       */
      typedef nuklei_trsl::reorder_iterator<iterator> sort_iterator;
      /**
       * @brief Sort Iterator type.
       *
       * See KernelCollection::sortBegin.
       */
      typedef nuklei_trsl::reorder_iterator<const_iterator> const_sort_iterator;
      
      /**
       * @brief Returns an iterator that iterates through the @p sortSize
       * kernels of highest weight, in order of decreasing weight.
       */
      sort_iterator sortBegin(size_t sortSize);
      /**
       * @brief Returns an iterator that iterates through the @p sortSize
       * kernels of highest weight, in order of decreasing weight.
       */
      const_sort_iterator sortBegin(size_t sortSize) const;
      
      // Particle-related methods

      /**
       * @brief Computes the total weight of the kernels, and the
       * maximum kernel width.
       *
       * Necessary for methods such as KernelCollection::sampleBegin
       * or KernelCollection::evaluationAt which require these
       * statistics. If a method such as KernelCollection::sampleBegin
       * is called at a point where kernel statistics are not
       * available, an exception will be thrown.
       */
      void computeKernelStatistics();
      /**
       * @brief Returns the sum of kernel weights.
       */
      weight_t totalWeight() const;
      coord_t maxLocCutPoint() const;
      /**
       * @brief Divides all weights by the total weight.
       */
      void normalizeWeights();
      /**
       * @brief Sets all weights to @f$ 1 / t @f$, where @f$ t @f$ is
       * the total weight of the collection.
       */
      void uniformizeWeights();

      // Statistical moments
      
      /**
       * @brief Returns a kernel holding the mean position and orientation of
       * the data.
       */
      kernel::base::ptr mean() const;
      /**
       * @brief Returns a kernel holding the mean and standard deviation in
       * position and orientation of the data.
       *
       * Standard deviations for position and orientation are stored in the
       * kernel bandwidths.
       */
      kernel::base::ptr moments() const;
      
      // Geometrical properties
      
      /**
       * @brief Transforms the data with @p t.
       */
      void transformWith(const kernel::se3& t);
      /**
       * @brief Transforms the data with the provided translation and rotation.
       */
      void transformWith(const Location &translation,
                         const Rotation &rotation);

      /**
       * @brief Computes the local differential properties of the nearest
       * neighbors of @p k.
       *
       * This function uses the CGAL <a
       * href="http://www.cgal.org/Manual/3.3/doc_html/cgal_manual/Jet_fitting_3/Chapter_main.html">Monge
       * fit</a> functions.
       */
      boost::tuple<Matrix3, Vector3, coord_t>
      localLocationDifferential(const Vector3& k) const;
      
      /**
       * @brief Fits a plane to the positions of the kernels contained in @p
       * *this.
       *
       * The location of the returned kernel is a point of the plane. The
       * orientation of the returned kernel is such that its @f$ z @f$ axis is
       * normal to the plane.
       */
      kernel::se3 linearLeastSquarePlaneFit() const;
      /**
       * @brief Fits a plane to the positions of the kernels contained in @p
       * *this.
       *
       * The location of the returned kernel is a point of the plane. The
       * orientation of the returned kernel is such that its @f$ z @f$ axis is
       * normal to the plane.
       */
      kernel::se3
      ransacPlaneFit(coord_t inlinerThreshold, unsigned nSeeds = 100) const;

      /**
       * @brief Returns the locations of the contained kernels in an std::vector.
       */
      std::vector<Vector3> get3DPointCloud() const;

      /**
       * @brief Builds a kd-tree of the kernel positions and stores the tree internally. The tree is used by the method
       */
      void buildKdTree();
      void buildNeighborSearchTree();
      void buildConvexHull(unsigned n);
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
