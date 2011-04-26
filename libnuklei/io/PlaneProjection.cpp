// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/PlaneProjection.h>
#include <nuklei/Color.h>


namespace nuklei
{
    const std::auto_ptr<Color> PlaneProjection::getColor() const
    {
      return std::auto_ptr<Color>(new RGBColor(color_[0]/255.,
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

    const std::auto_ptr<Color> StereoPlaneProjection::getColor() const
    {
      return std::auto_ptr<Color>(new RGBColor(color_[0]/255.,
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
