// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/ObservationIO.h>
#include <nuklei/CoViS3DObservationIO.h>
#include <nuklei/NukleiObservationIO.h>
#include <nuklei/OsuTxtObservationIO.h>
#include <nuklei/PCDObservationIO.h>
#include <nuklei/PLYObservationIO.h>
#include <nuklei/RIFObservationIO.h>
#include <nuklei/CrdObservationIO.h>
#include <nuklei/OffObservationIO.h>
#include <nuklei/SerializedKernelObservationIO.h>
#include <nuklei/BuiltinVTKObservationIO.h>
#include <nuklei/KernelCollection.h>
#include <nuklei/TxtObservationIO.h>
#include <nuklei/IisObservationIO.h>

namespace nuklei {

  ObservationReader::~ObservationReader()
  {
    if (!oc.empty())
      NUKLEI_LOG("Input stats for reader `" << oc.type_ << "':\n  " << oc);
  }

   NUKLEI_UNIQUE_PTR<Observation> ObservationReader::readObservation()
  {
    NUKLEI_TRACE_BEGIN();
    for (;;)
    {
       NUKLEI_UNIQUE_PTR<Observation> observation = readObservation_();
      if (observation.get() == NULL) return observation;
      oc.incLabel("input");
      if (roi_ && !roi_->contains(observation->getKernel()->getLoc()))
        continue;
      oc.incLabel("inROI");
      return observation;
    }
    NUKLEI_TRACE_END();
  }

   NUKLEI_UNIQUE_PTR<KernelCollection> ObservationReader::readObservations()
  {
    NUKLEI_TRACE_BEGIN();
     NUKLEI_UNIQUE_PTR<KernelCollection> kcp(new KernelCollection);
    readObservations(*kcp);
    return kcp;
    NUKLEI_TRACE_END();
  }
  
  void ObservationReader::readObservations(KernelCollection &kc)
  {
    NUKLEI_TRACE_BEGIN();
    kc.clear();
    
     NUKLEI_UNIQUE_PTR<Observation> o;
    while ( (o = readObservation()).get() != NULL )
    {
      kc.add(*o->getKernel());
    }
    
    NUKLEI_TRACE_END();
  }


  std::ostream& operator<<(std::ostream &out, const ObservationReader::Counter &c)
  {
    NUKLEI_TRACE_BEGIN();
    typedef ObservationReader::Counter Counter;
    for (Counter::list_t::const_iterator i = c.labels_.begin();
         i != c.labels_.end(); ++i)
    {
      out << *i << ": " << c.counts_.find(*i)->second << "; ";
    }
    return out;
    NUKLEI_TRACE_END();
  }

  void ObservationReader::Counter::incLabel(const std::string &label)
  {
    NUKLEI_TRACE_BEGIN();
    map_t::iterator i;
    if (counts_.count(label) == 0)
    {
      labels_.push_back(label);
      i = counts_.insert(std::make_pair(label, unsigned(0))).first;
    }
    else i = counts_.find(label);
    ++(i->second);
    NUKLEI_TRACE_END();
  }
    
  bool ObservationReader::Counter::empty() const
  {
    NUKLEI_TRACE_BEGIN();
    return labels_.empty();
    NUKLEI_TRACE_END();
  }

  void ObservationReader::addRegionOfInterest(boost::shared_ptr<RegionOfInterest> roi)
  {
    NUKLEI_TRACE_BEGIN();
    if (roi_)
      roi_->enqueue(roi);
    else
      roi_ = roi;
    NUKLEI_TRACE_END();
  }

