// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/CoViS3DObservationIO.h>
#include <nuklei/CoViS3DObservation.h>
#define TIXML_USE_TICPP
#include "ticpp.h"

namespace nuklei {



  CoViS3DReader::CoViS3DReader(const std::string &observationFileName) :
    observationFileName(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  CoViS3DReader::~CoViS3DReader()
  {
  }

  CoViS3DXMLReader::CoViS3DXMLReader(const std::string &observationFileName) :
    CoViS3DReader(observationFileName), in_(new ticpp::Document(observationFileName))
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  CoViS3DXMLReader::~CoViS3DXMLReader()
  {
  }

  CoViS3DWandererReader::CoViS3DWandererReader(const std::string &observationFileName) :
    CoViS3DReader(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  CoViS3DWandererReader::~CoViS3DWandererReader()
  {
  }


  void CoViS3DReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
    std::string errorsCat = std::string("Error in CoViS3DReader::init.") +
      "\nErrors at each format attempt were:";

    try {
      reader_ = boost::shared_ptr<CoViS3DReader>(new CoViS3DXMLReader(observationFileName));
      reader_->init();
      return;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }
    try {
      reader_ = boost::shared_ptr<CoViS3DReader>(new CoViS3DWandererReader(observationFileName));
      reader_->init();
      return;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }
    throw ObservationIOError(errorsCat);
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLReader::init_()
  {
    NUKLEI_TRACE_BEGIN();    
    try {
      in_->LoadFile();
      ticpp::Element* scene = in_->FirstChildElement( "Scene" );
      NUKLEI_ASSERT(scene != NULL);
      e_ = boost::shared_ptr<ElementIterator>(new ElementIterator( "Primitive3D" ));
      *e_ = e_->begin(scene);
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }

  void CoViS3DWandererReader::init_()
  {
    NUKLEI_TRACE_BEGIN();    
    NUKLEI_ASSERT(!in_.is_open());
    in_.open(observationFileName.c_str(), std::ios::in);
    if (!in_.is_open())
      throw ObservationIOError(std::string("Could not open file ") +
                                   observationFileName + " for reading.");
    int c = in_.peek();
    if (c != 'l')
    {
      in_.close();
      throw ObservationIOError
        (std::string("Unexpected first char `") +
         char(c) +
         "' in file " + observationFileName + ".");
    }
    NUKLEI_TRACE_END();
  }


  void CoViS3DReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }

  void CoViS3DWandererReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    in_.close();
    init();
    NUKLEI_TRACE_END();
  }

  std::auto_ptr<Observation> CoViS3DReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    return reader_->readObservation_();
    NUKLEI_TRACE_END();
  }
  
