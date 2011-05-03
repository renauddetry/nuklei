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

int test(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Test App." );
  
  /* Standard arguments */
  
  TCLAP::ValueArg<int> niceArg
  ("", "nice",
   "Proccess priority.",
   false, NICEINC, "int", cmd);
  
  /* Custom arguments */
  
  TCLAP::UnlabeledValueArg<std::string> fileArg
  ("file",
   "Nuklei file.",
   false, "", "filename", cmd);
    
  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  Quaternion q(1, 0, 0, 0);
  
  {
    Quaternion r;
    r.FromAxisAngle(Vector3::UNIT_Z, M_PI/2);
    q = r * q;
    std::cout << la::normalized(q) << std::endl;
  }

  {
    Quaternion r;
    r.FromAxisAngle(Vector3::UNIT_Z, M_PI);
    std::cout << la::normalized(r*q) << std::endl;
  }

  {
    Quaternion r;
    r.FromAxisAngle(Vector3::UNIT_Y, 90. / 180*M_PI);
    std::cout << la::normalized(r*q) << std::endl;
  }

  {
    Quaternion r;
    r.FromAxisAngle(Vector3::UNIT_Y, -90. / 180*M_PI);
    std::cout << la::normalized(r*q) << std::endl;
  }

  return 0;
  
  NUKLEI_TRACE_END();
}

