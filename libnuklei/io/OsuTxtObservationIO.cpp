// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/OsuTxtObservationIO.h>
#include <nuklei/OsuTxtObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>


#ifdef NUKLEI_USE_CIMG
#include "CImg.h"
#endif

namespace nuklei {



  OsuTxtReader::OsuTxtReader(const std::string &observationArgs) :
    rows_(0), columns_(0),
    currentIndex_(0)
  {
    size_t pos = observationArgs.find(':');
    if (pos == std::string::npos)
      geometryFileName = observationArgs;
    else
    {
      geometryFileName = observationArgs.substr(0, pos);
      appFileName = observationArgs.substr(pos+1);
    }
  }

  OsuTxtReader::~OsuTxtReader()
  {
  }



  void OsuTxtReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    
#ifdef NUKLEI_USE_CIMG
    
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(geometryFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   geometryFileName + " for reading.");
    try {
      if ( !(in_ >> rows_ >> Match("rows") >> columns_ >> Match("columns")) )
        throw ObservationIOError("Non-OsuTxt format.");
      if ( !(in_ >> Match("pixels") >> Match("(flag") >> Match("X") >>
                  Match("Y") >> Match("Z):")) )
        throw ObservationIOError("Non-OsuTxt format.");
    } catch (Error &e) {
        throw ObservationIOError("Non-OsuTxt format.");
    }
    flags_.clear();
    x_.clear();
    y_.clear();
    z_.clear();
    
    for (unsigned i = 0; i < rows_*columns_; ++i)
    {
      // Why does this not work?:
      // in_>>flags_[i];
    
      bool b;
      NUKLEI_ASSERT(in_>>b);
      flags_.push_back(b);
    }
    for (unsigned i = 0; i < rows_*columns_; ++i)
    {
      coord_t c;
      NUKLEI_ASSERT(in_>>c);
      x_.push_back(c);
    }
    for (unsigned i = 0; i < rows_*columns_; ++i)
    {
      coord_t c;
      NUKLEI_ASSERT(in_>>c);
      y_.push_back(c);
    }
    for (unsigned i = 0; i < rows_*columns_; ++i)
    {
      coord_t c;
      NUKLEI_ASSERT(in_>>c);
      z_.push_back(c);
    }
    
    rgb_.clear();
    if (!appFileName.empty())
    {
      try {
        cimg_library::CImg<unsigned char> img(appFileName.c_str());
        for (unsigned r = 0; r < rows_; r++)
          for (unsigned c = 0; c < columns_; c++)
          {
            Vector3 rgb(double(img(c*2,r*2,0,0))/255,
                        double(img(c*2,r*2,0,1))/255,
                        double(img(c*2,r*2,0,2))/255);
            rgb_.push_back(rgb);
          }
      } catch (cimg_library::CImgException &e) {
        NUKLEI_THROW("CImg error: " << e.what());
      }
    }
    else
    {
      for (unsigned r = 0; r < rows_; r++)
        for (unsigned c = 0; c < columns_; c++)
        {
          Vector3 rgb(0,0,0);
          rgb_.push_back(rgb);
        }
    }
    NUKLEI_ASSERT(in_.peek() != EOF);
    in_.close();
    currentIndex_ = 0;
    
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif
    
    NUKLEI_TRACE_END();
  }


  void OsuTxtReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }


  std::auto_ptr<Observation> OsuTxtReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (rows_ == 0 || columns_ == 0) NUKLEI_THROW("Reader does not seem inited.");
    
    for (;;)
    {
      if (currentIndex_ >= rows_*columns_)
        return std::auto_ptr<Observation>();
      
      unsigned index = currentIndex_;
      currentIndex_++;
      
      if (flags_[index] == false) continue;
      
      std::auto_ptr<OsuTxtObservation> observation(new OsuTxtObservation);

      Vector3 loc(x_[index], y_[index], z_[index]);
      observation->setLoc(loc);
      RGBColor c(rgb_[index]);
      observation->setColor(c);
            
      return std::auto_ptr<Observation>(observation);
    }
    
    return std::auto_ptr<Observation>();
    NUKLEI_TRACE_END();
  }

}