  std::auto_ptr<Observation> CoViS3DXMLReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (!e_) NUKLEI_THROW("Reader does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    
    while (*e_ != e_->end())
    {
      ElementIterator primitive = *e_;
      ++*e_;
      
      std::auto_ptr<CoViS3DObservation> observation(new CoViS3DObservation);
      
      ElementIterator primitiveElement;
      primitiveElement = primitiveElement.begin(&*primitive);
      NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      while (primitiveElement->Value() != "Location")
      {
        primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      }
      
      {
        NUKLEI_ASSERT(primitiveElement->Value() == "Location");
        {
          ticpp::Element* el = primitiveElement->FirstChildElement("Cartesian3D");
          Vector3 l;
          el->GetAttribute("x", &l[0]);
          el->GetAttribute("y", &l[1]);
          el->GetAttribute("z", &l[2]);
          observation->setLoc(l);
        }
        {
          ticpp::Element* el = primitiveElement->FirstChildElement("Cartesian3DCovariance");
          Matrix3 cov;
          std::string covString;
          el->GetText(&covString);
          std::istringstream iss(covString);
          for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
            NUKLEI_ASSERT(iss >> cov(i,j));
          observation->setCovMatrix(cov);
        }
      }
      
      primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      while (primitiveElement->Value() != "Orientation")
      {
        primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      }

      {
        NUKLEI_ASSERT(primitiveElement->Value() == "Orientation");
        ElementPtr dirGamma = primitiveElement->FirstChildElement("DirGammaOrientation");
        {
          ElementPtr el = dirGamma->FirstChildElement("Direction");
          el = el->FirstChildElement("Spherical");
          coord_t phi, psi;
          el->GetAttribute("phi", &phi);
          el->GetAttribute("psi", &psi);
          observation->setPhiPsi(phi, psi);
        }
        {
          ElementPtr el = dirGamma->FirstChildElement("GammaVector");
          el = el->FirstChildElement("Cartesian3D");
          Vector3 l;
          el->GetAttribute("x", &l[0]);
          el->GetAttribute("y", &l[1]);
          el->GetAttribute("z", &l[2]);
          observation->setGamma(l);
        }
      }
      
      primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      //IntrinsicDimensionality
      
      primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      //Source2D?
      
      if (primitiveElement->Value() == "Source2D")
        primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      if (false)
      {
        NUKLEI_ASSERT(primitiveElement->Value() == "Phase");
        ticpp::Element* el = primitiveElement->FirstChildElement("Angle");
        appear_t phase;
        el->GetText(&phase);
        //observation->setPhase(phase);
      }
      
      primitiveElement++; NUKLEI_ASSERT(primitiveElement != primitiveElement.end());
      
      {
        NUKLEI_ASSERT(primitiveElement->Value() == "Colors");
        RGBColor::ptr color(new RGBColor);
        {
          ElementPtr el = primitiveElement->FirstChildElement("Left");
          el = el->FirstChildElement("RGB");
          el->GetAttribute("r", &color->R());
          el->GetAttribute("g", &color->G());
          el->GetAttribute("b", &color->B());
          observation->setLeftColor(*color);
        }
        {
          ElementPtr el = primitiveElement->FirstChildElement("Right");
          el = el->FirstChildElement("RGB");
          el->GetAttribute("r", &color->R());
          el->GetAttribute("g", &color->G());
          el->GetAttribute("b", &color->B());
          observation->setRightColor(*color);
        }
        if (false)
        {
          ElementPtr el = primitiveElement->FirstChildElement("Middle");
          el = el->FirstChildElement("RGB");
          el->GetAttribute("r", &color->R());
          el->GetAttribute("g", &color->G());
          el->GetAttribute("b", &color->B());
          //observation->setMiddleColor(color);
        }
      }
            
      oc.incLabel("input");
      
      return std::auto_ptr<Observation>(observation);
   }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
    NUKLEI_TRACE_END();
  }

  std::auto_ptr<Observation> CoViS3DWandererReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (!in_.is_open()) NUKLEI_THROW("Reader does not seem inited.");
    float shaft;

    NUKLEI_ASSERT(in_.good());
    
