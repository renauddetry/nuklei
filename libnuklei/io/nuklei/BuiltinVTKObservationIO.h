// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_BUILTINVTKOBSERVATIONSERIAL_H
#define NUKLEI_BUILTINVTKOBSERVATIONSERIAL_H


#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/OsuTxtObservation.h>


namespace nuklei {


  class BuiltinVTKReader : public ObservationReader
    {
    public:
      BuiltinVTKReader(const std::string &observationFileName);
      ~BuiltinVTKReader();
  
      Observation::Type type() const { return Observation::BUILTINVTK; }

      void reset();
  
    protected:
      void init_();
      std::auto_ptr<Observation> readObservation_();
    private:
      std::ifstream in_;
      int idx_;
      int n_;
      std::string observationFileName_;
    };


}

#endif

