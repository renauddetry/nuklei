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
   "Tolerance t in distsance from a point to the mesh. If a point is at "
   "distance t behind the mesh (with respect to the viewpoint), it is "
   "considered visible.",
   false, 4, "float", cmd);

  TCLAP::SwitchArg colorizeArg
  ("c", "colorize_only",
   "Instead of ignoring occluded points, colorize them.", cmd);

  TCLAP::SwitchArg normalsArg
  ("n", "use_normals",
   "Use surface normals in computing visibility.", cmd);

  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  KernelCollection kc;
  readObservations(inFileArg.getValue(), kc);
  if (normalsArg.getValue())
  {
    kc.buildNeighborSearchTree();
    kc.computeSurfaceNormals();
  }
  
  Vector3 viewpoint = readSingleObservation(viewpointFileArg.getValue())->getLoc();
  
  if (!meshFileArg.getValue().empty())
    kc.readMeshFromOffFile(meshFileArg.getValue());
  else
    kc.buildMesh();

  double tol = tolArg.getValue();
  NUKLEI_ASSERT(tol >= 0);
  
  KernelCollection view;
  for (KernelCollection::const_iterator i = as_const(kc).begin();
       i != as_const(kc).end(); ++i)
  {
    bool visible = false;
    if (normalsArg.getValue())
      visible = kc.isVisibleFrom(kernel::r3xs2p(*i), viewpoint, tol);
    else
      visible = kc.isVisibleFrom(i->getLoc(), viewpoint, tol);
    if (visible)
    {
      view.add(*i);
      if (colorizeArg.getValue())
      {
        RGBColor c(0, 0, 1);
        ColorDescriptor d;
        d.setColor(c);
        view.back().setDescriptor(d);
      }
    }
    else
    {
      if (colorizeArg.getValue())
      {
        view.add(*i);
        RGBColor c(0, 1, 0);
        ColorDescriptor d;
        d.setColor(c);
        view.back().setDescriptor(d);
      }
    }
  }
  if (colorizeArg.getValue())
    writeObservations(outFileArg.getValue(), view, Observation::SERIAL);
  else
    writeObservations(outFileArg.getValue(), view);

  return 0;
  
  NUKLEI_TRACE_END();
}


int create_mesh(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Create Mesh App." );
  
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
  
  TCLAP::ValueArg<std::string> offFileArg
  ("o", "off",
   "File to which the mesh is written in OFF format.",
   false, "", "filename", cmd);

  TCLAP::ValueArg<std::string> plyFileArg
  ("p", "ply",
   "File to which the mesh is written in PLY format.",
   false, "", "filename", cmd);

  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  KernelCollection kc;
  readObservations(inFileArg.getValue(), kc);
  
  kc.buildMesh();
  
  if (!offFileArg.getValue().empty())
    kc.writeMeshToOffFile(offFileArg.getValue());
  if (!plyFileArg.getValue().empty())
    kc.writeMeshToPlyFile(plyFileArg.getValue());
  
  return 0;
  
  NUKLEI_TRACE_END();
}


