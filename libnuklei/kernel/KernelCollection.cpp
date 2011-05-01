// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>

#include <queue>
#include <set>
#include <list>

#include <nuklei/KernelCollection.h>
#include <nuklei/Common.h>
#include <nuklei/Indenter.h>


namespace nuklei {

  const int KernelCollection::HULL_KEY          = 0;
  const int KernelCollection::KDTREE_KEY        = 1;
  const int KernelCollection::NSTREE_KEY        = 2;

  std::istream& operator>>(std::istream &in, KernelCollection &v)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_THROW("not implemented.");
    return in;
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::invalidateHelperStructures()
  {
    totalWeight_ = boost::none;
    maxLocCutPoint_ = boost::none;
    deco_.clear();
  }

  void KernelCollection::assertConsistency() const
  {
    NUKLEI_TRACE_BEGIN();
    for (const_iterator i = begin(); i != end(); i++)
      i->assertConsistency();
    for (const_iterator i = begin(); i != end(); i++)
      NUKLEI_ASSERT(i->polyType() == *kernelType_);
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::clear()
  {
    NUKLEI_TRACE_BEGIN();
    kernels_.clear();
    invalidateHelperStructures();
    kernelType_ = boost::none;
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::add(const kernel::base &f)
  {
    NUKLEI_TRACE_BEGIN();
    invalidateHelperStructures();
    if (size() == 0)
      kernelType_ = f.polyType();
    else
      NUKLEI_ASSERT(*kernelType_ == f.polyType());
    kernels_.push_back(f.clone());
    NUKLEI_TRACE_END();
  }

  void KernelCollection::add(const KernelCollection &kv)
  {
    NUKLEI_TRACE_BEGIN();
    for (const_iterator i = kv.begin(); i != kv.end(); ++i)
      add(*i);
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::replace(const size_t idx, const kernel::base &k)
  {
    NUKLEI_TRACE_BEGIN();
    invalidateHelperStructures();
    NUKLEI_ASSERT(*kernelType_ == k.polyType());
    kernels_.replace(idx, k.clone());
    NUKLEI_TRACE_END();
  }

  kernel::base::Type KernelCollection::kernelType() const
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(kernelType_);
    return *kernelType_;
    NUKLEI_TRACE_END();
  }

  
  KernelCollection::sample_iterator
  KernelCollection::sampleBegin(size_t sampleSize)
  {
    invalidateHelperStructures();        
    is_picked predicate(sampleSize, totalWeight());
    return sample_iterator(predicate, begin(), end());
  }
  
  KernelCollection::const_sample_iterator
  KernelCollection::sampleBegin(size_t sampleSize) const
  {
    is_picked predicate(sampleSize, totalWeight());
    return const_sample_iterator(predicate, begin(), end());
  }
    
  KernelCollection::sort_iterator
  KernelCollection::sortBegin(size_t sortSize)
  {
    invalidateHelperStructures();
    return nuklei_trsl::sort_iterator(begin(), end(),
                                      std::greater<kernel::base>(), sortSize);
  }
  
  KernelCollection::const_sort_iterator
  KernelCollection::sortBegin(size_t sortSize) const
  {
    return nuklei_trsl::sort_iterator(begin(), end(),
                                      std::greater<kernel::base>(), sortSize);
  }

  
  void KernelCollection::computeKernelStatistics()
  {
    NUKLEI_TRACE_BEGIN();
    totalWeight_ = 0;
    maxLocCutPoint_ = 0;
    for (Container::const_iterator i = kernels_.begin(); i != kernels_.end(); i++)
    {
      *totalWeight_ += i->getWeight();
      *maxLocCutPoint_ = std::max(*maxLocCutPoint_, i->polyCutPoint());
    }
    NUKLEI_TRACE_END();
  }

  weight_t KernelCollection::totalWeight() const
  {
    NUKLEI_TRACE_BEGIN();
    if (!totalWeight_)
      NUKLEI_THROW("Undefined total weight. Call computeKernelStatistics() first.");
    return *totalWeight_;
    NUKLEI_TRACE_END();
  };

  weight_t KernelCollection::maxLocCutPoint() const
  {
    NUKLEI_TRACE_BEGIN();
    if (!maxLocCutPoint_)
      NUKLEI_THROW("Undefined max cut point. Call computeKernelStatistics() first.");
    return *maxLocCutPoint_;
    NUKLEI_TRACE_END();
  };
  
  void KernelCollection::normalizeWeights()
  {
    NUKLEI_TRACE_BEGIN();
    
    if (size() == 0)
    {
      totalWeight_ = 0;
      return;
    }
    
    if (!totalWeight_)
      computeKernelStatistics();
    
    for (Container::iterator i = kernels_.begin(); i != kernels_.end(); ++i)
    {
      i->setWeight( i->getWeight() / *totalWeight_ );
    }
    
    totalWeight_ = 1;
    
    NUKLEI_TRACE_END();
  }

  void KernelCollection::uniformizeWeights()
  {
    coord_t w = coord_t(1)/size();
    for (Container::iterator i = kernels_.begin(); i != kernels_.end(); ++i)
      i->setWeight(w);
    totalWeight_ = 1;
  }

  kernel::base::ptr KernelCollection::mean() const
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!empty());
    
    kernel::base::ptr moments = kernels_.back().create();
    
    coord_t w = 0;
    for (const_iterator i = begin(); i != end(); i++)
    {
      coord_t f = i->getWeight() / (w + i->getWeight());
      moments = moments->linearInterpolation(*i, f);
      w += i->getWeight();
    }
    
    moments->setWeight(w / size());
    return moments;
    NUKLEI_TRACE_END();
  }
  
