// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/CoViS3DObservation.h>


namespace nuklei {
  
  const double CoViS3DObservation::TOL = 1e-5;

  
  CoViS3DObservation::CoViS3DObservation()
  {
    NUKLEI_TRACE_BEGIN();
    ColorPairDescriptor d;
    k_.setDescriptor(d);
    NUKLEI_TRACE_END();
  }

  CoViS3DObservation::CoViS3DObservation(const kernel::r3xs2p& k) :
    k_(k)
  {}

  std::auto_ptr<kernel::base> CoViS3DObservation::getKernel() const
  {
    return k_.clone();
  }
  
  void CoViS3DObservation::setKernel(const kernel::base& k)
  {
    NUKLEI_TRACE_BEGIN();
    k_ = dynamic_cast<const kernel::r3xs2p&>(k);
    NUKLEI_TRACE_END();
  }

  void CoViS3DObservation::setLoc(Vector3 loc)
  {
    NUKLEI_TRACE_BEGIN();
    k_.loc_ = loc;
    NUKLEI_TRACE_END();
  }
  Vector3 CoViS3DObservation::getLoc() const { return k_.loc_; }
  
  void CoViS3DObservation::setDirection(Vector3 direction)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT_AFE_TOL(direction.Length(), 1, TOL);
    direction = la::normalized(direction);
    
    k_.dir_ = direction;
    NUKLEI_TRACE_END();
  }
  Vector3 CoViS3DObservation::getDirection() const { return k_.dir_; }
  void CoViS3DObservation::setPhiPsi(coord_t phi, coord_t psi)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_RANGE_CHECK(phi, - M_PI, M_PI);
    NUKLEI_RANGE_CHECK(psi, -M_PI, 2*M_PI);

    Vector3 l;
    // This is the way Nico does it
    l[0] = std::sin( phi ) * std::cos( psi );
    l[1] = - std::sin( phi ) * std::sin( psi );
    l[2] = std::cos( phi );
    
    setDirection(l);
    NUKLEI_TRACE_END();
  }
  coord_pair CoViS3DObservation::getPhiPsi() const
  {
    NUKLEI_TRACE_BEGIN();
    Vector3 direction = getDirection();
    coord_t phi = ACos(direction[2]);
    coord_t psi = std::atan2( -direction[1] / std::sin(phi),
                              direction[0]/std::sin(phi) );
    return std::make_pair(phi, psi);
    NUKLEI_TRACE_END();
  }
  
  void CoViS3DObservation::setCovMatrix(const Matrix3& cov)
  {
    cov_ = cov;
  }
  
  nullable<Matrix3> CoViS3DObservation::getCovMatrix() const
  {
    return cov_;
  }

  void CoViS3DObservation::setWeight(weight_t weight)
  {
    NUKLEI_TRACE_BEGIN();
    k_.setWeight(weight);
    NUKLEI_TRACE_END();
  }
  weight_t CoViS3DObservation::getWeight() const { return k_.getWeight(); }
    
  const Color& CoViS3DObservation::getLeftColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorPairDescriptor&>(k_.getDescriptor()).getLeftColor();
    NUKLEI_TRACE_END();
  }
  void CoViS3DObservation::setLeftColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorPairDescriptor&>(k_.getDescriptor()).setLeftColor(color);
    NUKLEI_TRACE_END();
  }
  const Color& CoViS3DObservation::getRightColor() const
  {
    NUKLEI_TRACE_BEGIN();
    return dynamic_cast<const ColorPairDescriptor&>(k_.getDescriptor()).getRightColor();
    NUKLEI_TRACE_END();
  }
  void CoViS3DObservation::setRightColor(const Color& color)
  {
    NUKLEI_TRACE_BEGIN();
    dynamic_cast<ColorPairDescriptor&>(k_.getDescriptor()).setRightColor(color);
    NUKLEI_TRACE_END();
  }
  
  void CoViS3DObservation::setGamma(const Vector3& gamma)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT_AFE(gamma.Length(), 1);
    
    Vector3 x = k_.dir_;
    Vector3 y = la::normalized(gamma);
    Vector3 z = la::normalized(x.Cross(y));
    
    NUKLEI_ASSERT_AFE_TOL(x.Dot(k_.dir_), 1, 1e-6);
    gamma_ = y;
    
    NUKLEI_TRACE_END();
  }
  const Vector3& CoViS3DObservation::getGamma() const
  {
    NUKLEI_TRACE_BEGIN();
    return gamma_;
    NUKLEI_TRACE_END();
  }
  


}

