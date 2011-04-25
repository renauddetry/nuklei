// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

#include <tclap/CmdLine.h>
#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

int main(int argc, char ** argv)
{
  // Parse command line arguments

  TCLAP::CmdLine cmd(nuklei::INFOSTRING + "Density evaluation example.");

  TCLAP::ValueArg<std::string> densityArg
    ("d", "density",
     "File containing a set of points that represent a density. "
     "If this file doesn't include kernel widths, "
     "the widths should be provided as arguments.",
     true, "", "filename", cmd);

  TCLAP::ValueArg<std::string> pointsArg
    ("p", "points",
     "File containing a set of points at which the density will be "
     "evaluated. If the file contains kernel widths, they willl be "
     "ignored.",
     true, "", "filename", cmd);

  TCLAP::ValueArg<double> locWidthArg
  ("l", "loc_width",
   "Location width.",
   false, -1, "float", cmd);
  
  TCLAP::ValueArg<double> oriWidthArg
  ("o", "ori_width",
   "Orientation width.",
   false, -1, "float", cmd);

  cmd.parse( argc, argv );

  // Read-in data
  
  nuklei::KernelCollection density, points;

  {
    std::auto_ptr<nuklei::ObservationReader> reader =
    nuklei::ObservationReader::createReader(densityArg.getValue());
    nuklei::readObservations(*reader, density);
  }
  {
    std::auto_ptr<nuklei::ObservationReader> reader =
    nuklei::ObservationReader::createReader(pointsArg.getValue());
    nuklei::readObservations(*reader, points);
  }
  
  density.normalizeWeights();
  density.computeKernelStatistics();
  density.buildKdTree();

  if (locWidthArg.getValue() > 0)
    density.setKernelLocH(locWidthArg.getValue());
  if (oriWidthArg.getValue() > 0)
    density.setKernelOriH(locWidthArg.getValue());
  
  for (nuklei::KernelCollection::const_iterator i = points.begin();
       i != points.end(); ++i)
  {
    std::cout << density.evaluationAt(*i) << std::endl;
  }
  
  return 0;
}
