// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifndef NUKLEI_DESCRIPTOR_H
#define NUKLEI_DESCRIPTOR_H

#include <memory>
#include <boost/utility.hpp>
#include <nuklei/LinearAlgebra.h>
#include <nuklei/Common.h>
#include <nuklei/Color.h>
#include <nuklei/Types.h>

namespace nuklei {
  
  class Descriptor : boost::noncopyable
  {
  public:
    
    typedef  NUKLEI_UNIQUE_PTR< Descriptor > ptr;
    
    virtual ~Descriptor() {}
    
    virtual void assertConsistency() const = 0;
    
    virtual  NUKLEI_UNIQUE_PTR<Descriptor> clone() const = 0;
    virtual  NUKLEI_UNIQUE_PTR<Descriptor> create() const = 0;
    
    virtual double distanceTo(const Descriptor &d) const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_THROW("Not implemented.");
      NUKLEI_TRACE_END();
    }
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    }
    
  };
  
  inline Descriptor* new_clone(const Descriptor& d)
  {
    return d.clone().release();
  }
  
  class VisualDescriptor : public Descriptor
  {
  public:
    typedef  NUKLEI_UNIQUE_PTR< VisualDescriptor > ptr;
    
    virtual void setColor(const Color& c) = 0;
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    }
  };
  
  class ColorDescriptor : public VisualDescriptor
  {
  public:
    
    typedef  NUKLEI_UNIQUE_PTR< ColorDescriptor > ptr;
    
    ColorDescriptor() : color_(new RGBColor) {}
    
    virtual void assertConsistency() const
    {
      color_->assertConsistency();
    };
    
     NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
       NUKLEI_UNIQUE_PTR<ColorDescriptor> c(new ColorDescriptor);
      c->color_ = color_->clone();
      return  NUKLEI_UNIQUE_PTR<Descriptor>(c);
    }
    
     NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return  NUKLEI_UNIQUE_PTR<Descriptor>(new ColorDescriptor);
    }
    
    Color& getColor() { return *color_; }
    const Color& getColor() const { return *color_; }
    void setColor(const Color& c) { color_ = c.clone(); }
    
    double distanceTo(const Descriptor &d) const;
  protected:
    
     NUKLEI_UNIQUE_PTR<Color> color_;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & boost::serialization::make_nvp
      ( "base",  
       boost::serialization::base_object<Descriptor>( *this ) );
      ar & BOOST_SERIALIZATION_NVP(color_);
    }
    
  };
  
  
  class ColorPairDescriptor : public VisualDescriptor
  {
  public:
    
    typedef  NUKLEI_UNIQUE_PTR< ColorPairDescriptor > ptr;
    
    ColorPairDescriptor() : leftColor_(new RGBColor), rightColor_(new RGBColor) {}
    
    virtual void assertConsistency() const
    {
      leftColor_->assertConsistency();
      rightColor_->assertConsistency();
    };
    
     NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
       NUKLEI_UNIQUE_PTR<ColorPairDescriptor> pair(new ColorPairDescriptor);
      pair->leftColor_ = leftColor_->clone();
      pair->rightColor_ = rightColor_->clone();
      return  NUKLEI_UNIQUE_PTR<Descriptor>(pair);
    }
    
     NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return  NUKLEI_UNIQUE_PTR<Descriptor>(new ColorPairDescriptor);
    }
    
    Color& getLeftColor() { return *leftColor_; }
    const Color& getLeftColor() const { return *leftColor_; }
    void setLeftColor(const Color& c) { leftColor_ = c.clone(); }
    
    Color& getRightColor() { return *rightColor_; }
    const Color& getRightColor() const { return *rightColor_; }
    void setRightColor(const Color& c) { rightColor_ = c.clone(); }
    
    void setColor(const Color& c) { rightColor_ = c.clone(); leftColor_ = c.clone(); }
    
    double distanceTo(const Descriptor &d) const;
  private:
    
     NUKLEI_UNIQUE_PTR<Color> leftColor_;
     NUKLEI_UNIQUE_PTR<Color> rightColor_;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & boost::serialization::make_nvp
      ( "base",  
       boost::serialization::base_object<Descriptor>( *this ) );
      ar & BOOST_SERIALIZATION_NVP(leftColor_);
      ar & BOOST_SERIALIZATION_NVP(rightColor_);
    }
    
  };
  
  
  class GraspDescriptor : public Descriptor
  {
  public:    
    typedef  NUKLEI_UNIQUE_PTR< GraspDescriptor > ptr;
    typedef enum { PHYSICAL_SUCCESS = 0,
      PHYSICAL_FAILURE,
      PHYSICAL_UNSTABLE,
      VIRTUAL_SUCCESS,
      VIRTUAL_FAILURE,
      PE_ERROR,
      KINEMATIC_ERROR,
      UNKNOWN
    } TrialOutcome;
    typedef TrialOutcome Type;
    static const Type defaultType = UNKNOWN;
    static const std::string TypeNames[];
    
    GraspDescriptor() : trialOutcome_(UNKNOWN) {}
    
    TrialOutcome getTrialOutcome() const { return trialOutcome_; }
    void setTrialOutcome(const TrialOutcome trialOutcome) { trialOutcome_ = trialOutcome; }
    
    std::vector<weight_t> getQuality() const { return graspQuality_; }
    void setQuality(const std::vector<weight_t> q) { graspQuality_ = q; }
    
  private:
    
    TrialOutcome trialOutcome_;
    std::vector<weight_t> graspQuality_;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      if (version >= 1)
      {
        ar & boost::serialization::make_nvp
        ( "base",  
         boost::serialization::base_object<Descriptor>( *this ) );
      }
      if (version == 1)
      {
        NUKLEI_THROW("Unsupported obsolete format.");
      }
      else if (version >= 2)
      {
        std::string trialOutcome = nameFromType<GraspDescriptor>(trialOutcome_);
        ar & boost::serialization::make_nvp("trialOutcome",
                                            trialOutcome);
        trialOutcome_ = typeFromName<GraspDescriptor>(trialOutcome);
      }
      if (version >= 3)
      {
        ar & BOOST_SERIALIZATION_NVP(graspQuality_);
      }
    }
  };
  
  class TwoFingerDescriptor : public GraspDescriptor
  {
  public:
    
    typedef  NUKLEI_UNIQUE_PTR< TwoFingerDescriptor > ptr;
    
    TwoFingerDescriptor() : gap_(0), closeToGrasp_(true), covisEuler_(Vector3::ZERO) {}
    
    virtual void assertConsistency() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_ASSERT(gap_ >= 0);
      NUKLEI_TRACE_END();
    };
    
     NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
       NUKLEI_UNIQUE_PTR<TwoFingerDescriptor> g(new TwoFingerDescriptor);
      g->gap_ = gap_;
      g->closeToGrasp_ = closeToGrasp_;
      g->setTrialOutcome(getTrialOutcome());
      g->setQuality(getQuality());
      return  NUKLEI_UNIQUE_PTR<Descriptor>(g);
    }
    
     NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return  NUKLEI_UNIQUE_PTR<Descriptor>(new TwoFingerDescriptor);
    }
    
    coord_t getGap() const { return gap_; }
    void setGap(const coord_t gap) { gap_ = gap; }
    
    bool getCloseToGrasp() const { return closeToGrasp_; }
    void setCloseToGrasp(const bool closeToGrasp) { closeToGrasp_ = closeToGrasp; }
    
    // This should move to a sub-class
    Vector3 getCoViSEuler() const { return covisEuler_; }
    void setCoViSEuler(const Vector3& covisEuler) { covisEuler_ = covisEuler; }
  private:
    
    coord_t gap_;
    bool closeToGrasp_;
    Vector3 covisEuler_;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      if (version == 0)
        ar & boost::serialization::make_nvp
        ( "base",  
         boost::serialization::base_object<Descriptor>( *this ) );
      else
        ar & boost::serialization::make_nvp
        ( "base",  
         boost::serialization::base_object<GraspDescriptor>( *this ) );
      ar & BOOST_SERIALIZATION_NVP(gap_);
      ar & BOOST_SERIALIZATION_NVP(closeToGrasp_);
      ar & BOOST_SERIALIZATION_NVP(covisEuler_);
    }
    
  };
  
  
  class GeometricDescriptor : public Descriptor
  {
  public:
    typedef  NUKLEI_UNIQUE_PTR< GeometricDescriptor > ptr;
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    }
  };
  
  class PlaneDescriptor : public GeometricDescriptor
  {
  public:
    typedef  NUKLEI_UNIQUE_PTR< PlaneDescriptor > ptr;
    
    PlaneDescriptor() {}
    
    virtual void assertConsistency() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_TRACE_END();
    };
    
     NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
       NUKLEI_UNIQUE_PTR<PlaneDescriptor> g(new PlaneDescriptor);
      return  NUKLEI_UNIQUE_PTR<Descriptor>(g);
    }
    
     NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return  NUKLEI_UNIQUE_PTR<Descriptor>(new PlaneDescriptor);
    }
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & boost::serialization::make_nvp
      ( "base",  
       boost::serialization::base_object<Descriptor>( *this ) );
    }
    
  };
  
  
}

