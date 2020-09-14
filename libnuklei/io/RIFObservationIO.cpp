// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/RIFObservationIO.h>
#include <nuklei/RIFObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>

namespace nuklei {



  RIFReader::RIFReader(const std::string &geometryFileName) :
    geometryFileName_(geometryFileName), rows_(0), columns_(0),
    currentIndex_(0)
  {
  }

  RIFReader::~RIFReader()
  {
  }



  void RIFReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(geometryFileName_.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   geometryFileName_ + " for reading.");
    try {
      std::string line;
      while (std::getline(in_, line))
        if (cleanLine(line) == "|") break;
      NUKLEI_ASSERT(line == "|");
      rows_ = columns_ = -1;
      if ( !(in_ >> Match("float") >> Match("ascii") >> rows_ >> columns_) )
        throw ObservationIOError("Non-RIF format.");
      NUKLEI_ASSERT(rows_ >= 0 && columns_ >= 0);
    } catch (Error &e) {
        throw ObservationIOError("Non-RIF format.");
    }
    flags_.clear();
    x_.clear();
    y_.clear();
    z_.clear();
    
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
    for (unsigned i = 0; i < rows_*columns_; ++i)
    {
      // Why does this not work?:
      // in_>>flags_[i];
    
      bool b;
      NUKLEI_ASSERT(in_>>b);
      flags_.push_back(b);
    }
    
    rgb_.clear();
    {
      for (unsigned r = 0; r < rows_; r++)
        for (unsigned c = 0; c < columns_; c++)
        {
          Vector3 rgb(.5,.5,.5);
          rgb_.push_back(rgb);
        }
    }
    in_.close();
    currentIndex_ = 0;
    NUKLEI_TRACE_END();
  }


  void RIFReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }


   NUKLEI_UNIQUE_PTR<Observation> RIFReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (rows_ == 0 || columns_ == 0) NUKLEI_THROW("Reader does not seem inited.");
    
    for (;;)
    {
      if (currentIndex_ >= rows_*columns_)
        return  NUKLEI_UNIQUE_PTR<Observation>();
      
      unsigned index = currentIndex_;
      currentIndex_++;
      
      if (flags_[index] == true) continue;
      
       NUKLEI_UNIQUE_PTR<RIFObservation> observation(new RIFObservation);

      Vector3 loc(x_[index], y_[index], z_[index]);
      observation->setLoc(loc);
      RGBColor c(rgb_[index]);
      observation->setColor(c);
            
      return  NUKLEI_UNIQUE_PTR<Observation>(observation);
    }
    
    return  NUKLEI_UNIQUE_PTR<Observation>();
    NUKLEI_TRACE_END();
  }

}
