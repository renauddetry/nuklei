// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

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
#else
#include <tinyxml2.h>
namespace tt = tinyxml2;
#endif

namespace nuklei {
  
  
  
  NukleiReader::NukleiReader(const std::string &observationFileName) :
  observationFileName_(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    in_ = boost::shared_ptr<ticpp::Document>(new ticpp::Document(observationFileName));
#else
    in_ = boost::shared_ptr<tt::XMLDocument>(new tt::XMLDocument());
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
    in_->LoadFile(observationFileName_.c_str());
    if (in_->ErrorID() != 0)
      throw ObservationIOError("Error parsing with TinyXML-2");
    tt::XMLElement* kc = in_->FirstChildElement( "kernelCollection" );
    if (kc == NULL)
      throw ObservationIOError("Cannot find element kernelCollection");
    e_ = kc->FirstChildElement( "kernel" );
#endif
    NUKLEI_TRACE_END();
  }
  
  void NukleiReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }
  
  NUKLEI_UNIQUE_PTR<Observation> NukleiReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_TICPP
    if (!e_) NUKLEI_THROW("Reader does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    
    while (*e_ != e_->end())
    {
      ElementIterator kernel = *e_;
      ++*e_;
      
      NUKLEI_UNIQUE_PTR<NukleiObservation> observation(new NukleiObservation);
      
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
      
      return NUKLEI_UNIQUE_PTR<Observation>(NUKLEI_MOVE(observation));
    }
    
    // End of file reached.
    return NUKLEI_UNIQUE_PTR<Observation>();
#else
    if (!in_->RootElement()) NUKLEI_THROW("Reader does not seem inited.");
    
    using namespace tt;
    while (e_)
    {
      XMLElement* kernel = e_;
      e_ = kernel->NextSiblingElement( "kernel" );
      
      NUKLEI_UNIQUE_PTR<NukleiObservation> observation(new NukleiObservation);

      XMLElement* kernelIter = kernel->FirstChildElement();
      NUKLEI_ASSERT(kernelIter);
      
      weight_t w = 1;
      
      if (std::string(kernelIter->Name()) == "weight")
      {
        NUKLEI_ASSERT(kernelIter->QueryDoubleText(&w) == 0);
        kernelIter = kernelIter->NextSiblingElement();
        NUKLEI_ASSERT(kernelIter);
      }
      
      Vector3 loc;
      coord_t loc_h = 0;
      
      {
        NUKLEI_ASSERT(std::string(kernelIter->Name()) == "location");
        XMLElement* li = kernelIter->FirstChildElement();
        NUKLEI_ASSERT(li);
        {
          NUKLEI_ASSERT(std::string(li->Name()) == "vector3");
          loc = numify<Vector3>(li->GetText());
        }
        li = li->NextSiblingElement();
        if (li)
        {
          NUKLEI_ASSERT(std::string(li->Name()) == "width");
          NUKLEI_ASSERT(li->QueryDoubleText(&loc_h) == 0);
          NUKLEI_ASSERT(li->NextSiblingElement() == NULL);
        }
      }
      
      kernel::base::ptr k;
      
      kernelIter = kernelIter->NextSiblingElement();
      
      if (kernelIter)
      {
        NUKLEI_ASSERT(std::string(kernelIter->Value()) == "orientation");
        std::string domain;
        {
          const char* tmp = kernelIter->Attribute("domain");
          NUKLEI_ASSERT(tmp);
          domain = tmp;
        }
        // domain == "se3" is to support files wirtten with a buggy Nuklei build.
        if (domain == "se3" || domain == "so3")
        {
          kernel::se3 se3k;
          se3k.loc_ = loc;
          se3k.loc_h_ = loc_h;
          
          XMLElement* oi = kernelIter->FirstChildElement();
          NUKLEI_ASSERT(oi);
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "quaternion");
            se3k.ori_ = la::normalized(numify<Quaternion>(oi->GetText()));
          }
          oi = oi->NextSiblingElement();
          if (oi)
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "width");
            NUKLEI_ASSERT(oi->QueryDoubleText(&se3k.ori_h_) == 0);
            NUKLEI_ASSERT(oi->NextSiblingElement() == NULL);
          }
          
          k.reset(new kernel::se3(se3k));
        }
        else if (domain == "s2p")
        {
          kernel::r3xs2p r3xs2pk;
          r3xs2pk.loc_ = loc;
          r3xs2pk.loc_h_ = loc_h;
          
          XMLElement* oi = kernelIter->FirstChildElement();
          NUKLEI_ASSERT(oi);
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "vector3");
            r3xs2pk.dir_ = la::normalized(numify<Vector3>(oi->GetText()));
          }
          oi = oi->NextSiblingElement();
          if (oi)
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "width");
            NUKLEI_ASSERT(oi->QueryDoubleText(&r3xs2pk.dir_h_) == 0);
            NUKLEI_ASSERT(oi->NextSiblingElement() == NULL);
          }
          
          k.reset(new kernel::r3xs2p(r3xs2pk));
        }
        else if (domain == "s2")
        {
          kernel::r3xs2 r3xs2k;
          r3xs2k.loc_ = loc;
          r3xs2k.loc_h_ = loc_h;
          
          XMLElement* oi = kernelIter->FirstChildElement();
          NUKLEI_ASSERT(oi);
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "vector3");
            r3xs2k.dir_ = la::normalized(numify<Vector3>(oi->GetText()));
          }
          oi = oi->NextSiblingElement();
          if (oi)
          {
            NUKLEI_ASSERT(std::string(oi->Name()) == "width");
            NUKLEI_ASSERT(oi->QueryDoubleText(&r3xs2k.dir_h_) == 0);
            NUKLEI_ASSERT(oi->NextSiblingElement() == NULL);
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
      
      return NUKLEI_UNIQUE_PTR<Observation>(NUKLEI_MOVE(observation));
    }
    // End of file reached.
    return NUKLEI_UNIQUE_PTR<Observation>();
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
    totalWeight_ = -1;
    out_.reset(new tt::XMLDocument());
    
    out_->InsertEndChild(out_->NewDeclaration());
    
    kc_ = out_->NewElement("kernelCollection");
    kc_->SetAttribute( "version", "1.0" );
    out_->InsertEndChild(kc_);
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
    NUKLEI_ASSERT(out_->SaveFile(observationFileName_.c_str()) == 0);
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
#else
  static tt::XMLElement* append(tt::XMLElement* parent, const std::string& value)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_ASSERT(parent != NULL);
    return parent->InsertEndChild(parent->GetDocument()->NewElement(value.c_str()))->ToElement();
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
    if (!out_) NUKLEI_THROW("Writer does not seem inited.");
    
    using namespace tt;

    const Observation& observation = dynamic_cast<const Observation&>(o);
    
    kernel::base::ptr k = observation.getKernel();
    
    XMLElement* kernel = append(kc_, "kernel");

    if (totalWeight_ == -1) totalWeight_ = k->getWeight();
    else totalWeight_ += k->getWeight();
    
    {
      XMLElement* w = append(kernel, "weight");
      w->SetText(stringify(k->getWeight(), PRECISION).c_str());
    }
    {
      XMLElement* loc = append(kernel, "location");
      loc->SetAttribute("domain", "r3");
      XMLElement* v3 = append(loc, "vector3");
      v3->SetText(stringify(k->getLoc(), PRECISION).c_str());
      XMLElement* width = append(loc, "width");
      width->SetText(stringify(k->getLocH(), PRECISION).c_str());
    }
    
    if (k->polyType() == kernel::base::SE3)
    {
      const kernel::se3& se3k = dynamic_cast<const kernel::se3&>(*k);
      XMLElement* ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "so3");
      XMLElement* q = append(ori, "quaternion");
      q->SetAttribute("format", "wxyz");
      q->SetText(stringify(se3k.ori_, PRECISION).c_str());
      XMLElement* width = append(ori, "width");
      width->SetText(stringify(se3k.ori_h_, PRECISION).c_str());
    }
    else if (k->polyType() == kernel::base::R3XS2P)
    {
      const kernel::r3xs2p& r3xs2pk = dynamic_cast<const kernel::r3xs2p&>(*k);
      XMLElement* ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "s2p");
      XMLElement* q = append(ori, "vector3");
      q->SetText(stringify(r3xs2pk.dir_, PRECISION).c_str());
      XMLElement* width = append(ori, "width");
      width->SetText(stringify(r3xs2pk.dir_h_, PRECISION).c_str());
    }
    else if (k->polyType() == kernel::base::R3XS2)
    {
      const kernel::r3xs2& r3xs2k = dynamic_cast<const kernel::r3xs2&>(*k);
      XMLElement* ori = append(kernel, "orientation");
      ori->SetAttribute("domain", "s2");
      XMLElement* q = append(ori, "vector3");
      q->SetText(stringify(r3xs2k.dir_, PRECISION).c_str());
      XMLElement* width = append(ori, "width");
      width->SetText(stringify(r3xs2k.dir_h_, PRECISION).c_str());
    }
#endif
    NUKLEI_TRACE_END();
  }
  
}
