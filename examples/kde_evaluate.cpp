// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

// This example demonstrates how Nulkei can be used to estimate a density
// function at a given point.
//
// This example can be compiled with
//
// g++ `pkg-config --cflags --libs nuklei` -O3 kde_evaluate.cpp -o kde_evaluate
//
// (It may be necessary to add -I/path/to/boost/include -L/path/to/boost/lib if
// Boost doesn't reside at a standard place.)

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

int main(int argc, char ** argv)
{
  try {
    
    // ----------- //
    // Parameters: //
    // ----------- //
    
    // Set of datapoints that represent a density
    std::string densityFilename = "data/points1.txt";
    
    // Set of points at which the density will be evaluated
    std::string pointsFilename = "data/points2.txt";
    
    // Kernel widths, for position and orientation:
    double locH = 40; // in the same unit as the datapoints forming the density
    double oriH = .4; // in radians
    
    // ------------- //
    // Read-in data: //
    // ------------- //
    
    nuklei::KernelCollection density, points;
    
    nuklei::readObservations(densityFilename, density);
    nuklei::readObservations(pointsFilename, points);
    
    // ------------------------------- //
    // Prepare density for evaluation: //
    // ------------------------------- //
    
    density.setKernelLocH(locH);
    density.setKernelOriH(oriH);
    
    density.normalizeWeights();
    density.buildKdTree();
    
    // At this point, "density" should not be modified anymore. (Modifying it will
    // destroy the kd-tree, and the kernel statistics.)
    
    // --------------------- //
    // Evaluate the density: //
    // --------------------- //
    
    for (nuklei::KernelCollection::const_iterator i = points.begin();
         i != points.end(); ++i)
    {
      std::cout << density.evaluationAt(*i) << std::endl;
    }
    
    return 0;
    
  }
  catch (std::exception &e) {
    std::cerr << "Exception caught: ";
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Caught unknown exception." << std::endl;
    return EXIT_FAILURE;
  }
  
}