    while (in_.peek() != EOF)
    {
      NUKLEI_ASSERT(in_.good());
      
      std::auto_ptr<CoViS3DObservation> observation(new CoViS3DObservation);
      
      // Careful:  LS.normal_vector_plane_C is a Vector!
      
      std::string s;
      Vector3 l;
      NUKLEI_ASSERT(in_ >> s && s == "l");
      
      //2 3 4: LS.X() LS.Y() LS.Z()
      for (int d = 0; d < 3; d++)
        NUKLEI_ASSERT(in_ >> l[d]);
      observation->setLoc(l);
      
      //5 6: LS.len LS.conf()
      //NUKLEI_ASSERT( in_ >> shaft && in_ >> shaft );
      
      //7 8 9: LS.confIDim0C LS.confIDim1C LS.confIDim2C
      {
        appear_t c0, c1, c2;
        NUKLEI_ASSERT(in_ >> c0 >> c1 >> c2);
        //observation->setIntrinsicDim(c0, c1, c2);
      }
      // confidence_C
      NUKLEI_ASSERT(in_ >> shaft);
      
      
      //11 12 13: LS.normal_vector_plane_C
      for (int d = 0; d < 3; d++)
        NUKLEI_ASSERT(in_ >> l[d]);
      //observation->setGamma(l);
      
      
      // length_C
      NUKLEI_ASSERT(in_ >> shaft);
      
      
      //0: LS.source2DLineSegmentsC.at(0)
      //1: LS.source2DLineSegmentsC.at(1)
      NUKLEI_ASSERT(in_ >> shaft && in_ >> shaft);
      
      //10: LS.deviation1PixelC
      NUKLEI_ASSERT(in_ >> shaft);
      
      
      // 14 15 16: 3 x dummy
      //directionVectorLineSegmentL[0]
      //directionVectorLineSegmentL[1]
      //directionVectorLineSegmentL[2]
      Vector3 dummy;
      NUKLEI_ASSERT(in_ >> dummy[0] && in_ >> dummy[1] && in_ >> dummy[2]);
      
      
      //17 18 19: LS.PhiRad LS.PsiRad LS.oriConfC
      {
        coord_t phi, psi;
        NUKLEI_ASSERT(in_ >> phi && in_ >> psi && in_ >> shaft);
        observation->setPhiPsi(phi, psi);
      }
      
      NUKLEI_ASSERT(rfe(observation->getDirection().Dot(dummy), 1));
      
      //20 21: LS.phaseC LS.dPhaseConf
      {
        appear_t phase;
        NUKLEI_ASSERT(in_ >> phase && in_ >> shaft );
        //observation->setPhase(phase);
      }
        
      //LS.colorTripletLeft().at(0)
      //LS.colorTripletLeft().at(1)
      //LS.colorTripletLeft().at(2)
      //LS.colConfL()
      //
      //LS.colorTripletMiddle().at(0)
      //LS.colorTripletMiddle().at(1)
      //LS.colorTripletMiddle().at(2)
      //LS.colConfM()
      //
      //LS.colorTripletRight().at(0)
      //LS.colorTripletRight().at(1)
      //LS.colorTripletRight().at(2)
      //LS.colConfR()
        
      
      RGBColor::ptr color(new RGBColor);
      for (int d = 0; d < 3; d++)
        NUKLEI_ASSERT(in_ >> color->at(d));
      observation->setLeftColor(*color);
      NUKLEI_ASSERT(in_ >> shaft);
      for (int d = 0; d < 3; d++)
        NUKLEI_ASSERT(in_ >> color->at(d));
      //observation->setMiddleColor(color);
      NUKLEI_ASSERT(in_ >> shaft);
      for (int d = 0; d < 3; d++)
        NUKLEI_ASSERT(in_ >> color->at(d));
      observation->setRightColor(*color);
      NUKLEI_ASSERT(in_ >> shaft);
      
      char c;
      NUKLEI_ASSERT(in_.get(c));
      NUKLEI_ASSERT(c == '\t');
      NUKLEI_ASSERT(in_.get(c));
      NUKLEI_ASSERT(c == '\n');
      
      NUKLEI_ASSERT( in_.good() );
      
      oc.incLabel("input");
      
      return std::auto_ptr<Observation>(observation);
    }
    
