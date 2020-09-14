// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/OffObservationIO.h>
#include <nuklei/OffObservation.h>

namespace nuklei {



  OffReader::OffReader(const std::string &observationFileName) :
    observationFileName(observationFileName), count_(0)
  {
  }

  OffReader::~OffReader()
  {
  }



  void OffReader::init_()
  {
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName + " for reading.");
    
    int count = -1;
    {
      std::string firstline;
      if (! std::getline(in_, firstline) )
        throw ObservationIOError("Input does not look like OFF (file read error).");
      cleanLine(firstline);
      if (firstline != "OFF")
        throw ObservationIOError("Input does not look like OFF (marker error).");
      if (! std::getline(in_, firstline) )
        throw ObservationIOError("Input does not look like OFF (file read error).");
      try {
        count = numify<int>(firstline, false);        
      } catch (BadStrNumConversion &e)
      {
        throw ObservationIOError("Input does not look like OFF (no point count).");
      }
      if (!(count >= 0))
        throw ObservationIOError("Input does not look like OFF (no point count).");
    }
    count_ = count;
  }


  void OffReader::reset()
  {
    in_.close();
    init();
  }


  NUKLEI_UNIQUE_PTR<Observation> OffReader::readObservation_()
  {
    if (!in_.is_open()) NUKLEI_THROW("Reader does not seem inited.");

    if (!in_.good()) return NUKLEI_UNIQUE_PTR<Observation>();
    
    std::string line;
        
    if (count_ > 0)
    {
      if (!std::getline(in_, line))
      {
        throw ObservationIOError("Unexpected end of file.");
      }
      cleanLine(line);

      std::istringstream iss(line);
      
      NUKLEI_UNIQUE_PTR<OffObservation> observation(new OffObservation);
      
      Vector3 loc;
      if ( ! (iss >> loc[0] >> loc[1] >> loc[2]) )
        throw ObservationIOError("Parsing error.");
      observation->setLoc(loc);
      
      RGBColor c(.5,.5,.5);
      observation->setColor(c);
      
      count_--;
      
      return NUKLEI_UNIQUE_PTR<Observation>(NUKLEI_MOVE(observation));
    }
    
    // End of file reached.
    return NUKLEI_UNIQUE_PTR<Observation>();
  }

  
  
  
  OffWriter::OffWriter(const std::string &observationFileName) :
  observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  OffWriter::~OffWriter()
  {
  }
  
  void OffWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      points_.clear();
    } catch (std::exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }
  
  void OffWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  
  void OffWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    
    std::ofstream ofs(observationFileName_.c_str());
    ofs << "OFF\n";
    ofs << points_.size() << " 0 0\n"; // we have 0 faces
    for (std::vector<Vector3>::const_iterator i = points_.begin();
         i != points_.end(); ++i)
      ofs << stringify(*i, PRECISION) << std::endl;
    
    NUKLEI_TRACE_END();
  }
  
  void OffWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    
    kernel::base::ptr k = o.getKernel();
    
    points_.push_back(k->getLoc());
    
    NUKLEI_TRACE_END();
  }
  
  
  
  
}