BOOST_CLASS_VERSION(nuklei::GraspDescriptor, 3);
BOOST_CLASS_VERSION(nuklei::TwoFingerDescriptor, 1);

#if BOOST_VERSION < 104100

#else

BOOST_CLASS_EXPORT_KEY2(nuklei::ColorPairDescriptor, "mdfh_Descriptor_ColorPair")
BOOST_CLASS_EXPORT_KEY2(nuklei::ColorDescriptor, "mdfh_Descriptor_Color")
BOOST_CLASS_EXPORT_KEY2(nuklei::GraspDescriptor, "mdfh_Descriptor_Grasp")
BOOST_CLASS_EXPORT_KEY2(nuklei::TwoFingerDescriptor, "mdfh_Descriptor_TwoFinger")
BOOST_CLASS_EXPORT_KEY2(nuklei::PlaneDescriptor, "mdfh_Descriptor_Plane")
BOOST_CLASS_EXPORT_KEY2(nuklei::RGBColor, "mdfh_Color_RGB")
BOOST_CLASS_EXPORT_KEY2(nuklei::HSVColor, "mdfh_Color_HSV")
BOOST_CLASS_EXPORT_KEY2(nuklei::HSVConeColor, "mdfh_Color_HSVCone")

#endif // BOOST_VERSION

#endif