  kernel::base::ptr KernelCollection::deviation(const kernel::base &center) const
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!empty());
    
    kernel::base::ptr dev = center.clone();
    
    coord_t w = 0;
    for (const_iterator i = begin(); i != end(); i++)
    {
      coord_t f = i->getWeight() / (w + i->getWeight());
      dev->updateWidth(*i, f);
      w += i->getWeight();
    }
    
    return dev;
    NUKLEI_TRACE_END();
  }
  
  kernel::base::ptr KernelCollection::moments() const
  {
    NUKLEI_TRACE_BEGIN();
    kernel::base::ptr moments = mean();
    moments = deviation(*moments);
    return moments;
    NUKLEI_TRACE_END();
  }

  void KernelCollection::transformWith(const kernel::se3& t)
  {
    NUKLEI_TRACE_BEGIN();
    invalidateHelperStructures();
    for (iterator i = begin(); i != end(); i++)
      i->polyMakeTransformWith(t);
    NUKLEI_TRACE_END();
  }

  void KernelCollection::transformWith(const Location &translation,
                                   const Rotation &rotation)
  {
    NUKLEI_TRACE_BEGIN();
    kernel::se3 transfo;
    transfo.loc_ = translation;
    la::copyRotation(transfo.ori_, rotation);
    transformWith(transfo);
    NUKLEI_TRACE_END();
  }

  std::vector<Vector3> KernelCollection::get3DPointCloud() const
  {
    std::vector<Vector3> proj;
    for (const_iterator i = begin(); i != end(); ++i)
      proj.push_back(i->getLoc());
    return proj;
  }
  
  void KernelCollection::resetWithSampleOf(const KernelCollection &kc,
                                           int sampleSize)
  {
    NUKLEI_TRACE_BEGIN();
    clear();
        
    for (const_sample_iterator
         i = kc.sampleBegin(sampleSize);
         i != i.end(); ++i)
    {
      kernel::base::ptr k = i->polySample();
      k->setWeight( 1.0/sampleSize );
      add(*k);
    }
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::resetWithHeaviestKernelsOf(const KernelCollection &kc,
                                                    int sampleSize)
  {
    NUKLEI_TRACE_BEGIN();
    clear();
    
    if (sampleSize < int(kc.size())) sampleSize = kc.size();
    for (const_sort_iterator i = kc.sortBegin(sampleSize); i != i.end(); ++i)
      add(*i);
    NUKLEI_TRACE_END();
  }

  const kernel::base& KernelCollection::randomKernel() const
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!empty());
    return *sampleBegin(1);
    NUKLEI_TRACE_END();
  }

  void KernelCollection::setKernelLocH(coord_t h)
  {
    NUKLEI_TRACE_BEGIN();
    for (iterator i = kernels_.begin(); i != kernels_.end(); i++)
    {
      i->setLocH(h);
      maxLocCutPoint_ = i->polyCutPoint();
    }
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::setKernelOriH(coord_t h)
  {
    NUKLEI_TRACE_BEGIN();
    for (iterator i = kernels_.begin(); i != kernels_.end(); i++)
      i->setOriH(h);
    NUKLEI_TRACE_END();
  }
  
  void KernelCollection::clearDescriptors()
  {
    NUKLEI_TRACE_BEGIN();
    for (iterator i = kernels_.begin(); i != kernels_.end(); ++i)
      i->clearDescriptor();
    NUKLEI_TRACE_END();
  }

  void KernelCollection::setFlag(const bitfield_t flag)
  {
    for (iterator i = kernels_.begin(); i != kernels_.end(); ++i)
      i->setFlag(flag);
  }

}

