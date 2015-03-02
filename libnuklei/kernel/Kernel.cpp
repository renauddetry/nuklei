// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/Kernel.h>
#include <nuklei/Common.h>
#include <nuklei/Indenter.h>
#include <boost/static_assert.hpp>

namespace nuklei
{
  
  
  namespace kernel
  {
    
    const std::string base::TypeNames[] = { "r3", "r3xs2", "r3xs2p", "se3" };
    
    std::ostream& operator<<(std::ostream &out, const kernel::base &k)
    {
      return k.polyPrint(out);
    }
    
    // SE(3)
    
    void se3::assertConsistency() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_ASSERT(w_ >= 0);
      la::check(ori_, __FUNCTION__);
      NUKLEI_ASSERT(loc_h_ >= 0);
      NUKLEI_ASSERT(ori_h_ >= 0);
      NUKLEI_TRACE_END();
    }
    
    std::ostream& se3::print(std::ostream &out) const
    {
      Indenter idt(out);
      idt << "Kernel SE(3): [ weight = " << w_ << " ]" << std::endl;
      {
        Indenter idt2(out);
        idt2 << "Location: " << loc_ << std::endl;
        idt2 << "LocWidth: " << loc_h_ << std::endl;
        idt2 << "Orientation: ";
        ::nuklei::operator<<(idt2, ori_) << std::endl;
        idt2 << "OriWidth: " << ori_h_ << std::endl;
      }
      return out;
    }
    
    kernel::base::Type se3::type() const
    {
      return SE3;
    }
    
    //coord_t se3::eval(const kernel::se3& k) const
    //cf. header.
    
    kernel::se3 se3::sample() const
    {
      kernel::se3 s;
      typedef nuklei::sampler<PositionKernel> PositionSampler;
      
      typedef nuklei::sampler<OrientationKernel> OrientationSampler;
      
      s.loc_ = PositionSampler::s(loc_, loc_h_);
      s.ori_ = OrientationSampler::s(ori_, ori_h_);
      
      if (hasDescriptor()) s.setDescriptor(getDescriptor());
      return s;
    }
    
    kernel::se3 se3::se3Sample() const
    {
      return sample();
    }
    
    kernel::se3 se3::se3Proj() const
    {
      kernel::se3 k;
      k.loc_ = loc_;
      k.ori_ = ori_;
      if (hasDescriptor()) k.setDescriptor(getDescriptor());
      return *this;
    }
    
    kernel::se3 se3::projectedOn(const kernel::se3& k) const
    {
      kernel::se3 p;
      la::project(p.loc_, p.ori_, k.loc_, k.ori_, loc_, ori_);
      if (hasDescriptor()) p.setDescriptor(getDescriptor());
      return p;
    }
    
    kernel::se3 se3::transformedWith(const kernel::se3& k) const
    {
      kernel::se3 p;
      la::transform(p.loc_, p.ori_, k.loc_, k.ori_, loc_, ori_);
      if (hasDescriptor()) p.setDescriptor(getDescriptor());
      return p;
    }
    
    void se3::makeTransformWith(const kernel::se3& k)
    {
      la::transform(loc_, ori_, k.loc_, k.ori_, loc_, ori_);
    }
    
    kernel::se3 se3::transformationFrom(const kernel::se3& k) const
    {
      kernel::se3 p;
      la::transfoTo(p.loc_, p.ori_, k.loc_, k.ori_, loc_, ori_);
      if (hasDescriptor()) p.setDescriptor(getDescriptor());
      return p;
    }

    kernel::se3 se3::inverseTransformation() const
    {
      kernel::se3 origin;
      kernel::se3 it = origin.transformationFrom(*this);
      if (hasDescriptor()) it.setDescriptor(getDescriptor());
      return it;
    }

    kernel::se3
    kernel::se3::linearInterpolation(const kernel::se3& k,
                                     const coord_t x) const
    {
      NUKLEI_ASSERT(0 <= x && x <= 1);
      kernel::se3 i;
      i.loc_ = (1-x) * loc_ + x * k.loc_;
      Quaternion q = k.ori_;
      if (ori_.Dot(q) < 0) q = - q;
      //fixme: this should use slerp.
      i.ori_ = (1-x) * ori_ + x * q;
      i.ori_.Normalize();
      return i;
    }
    
    void kernel::se3::updateWidth(const kernel::se3& k,
                                  const coord_t x)
    {
      loc_h_ = std::sqrt( (1-x) * loc_h_*loc_h_ +
                         x * std::pow(dist<groupS::r3>::d(loc_, k.loc_), 2) );
      ori_h_ = std::sqrt( (1-x) * ori_h_*ori_h_ +
                         x * std::pow(dist<groupS::so3>::d(ori_, k.ori_), 2) );
    }
    