   NUKLEI_UNIQUE_PTR<ObservationReader>
  ObservationReader::createReader(const std::string& arg)
  {
    NUKLEI_TRACE_BEGIN();
     NUKLEI_UNIQUE_PTR<ObservationReader> reader;

    std::string errorsCat = std::string("Error in ObservationReader::createReader.") +
      "\nErrors at each format attempt were:";

#ifdef NUKLEI_USE_TICPP
    try {
      reader = createReader(arg, Observation::COVIS3D);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }
#endif
    
    try {
      reader = createReader(arg, Observation::NUKLEI);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }
    
#ifdef NUKLEI_USE_TICPP
    try {
      reader = createReader(arg, Observation::IIS);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }
#endif
    
    try {
      reader = createReader(arg, Observation::OSUTXT);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::PCD);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::PLY);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::RIF);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::SERIAL);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::CRD);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::OFF);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::BUILTINVTK);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    try {
      reader = createReader(arg, Observation::TXT);
      return reader;
    } catch (ObservationIOError &e) {
      errorsCat += "\n" + std::string(e.what());
    }

    throw ObservationIOError
      ("Error loading observations with automatic type detection. "
       "Maybe the filename `" + arg + "' is incorrect. "
       "Else please try again with a defined type.");
    return reader;
    NUKLEI_TRACE_END();
  }

   NUKLEI_UNIQUE_PTR<ObservationReader>
  ObservationReader::createReader(const std::string& arg, const Observation::Type t)
  {
    NUKLEI_TRACE_BEGIN();
     NUKLEI_UNIQUE_PTR<ObservationReader> reader;
    switch (t)
    {
      case Observation::COVIS3D:
      {
        reader.reset(new CoViS3DReader(arg));
        break;
      }
      case Observation::NUKLEI:
      {
        reader.reset(new NukleiReader(arg));
        break;
      }
      case Observation::OSUTXT:
      {
        reader.reset(new OsuTxtReader(arg));
        break;
      }
      case Observation::PCD:
      {
        reader.reset(new PCDReader(arg));
        break;
      }
      case Observation::PLY:
      {
        reader.reset(new PLYReader(arg));
        break;
      }
      case Observation::RIF:
      {
        reader.reset(new RIFReader(arg));
        break;
      }
      case Observation::CRD:
      {
        reader.reset(new CrdReader(arg));
        break;
      }
      case Observation::OFF:
      {
        reader.reset(new OffReader(arg));
        break;
      }
      case Observation::SERIAL:
      {
        reader.reset(new KernelReader(arg));
        break;
      }
      case Observation::BUILTINVTK:
      {
        reader.reset(new BuiltinVTKReader(arg));
        break;
      }
      case Observation::TXT:
      {
        reader.reset(new TxtReader(arg));
        break;
      }
      case Observation::IIS:
      {
        reader.reset(new IisReader(arg));
        break;
      }
      default:
      {
        NUKLEI_THROW("Unknown format.");
        break;
      }
    }
    reader->init();
    return reader;
    NUKLEI_TRACE_END();
  }


  ObservationWriter::~ObservationWriter()
  {
  }

  void ObservationWriter::writeObservations(const KernelCollection &kc)
  {
    NUKLEI_TRACE_BEGIN();
    
     NUKLEI_UNIQUE_PTR<Observation> o = templateObservation();
    for (KernelCollection::const_iterator i = kc.begin(); i != kc.end(); ++i)
    {
      o->setKernel(*i);
      writeObservation(*o);
    }
    
    NUKLEI_TRACE_END();
  }

   NUKLEI_UNIQUE_PTR<ObservationWriter>
  ObservationWriter::createWriter(const std::string& arg, const Observation::Type t)
  {
    NUKLEI_TRACE_BEGIN();
     NUKLEI_UNIQUE_PTR<ObservationWriter> writer;
    switch (t)
    {
      case Observation::COVIS3D:
      {
        writer.reset(new CoViS3DXMLWriter(arg));
        break;
      }
      case Observation::NUKLEI:
      {
        writer.reset(new NukleiWriter(arg));
        break;
      }
      case Observation::OSUTXT:
      {
        NUKLEI_THROW("Not implemented.");
        //writer.reset(new OsuTxtWriter(arg));
        break;
      }
      case Observation::PCD:
      {
        writer.reset(new PCDWriter(arg));
        break;
      }
      case Observation::PLY:
      {
        writer.reset(new PLYWriter(arg));
        break;
      }
      case Observation::RIF:
      {
        NUKLEI_THROW("Not implemented.");
        //writer.reset(new OsuTxtWriter(arg));
        break;
      }
      case Observation::CRD:
      {
        writer.reset(new CrdWriter(arg));
        break;
      }
      case Observation::OFF:
      {
        writer.reset(new OffWriter(arg));
        break;
      }
      case Observation::SERIAL:
      {
        writer.reset(new KernelWriter(arg));
        break;
      }
      case Observation::BUILTINVTK:
      {
        NUKLEI_THROW("Not implemented.");
        //writer.reset(new OsuTxtWriter(arg));
        break;
      }
      case Observation::TXT:
      {
        writer.reset(new TxtWriter(arg));
        break;
      }
      case Observation::IIS:
      {
        writer.reset(new IisWriter(arg));
        break;
      }
      default:
      {
        NUKLEI_THROW("Unknown format.");
        break;
      }
    }
    writer->init();
    return writer;
    NUKLEI_TRACE_END();
  }

  void readObservations(ObservationReader &r, KernelCollection &kc)
  {
    r.readObservations(kc);
  }
  
  void readObservations(const std::string &s, KernelCollection &kc)
  {
    Observation::Type t;
    readObservations(s, kc, t);
  }

  void readObservations(const std::string &s, KernelCollection &kc,
                        Observation::Type& t)
  {
     NUKLEI_UNIQUE_PTR<ObservationReader> reader =
      ObservationReader::createReader(s);
    t = reader->type();
    readObservations(*reader, kc);
  }

  void readObservationsWithSpecificFormat(const std::string &s,
                                          KernelCollection &kc,
                                          const Observation::Type& t)
  {
     NUKLEI_UNIQUE_PTR<ObservationReader> reader =
    ObservationReader::createReader(s, t);
    readObservations(*reader, kc);
  }


  kernel::base::ptr readSingleObservation(const std::string &s)
  {
    Observation::Type t;
    return readSingleObservation(s, t);
  }

  kernel::base::ptr readSingleObservation(const std::string &s,
                                          Observation::Type& t)
  {
    KernelCollection kc;
    readObservations(s, kc, t);
    if (kc.size() != 1)
      NUKLEI_THROW("File `" << s << "' does not contain a single observation.");
    return kc.front().clone();
  }
  
  kernel::base::ptr
  readSingleObservationWithSpecificFormat(const std::string &s,
                                          const Observation::Type& t)
  {
    KernelCollection kc;
    readObservationsWithSpecificFormat(s, kc, t);
    if (kc.size() != 1)
      NUKLEI_THROW("File `" << s << "' does not contain a single observation.");
    return kc.front().clone();
  }
  
  void writeObservations(ObservationWriter &w, const KernelCollection &kc)
  {
    w.writeObservations(kc);
  }
  
  void writeObservations(const std::string &s, const KernelCollection &kc,
                         const Observation::Type &t)
  {
     NUKLEI_UNIQUE_PTR<ObservationWriter> writer =
      ObservationWriter::createWriter(s, t);
    nuklei::writeObservations(*writer, kc);
    writer->writeBuffer();
  }
  
  void writeSingleObservation(const std::string &s, const kernel::base &k,
                              const Observation::Type &t)
  {
    KernelCollection kc;
    kc.add(k);
    writeObservations(s, kc, t);
  }

}
