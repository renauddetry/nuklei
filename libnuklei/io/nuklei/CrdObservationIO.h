// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_CRDOBSERVATIONSERIAL_H
#define NUKLEI_CRDOBSERVATIONSERIAL_H


#include <nuklei/Definitions.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/CrdObservation.h>

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
  class CrdReader : public ObservationReader
    {
    public:
      CrdReader(const std::string &observationFileName);
      ~CrdReader();
  
      Observation::Type type() const { return Observation::CRD; }

      void reset();
  
    protected:
      void init_();
      std::auto_ptr<Observation> readObservation_();
    private:
      std::ifstream in_;
      std::string observationFileName;
    };

  
  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class CrdWriter : public ObservationWriter
  {
  public:
    CrdWriter(const std::string &observationFileName, bool syncpc = false);
    ~CrdWriter();
    
    Observation::Type type() const { return Observation::CRD; }
    
    void init();
    void reset();
    
    std::auto_ptr<Observation> templateObservation() const
    { return std::auto_ptr<Observation>(new CrdObservation); }
    
    void writeObservation(const Observation &o);
    void writeBuffer();
    
  private:
    std::string observationFileName_;
    bool syncpc_; // write "syncpc" on the first line.
    std::vector<Vector3> points_;
  };
  

}

#endif

