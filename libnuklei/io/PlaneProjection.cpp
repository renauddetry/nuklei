// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/PlaneProjection.h>
#include <nuklei/Color.h>


namespace nuklei
{
    const NUKLEI_UNIQUE_PTR<Color> PlaneProjection::getColor() const
    {
      return NUKLEI_UNIQUE_PTR<Color>(new RGBColor(color_[0]/255.,
                                               color_[1]/255.,
                                               color_[2]/255.));
    }
    
    void PlaneProjection::setColor(const Color& c)
    {
      RGBColor rgb(c);
      color_[0] = rgb.R()*255;
      color_[1] = rgb.G()*255;
      color_[2] = rgb.B()*255;
    }  

    Vector3 PlaneProjection::getCamPosition() const
    {
      Vector3 v;
      for (int i = 0; i < 3; ++i) v[i] = translationVectorData_.at(i);
      return v;
    }

    Quaternion PlaneProjection::getCamOrientation() const
    {
      Matrix3 m;
      for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
        m(i,j) = rotationMatrixData_.at(3*i+j);
      m = la::normalized(m);
      return la::quaternionCopy(m);
    }

    const NUKLEI_UNIQUE_PTR<Color> StereoPlaneProjection::getColor() const
    {
      return NUKLEI_UNIQUE_PTR<Color>(new RGBColor(color_[0]/255.,
                                               color_[1]/255.,
                                               color_[2]/255.));
    }
    
    void StereoPlaneProjection::setColor(const Color& c)
    {
      RGBColor rgb(c);
      color_[0] = rgb.R()*255;
      color_[1] = rgb.G()*255;
      color_[2] = rgb.B()*255;
      left_.setColor(c);
      right_.setColor(c);
    }  


}
