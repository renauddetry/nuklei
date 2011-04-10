// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/Descriptor.h>

namespace nuklei
{

  const std::string Color::TypeNames[] = { "rgb", "hsv", "hsvcone", "unknown" };

  //const appear_t RGBColor::MAX_DIST = 1.73205080756888 + 1e-6; //sqrt(3)
  const appear_t RGBColor::MAX_DIST = 1.73205180756888; // GCC 4.2 -frounding-math bug
  
  RGBColor::RGBColor(const Color&c)
  {
    const RGBColor* rgbc;
    const HSVColor* hsvc;
    const HSVConeColor* hsvcc;
    if ( (rgbc = dynamic_cast<const RGBColor*>(&c)) != NULL) c_ = rgbc->c_;
    else if ( (hsvc = dynamic_cast<const HSVColor*>(&c)) != NULL)
    {
      appear_t h = hsvc->H() * 180 / M_PI, s = hsvc->S(), v = hsvc->V();
      NUKLEI_RANGE_CHECK(h, 0, 360);
      
      int i; 
      appear_t f, p, q, t, hTemp; 
      
      if(s == 0.0)
      { 
        R() = G() = B() = v; 
        return; 
      }
      
      hTemp = h/60.0f; 
      i = (int)std::floor( hTemp );
      f = hTemp - i;
      p = v * ( 1 - s ); 
      q = v * ( 1 - s * f ); 
      t = v * ( 1 - s * ( 1 - f ) ); 
      
      switch( i )  
      { 
      case 0:{R() = v;G() = t;B() = p;break;} 
      case 1:{R() = q;G() = v;B() = p;break;} 
      case 2:{R() = p;G() = v;B() = t;break;} 
      case 3:{R() = p;G() = q;B() = v;break;}  
      case 4:{R() = t;G() = p;B() = v;break;} 
      default:{R() = v;G() = p;B() = q;break;} 
      }
    }
    else if ( (hsvcc = dynamic_cast<const HSVConeColor*>(&c)) != NULL)
    {
      HSVColor tmp(*hsvcc);
      RGBColor tmp2(tmp);
      c_ = tmp2.getRGB();
    }
    else NUKLEI_THROW("Unknown color type");
    assertConsistency();
  }

  void RGBColor::makeRandom()
  {
    for (int i = 0; i < 3; ++i)
      c_[i] = Random::uniform();
  }

  // This is wrong:
  //const appear_t HSVColor::MAX_DIST = 2 + 1e-6;
  const appear_t HSVColor::MAX_DIST = 2.000001; // GCC 4.2 -frounding-math bug

  HSVColor::HSVColor(const Color& c)
  {
    const RGBColor* rgbc = NULL;
    const HSVColor* hsvc = NULL;
    const HSVConeColor* hsvcc = NULL;
    if ( (rgbc = dynamic_cast<const RGBColor*>(&c)) != NULL)
    {
      appear_t rgbMin, rgbMax, delta;
      appear_t r = rgbc->R(), g = rgbc->G(), b = rgbc->B();

      rgbMin = std::min(r,std::min(g,b));
      rgbMax = std::max(r,std::max(g,b));
      delta = rgbMax - rgbMin;
      V() = rgbMax;
     
      S() = 0;
      if (rgbMax > 0)
        S() = delta / rgbMax;
      // fixme: handle floating point check here.
      NUKLEI_ASSERT( 0 <= S() && S() <= 1 );
     
      H() = 0;
     
      if (delta > 0)
      {
        if (rgbMax == r && rgbMax != g)
          H() += (g - b) / delta;
        if (rgbMax == g && rgbMax != b)
          H() += (2 + (b - r) / delta);
        if (rgbMax == b && rgbMax != r)
          H() += (4 + (r - g) / delta);
        H() *= 60;
        if (H() < 0)
          H() += 360;
      }
     
      H() *= M_PI / 180;
    }
    else if ( (hsvc = dynamic_cast<const HSVColor*>(&c)) != NULL) c_ = hsvc->c_;
    else if ( (hsvcc = dynamic_cast<const HSVConeColor*>(&c)) != NULL)
    {
      NUKLEI_ASSERT(hsvcc->W() > 0);
      V() = hsvcc->WV() / hsvcc->W();
      if (V() > 0)
      {
        S() = std::sqrt( std::pow(hsvcc->SCosH(), 2) + std::pow(hsvcc->SSinH(), 2) ) / V();
        if (hsvcc->SSinH() >= 0) H() = ACos(hsvcc->SCosH() / (V()*S()));
        else H() = 2*M_PI-ACos(hsvcc->SCosH() / (V()*S()));
      }
      else if (V() == 0)
      {
        S() = 1;
        H() = 0;
      }
      else NUKLEI_ASSERT(false);
    }
    else NUKLEI_THROW("Unknown color type");
    assertConsistency();
  }


