// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

// This example demonstrates how Nulkei can be used to sample a density
// function.
//
// This example can be compiled with
//
// g++ `pkg-config --cflags --libs nuklei` -O3 kde_sample.cpp -o kde_sample
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
    
    // File to which the samples will be written
    std::string samplesFilename = "/tmp/nuklei_samples.txt";
    
    // How many samples?
    int nSamples = 10;
    
    // Kernel widths, for position and orientation:
    double locH = 40; // in the same unit as the datapoints supporting the density
    double oriH = .4; // in radians
    
    // ------------- //
    // Read-in data: //
    // ------------- //
    
    nuklei::KernelCollection density, samples;
    
    nuklei::readObservations(densityFilename, density);
    
    // ------------------------------- //
    // Prepare density for evaluation: //
    // ------------------------------- //
    
    density.setKernelLocH(locH);
    density.setKernelOriH(oriH);
    
    density.computeKernelStatistics();
    density.normalizeWeights();
    
    // At this point, "density" should not be modified anymore. (Modifying it will
    // destroy the kernel statistics.)
    
    // ------------------------ //
    // Sample from the density: //
    // ------------------------ //
    
    samples = density.sample(nSamples);
        
    // -------------------------- //
    // Write the samples to disk: //
    // -------------------------- //
    
    nuklei::writeObservations(samplesFilename, samples);
    
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
