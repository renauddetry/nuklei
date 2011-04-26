// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

// This example demonstrates how Nulkei can be used to sample a density.
// This example can be compiled with
//
//   g++ `pkg-config --cflags --libs nuklei` -O3 \
//       nuklei_example_sample.cpp -o nuklei_example_sample
//
// (It may be necessary to add paths to Boost or GSL by adding
// $CPPFLAGS $CXXFLAGS $LDFLAGS to this compilation command)

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/NukleiObservationIO.h>

int main(int argc, char ** argv)
{
  // ----------- //
  // Parameters: //
  // ----------- //
  
  // File containing a set of points that represent a density
  std::string densityFilename = "data/density1.txt";

  // File containing to which the samples will be written
  std::string samplesFilename = "/tmp/nuklei_samples.txt";

  // How many samples?
  int nSamples = 10;
  
  // Kernel widths, for position and orientation:
  double locH = 20; // in the same unit as the datapoints forming the density
  double oriH = .2; // in radians
  
  // ------------- //
  // Read-in data: //
  // ------------- //
  
  nuklei::KernelCollection density, samples;

  {
    std::auto_ptr<nuklei::ObservationReader> reader =
      nuklei::ObservationReader::createReader(densityFilename);
    nuklei::readObservations(*reader, density);
    // The reader is automatically destroyed when exiting this scope.
  }
  
  density.setKernelLocH(locH);
  density.setKernelOriH(oriH);

  // ------------------------------- //
  // Prepare density for evaluation: //
  // ------------------------------- //
  
  density.normalizeWeights();
  density.computeKernelStatistics();
  
  // At this point, density should not be modified anymore.
  // (Modifying it will destroy the kernel statistics.)
  
  // ----------------- //
  // Evaluate density: //
  // ----------------- //
  
  // The sample iterator iterates through nSamples points supporting the
  // density. The probability that a given point is chosen is proportional to
  // its weight.
  nuklei::KernelCollection::const_sample_iterator i = density.sampleBegin(nSamples);
  
  for (; i != i.end(); ++i) // note i.end() instead of density.end()
  {
    // The iterator returns a reference to a point of the density
    const nuklei::kernel::base& p = *i;
    
    // Now we need to draw a sample from the kernel associated to that point
    nuklei::kernel::base::ptr s = p.polySample();
    // And add it to the set of samples
    samples.add(*s);
  }
  
  // -------------------------- //
  // Write the samples to disk: //
  // -------------------------- //

  nuklei::NukleiWriter writer(samplesFilename);
  writer.init();
  nuklei::writeObservations(writer, samples);
  writer.writeBuffer();

  return 0;
}
