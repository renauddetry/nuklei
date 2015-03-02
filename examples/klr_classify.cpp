// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

// This example demonstrates how Nulkei can be used for nonlinear
// classification.
//
// This example can be compiled with
//
// g++ `pkg-config --cflags --libs nuklei` -O3 klr_classify.cpp -o klr_classify
//
// (It may be necessary to add -I/path/to/boost/include -L/path/to/boost/lib if
// Boost doesn't reside at a standard place.)

#include <nuklei/KernelLogisticRegressor.h>
#include <nuklei/ObservationIO.h>

int main(int argc, char ** argv)
{
  try {
    
    // ----------- //
    // Parameters: //
    // ----------- //
    
    // Set of datapoints that belong to class 1
    std::string data1Filename = "data/points_class1.xml";
    
    // Set of datapoints that belong to class 2
    std::string data2Filename = "data/points_class2.xml";
    
    // Kernel widths, for position and orientation:
    double locH = 40; // in the same unit as the datapoints forming the density
    double oriH = .4; // in radians
    
    // ------------- //
    // Read-in data: //
    // ------------- //
    
    nuklei::KernelCollection data1, data2;
    
    nuklei::readObservations(data1Filename, data1);
    nuklei::readObservations(data2Filename, data2);
    
    // ---------------------------------------- //
    // Split the data into train and test sets: //
    // ---------------------------------------- //
    
    nuklei::KernelCollection trainData, testData;
    std::vector<int> trainLabels, testLabels;
    for (unsigned i = 0; i < data1.size(); ++i)
    {
      if (i%3 != 0)
      {
        trainData.add(data1.at(i));
        trainLabels.push_back(1);
      }
      else
      {
        testData.add(data1.at(i));
        testLabels.push_back(1);
      }
    }
    for (unsigned i = 0; i < data2.size(); ++i)
    {
      if (i%3 != 0)
      {
        trainData.add(data2.at(i));
        trainLabels.push_back(2);
      }
      else
      {
        testData.add(data2.at(i));
        testLabels.push_back(2);
      }
    }

    // ------ //
    // Train: //
    // ------ //

    trainData.setKernelLocH(locH);
    trainData.setKernelOriH(oriH);
    
    nuklei::KernelLogisticRegressor klr(trainData, trainLabels);
    klr.train();
    
    // ----- //
    // Test: //
    // ----- //
    
    nuklei::GMatrix m = klr.test(testData);
    int nSuccess = 0;
    for (unsigned i = 0; i < testData.size(); ++i)
    {
      if ((m(0,i) > .5 && testLabels.at(i) == 1) ||
          (m(1,i) > .5 && testLabels.at(i) == 2)    )
        nSuccess++;
    }
    
    std::cout << double(nSuccess)/testData.size() << std::endl;
    
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
