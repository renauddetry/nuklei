// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>

#include <nuklei/TxtObservationIO.h>
#include <nuklei/TxtObservation.h>

namespace nuklei {



  TxtReader::TxtReader(const std::string &observationFileName) :
    observationFileName(observationFileName)
  {
  }

  TxtReader::~TxtReader()
  {
  }



  void TxtReader::init_()
  {
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName + " for reading.");    

    std::string line;
    int ntok = -1;
    while (std::getline(in_, line))
    {
      std::vector<std::string> tokens;
      boost::split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
      if (tokens.front() == "")
        tokens.erase(tokens.begin());
      if (tokens.back() == "")
        tokens.pop_back();
      if (tokens.size() == 0)
      {
        if (in_.good())
          throw ObservationIOError("Unexpected empty line.");
      }
      if (tokens.size() == 3)
      {
        if (ntok == -1) ntok = 3;
        else if (ntok != 3) throw ObservationIOError("Wrong number of tokens.");
      }
      else if (tokens.size() == 7)
      {
        if (ntok == -1) ntok = 7;
        else if (ntok != 7) throw ObservationIOError("Wrong number of tokens.");
      }
      else {
        throw ObservationIOError("Wrong number of tokens on line");
      }
    }
    
    in_.close();
    in_.open(observationFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                               observationFileName + " for reading.");
  }


  void TxtReader::reset()
  {
    in_.close();
    init();
  }


  std::auto_ptr<Observation> TxtReader::readObservation_()
  {
    if (!in_.is_open()) NUKLEI_THROW("Reader does not seem inited.");

    if (!in_.good()) return std::auto_ptr<Observation>();
    
    std::string line;
    
    while (std::getline(in_, line))
    {
      std::auto_ptr<TxtObservation> observation(new TxtObservation);

      std::vector<std::string> tokens;
      boost::split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
      if (tokens.front() == "")
        tokens.erase(tokens.begin());
      if (tokens.back() == "")
        tokens.pop_back();
      if (tokens.size() == 0)
      {
        // end of file
      }
      if (tokens.size() == 3)
      {
        kernel::r3 k;
        k.loc_ = Vector3(numify<double>(tokens.at(0)), numify<double>(tokens.at(1)), numify<double>(tokens.at(2)));
        observation->setKernel(k);
      }
      else if (tokens.size() == 7)
      {
        kernel::se3 k;
        k.loc_ = Vector3(numify<double>(tokens.at(0)), numify<double>(tokens.at(1)), numify<double>(tokens.at(2)));
        k.ori_ = la::normalized(Quaternion(numify<double>(tokens.at(3)),
                                           numify<double>(tokens.at(4)),
                                           numify<double>(tokens.at(5)),
                                           numify<double>(tokens.at(6))
                                           )
                                );
        observation->setKernel(k);
      }
      else {
        NUKLEI_THROW("Wrong number of tokens on line");
      }
      
      return std::auto_ptr<Observation>(observation);
    }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
  }

  
  
  
  TxtWriter::TxtWriter(const std::string &observationFileName) :
  observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  TxtWriter::~TxtWriter()
  {
  }
  
  void TxtWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      points_.clear();
    } catch (std::exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }
  
  void TxtWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  
  void TxtWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    
    std::ofstream ofs(observationFileName_.c_str());
    for (KernelCollection::iterator i = points_.begin(); i != points_.end(); ++i)
    {
      {
        kernel::r3* k = dynamic_cast<kernel::r3*>(&*i);
        if (k != NULL)
        {
          ofs << stringify(k->loc_, PRECISION) << std::endl;
          continue;
        }
      }
      {
        kernel::se3* k = dynamic_cast<kernel::se3*>(&*i);
        if (k != NULL)
        {
          ofs << stringify(k->loc_, PRECISION) << " " << stringify(k->ori_, PRECISION) << std::endl;
          continue;
        }
      }
      NUKLEI_THROW("Unsupported kernel.");
    }
    
    NUKLEI_TRACE_END();
  }
  
  void TxtWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    
    kernel::base::ptr k = o.getKernel();
    
    points_.add(*k);
    
    NUKLEI_TRACE_END();
  }
  
  
  
  
}
