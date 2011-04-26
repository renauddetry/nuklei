// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

// This example demonstrates how Nulkei can be used to estimate a density at
// a given point.
//
//   g++ `pkg-config --cflags --libs nuklei` -O3 \
//       nuklei_example_evaluate.cpp -o nuklei_example_evaluate
//
// (It may be necessary to add paths to Boost or GSL by adding
// $CPPFLAGS $CXXFLAGS $LDFLAGS to this compilation command)

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

int main(int argc, char ** argv)
{
  // ----------- //
  // Parameters: //
  // ----------- //
  
  // File containing a set of points that represent a density
  std::string densityFilename = "data/density1.txt";
  
  // File containing a set of points at which the density will
  // be evaluated
  std::string pointsFilename = "data/points1.txt";
  
  // Kernel widths, for position and orientation:
  double locH = 20; // in the same unit as the datapoints forming the density
  double oriH = .2; // in radians
  
  // ------------- //
  // Read-in data: //
  // ------------- //
  
  nuklei::KernelCollection density, points;

  {
    std::auto_ptr<nuklei::ObservationReader> reader =
    nuklei::ObservationReader::createReader(densityFilename);
    nuklei::readObservations(*reader, density);
    // The reader is automatically destroyed when exiting this scope.
  }
  {
    std::auto_ptr<nuklei::ObservationReader> reader =
    nuklei::ObservationReader::createReader(pointsFilename);
    nuklei::readObservations(*reader, points);
  }
  
  density.setKernelLocH(locH);
  density.setKernelOriH(oriH);

  // ------------------------------- //
  // Prepare density for evaluation: //
  // ------------------------------- //
  
  density.normalizeWeights();
  density.computeKernelStatistics();
  density.buildKdTree();
  
  // At this point, density should not be modified anymore.  (Modifying it will
  // destroy the kd-tree, and the kernel statistics.)
  
  // ----------------- //
  // Evaluate density: //
  // ----------------- //
  
  for (nuklei::KernelCollection::const_iterator i = points.begin();
       i != points.end(); ++i)
  {
    std::cout << density.evaluationAt(*i) << std::endl;
  }
  
  return 0;
}
