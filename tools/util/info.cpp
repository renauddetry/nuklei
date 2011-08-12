// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

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
  
  TCLAP::UnlabeledMultiArg<std::string> fileListArg
  ("file",
   "Nuklei-readable files.",
   true, "filename", cmd);
  
  TCLAP::SwitchArg nArg
  ("n", "n_points",
   "Print number of kernels/points/etc.", cmd);
  
  TCLAP::SwitchArg momentsArg
  ("m", "moments",
   "Print moments.", cmd);
  
  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  for (std::vector<std::string>::const_iterator file = fileListArg.getValue().begin();
       file != fileListArg.getValue().end(); file++)
  {
    
    if (!nArg.getValue() && !momentsArg.getValue())
      std::cout << "[" << *file << "]\n";
    
    KernelCollection kc;
    Observation::Type type;
    
    readObservations(*file, kc, type);
    
    if (nArg.getValue())
    {
      std::cout << kc.size() << std::endl;
      continue;
    }
    
    if (momentsArg.getValue())
    {
      kernel::base::ptr m = kc.moments();
      std::cout << stringify(m->getLoc()) << " " << m->getLocH() << std::endl;
      continue;
    }
    
    std::cout << "reader: " << nameFromType<Observation>(type) << std::endl;
    
    std::cout << "number_of_points: " << kc.size() << std::endl;
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
      std::cout << "mean_position: " << stringify(m->getLoc()) << std::endl;
      std::cout << "deviation_around_mean_position: " << m->getLocH() << std::endl;
      std::cout << "radius_of_bounding sphere: " << mdist/2 << std::endl;
      
      std::cout << "mean_weight: " << mean_weight << " +- " << stdev_weight << std::endl;
      std::cout << "mean_loc_h: " << mean_loc_h << " +- " << stdev_loc_h << std::endl;
      std::cout << "mean_ori_h: " << mean_ori_h << " +- " << stdev_ori_h << std::endl;
      
      if (file+1 != fileListArg.getValue().end())
        std::cout << std::endl;
    }
  }
  
  return 0;
  
  NUKLEI_TRACE_END();
}

