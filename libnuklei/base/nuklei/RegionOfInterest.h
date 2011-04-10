// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_REGIONOFINTEREST_H
#define NUKLEI_REGIONOFINTEREST_H


#include <nuklei/Definitions.h>
#include <nuklei/LinearAlgebra.h>
#include <nuklei/GenericKernel.h>

namespace nuklei {
  
  class RegionOfInterest
  {
  public:
    RegionOfInterest(bool positive = true) : positive_(positive) {}
    
    virtual ~RegionOfInterest() {}
    
    bool contains(const Vector3 &v) const
    {
      return (positive_ == contains_(v)) || (next_ && next_->contains(v));
    }
    
    virtual void enqueue(boost::shared_ptr<RegionOfInterest> &roi)
    {
      if (next_)
        next_->enqueue(roi);
      else
        next_ = roi;
    }
    
    virtual void setSign(bool positive)
    {
      positive_ = positive;
    }
  protected:
    virtual bool contains_(const Vector3 &v) const = 0;
    bool positive_;
  private:
    boost::shared_ptr<RegionOfInterest> next_;
  };
  
  
  class SphereROI : public RegionOfInterest
  {
  public:
    SphereROI(const Vector3 &center, double radius) :
    center_(center), radius_(radius) {}
    
    SphereROI(const std::string &centerAndRadius);
    
    ~SphereROI() {}
  protected:
    bool contains_(const Vector3 &v) const;
  private:
    Vector3 center_;
    double radius_;
  };
  
  class BoxROI : public RegionOfInterest
  {
  public:
    BoxROI(const Vector3 &centerLoc, const Orientation &centerOri,
           const Vector3 &edgeLengths) :
    centerLoc_(centerLoc), centerOri_(centerOri), edgeLengths_(edgeLengths) {}
    
    BoxROI(const std::string &s);
    
    void setCenterOriSize(const std::string &centerSize);
    void setCenterAxesSize(const std::string &centerSize);
    
    ~BoxROI() {}
  protected:
    bool contains_(const Vector3 &v) const;
  private:
    Vector3 centerLoc_;
    Orientation centerOri_;
    Vector3 edgeLengths_;
  };
  
}

#endif

