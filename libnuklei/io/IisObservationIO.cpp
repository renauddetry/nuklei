// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/IisObservationIO.h>
#include <nuklei/IisObservation.h>
#define TIXML_USE_TICPP
#include "ticpp.h"

namespace nuklei {
  
  
  
  IisReader::IisReader(const std::string &observationFileName) :
  observationFileName_(observationFileName), in_(new ticpp::Document(observationFileName))
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  IisReader::~IisReader()
  {
  }
  
  
  void IisReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      in_->LoadFile();
      ticpp::Element* kc = in_->FirstChildElement( "grasps" );
      NUKLEI_ASSERT(kc != NULL);
      e_ = boost::shared_ptr<ElementIterator>(new ElementIterator( "grasp" ));
      *e_ = e_->begin(kc);
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }
  
  void IisReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  std::auto_ptr<Observation> IisReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (!e_) NUKLEI_THROW("Reader does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    
    while (*e_ != e_->end())
    {
      ElementIterator grasp = *e_;
      ++*e_;
      
      std::auto_ptr<IisObservation> observation(new IisObservation);
      
      ElementIterator graspElement;
      graspElement = graspElement.begin(&*grasp);
      NUKLEI_ASSERT(graspElement != graspElement.end());
      
      
      
      while (graspElement->Value() != "pose")
      {
        if (graspElement == graspElement.end())
          NUKLEI_THROW("Pose element not found.");
        graspElement++;
      }
      
      kernel::se3 k;
      
      {
        ElementIterator pi;
        pi = pi.begin(&*graspElement);        
        NUKLEI_ASSERT(pi != pi.end());
        {
          ticpp::Element* el = pi.Get();
          NUKLEI_ASSERT(el->Value() == "position");
          NUKLEI_ASSERT(el->GetAttribute("domain") == "R3");
          el = el->FirstChildElement("euclidean");
          std::string ls;
          el->GetText(&ls);
          k.loc_ = numify<Vector3>(ls);
        }
        pi++;
        NUKLEI_ASSERT(pi != pi.end())
        {
          ticpp::Element* el = pi.Get();
          NUKLEI_ASSERT(el->Value() == "orientation");
          NUKLEI_ASSERT(el->GetAttribute("domain") == "SO3");
          
          ticpp::Element* quat = el->FirstChildElement("quaternion", false);
          if (quat != NULL)
          {
            NUKLEI_ASSERT(quat->GetAttribute("format") == "wxyz");
            std::string os;
            quat->GetText(&os);
            k.ori_ = la::normalized(numify<Quaternion>(os));
          }
          else
          {
            ticpp::Element* mat = el->FirstChildElement("rotmatrix");
            std::string os;
            mat->GetText(&os);
            std::istringstream iss(os);
            Matrix3 m;
            NUKLEI_ASSERT(iss
                          >> m(0,0) >> m(0,1) >> m(0,2)
                          >> m(1,0) >> m(1,1) >> m(1,2)
                          >> m(2,0) >> m(2,1) >> m(2,2));
            k.ori_ = la::quaternionCopy(la::normalized(m));
          }
        }
      }
      
      observation->setKernel(k);
      
      oc.incLabel("input");
      
      return std::auto_ptr<Observation>(observation);
    }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
    NUKLEI_TRACE_END();
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  IisWriter::IisWriter(const std::string &observationFileName) :
  observationFileName_(observationFileName), totalWeight_(-1)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  IisWriter::~IisWriter()
  {
  }
  
  void IisWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    totalWeight_ = -1;
    try {
      out_.reset(new ticpp::Document(observationFileName_));
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
    ticpp::Declaration dec("1.0", "UTF-8", "");
    out_->InsertEndChild(dec);
    ticpp::Element kc;
    kc.SetValue( "grasps" );
    kc.SetAttribute( "xmlns", "http://iis.uibk.ac.at/ns/grasping" );
    kc_ = out_->InsertEndChild(kc)->ToElement();
    NUKLEI_TRACE_END();
  }
  
  void IisWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  void IisWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    out_->SaveFile();
    NUKLEI_TRACE_END();
  }
  
  static ticpp::Element* append(ticpp::Element* parent, const std::string& value)
  {
    NUKLEI_TRACE_BEGIN();
    ticpp::Element child(value);
    NUKLEI_ASSERT(parent != NULL);
    return parent->InsertEndChild(child)->ToElement();
    NUKLEI_TRACE_END();
  }
  
  void IisWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    if (!out_) NUKLEI_THROW("Writer does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    const Observation& observation = dynamic_cast<const Observation&>(o);
    
    kernel::base::ptr k = observation.getKernel();
    const kernel::se3& se3k = dynamic_cast<const kernel::se3&>(*k);

    ElementPtr grasp = append(kc_, "grasp");
    
    {
      ElementPtr pose = append(grasp, "pose");
      
      ElementPtr position = append(pose, "position");
      position->SetAttribute("domain", "R3");
      ElementPtr euclidean = append(position, "euclidean");
      euclidean->SetText(stringify(se3k.getLoc(), PRECISION));

      ElementPtr orientation = append(pose, "orientation");
      orientation->SetAttribute("domain", "SO3");
      ElementPtr q = append(orientation, "quaternion");
      q->SetAttribute("format", "wxyz");
      q->SetText(stringify(se3k.ori_, PRECISION));
      ElementPtr rotmatrix = append(orientation, "rotmatrix");
      std::ostringstream oss;
      Matrix3 m = la::matrixCopy(se3k.ori_);
      oss.precision(PRECISION);
      oss << m(0,0) << " " << m(0,1) << " " << m(0,2)
      << " " << m(1,0) << " " << m(1,1) << " " << m(1,2)
      << " " << m(2,0) << " " << m(2,1) << " " << m(2,2);
      rotmatrix->SetText(oss.str());
    }
    
    NUKLEI_TRACE_END();
  }
  
}