    coord_pair kernel::se3::distanceTo(const kernel::se3& k) const
    {
      return std::make_pair(dist<groupS::r3>::d(loc_, k.loc_),
                            dist<groupS::so3>::d(ori_, k.ori_));
    }
    
    // S^2(_+)
    
    // R^3
    
    void r3::assertConsistency() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_ASSERT(w_ >= 0);
      NUKLEI_ASSERT(loc_h_ >= 0);
      NUKLEI_TRACE_END();
    }
    
    std::ostream& r3::print(std::ostream &out) const
    {
      Indenter idt(out);
      idt << "Kernel R^3: [ weight = " << w_ << " ]" << std::endl;
      {
        Indenter idt2(out);
        idt2 << "Location: " << loc_ << std::endl;
        idt2 << "LocWidth: " << loc_h_ << std::endl;
      }
      return out;
    }
    
    kernel::base::Type r3::type() const
    {
      return R3;
    }
    
    //coord_t r3::eval(const kernel::r3& k) const
    //cf. header.
    
    kernel::r3 r3::sample() const
    {
      kernel::r3 s;
            
      typedef nuklei::sampler<PositionKernel> PositionSampler;
      
      s.loc_ = PositionSampler::s(loc_, loc_h_);
      
      if (hasDescriptor()) s.setDescriptor(getDescriptor());
      return s;
    }
    
    kernel::se3 r3::se3Sample() const
    {
      kernel::r3 r3k = sample();
      kernel::se3 se3k;
      se3k.loc_ = r3k.loc_;
      se3k.ori_ = random_element<groupS::so3>::r();
      if (hasDescriptor()) se3k.setDescriptor(getDescriptor());
      return se3k;
    }
    
    kernel::se3 r3::se3Proj() const
    {
      kernel::r3 r3k = *this;
      kernel::se3 se3k;
      se3k.loc_ = r3k.loc_;
      se3k.ori_ = random_element<groupS::so3>::r();
      if (hasDescriptor()) se3k.setDescriptor(getDescriptor());
      return se3k;
    }
    
    kernel::r3 r3::projectedOn(const kernel::se3& k) const
    {
      kernel::r3 p;
      p.loc_ = la::project(k.loc_, k.ori_, loc_);
      if (hasDescriptor()) p.setDescriptor(getDescriptor());
      return p;
    }
    
    kernel::r3 r3::transformedWith(const kernel::se3& k) const
    {
      kernel::r3 p;
      p.loc_ = la::transform(k.loc_, k.ori_, loc_);
      if (hasDescriptor()) p.setDescriptor(getDescriptor());
      return p;
    }
    
    void r3::makeTransformWith(const kernel::se3& k)
    {
      loc_ = la::transform(k.loc_, k.ori_, loc_);
    }
    
    kernel::r3
    kernel::r3::linearInterpolation
    (const kernel::r3& k,
     const coord_t x) const
    {
      NUKLEI_ASSERT(0 <= x && x <= 1);
      kernel::r3 i;
      i.loc_ = (1-x) * loc_ + x * k.loc_;
      return i;
    }
    
    void kernel::r3::updateWidth(const kernel::r3& k,
                                 const coord_t x)
    {
      loc_h_ = std::sqrt( (1-x) * loc_h_*loc_h_ +
                         x * std::pow(dist<groupS::r3>::d(loc_, k.loc_), 2) );
    }
    
    coord_pair
    kernel::r3::distanceTo(const kernel::r3& k) const
    {
      return std::make_pair(dist<groupS::r3>::d(loc_, k.loc_), -1.);
    }
    
  }
  
  namespace enforce_data_align {
    
    BOOST_STATIC_ASSERT(sizeof(kernel::base) % 8 == 0);
    
  }
  
}

#if BOOST_VERSION < 104100

BOOST_CLASS_EXPORT_GUID(nuklei::kernel::se3, "mdfh_kernel_se3")
BOOST_CLASS_EXPORT_GUID(nuklei::kernel::r3xs2, "mdfh_kernel_r3xs2")
BOOST_CLASS_EXPORT_GUID(nuklei::kernel::r3xs2p, "mdfh_kernel_r3xs2p")
BOOST_CLASS_EXPORT_GUID(nuklei::kernel::r3, "mdfh_kernel_r3")

#else

BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::kernel::se3)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::kernel::r3xs2)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::kernel::r3xs2p)
BOOST_CLASS_EXPORT_IMPLEMENT(nuklei::kernel::r3)

#endif // BOOST_VERSION
