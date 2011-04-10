// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifdef NUKLEI_USE_EXR_LIB

#include <iostream>

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include <nuklei/EXRObservationIO.h>
#include <nuklei/EXRObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>

#include "CImg.h"

using namespace Imf;
using namespace Imath;

namespace nuklei {



  EXRReader::EXRReader(const std::string &observationFileName) :
    width_(-1), height_(-1), i_(0), j_(0), observationFileName_(observationFileName)
  {
  }

  EXRReader::~EXRReader()
  {

  }



  void EXRReader::init_()
  {
    NUKLEI_TRACE_BEGIN();

    try {
      // Open file
      RgbaInputFile file(observationFileName_.c_str());

      // Allocate memory for the pixels
      Box2i dw = file.dataWindow();
      width_  = dw.max.x - dw.min.x + 1;
      height_ = dw.max.y - dw.min.y + 1;
      pixels_.resizeErase(height_, width_);

      // Describe the memory layout of the pixels
      file.setFrameBuffer(&pixels_[0][0] - dw.min.x - dw.min.y * width_, 1, width_);
      // Read the pixels from the file
      file.readPixels(dw.min.y, dw.max.y);
    } catch (Error &e) {
      throw ObservationIOError("Non-EXR format.");
    }

    NUKLEI_TRACE_END();
  }


  void EXRReader::reset()
  {
    NUKLEI_TRACE_BEGIN();
    init();
    NUKLEI_TRACE_END();
  }


  std::auto_ptr<Observation> EXRReader::readObservation_()
  {
    if (width_ < 0 || height_ < 0) NUKLEI_THROW("Reader does not seem inited.");

    if (i_ == height_) // End of file reached
      return std::auto_ptr<Observation>();

    do {
      // Go to next point
      j_++;
      if (j_ == width_) { // End of line
        j_ = 0;
        i_++;
        if (i_ == height_) // End of file
          return std::auto_ptr<Observation>();
      }
    } while (pixels_[i_][j_].r == 0);

    // Forge observation
    std::auto_ptr<EXRObservation> observation(new EXRObservation);
    Vector3 loc;
    loc[0] = i_;
    loc[1] = j_;
    loc[2] = pixels_[i_][j_].r;
    backprojectPoint_(loc);
    observation->setLoc(loc);

    // Set (constant) color    
    RGBColor c(.5,.5,.5);
    observation->setColor(c);
    
    return std::auto_ptr<Observation>(observation);
  }

  void EXRReader::backprojectPoint_(Vector3 &p)  {
    // Input
    double imgx = p[0] - 256; // X index from image center
    double imgy = p[1] - 256; // Y index from image center
    double depth = p[2]; // Distance from camera center to point

    // We have:
    // tan(0.523599 / 2) = 256  * scaleFactor
    // tan(betax)        = imgx * scaleFactor
    // tan(betay)        = imgy * scaleFactor

    // Therefore:
    double scaleFactor = tan(0.523599 / 2) / 256.0;
    double betax = atan(imgx * scaleFactor);
    double betay = atan(imgy * scaleFactor);

    // And:
    double d = cos(betax) * depth; // Distance from camera (on the Z axis)
    p[1] = sin(betax) * depth;
    p[0] = sin(betay) * depth;
    p[2] = -5 + d; // Adjust wrt camera center (which is at (0,0,-5))
  }
}

#endif
#include <fstream>

