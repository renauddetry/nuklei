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
    
    typedef NUKLEI_UNIQUE_PTR< Descriptor > ptr;
    
    virtual ~Descriptor() {}
    
    virtual void assertConsistency() const = 0;
    
    virtual NUKLEI_UNIQUE_PTR<Descriptor> clone() const = 0;
    virtual NUKLEI_UNIQUE_PTR<Descriptor> create() const = 0;
    
    virtual double distanceTo(const Descriptor &d) const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_THROW("Not implemented.");
      NUKLEI_TRACE_END();
    }
    
  private:
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
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
    typedef NUKLEI_UNIQUE_PTR< VisualDescriptor > ptr;
    
    virtual void setColor(const Color& c) = 0;
    
  private:
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    }
  };
  
  class ColorDescriptor : public VisualDescriptor
  {
  public:
    
    typedef NUKLEI_UNIQUE_PTR< ColorDescriptor > ptr;
    
    ColorDescriptor() : color_(new RGBColor) {}
    
    virtual void assertConsistency() const
    {
      color_->assertConsistency();
    };
    
    NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
      NUKLEI_UNIQUE_PTR<ColorDescriptor> c(new ColorDescriptor);
      c->color_ = color_->clone();
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(c));
    }
    
    NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(new ColorDescriptor));
    }
    
    Color& getColor() { return *color_; }
    const Color& getColor() const { return *color_; }
    void setColor(const Color& c) { color_ = c.clone(); }
    
    double distanceTo(const Descriptor &d) const;
  protected:
    
    NUKLEI_UNIQUE_PTR<Color> color_;
    
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & NUKLEI_SERIALIZATION_MAKE_NVP
      ( "base",  
       NUKLEI_SERIALIZATION_BASE(Descriptor) );
      ar & NUKLEI_SERIALIZATION_NVP(color_);
    }
    
  };
  
  
  class ColorPairDescriptor : public VisualDescriptor
  {
  public:
    
    typedef NUKLEI_UNIQUE_PTR< ColorPairDescriptor > ptr;
    
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
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(pair));
    }
    
    NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(new ColorPairDescriptor));
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
    
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & NUKLEI_SERIALIZATION_MAKE_NVP
      ( "base",  
       NUKLEI_SERIALIZATION_BASE(Descriptor) );
      ar & NUKLEI_SERIALIZATION_NVP(leftColor_);
      ar & NUKLEI_SERIALIZATION_NVP(rightColor_);
    }
    
  };
  
  
  class GraspDescriptor : public Descriptor
  {
  public:    
    typedef NUKLEI_UNIQUE_PTR< GraspDescriptor > ptr;
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
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      if (version >= 1)
      {
        ar & NUKLEI_SERIALIZATION_MAKE_NVP
        ( "base",  
         NUKLEI_SERIALIZATION_BASE(Descriptor) );
      }
      if (version == 1)
      {
        NUKLEI_THROW("Unsupported obsolete format.");
      }
      else if (version >= 2)
      {
        std::string trialOutcome = nameFromType<GraspDescriptor>(trialOutcome_);
        ar & NUKLEI_SERIALIZATION_MAKE_NVP("trialOutcome",
                                            trialOutcome);
        trialOutcome_ = typeFromName<GraspDescriptor>(trialOutcome);
      }
      if (version >= 3)
      {
        ar & NUKLEI_SERIALIZATION_NVP(graspQuality_);
      }
    }
  };
  
  class TwoFingerDescriptor : public GraspDescriptor
  {
  public:
    
    typedef NUKLEI_UNIQUE_PTR< TwoFingerDescriptor > ptr;
    
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
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(g));
    }
    
    NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(new TwoFingerDescriptor));
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
    
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      if (version == 0)
        ar & NUKLEI_SERIALIZATION_MAKE_NVP
        ( "base",  
         NUKLEI_SERIALIZATION_BASE(Descriptor) );
      else
        ar & NUKLEI_SERIALIZATION_MAKE_NVP
        ( "base",  
         NUKLEI_SERIALIZATION_BASE(GraspDescriptor) );
      ar & NUKLEI_SERIALIZATION_NVP(gap_);
      ar & NUKLEI_SERIALIZATION_NVP(closeToGrasp_);
      ar & NUKLEI_SERIALIZATION_NVP(covisEuler_);
    }
    
  };
  
  
  class GeometricDescriptor : public Descriptor
  {
  public:
    typedef NUKLEI_UNIQUE_PTR< GeometricDescriptor > ptr;
    
  private:
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    }
  };
  
  class PlaneDescriptor : public GeometricDescriptor
  {
  public:
    typedef NUKLEI_UNIQUE_PTR< PlaneDescriptor > ptr;
    
    PlaneDescriptor() {}
    
    virtual void assertConsistency() const
    {
      NUKLEI_TRACE_BEGIN();
      NUKLEI_TRACE_END();
    };
    
    NUKLEI_UNIQUE_PTR<Descriptor> clone() const
    {
      NUKLEI_UNIQUE_PTR<PlaneDescriptor> g(new PlaneDescriptor);
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(g));
    }
    
    NUKLEI_UNIQUE_PTR<Descriptor> create() const
    {
      return NUKLEI_UNIQUE_PTR<Descriptor>(NUKLEI_MOVE(new PlaneDescriptor));
    }
  private:
    friend class NUKLEI_SERIALIZATION_FRIEND_CLASSNAME;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
      ar & NUKLEI_SERIALIZATION_MAKE_NVP
      ( "base",  
       NUKLEI_SERIALIZATION_BASE(Descriptor) );
    }
    
  };
  
  
}

NUKLEI_SERIALIZATION_CLASS_VERSION(nuklei::GraspDescriptor, 3);
NUKLEI_SERIALIZATION_CLASS_VERSION(nuklei::TwoFingerDescriptor, 1);

#if BOOST_VERSION < 104100

#else

NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::ColorPairDescriptor, "mdfh_Descriptor_ColorPair")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::ColorDescriptor, "mdfh_Descriptor_Color")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::GraspDescriptor, "mdfh_Descriptor_Grasp")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::TwoFingerDescriptor, "mdfh_Descriptor_TwoFinger")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::PlaneDescriptor, "mdfh_Descriptor_Plane")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::RGBColor, "mdfh_Color_RGB")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::HSVColor, "mdfh_Color_HSV")
NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(nuklei::HSVConeColor, "mdfh_Color_HSVCone")

#endif // BOOST_VERSION

#endif
