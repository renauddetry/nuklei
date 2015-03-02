// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

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
  
  return 0;
  
  NUKLEI_TRACE_END();
}

