// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#ifdef NUKLEI_USE_OPENCV
#include "cvaux.h"
#endif

#ifdef NUKLEI_USE_CIMG
#include "CImg.h"
#endif

#include <nuklei/PlaneProjection.h>


namespace nuklei
{
  PlaneProjection::PlaneProjection(const std::string& opencvStereoCalibFile,
                                   unsigned camNum) :
    rotationMatrixData_(9), translationVectorData_(3),
      intrinsicMatrixData_(9), distortionCoeffsData_(4),
      opacity_(1)
  {
    color_[0] = color_[1] = color_[2] = 0; 
    readParameters(opencvStereoCalibFile, camNum);
  }
  
  PlaneProjection::PlaneProjection(const PlaneProjection& pp)
  {
    rotationMatrixData_ = pp.rotationMatrixData_;
    translationVectorData_ = pp.translationVectorData_;
    intrinsicMatrixData_ = pp.intrinsicMatrixData_;
    distortionCoeffsData_ = pp.distortionCoeffsData_;
    for (int i = 0; i < 3; ++i) color_[i] = pp.color_[i];
    opacity_ = pp.opacity_;
#ifdef NUKLEI_USE_CIMG
    if (pp.image_.get() != NULL)
    {
      image_.reset(new image_t(pp.getImage()));
    }
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif
  }
  
  PlaneProjection& PlaneProjection::operator=(const PlaneProjection& pp)
  {
    NUKLEI_TRACE_BEGIN();
    if (&pp == this) return *this;
    rotationMatrixData_ = pp.rotationMatrixData_;
    translationVectorData_ = pp.translationVectorData_;
    intrinsicMatrixData_ = pp.intrinsicMatrixData_;
    distortionCoeffsData_ = pp.distortionCoeffsData_;
    for (int i = 0; i < 3; ++i) color_[i] = pp.color_[i];
    opacity_ = pp.opacity_;
#ifdef NUKLEI_USE_CIMG      
    if (pp.image_.get() != NULL)
    {
      image_.reset(new image_t(pp.getImage()));
    }
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif    
    return *this;
    NUKLEI_TRACE_END();
  }

  // Force the destructor to be defined after CImg has been defined:
  PlaneProjection::~PlaneProjection() {}

  void PlaneProjection::readParameters(const std::string& opencvStereoCalibFile,
                                       unsigned camNum)
  {
    NUKLEI_TRACE_BEGIN();
    
#ifdef NUKLEI_USE_OPENCV
    CvCalibFilter calibFilter;
    NUKLEI_ASSERT(calibFilter.LoadCameraParams(opencvStereoCalibFile.c_str()));
    const CvCamera* camera = calibFilter.GetCameraParams(camNum);
    
    for (int i = 0; i < 9; ++i) rotationMatrixData_.at(i) = camera->rotMatr[i];
    for (int i = 0; i < 3; ++i) translationVectorData_.at(i) = camera->transVect[i];
    for (int i = 0; i < 9; ++i) intrinsicMatrixData_.at(i) = camera->matrix[i];
    for (int i = 0; i < 4; ++i) distortionCoeffsData_.at(i) = camera->distortion[i];      
#else
    NUKLEI_THROW("This function requires OpenCV.");
#endif
    
    NUKLEI_TRACE_END();
  }
  
  std::vector<Vector2>
  PlaneProjection::project(const std::vector<Vector3> worldPoints) const
  {
    NUKLEI_TRACE_BEGIN();
    
#ifdef NUKLEI_USE_CIMG
    
    // cvProjectPoints2 projects p points to the image plane.
    // cvProjectPoints2 assumes that p > 3 :-(.
    const unsigned p = worldPoints.size();
    const unsigned large_p = std::max(p, 4u);
    std::vector<double> objectPointsData(3*large_p, 0);
    std::vector<double> imagePointsData(2*large_p, 0);
    
    for (unsigned pc = 0; pc < p; ++pc)
      for (unsigned i = 0; i < 3; ++i)
        objectPointsData.at(3*pc+i) = worldPoints.at(pc)[i];

#ifdef NUKLEI_USE_OPENCV
    CvMat object_points = cvMat(large_p, 3, CV_64FC1, &objectPointsData.front());
    CvMat rotation_vector = cvMat(3, 3, CV_64FC1, const_cast<double*>(&rotationMatrixData_.front()));
    CvMat translation_vector = cvMat(1, 3, CV_64FC1, const_cast<double*>(&translationVectorData_.front()));
    CvMat intrinsic_matrix = cvMat(3, 3, CV_64FC1, const_cast<double*>(&intrinsicMatrixData_.front()));
    CvMat distortion_coeffs = cvMat(1, 4, CV_64FC1, const_cast<double*>(&distortionCoeffsData_.front()));
    CvMat image_points = cvMat(large_p, 2, CV_64FC1, &imagePointsData.front());
    
    cvProjectPoints2(&object_points,
                     &rotation_vector,
                     &translation_vector,
                     &intrinsic_matrix,
                     &distortion_coeffs,
                     &image_points);
#else
    NUKLEI_THROW("This function requires OpenCV.");
#endif
    
    std::vector<Vector2> imagePoints(p);
    for (unsigned pc = 0; pc < p; ++pc)
      for (unsigned i = 0; i < 2; ++i)
        imagePoints.at(pc)[i] = imagePointsData.at(2*pc+i);

    if (image_.get() != NULL)
    {
      try {
        for (std::vector<Vector2>::iterator i = imagePoints.begin();
            i != imagePoints.end(); ++i) {
          image_->draw_circle(i->X(), i->Y(), IMAGE_PROJECTION_RADIUS,
                              color_, opacity_);
          unsigned char black[] = { 0,0,0 };
          image_->draw_circle(i->X(), i->Y(), IMAGE_PROJECTION_RADIUS,
                              black, 1, static_cast<unsigned int>(0));
        }
      } catch (cimg_library::CImgException &e) {
        NUKLEI_THROW("CImg error: " << e.what());
      }
    }

    return imagePoints;
    
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif

    NUKLEI_TRACE_END();
  }
  
  Vector2 PlaneProjection::project(const Vector3& p) const
  {
    NUKLEI_TRACE_BEGIN();
    std::vector<Vector3> worldPoints;
    worldPoints.push_back(p);
    std::vector<Vector2> imagePoints = project(worldPoints);
    NUKLEI_ASSERT(imagePoints.size() == 1);
    return imagePoints.front();
    NUKLEI_TRACE_END();
  }

  void PlaneProjection::readImage(const std::string& name)
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CIMG
    try {
      image_ = std::auto_ptr<image_t>(new image_t(name.c_str()));
    } catch (cimg_library::CImgException &e) {
      NUKLEI_THROW("CImg error: " << e.what());
    }
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif    
    NUKLEI_TRACE_END();
  }

  void PlaneProjection::writeImage(const std::string& name) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CIMG
    NUKLEI_ASSERT(image_.get() != NULL);
    try {
      image_->save(name.c_str());
    } catch (cimg_library::CImgException &e) {
      NUKLEI_THROW("CImg error: " << e.what());
    }
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif    
    NUKLEI_TRACE_END();
  }

  void StereoPlaneProjection::writeStereoImage(const std::string& name) const
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_CIMG
    try {
      image_t image = (left_.getImage().get_append(right_.getImage(), 'x'));
      image.save(name.c_str());
    } catch (cimg_library::CImgException &e) {
      NUKLEI_THROW("CImg error: " << e.what());
    }
#else
    NUKLEI_THROW("This function requires CIMG.");
#endif    
    NUKLEI_TRACE_END();
  }


}
