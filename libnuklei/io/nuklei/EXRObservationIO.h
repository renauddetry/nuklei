// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifdef NUKLEI_USE_EXR_LIB

#ifndef NUKLEI_EXROBSERVATIONSERIAL_H
#define NUKLEI_EXROBSERVATIONSERIAL_H

#include <ImfRgbaFile.h>
#include <ImfArray.h>

#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/EXRObservation.h>

namespace nuklei {


  /**
   * @author Damien Teney <damien.teney@ulg.ac.be>
   */
  class EXRReader : public ObservationReader
    {
    public:
      EXRReader(const std::string &observationFileName);
      ~EXRReader();
  
      Observation::Type type() const { return Observation::EXR; }

      void reset();
  
    protected:
      void init_();
      std::auto_ptr<Observation> readObservation_();
    private:
      Imf::Array2D<Imf::Rgba> pixels_; // Contents of input file
      int width_, height_; // Dimensions of input file
      int i_, j_; // Current position in the input file
      std::string observationFileName_;
      void backprojectPoint_(Vector3 &p);
    };

}

#endif

#endif

