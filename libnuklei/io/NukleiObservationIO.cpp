// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/NukleiObservationIO.h>
#include <nuklei/NukleiObservation.h>
#ifdef NUKLEI_USE_TICPP
#define TIXML_USE_TICPP
#include "ticpp.h"
#endif

namespace nuklei {
  
  
  
  NukleiReader::NukleiReader(const std::string &observationFileName) :
  observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    in_ = boost::shared_ptr<ticpp::Document>(new ticpp::Document(observationFileName));
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
  NukleiReader::~NukleiReader()
  {
  }
  
  
  void NukleiReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    try {
      in_->LoadFile();
      ticpp::Element* kc = in_->FirstChildElement( "kernelCollection" );
      NUKLEI_ASSERT(kc != NULL);
      e_ = boost::shared_ptr<ElementIterator>(new ElementIterator( "kernel" ));
      *e_ = e_->begin(kc);
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
  void NukleiReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  std::auto_ptr<Observation> NukleiReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    if (!e_) NUKLEI_THROW("Reader does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    
    while (*e_ != e_->end())
    {
      ElementIterator kernel = *e_;
      ++*e_;
      
      std::auto_ptr<NukleiObservation> observation(new NukleiObservation);
      
      ElementIterator kernelElement;
      kernelElement = kernelElement.begin(&*kernel);
      NUKLEI_ASSERT(kernelElement != kernelElement.end());
      
      weight_t w = 1;
      
      if (kernelElement->Value() == "weight")
      {
        kernelElement->GetText(&w);
        kernelElement++; NUKLEI_ASSERT(kernelElement != kernelElement.end());
      }
      
      Vector3 loc;
      coord_t loc_h = 0;
      
      {
        NUKLEI_ASSERT(kernelElement->Value() == "location");
        ElementIterator li;
        li = li.begin(&*kernelElement);        
        NUKLEI_ASSERT(li != li.end());
        {
          ticpp::Element* el = li.Get();
          NUKLEI_ASSERT(el->Value() == "vector3");
          std::string ls;
          el->GetText(&ls);
          loc = numify<Vector3>(ls);
        }
        li++;
        if (li != li.end())
        {
          ticpp::Element* el = li.Get();
          NUKLEI_ASSERT(el->Value() == "width");
          std::string ws;
          el->GetText(&ws);
          loc_h = numify<coord_t>(ws);
          li++;
          NUKLEI_ASSERT(li == li.end());
        }
      }
      
      kernel::base::ptr k;
      
      kernelElement++;
      
      if (kernelElement != kernelElement.end())
      {
        NUKLEI_ASSERT(kernelElement->Value() == "orientation");
        std::string domain = kernelElement->GetAttribute("domain");
        // domain == "se3" is to support files wirtten with a buggy Nuklei build.
        if (domain == "se3" || domain == "so3")
        {
          kernel::se3 se3k;
          se3k.loc_ = loc;
          se3k.loc_h_ = loc_h;
          
          ElementIterator oi;
          oi = oi.begin(&*kernelElement);
          NUKLEI_ASSERT(oi != oi.end());
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "quaternion");
            std::string os;
            el->GetText(&os);
            se3k.ori_ = la::normalized(numify<Quaternion>(os));
          }
          oi++;
          if (oi != oi.end())
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "width");
            std::string ws;
            el->GetText(&ws);
            se3k.ori_h_ = numify<coord_t>(ws);
            oi++;
            NUKLEI_ASSERT(oi == oi.end());
          }
          k.reset(new kernel::se3(se3k));
        }
        else if (domain == "s2p")
        {
          kernel::r3xs2p r3xs2pk;
          r3xs2pk.loc_ = loc;
          r3xs2pk.loc_h_ = loc_h;
          
          ElementIterator oi;
          oi = oi.begin(&*kernelElement);
          NUKLEI_ASSERT(oi != oi.end());
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "vector3");
            std::string os;
            el->GetText(&os);
            r3xs2pk.dir_ = la::normalized(numify<Vector3>(os));
          }
          oi++;
          if (oi != oi.end())
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "width");
            std::string ws;
            el->GetText(&ws);
            r3xs2pk.dir_h_ = numify<coord_t>(ws);
            oi++;
            NUKLEI_ASSERT(oi == oi.end());
          }
          k.reset(new kernel::r3xs2p(r3xs2pk));
        }
        else if (domain == "s2")
        {
          kernel::r3xs2 r3xs2k;
          r3xs2k.loc_ = loc;
          r3xs2k.loc_h_ = loc_h;
          
          ElementIterator oi;
          oi = oi.begin(&*kernelElement);
          NUKLEI_ASSERT(oi != oi.end());
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "vector3");
            std::string os;
            el->GetText(&os);
            r3xs2k.dir_ = la::normalized(numify<Vector3>(os));
          }
          oi++;
          if (oi != oi.end())
          {
            ticpp::Element* el = &*oi;
            NUKLEI_ASSERT(el->Value() == "width");
            std::string ws;
            el->GetText(&ws);
            r3xs2k.dir_h_ = numify<coord_t>(ws);
            oi++;
            NUKLEI_ASSERT(oi == oi.end());
          }
          k.reset(new kernel::r3xs2(r3xs2k));
        }
      }
      else
      {
        kernel::r3 r3k;
        r3k.loc_ = loc;
        r3k.loc_h_ = loc_h;
        k.reset(new kernel::r3(r3k));
      }
      k->setWeight(w);
      
      //TwoFingerDescriptor d;
      //k->setDescriptor(d);
      
      observation->setKernel(*k);
      
      oc.incLabel("input");
      
      return std::auto_ptr<Observation>(observation);
    }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  NukleiWriter::NukleiWriter(const std::string &observationFileName) :
  observationFileName_(observationFileName), totalWeight_(-1)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  NukleiWriter::~NukleiWriter()
  {
  }
  
  void NukleiWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    totalWeight_ = -1;
    try {
      out_.reset(new ticpp::Document(observationFileName_));
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
    ticpp::Declaration dec("1.0", "UTF-8", "");
    out_->InsertEndChild(dec);
    ticpp::Element kc;
    kc.SetValue( "kernelCollection" );
    kc.SetAttribute( "version", "1.0" );
    kc_ = out_->InsertEndChild(kc)->ToElement();
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
  void NukleiWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  void NukleiWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    out_->SaveFile();
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
#ifdef NUKLEI_USE_TICPP
  static ticpp::Element* append(ticpp::Element* parent, const std::string& value)
  {
    NUKLEI_TRACE_BEGIN();
    ticpp::Element child(value);
    NUKLEI_ASSERT(parent != NULL);
    return parent->InsertEndChild(child)->ToElement();
    NUKLEI_TRACE_END();
  }
#endif

  void NukleiWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    if (!out_) NUKLEI_THROW("Writer does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    const Observation& observation = dynamic_cast<const Observation&>(o);
    
    kernel::base::ptr k = observation.getKernel();
    
    ElementPtr kernel = append(kc_, "kernel");
    
    if (totalWeight_ == -1) totalWeight_ = k->getWeight();
    else totalWeight_ += k->getWeight();
    
    {
      ElementPtr w = append(kernel, "weight");
      w->SetText(stringify(k->getWeight(), PRECISION));
    }
    {
      ElementPtr loc = append(kernel, "location");
      loc->SetAttribute("domain", "r3");
      ElementPtr v3 = append(loc, "vector3");
      v3->SetText(stringify(k->getLoc(), PRECISION));
      ElementPtr width = append(loc, "width");
      width->SetText(stringify(k->getLocH(), PRECISION));
    }
    
    if (k->polyType() == kernel::base::SE3)
    {
      const kernel::se3& se3k = dynamic_cast<const kernel::se3&>(*k);
      ElementPtr ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "so3");
      ElementPtr q = append(ori, "quaternion");
      q->SetAttribute("format", "wxyz");
      q->SetText(stringify(se3k.ori_, PRECISION));
      ElementPtr width = append(ori, "width");
      width->SetText(stringify(se3k.ori_h_, PRECISION));
    }
    else if (k->polyType() == kernel::base::R3XS2P)
    {
      const kernel::r3xs2p& r3xs2pk = dynamic_cast<const kernel::r3xs2p&>(*k);
      ElementPtr ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "s2p");
      ElementPtr q = append(ori, "vector3");
      q->SetText(stringify(r3xs2pk.dir_, PRECISION));
      ElementPtr width = append(ori, "width");
      width->SetText(stringify(r3xs2pk.dir_h_, PRECISION));
    }
    else if (k->polyType() == kernel::base::R3XS2)
    {
      const kernel::r3xs2& r3xs2k = dynamic_cast<const kernel::r3xs2&>(*k);
      ElementPtr ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "s2");
      ElementPtr q = append(ori, "vector3");
      q->SetText(stringify(r3xs2k.dir_, PRECISION));
      ElementPtr width = append(ori, "width");
      width->SetText(stringify(r3xs2k.dir_h_, PRECISION));
    }
#else
    NUKLEI_THROW("This function requires TICPP.");
#endif
    NUKLEI_TRACE_END();
  }
  
}
