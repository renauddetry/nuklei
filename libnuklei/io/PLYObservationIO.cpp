// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>

#include <nuklei/PLYObservationIO.h>
#include <nuklei/PLYObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>

namespace nuklei {



  PLYReader::PLYReader(const std::string &observationFileName) :
    index_(-1), n_(-1), observationFileName_(observationFileName)
  {
  }

  PLYReader::~PLYReader()
  {
  }



  void PLYReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName_.c_str(), std::ios::in);
    std::string dump;
    n_ = -1;
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName_ + " for reading.");
    try {
      // Note: we only read PLY that *begin* with verticies.

      
      std::string line;
      if (!std::getline(in_, line) || cleanLine(line) != "ply")
        throw ObservationIOError("Non-PLY format (PLY must start with a line `ply'.");
      
      bool format = false, px = false, py = false, pz = false, header = false;
      while (std::getline(in_, line))
      {
        cleanLine(line);
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
        if (tokens.front() == "")
          tokens.erase(tokens.begin());
        if (tokens.back() == "")
          tokens.pop_back();
        
        if (tokens.size() == 1 && tokens.front() == "end_header")
        {
          header = true;
          break;
        }
        
        if (tokens.size() == 0) continue;
        
        if (tokens.front() == "comment") continue;
        else if (line == "format ascii 1.0") format = true;
        else if (tokens.front() == "element" && tokens.size() >= 3 &&
            tokens.at(1) == "vertex")
        {
          n_ = numify<int>(tokens.at(2));
        }
        else if (tokens.front() == "property" && tokens.size() >= 3 &&
            tokens.at(2) == "x")
          px = true;
        else if (tokens.front() == "property" && tokens.size() >= 3 &&
            tokens.at(2) == "y")
          py = true;
        else if (tokens.front() == "property" && tokens.size() >= 3 &&
            tokens.at(2) == "z")
          pz = true;
      }
      if ( n_ < 0 || !header )
        throw ObservationIOError("Non-PLY format.");
      
      if (!format || !px || !py || !pz)
        NUKLEI_WARN("Unsupported PLY header. PLY parsing may not work as expected.");
      
    } catch (Error &e) {
      throw ObservationIOError("Non-PLY format.");
    }
    index_ = 0;
    NUKLEI_TRACE_END();
  }


  void PLYReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }


  NUKLEI_UNIQUE_PTR<Observation> PLYReader::readObservation_()
  {
    if (!in_.is_open()) NUKLEI_THROW("Reader does not seem inited.");

    if (index_ == n_)
    {
      // We don't check that we reached the EOF because
      // there may be more data following (e.g. triangles).
      return NUKLEI_UNIQUE_PTR<Observation>();
    }
    index_++;
    
    std::string line;
    
    NUKLEI_ASSERT(std::getline(in_, line));
    cleanLine(line);

    std::istringstream iss(line);
    
    NUKLEI_UNIQUE_PTR<PLYObservation> observation(new PLYObservation);
    
    Vector3 loc;
    NUKLEI_ASSERT(iss >> loc[0] >> loc[1] >> loc[2]);
    observation->setLoc(loc);
    
    RGBColor c(.5,.5,.5);
    observation->setColor(c);
    
    return NUKLEI_UNIQUE_PTR<Observation>(NUKLEI_MOVE(observation));
  }

  
  
  
  PLYWriter::PLYWriter(const std::string &observationFileName) :
  observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  PLYWriter::~PLYWriter()
  {
  }
  
  void PLYWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      points_.clear();
    } catch (std::exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }
  
  void PLYWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  
  void PLYWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    
    std::ofstream ofs(observationFileName_.c_str());
    ofs << "ply\nformat ascii 1.0\nelement vertex " << points_.size() <<
      "\nproperty float x\nproperty float y\nproperty float z\nend_header\n";
    for (std::vector<Vector3>::const_iterator i = points_.begin();
         i != points_.end(); ++i)
      ofs << stringify(*i, PRECISION) << std::endl;
    
    NUKLEI_TRACE_END();
  }
    
  void PLYWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
        
    kernel::base::ptr k = o.getKernel();
    
    points_.push_back(k->getLoc());

    NUKLEI_TRACE_END();
  }
  
  
  
}
