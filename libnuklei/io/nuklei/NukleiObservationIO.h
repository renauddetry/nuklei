// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_NUKLEI_OBSERVATIONSERIAL_H
#define NUKLEI_NUKLEI_OBSERVATIONSERIAL_H


#include <boost/ptr_container/ptr_list.hpp>
#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/NukleiObservation.h>

namespace ticpp {
  class Document;
  class Node;
  class Element;
  template < class T > class Iterator;
}

namespace nuklei {

  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class NukleiReader : public ObservationReader
    {
    public:
      NukleiReader(const std::string &observationFileName);
      ~NukleiReader();
  
      Observation::Type type() const { return Observation::NUKLEI; }

      void reset();
  
    protected:
      void init_();
      std::auto_ptr<Observation> readObservation_();
      std::string observationFileName_;
    private:
      boost::shared_ptr<ticpp::Document> in_;
      typedef ticpp::Iterator< ticpp::Element > ElementIterator;
      boost::shared_ptr<ElementIterator> e_;
    };

  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class NukleiWriter : public ObservationWriter
    {
    public:
      NukleiWriter(const std::string &observationFileName);
      ~NukleiWriter();
  
      Observation::Type type() const { return Observation::NUKLEI; }

      void init();
      void reset();
      
      std::auto_ptr<Observation> templateObservation() const
      { return std::auto_ptr<Observation>(new NukleiObservation); }

      void writeObservation(const Observation &o);
      void writeBuffer();
      
    private:
      std::string observationFileName_;
      boost::shared_ptr<ticpp::Document> out_;
      ticpp::Element* kc_;
      coord_t totalWeight_;
    };

}

#endif