    // End of file reached.
    return std::auto_ptr<Observation>();
    NUKLEI_TRACE_END();
  }

  CoViS3DXMLWriter::CoViS3DXMLWriter(const std::string &observationFileName) :
    observationFileName(observationFileName)
  {
  }

  CoViS3DXMLWriter::~CoViS3DXMLWriter()
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLWriter::init()
  {
    NUKLEI_TRACE_BEGIN();
    try {
      out_.reset(new ticpp::Document(observationFileName));
      ticpp::Declaration dec("1.0", "UTF-8", "");
      out_->InsertEndChild(dec);
      ticpp::Element scene;
      scene.SetValue( "Scene" );
      scene.SetAttribute( "version", "1.0" );
      scene_ = out_->InsertEndChild(scene)->ToElement();
    } catch (ticpp::Exception& e) {
      throw ObservationIOError(e.what());
    }
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLWriter::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    out_->SaveFile();
    NUKLEI_TRACE_END();
  }

  static ticpp::Element* append(ticpp::Element* parent, const std::string& value)
  {
    NUKLEI_TRACE_BEGIN();
    ticpp::Element child(value);
    return parent->InsertEndChild(child)->ToElement();
    NUKLEI_TRACE_END();
  }

  void CoViS3DXMLWriter::writeObservation(const Observation &o)
  {
    NUKLEI_TRACE_BEGIN();
    if (!out_) NUKLEI_THROW("Writer does not seem inited.");
    
    typedef ticpp::Element* ElementPtr;
    const CoViS3DObservation& observation = dynamic_cast<const CoViS3DObservation&>(o);
    
    ElementPtr primitive = append(scene_, "Primitive3D");
    primitive->SetAttribute("type", "l");
    primitive->SetAttribute("confidence", observation.getWeight());
    primitive->SetAttribute("length", 4);
    
    {
      ElementPtr loc = append(primitive, "Location");
      ElementPtr el = append(loc, "Cartesian3D");
      Vector3 l = observation.getLoc();
      Matrix3 cov;
      if (observation.getCovMatrix().isDefined()) cov = *observation.getCovMatrix();
      else cov = Matrix3::IDENTITY;
      el->SetAttribute("x", l[0]);
      el->SetAttribute("y", l[1]);
      el->SetAttribute("z", l[2]);
      el = append(loc, "Cartesian3DCovariance");
      el->SetText(stringify(Vector3(cov.GetRow(0))) + " " +
                  stringify(Vector3(cov.GetRow(1))) + " " +
                  stringify(Vector3(cov.GetRow(2))));
    }

    {
      ElementPtr ori = append(primitive, "Orientation");
      ElementPtr dirGamma = append(ori, "DirGammaOrientation");
      {
        ElementPtr el = append(append(dirGamma, "GammaVector"), "Cartesian3D");
        kernel::se3::ptr se3k = observation.getKernel()->polySe3Proj();
        Matrix3 ori;
        la::copyRotation(ori, se3k->ori_);
        Vector3 l = ori.GetColumn(1);
        //Vector3 l = observation.getGamma();
        el->SetAttribute("x", l[0]);
        el->SetAttribute("y", l[1]);
        el->SetAttribute("z", l[2]);
      }
      {
        ElementPtr dir = append(dirGamma, "Direction");
        ElementPtr el = append(dir, "Spherical");
        coord_t phi, psi;
        boost::tie(phi, psi) = observation.getPhiPsi();
        el->SetAttribute("phi", phi);
        el->SetAttribute("psi", psi);
        append(dir, "Conf")->SetText(-1);
      }
      append(ori, "Cartesian3DCovariance")->SetText
        (stringify(Vector3::UNIT_X) + " " +
         stringify(Vector3::UNIT_Y) + " " +
         stringify(Vector3::UNIT_Z));
    }

    {
      ElementPtr el = append(append(primitive, "IntrinsicDimensionality"),
                             "Barycentric");
      el->SetAttribute("c0", 0);
      el->SetAttribute("c1", 1);
      el->SetAttribute("c2", 0);
    }
    
    {
      // Source2D is OPTIONAL in MoInSScene.rng
      // However CoViS crashes if it doesn't find it.
      ElementPtr phase = append(primitive, "Source2D");
      ElementPtr first = append(phase, "First");
      first->SetText(0);
      ElementPtr second = append(phase, "Second");
      second->SetText(0);
    }
    
    {
      ElementPtr phase = append(primitive, "Phase");
      ElementPtr el = append(phase, "Angle");
      el->SetText(0);
      //el->SetText(observation.getPhase());
      el = append(phase, "Conf");
      el->SetText(0);
    }

    {
      ElementPtr colors = append(primitive, "Colors");
      {
        const RGBColor c(observation.getLeftColor());
        ElementPtr side = append(colors, "Left");
        ElementPtr rgb = append(side, "RGB");
        rgb->SetAttribute("r", c.at(0));
        rgb->SetAttribute("g", c.at(1));
        rgb->SetAttribute("b", c.at(2));
        append(side, "Conf")->SetText(1);
      }
      {
        const RGBColor c(observation.getRightColor());
        ElementPtr side = append(colors, "Right");
        ElementPtr rgb = append(side, "RGB");
        rgb->SetAttribute("r", c.at(0));
        rgb->SetAttribute("g", c.at(1));
        rgb->SetAttribute("b", c.at(2));
        append(side, "Conf")->SetText(1);
      }
      {
        ElementPtr side = append(colors, "Middle");
        ElementPtr rgb = append(side, "RGB");
        rgb->SetAttribute("r", 0);
        rgb->SetAttribute("g", 0);
        rgb->SetAttribute("b", 0);
        append(side, "Conf")->SetText(1);
      }
      
    }
    NUKLEI_TRACE_END();
  }

}
