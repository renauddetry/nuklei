// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_IIS_OBSERVATIONSERIAL_H
#define NUKLEI_IIS_OBSERVATIONSERIAL_H


#include <boost/ptr_container/ptr_list.hpp>
#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/IisObservation.h>

namespace ticpp {
  class Document;
  class Node;
  class Element;
  template < class T > class Iterator;
}

namespace nuklei {

  class IisReader : public ObservationReader
    {
    public:
      IisReader(const std::string &observationFileName);
      ~IisReader();
  
      Observation::Type type() const { return Observation::IIS; }

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

  class IisWriter : public ObservationWriter
    {
    public:
      IisWriter(const std::string &observationFileName);
      ~IisWriter();
  
      Observation::Type type() const { return Observation::IIS; }

      void init();
      void reset();
      
      std::auto_ptr<Observation> templateObservation() const
      { return std::auto_ptr<Observation>(new IisObservation); }

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

