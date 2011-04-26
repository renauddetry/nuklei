// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_RIFOBSERVATIONSERIAL_H
#define NUKLEI_RIFOBSERVATIONSERIAL_H


#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/RIFObservation.h>


namespace nuklei {


  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class RIFReader : public ObservationReader
    {
    public:
      RIFReader(const std::string &geometryFileName);
      ~RIFReader();
  
      Observation::Type type() const { return Observation::RIF; }

      void reset();
  
    protected:
      void init_();
      std::auto_ptr<Observation> readObservation_();
    private:
      std::ifstream in_;
      std::string geometryFileName_;
      
      unsigned rows_;
      unsigned columns_;
      unsigned currentIndex_;
      
      std::vector<bool> flags_;
      std::vector<coord_t> x_;
      std::vector<coord_t> y_;
      std::vector<coord_t> z_;
      std::vector<Vector3> rgb_;
    };

}

#endif

