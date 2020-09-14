// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifndef NUKLEI_PCDOBSERVATIONSERIAL_H
#define NUKLEI_PCDOBSERVATIONSERIAL_H


#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/PCDObservation.h>
#include <nuklei/SerializedKernelObservationIO.h>


namespace nuklei {


  class PCDReader : public KernelReader
    {
    public:
      PCDReader(const std::string &observationFileName);
      ~PCDReader();
  
      Observation::Type type() const { return Observation::PCD; }
    protected:
      void init_();
       NUKLEI_UNIQUE_PTR<Observation> readObservation_();
    };

  class PCDWriter : public KernelWriter
  {
  public:
    PCDWriter(const std::string &observationFileName);
    ~PCDWriter();
    
    Observation::Type type() const { return Observation::PCD; }
    
     NUKLEI_UNIQUE_PTR<Observation> templateObservation() const
    { return  NUKLEI_UNIQUE_PTR<Observation>(new PCDObservation); }

    void writeBuffer();

  };
  
}

#endif

