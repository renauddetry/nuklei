// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/CrdObservationIO.h>
#include <nuklei/CrdObservation.h>

namespace nuklei {



  CrdReader::CrdReader(const std::string &observationFileName) :
    observationFileName(observationFileName)
  {
  }

  CrdReader::~CrdReader()
  {
  }



  void CrdReader::init_()
  {
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName + " for reading.");
    
    int count = -1;
    bool syncpc = false;
    // In a CRD file, the first line shows the number of points in the file.
    // To be compatible with SYNCPC, if the first line is syncpc, it is ignored.
    {
      std::string firstline;
      if (! std::getline(in_, firstline) )
        throw ObservationIOError("Input does not look like CRD (file read error).");
      if (firstline == "syncpc")
      {
        if (! std::getline(in_, firstline) )
          throw ObservationIOError("Input does not look like CRD (file read error).");
        syncpc = true;
      }
      count = numify<int>(firstline);
      if (!(count >= 0))
        throw ObservationIOError("Input does not look like CRD (no point count).");
    }
    //if (! (in_ >> count) || ! (count >= 0) )
    //  throw ObservationIOError("Input does not look like CRD.");
    in_.close();
    
    // Since starting with an int is a pretty weak file signature,
    // we run a thourough line count.
    
    in_.open(observationFileName.c_str(), std::ios::in);
    int lcount = 0; std::string dump;
    while (std::getline(in_, dump)) lcount++;
    if (lcount != count+1 + (syncpc?1:0))
      throw ObservationIOError("Input does not look like CRD (wrong count).");
    in_.close();
    
    in_.open(observationFileName.c_str(), std::ios::in);
    std::string line;
    std::getline(in_, line);
    if (syncpc) std::getline(in_, line);
  }


  void CrdReader::reset()
  {
    in_.close();
    init();
  }


  std::auto_ptr<Observation> CrdReader::readObservation_()
  {
    if (!in_.is_open()) NUKLEI_THROW("Reader does not seem inited.");

    if (!in_.good()) return std::auto_ptr<Observation>();
    
    std::string line;
    
    while (std::getline(in_, line))
    {
      std::istringstream iss(line);
      
      std::auto_ptr<CrdObservation> observation(new CrdObservation);
      
      Vector3 loc;
      if ( ! (iss >> loc[0] >> loc[1] >> loc[2]) ) return std::auto_ptr<Observation>();
      observation->setLoc(loc);
      
      Vector3 rgbColor(.5,.5,.5);
      coord_t ix(0), iy(0);
      if ( (iss >> ix >> iy >> rgbColor[0] >> rgbColor[1] >> rgbColor[2]) )
      {
        RGBColor c(rgbColor/255.);
        observation->setColor(c);
      }
      else
      {
        RGBColor c(.5,.5,.5);
        observation->setColor(c);
      }
      
      return std::auto_ptr<Observation>(observation);
    }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
  }

  
  
  
  CrdWriter::CrdWriter(const std::string &observationFileName, bool syncpc) :
  observationFileName_(observationFileName), syncpc_(syncpc)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  CrdWriter::~CrdWriter()
  {
  }
  
  void CrdWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      points_.clear();
    } catch (std::exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }
  
  void CrdWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  
  void CrdWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    
    std::ofstream ofs(observationFileName_.c_str());
    if (syncpc_)
    {
      ofs << "syncpc\n";
    }
    ofs << points_.size() << "\n";
    for (std::vector<Vector3>::const_iterator i = points_.begin();
         i != points_.end(); ++i)
      ofs << stringify(*i, PRECISION) << std::endl;
    
    NUKLEI_TRACE_END();
  }
  
  void CrdWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    
    kernel::base::ptr k = o.getKernel();
    
    points_.push_back(k->getLoc());
    
    NUKLEI_TRACE_END();
  }
  
  
  
  
}
