// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <string>
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/tuple/tuple.hpp>

#include <tclap/CmdLine.h>
#include "tclap-wrappers.h"
#include <nuklei/CoViS3DObservationIO.h>
#include <nuklei/SerializedKernelObservationIO.h>
#include <nuklei/Serial.h>
#include <nuklei/nullable.h>

using namespace nuklei;

int info(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Convert App." );

  /* Standard arguments */

  TCLAP::ValueArg<int> niceArg
    ("", "nice",
     "Proccess priority.",
     false, NICEINC, "int", cmd);

  /* Custom arguments */

  TCLAP::UnlabeledValueArg<std::string> fileArg
    ("file",
     "Nuklei file.",
     true, "", "filename", cmd);
  
  TCLAP::SwitchArg nArg
    ("n", "n_points",
     "Print number of kernels/points/etc.", cmd);
  
  TCLAP::SwitchArg momentsArg
    ("m", "moments",
     "Print moments.", cmd);
    
  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  Vector3 t(30, 33, 22);
  
  kernel::se3 o1, o2, h1, h2, m;
  o1.loc_ = Vector3(100, 30, 10) + t;
  o1.ori_ = Random::uniformQuaternion();
  h1.loc_ = Vector3(120, 30, 20);
  h1.ori_ = Random::uniformQuaternion();
  o2.loc_ = Vector3(-10, 400, -300) + t;
  o2.ori_ = Random::uniformQuaternion();
  
  la::project(m.loc_, m.ori_, o1.loc_, o1.ori_, h1.loc_, h1.ori_);
  
  la::transform(h2.loc_, h2.ori_, o2.loc_, o2.ori_, m.loc_, m.ori_);
  
  std::cout << h2 << std::endl;
  return 0;
  
  KernelCollection kc;
  Observation::Type type;

  {  
    std::auto_ptr<ObservationReader> reader =
      ObservationReader::createReader(fileArg.getValue());
    type = reader->type();
    readObservations(*reader, kc);    
  }
  
  if (nArg.getValue())
  {
    std::cout << kc.size() << std::endl;
    return 0;
  }
  
  if (momentsArg.getValue())
  {
    kernel::base::ptr m = kc.moments();
    std::cout << stringify(m->getLoc()) << " " << m->getLocH() << std::endl;
    return 0;
  }
  
  std::cout << "reader: " << nameFromType<Observation>(type) << std::endl;
  
  std::cout << "Number of points: " << kc.size() << std::endl;
  if (kc.size() > 0)
  {
    coord_t mean_loc_h = 0;
    coord_t mean_ori_h = 0;
    weight_t mean_weight = 0;
    coord_t mdist = 0;
    for (KernelCollection::const_iterator i = kc.begin(); i != kc.end(); ++i)
    {
      for (KernelCollection::const_iterator j = i; j != kc.end(); ++j)
      {
        mdist = std::max(mdist, (i->getLoc() - j->getLoc()).Length());
      }
      mean_loc_h += i->getLocH();
      mean_ori_h += i->getOriH();
      mean_weight += i->getWeight();
    }
    mean_loc_h /= kc.size();
    mean_ori_h /= kc.size();
    mean_weight /= kc.size();
    
    coord_t stdev_loc_h = 0;
    coord_t stdev_ori_h = 0;
    weight_t stdev_weight = 0;
    for (KernelCollection::const_iterator i = kc.begin(); i != kc.end(); ++i)
    {
      stdev_loc_h += std::pow(mean_loc_h - i->getLocH(), 2);
      stdev_ori_h += std::pow(mean_ori_h - i->getOriH(), 2);
      stdev_weight += std::pow(mean_weight - i->getWeight(), 2);
    }
    stdev_loc_h = std::sqrt(stdev_loc_h/kc.size());
    stdev_ori_h = std::sqrt(stdev_ori_h/kc.size());
    stdev_weight = std::sqrt(stdev_weight/kc.size());
    
    kernel::base::ptr m = kc.moments();
    std::cout << "mean position: " << stringify(m->getLoc()) << std::endl;
    std::cout << "deviation around mean position: " << m->getLocH() << std::endl;
    std::cout << "radius of bounding sphere: " << mdist/2 << std::endl;
    
    std::cout << "mean weight: " << mean_weight << " +- " << stdev_weight << std::endl;
    std::cout << "mean loc_h: " << mean_loc_h << " +- " << stdev_loc_h << std::endl;
    std::cout << "mean ori_h: " << mean_ori_h << " +- " << stdev_ori_h << std::endl;
  }

  
  return 0;
  
  NUKLEI_TRACE_END();
}

int size(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Convert App." );
  
  /* Standard arguments */
  
  TCLAP::ValueArg<int> niceArg
  ("", "nice",
   "Proccess priority.",
   false, NICEINC, "int", cmd);
  
  /* Custom arguments */
  
  TCLAP::UnlabeledValueArg<std::string> fileArg
  ("file",
   "Nuklei file.",
   true, "", "filename", cmd);

  TCLAP::UnlabeledValueArg<std::string> file2Arg
  ("file2",
   "Nuklei file.",
   true, "", "filename", cmd);
  
  TCLAP::UnlabeledValueArg<std::string> file3Arg
  ("file3",
   "Nuklei file.",
   true, "", "filename", cmd);
    
  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  
  KernelCollection kc1, kc2;
  
  {  
    std::auto_ptr<ObservationReader> reader =
    ObservationReader::createReader(fileArg.getValue());
    readObservations(*reader, kc1);    
  }
  {  
    std::auto_ptr<ObservationReader> reader =
    ObservationReader::createReader(file2Arg.getValue());
    readObservations(*reader, kc2);    
  }
  
  KernelCollection kc;
  for (KernelCollection::const_iterator i = kc1.begin(); i != kc1.end(); ++i)
  {
    kernel::r3 k;
    k.loc_ = i->getLoc();
    kc.add(k);
  }
  for (KernelCollection::const_iterator i = kc2.begin(); i != kc2.end(); ++i)
  {
    kernel::r3 k;
    k.loc_ = i->getLoc();
    kc.add(k);
  }
  
  {
    coord_t mdist = 0;
    for (KernelCollection::const_iterator i = kc.begin(); i != kc.end(); ++i)
    {
      for (KernelCollection::const_iterator j = i; j != kc.end(); ++j)
      {
        mdist = std::max(mdist, (i->getLoc() - j->getLoc()).Length());
      }
    }
        
    kernel::base::ptr m = kc.moments();
    
    std::ofstream ofs(file3Arg.getValue().c_str(), std::ios::app);
    
    ofs << "dev_around_mean_position: " << m->getLocH() << std::endl;
    ofs << "radius_of_bounding_sphere: " << mdist/2 << std::endl;
  }
  
  
  return 0;
  
  NUKLEI_TRACE_END();
}

