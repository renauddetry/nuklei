// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

// This example demonstrates how Nulkei can be used to sample a density
// function.
//
// This example can be compiled with
//
//   g++ `pkg-config --cflags --libs nuklei` -O3
//       nuklei_example_sample.cpp -o nuklei_example_sample
//
// (It may be necessary to add -I/path/to/boost/include -L/path/to/boost/lib if
// Boost doesn't reside at a standard place.)

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>

int main(int argc, char ** argv)
{
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
  double locH = 20; // in the same unit as the datapoints supporting the density
  double oriH = .2; // in radians
  
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
  
  density.normalizeWeights();
  density.computeKernelStatistics();
  
  // At this point, "density" should not be modified anymore. (Modifying it will
  // destroy the kernel statistics.)
  
  // ------------------------ //
  // Sample from the density: //
  // ------------------------ //
  
  // The `sample_iterator' iterates through `nSamples' datapoints supporting the
  // density. The probability that a given datapoint is chosen is proportional to
  // its weight.
  nuklei::KernelCollection::const_sample_iterator i = density.sampleBegin(nSamples);
  
  for (; i != i.end(); ++i) // note i.end() instead of density.end()
  {
    // `*i' returns a reference to a datapoint/kernel of `density'
    const nuklei::kernel::base& p = *i;
    
    // Now we need to draw a sample from the that kernel
    nuklei::kernel::base::ptr s = p.polySample();
    // And add it to the set of samples
    samples.add(*s);
  }
  
  // -------------------------- //
  // Write the samples to disk: //
  // -------------------------- //

  nuklei::writeObservations(samplesFilename, samples);

  return 0;
}
