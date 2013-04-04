// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/PCLBridge.h>

namespace nuklei {
  
#ifdef NUKLEI_USE_PCL
  
  kernel::r3 nukleiKernelFromPclPoint(const pcl::PointXYZ& p)
  {
    kernel::r3 k;
    k.loc_ = Vector3(p.x, p.y, p.z);
    return k;
  }
  
  kernel::r3 nukleiKernelFromPclPoint(const pcl::PointXYZRGB& p)
  {
    kernel::r3 k;
    k.loc_ = Vector3(p.x, p.y, p.z);
    ColorDescriptor d;
    const uint32_t rgb = *reinterpret_cast<const int*>(&p.rgb);
    uint8_t r = (rgb >> 16) & 0x0000ff;
    uint8_t g = (rgb >> 8)  & 0x0000ff;
    uint8_t b = (rgb)       & 0x0000ff;
    RGBColor c(double(r)/255, double(g)/255, double(b)/255);
    d.setColor(c);
    k.setDescriptor(d);
    return k;
  }
  
  /**
   *
   *
   * Warning: converts to axial orientation. 
   */
  kernel::r3xs2p nukleiKernelFromPclPoint(const pcl::PointNormal& p)
  {
    kernel::r3xs2p k;
    k.loc_ = Vector3(p.x, p.y, p.z);
    k.dir_ = la::normalized(Vector3(p.normal_x, p.normal_y, p.normal_z));
    return k;
  }
  
  /**
   *
   *
   * Warning: converts to axial orientation. 
   */
  kernel::r3xs2p nukleiKernelFromPclPoint(const pcl::PointXYZRGBNormal& p)
  {
    kernel::r3xs2p k;
    k.loc_ = Vector3(p.x, p.y, p.z);
    k.dir_ = la::normalized(Vector3(p.normal_x, p.normal_y, p.normal_z));
    ColorDescriptor d;
    const uint32_t rgb = *reinterpret_cast<const int*>(&p.rgb);
    uint8_t r = (rgb >> 16) & 0x0000ff;
    uint8_t g = (rgb >> 8)  & 0x0000ff;
    uint8_t b = (rgb)       & 0x0000ff;
    RGBColor c(double(r)/255, double(g)/255, double(b)/255);
    d.setColor(c);
    k.setDescriptor(d);
    return k;
  }
  
  
  
  void pclPointFromNukleiKernel(pcl::PointXYZ& p, const kernel::r3& k)
  {
    p.x = k.loc_.X();
    p.y = k.loc_.Y();
    p.z = k.loc_.Z();
  }
  
  void pclPointFromNukleiKernel(pcl::PointXYZ& p, const kernel::base& k)
  {
    pclPointFromNukleiKernel(p, dynamic_cast<const kernel::r3&>(k));
  }
  
  
  void pclPointFromNukleiKernel(pcl::PointXYZRGB& p, const kernel::r3& k)
  {
    p.x = k.loc_.X();
    p.y = k.loc_.Y();
    p.z = k.loc_.Z();
    const ColorDescriptor& d = dynamic_cast<const ColorDescriptor&>(k.getDescriptor());
    const RGBColor c(d.getColor());
    uint8_t r = c.R()*255, g = c.G()*255, b = c.B()*255;
    uint32_t rgb = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
    p.rgb = *reinterpret_cast<float*>(&rgb);
  }
  
  void pclPointFromNukleiKernel(pcl::PointXYZRGB& p, const kernel::base& k)
  {
    pclPointFromNukleiKernel(p, dynamic_cast<const kernel::r3&>(k));
  }
  
  
  /**
   *
   *
   * Warning: converts to axial orientation. 
   */
  void pclPointFromNukleiKernel(pcl::PointNormal& p, const kernel::r3xs2p& k)
  {
    p.x = k.loc_.X();
    p.y = k.loc_.Y();
    p.z = k.loc_.Z();
    p.normal_x = k.dir_.X();
    p.normal_y = k.dir_.Y();
    p.normal_z = k.dir_.Z();
  }
  
  void pclPointFromNukleiKernel(pcl::PointNormal& p, const kernel::base& k)
  {
    pclPointFromNukleiKernel(p, dynamic_cast<const kernel::r3xs2p&>(k));
  }
  
  
  /**
   *
   *
   * Warning: converts to axial orientation. 
   */
  void pclPointFromNukleiKernel(pcl::PointXYZRGBNormal& p, const kernel::r3xs2p& k)
  {
    p.x = k.loc_.X();
    p.y = k.loc_.Y();
    p.z = k.loc_.Z();
    p.normal_x = k.dir_.X();
    p.normal_y = k.dir_.Y();
    p.normal_z = k.dir_.Z();
    const ColorDescriptor& d = dynamic_cast<const ColorDescriptor&>(k.getDescriptor());
    const RGBColor c(d.getColor());
    uint8_t r = c.R()*255, g = c.G()*255, b = c.B()*255;
    uint32_t rgb = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
    p.rgb = *reinterpret_cast<float*>(&rgb);
  }
  
  void pclPointFromNukleiKernel(pcl::PointXYZRGBNormal& p, const kernel::base& k)
  {
    pclPointFromNukleiKernel(p, dynamic_cast<const kernel::r3xs2p&>(k));
  }

#endif
  
}