  void HSVColor::makeRandom()
  {
    while (true)
    {
      Vector3 v(Random::uniform()*2-1, Random::uniform()*2-1, 0);
      if (dist<groupS::r3>::d(v, Vector3::ZERO) > 1.0 || dist<groupS::r3>::d(v, Vector3::ZERO) < .1)
        continue;
      
      HSVColor hsv;
      if (v.Y() >= 0) c_ = Vector3(ACos(la::normalized(v).Dot(Vector3::UNIT_X)),
                                   v.Length(),
                                   1);
      else c_ = Vector3(2*M_PI-ACos(la::normalized(v).Dot(Vector3::UNIT_X)),
                        v.Length(),
                        1);
      break;
    }
  }

  appear_t HSVColor::distanceTo(const Color& c) const
  {
    NUKLEI_ASSERT("Recently changed. Please don't use.");
    return distanceTo(dynamic_cast<const HSVColor&>(c));
  }
    
  appear_t HSVColor::distanceTo(const HSVColor& hsv) const
  {
    NUKLEI_ASSERT("Recently changed. Please don't use.");
    return (c_-hsv.c_).Length();
  }


  //const appear_t HSVConeColor::MAX_DIST = 2 + 1e-6;
  const appear_t HSVConeColor::MAX_DIST = 2.000001; // GCC 4.2 -frounding-math bug
  
  HSVConeColor::HSVConeColor(const Color& c)
  {
    const RGBColor* rgbc = NULL;
    const HSVColor* hsvc = NULL;
    const HSVConeColor* hsvcc = NULL;
    if ( (rgbc = dynamic_cast<const RGBColor*>(&c)) != NULL)
    {
      HSVColor hsv(c);
      HSVConeColor hsvCone(hsv);
      c_ = hsvCone.c_;
      valueWeight_ = hsvCone.valueWeight_;
    }
    else if ( (hsvc = dynamic_cast<const HSVColor*>(&c)) != NULL)
    {
      Vector3 ec;
      ec.X() = std::cos(hsvc->H()) * hsvc->S() * hsvc->V();
      ec.Y() = std::sin(hsvc->H()) * hsvc->S() * hsvc->V();
      NUKLEI_RANGE_CHECK(HSV_METRIC_VALUE_WEIGHT, 0, 1);
      ec.Z() = hsvc->V()*HSV_METRIC_VALUE_WEIGHT;
      c_ = ec;
      valueWeight_ = HSV_METRIC_VALUE_WEIGHT;
    }
    else if ( (hsvcc = dynamic_cast<const HSVConeColor*>(&c)) != NULL)
    {
      c_ = hsvcc->c_;
      valueWeight_ = hsvcc->valueWeight_;
    }
    else NUKLEI_THROW("Unknown color type");
    assertConsistency();
  }


  void HSVConeColor::makeRandom()
  {
    while (true)
    {
      Vector3 v(Random::uniform()*2-1, Random::uniform()*2-1, 0);
      appear_t value = Random::uniform()*HSV_METRIC_VALUE_WEIGHT;
      
      if (dist<groupS::r3>::d(v, Vector3::ZERO) > value/HSV_METRIC_VALUE_WEIGHT)
        continue;
      
      v.Z() = value;
      c_ = v;
      valueWeight_ = HSV_METRIC_VALUE_WEIGHT;
      break;
    }
  }

  appear_t HSVConeColor::distanceTo(const Color& c) const
  {
    return distanceTo(dynamic_cast<const HSVConeColor&>(c));
  }
    
  appear_t HSVConeColor::distanceTo(const HSVConeColor& hsvc) const
  {
    return (c_-hsvc.c_).Length();
  }

}
