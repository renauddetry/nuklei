// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifndef NUKLEI_KERNELOBSERVATIONSERIAL_H
#define NUKLEI_KERNELOBSERVATIONSERIAL_H


#include <boost/ptr_container/ptr_list.hpp>
#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/SerializedKernelObservation.h>
#include <nuklei/KernelCollection.h>

namespace nuklei {

  class KernelReader : public ObservationReader
    {
    public:
      KernelReader(const std::string &observationFileName);
      ~KernelReader();
  
      Observation::Type type() const { return Observation::SERIAL; }

      nullable<unsigned> nObservations() const
      { NUKLEI_ASSERT(idx_ >= 0); return kc_.size(); }

      void reset();
  
    protected:
      void init_();
      NUKLEI_UNIQUE_PTR<Observation> readObservation_();
      std::string observationFileName_;
      int idx_;
      KernelCollection kc_;
    };

  class KernelWriter : public ObservationWriter
    {
    public:
      KernelWriter(const std::string &observationFileName);
      ~KernelWriter();
  
      Observation::Type type() const { return Observation::SERIAL; }

      void init();
      void reset();
      
      NUKLEI_UNIQUE_PTR<Observation> templateObservation() const
      { return NUKLEI_UNIQUE_PTR<Observation>(NUKLEI_MOVE(new SerializedKernelObservation)); }

      void writeObservation(const Observation &o);
      void writeBuffer();
      
    protected:
      std::string observationFileName_;
      KernelCollection kc_;
    };

}

#endif

