// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/BuiltinVTKObservationIO.h>
#include <nuklei/BuiltinVTKObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>

namespace nuklei {



  BuiltinVTKReader::BuiltinVTKReader(const std::string &observationFileName) :
    idx_(-1), n_(0), observationFileName_(observationFileName)
  {
  }

  BuiltinVTKReader::~BuiltinVTKReader()
  {
  }



  void BuiltinVTKReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName_.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName_ + " for reading.");
    try {
      std::string dump;
      std::getline(in_, dump);
      std::getline(in_, dump);
      if ( !(in_ >> Match("ASCII") >> Match("DATASET") >> Match("POLYDATA") ) )
        throw ObservationIOError("Non-OsuTxt format.");
      if ( !(in_ >> Match("POINTS") >> n_ >> Match("float") ) )
        throw ObservationIOError("Non-OsuTxt format.");
    } catch (std::exception &e) {
        throw ObservationIOError("Non-OsuTxt format.");
    }
    idx_ = 0;
    NUKLEI_TRACE_END();
  }


  void BuiltinVTKReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    in_.close();
    idx_ = -1;
    n_ = 0;
    init();
    NUKLEI_TRACE_END();
  }


  std::auto_ptr<Observation> BuiltinVTKReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (idx_ < 0) NUKLEI_THROW("Reader does not seem inited.");

    if (idx_ >= n_)
      return std::auto_ptr<Observation>();
    
    idx_++;
    
    Vector3 loc;
    NUKLEI_ASSERT(in_ >> loc[0] >> loc[1] >> loc[2]);
          
    std::auto_ptr<BuiltinVTKObservation> observation(new BuiltinVTKObservation);
    observation->setLoc(loc);
    
    return std::auto_ptr<Observation>(observation);
    
    NUKLEI_TRACE_END();
  }


}
