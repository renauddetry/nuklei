// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/SerializedKernelObservationIO.h>
#include <nuklei/SerializedKernelObservation.h>
#include <nuklei/Serial.h>

namespace nuklei {



  KernelReader::KernelReader(const std::string &observationFileName) :
    observationFileName_(observationFileName), idx_(-1)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  KernelReader::~KernelReader()
  {
  }


  void KernelReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    kc_.clear();
    try {
      Serial::readObject(kc_, observationFileName_);
    } catch (SerialError& e) {
      throw ObservationIOError(e.what());
    }
    idx_ = 0;
    NUKLEI_TRACE_END();
  }

  void KernelReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }

  std::auto_ptr<Observation> KernelReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (idx_ < 0) NUKLEI_THROW("Reader does not seem inited.");
    if (idx_ >= int(kc_.size())) return std::auto_ptr<Observation>();
    else return std::auto_ptr<Observation>(new SerializedKernelObservation(kc_.at(idx_++)));
    NUKLEI_TRACE_END();
  }

  KernelWriter::KernelWriter(const std::string &observationFileName) :
    observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  KernelWriter::~KernelWriter()
  {
  }

  void KernelWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    kc_.clear();
    NUKLEI_TRACE_END();
  }

  void KernelWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }

  void KernelWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    Serial::writeObject(kc_, observationFileName_);
    NUKLEI_TRACE_END();
  }

  void KernelWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    kc_.add(*o.getKernel());
    NUKLEI_TRACE_END();
  }

}
