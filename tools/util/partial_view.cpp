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

int partial_view(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Partial View App." );
  
  /* Standard arguments */
  
  TCLAP::ValueArg<int> niceArg
  ("", "nice",
   "Proccess priority.",
   false, NICEINC, "int", cmd);
  
  /* Custom arguments */
  
  TCLAP::UnlabeledValueArg<std::string> inFileArg
  ("input",
   "Input point cloud.",
   true, "", "filename", cmd);
  
  TCLAP::UnlabeledValueArg<std::string> outFileArg
  ("output",
   "Output point cloud = partial view of input from viewpoint.",
   true, "", "filename", cmd);
  
  TCLAP::ValueArg<std::string> viewpointFileArg
  ("v", "viewpoint",
   "File containing the viewpoint X Y Z coordinates.",
   true, "", "filename", cmd);

  TCLAP::ValueArg<std::string> meshFileArg
  ("m", "mesh",
   "File containing the occluding mesh in OFF format. If omitted, the mesh is "
   "computed from the input file.",
   false, "", "filename", cmd);

  TCLAP::ValueArg<double> tolArg
  ("t", "tolerance",
   "File containing the viewpoint X Y Z coordinates.",
   false, FLOATTOL, "float", cmd);

  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  KernelCollection kc;
  readObservations(inFileArg.getValue(), kc);

  Vector3 viewpoint = readSingleObservation(viewpointFileArg.getValue())->getLoc();
  
  if (!meshFileArg.getValue().empty())
    kc.readMeshFromOffFile(meshFileArg.getValue());
  else
    kc.buildMesh();

  double tol = tolArg.getValue();
  NUKLEI_ASSERT(tol >= 0);
  
  KernelCollection view;
  KernelCollection::const_partialview_iterator viewIterator =
  kc.partialViewBegin(viewpoint, tol);
  for (KernelCollection::const_partialview_iterator i = viewIterator;
       i != i.end(); ++i)
    view.add(*i);

  writeObservations(outFileArg.getValue(), view);
  
  return 0;
  
  NUKLEI_TRACE_END();
}

