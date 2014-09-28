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
#include <nuklei/ProgressIndicator.h>
#include <nuklei/ObservationIO.h>

using namespace nuklei;

void convert(const std::vector<std::string>& files,
             const kernel::se3* transfo = NULL,
             const double scale = 0,
             const bool normalizePose = false,
             const std::string& normalizingTransfoFile = "",
             const bool normalizeScale = false,
             const std::string& normalizingScaleFile = "",
             const bool uniformizeWeights = false,
             const Observation::Type inType = Observation::UNKNOWN,
             const Observation::Type outType = Observation::UNKNOWN,
             boost::shared_ptr<RegionOfInterest> roi = boost::shared_ptr<RegionOfInterest>(),
             const int nObs = -1,
             const double minDist = 0,
             const bool removePlane = false,
             const int ransacIter = 100,
             const double inlierThreshold = 8,
             const std::string& fittedPlaneFile = "",
             bool makeR3xS2P = false,
             bool removeNormals = false,
             const std::string &filterRGB = "",
             const std::string &setRGB = "",
             const Color::Type colorToLoc = Color::UNKNOWN)
{
  bool storeInKc = removePlane || !fittedPlaneFile.empty() ||
    nObs >= 0 || minDist > 0 || minDist == -1 ||
    normalizePose || !normalizingTransfoFile.empty() ||
    normalizeScale || !normalizingScaleFile.empty() ||
    makeR3xS2P || removeNormals ||
    !filterRGB.empty() || !setRGB.empty() || colorToLoc != Color::UNKNOWN;

  std::auto_ptr<ObservationWriter> writer;
  Observation::Type writerType = outType;
  Observation::Type readerType = Observation::UNKNOWN;
  std::vector< boost::shared_ptr<Observation> > observations;

  for (std::vector<std::string>::const_iterator i = files.begin();
       i != --files.end(); ++i)
  {
    std::auto_ptr<ObservationReader> reader;
    if (inType == Observation::UNKNOWN)
      reader = ObservationReader::createReader(*i);
    else
      reader = ObservationReader::createReader(*i, inType);

    reader->addRegionOfInterest(roi);
    readerType = reader->type();

    if (writer.get() == NULL)
    {
     
      if (writerType == Observation::UNKNOWN)
        writerType = reader->type();
      
      writer = ObservationWriter::createWriter(files.back(), writerType);
    }
    else
    {
//      if (writerType != Observation::SERIAL &&
//          (writer->type() != reader->type()))
//        NUKLEI_WARN("Writer of type `" << nameFromType<Observation>(writer->type()) <<
//                  "' may not be able to write observations of type `" <<
//                  nameFromType<Observation>(reader->type()) << "'.");
    }
    
    std::auto_ptr<Observation> o;
    while ( (o = reader->readObservation()).get() != NULL )
    {
      if (transfo != NULL || scale > 0)
      {
        kernel::base::ptr k = o->getKernel();
        if (transfo != NULL) k->polyMakeTransformWith(*transfo);
        if (scale > 0)
        {
          k->setLoc(k->getLoc()*scale);
        }
        o->setKernel(*k);
      }
      
      if (uniformizeWeights)
      {
        kernel::base::ptr k = o->getKernel();
        k->setWeight(1);
        o->setKernel(*k);
      }
      
      if (storeInKc)
        observations.push_back(boost::shared_ptr<Observation>(o));
      else
        writer->writeObservation(*o);
    }
  }
  
  if (files.size() > 2 && !uniformizeWeights &&
      (readerType == Observation::NUKLEI || readerType == Observation::SERIAL))
  {
    std::cout << "Warning: concatenating several files. "
    "Keep in mind that weights may not be consistently mixed. "
    "Use --uniformize_weights if appropriate." << std::endl;
  }
  
  if (storeInKc)
  {
    if (removePlane || !fittedPlaneFile.empty())
    {
      KernelCollection kc;
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
      {
        kernel::r3 r3k;
        r3k.loc_ = (*i)->getKernel()->getLoc();
        kc.add(r3k);
      }
      kernel::se3 k = kc.ransacPlaneFit(inlierThreshold, ransacIter);
      Plane3 plane(la::matrixCopy(k.ori_).GetColumn(2), k.loc_);
      
      std::vector< boost::shared_ptr<Observation> > tmp = observations;
      observations.clear();
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = tmp.begin();
           i != tmp.end(); ++i)
      {
        Vector3 loc = (*i)->getKernel()->getLoc();
        if (removePlane && std::fabs(plane.DistanceTo(loc)) < inlierThreshold)
          continue;
        observations.push_back(*i);
      }
      
      if (!fittedPlaneFile.empty())
      {
        writeSingleObservation(fittedPlaneFile, k, Observation::SERIAL);
      }
    }

    if (!filterRGB.empty())
    {
      Vector3 colorVector = numify<Vector3>(filterRGB);
      RGBColor rgbColor(colorVector.X(), colorVector.Y(), colorVector.Z());
      HSVConeColor hsvColor(rgbColor);
      std::vector< boost::shared_ptr<Observation> > tmp = observations;
      observations.clear();
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = tmp.begin();
           i != tmp.end(); ++i)
      {
        if ((*i)->getKernel()->hasDescriptor())
        {
          kernel::base::ptr k = (*i)->getKernel();
          ColorDescriptor* cDesc = dynamic_cast<ColorDescriptor*>(&k->getDescriptor());
          HSVConeColor c(cDesc->getColor());
          if (cDesc != NULL &&
              (c.distanceTo(hsvColor) < HSVConeColor().getMaxDist() / 2))
            observations.push_back(*i);
        }
      }
    }

    if (removeNormals)
    {
      KernelCollection kc1;
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
      {
        kernel::r3 k;
        k.loc_ = (*i)->getKernel()->getLoc();
        k.setWeight((*i)->getKernel()->getWeight());
        if ((*i)->getKernel()->hasDescriptor()) k.setDescriptor((*i)->getKernel()->getDescriptor());
        kc1.add(k);
      }
      
      observations.clear();
      
      for (KernelCollection::const_iterator i = as_const(kc1).begin();
           i != as_const(kc1).end(); ++i)
      {
        observations.push_back(boost::shared_ptr<Observation>(new SerializedKernelObservation(*i)));
      }
    }
    
    if (makeR3xS2P)
    {
      NUKLEI_ASSERT(setRGB.empty());
      if (!(writerType == Observation::SERIAL ||
            writerType == Observation::NUKLEI ||
            writerType == Observation::TXT ||
            writerType == Observation::PCD))
        NUKLEI_THROW("Normal computation only available when outputing " <<
                     nameFromType<Observation>(Observation::SERIAL) << "or " <<
                     nameFromType<Observation>(Observation::NUKLEI) << "or " <<
                     nameFromType<Observation>(Observation::TXT) << "or " <<
                     nameFromType<Observation>(Observation::PCD) << ".");
    
      KernelCollection kc1;
      
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
        kc1.add(*(*i)->getKernel());
      
      kc1.computeSurfaceNormals();
      
      observations.clear();
      
      for (KernelCollection::const_iterator i = as_const(kc1).begin();
           i != as_const(kc1).end(); ++i)
      {
        observations.push_back(boost::shared_ptr<Observation>(new SerializedKernelObservation(*i)));
      }
    }
    
    if (!setRGB.empty())
    {
      NUKLEI_ASSERT(!makeR3xS2P);
      RGBColor rgbColor;
      try {
        Vector3 colorVector = numify<Vector3>(setRGB);
        rgbColor.setRGB(colorVector);
      } catch (Error & e) {
        Serial::readObject(rgbColor, setRGB);
      }
      for (std::vector< boost::shared_ptr<Observation> >::iterator
           i = observations.begin();
           i != observations.end(); ++i)
      {
        kernel::base::ptr k = (*i)->getKernel();
        if (k->hasDescriptor())
          dynamic_cast<VisualDescriptor&>(k->getDescriptor()).setColor(rgbColor);
        else
        {
          ColorDescriptor cd;
          cd.setColor(rgbColor);
          k->setDescriptor(cd);
        }
        (*i)->setKernel(*k);
      }
    }
  
    if (normalizePose || !normalizingTransfoFile.empty())
    {
      KernelCollection kc1;
      
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
        kc1.add(*(*i)->getKernel());
      
      kc1.uniformizeWeights();
      kernel::se3 p = kc1.linearLeastSquarePlaneFit();
      kernel::se3 origin;
      kernel::se3 transfo = origin.transformationFrom(p);

      if (!normalizingTransfoFile.empty())
      {
        Serial::writeObject(transfo, normalizingTransfoFile);
      }

      if (normalizePose)
      {        
        std::cout << "Normalizing translation: " << transfo.loc_ << "\n" <<
          "Normalizing quaternion: " << transfo.ori_ << std::endl;

        for (std::vector< boost::shared_ptr<Observation> >::iterator
             i = observations.begin();
             i != observations.end(); ++i)
        {
          kernel::base::ptr k = (*i)->getKernel();
          k->polyMakeTransformWith(transfo);
          (*i)->setKernel(*k);
        }
      }
    }
  
    if (normalizeScale || !normalizingScaleFile.empty())
    {
      KernelCollection kc1;
      
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
        kc1.add(*(*i)->getKernel());
      
      kc1.uniformizeWeights();
      coord_t stdev = kc1.moments()->getLocH();
      
      if (!normalizingScaleFile.empty())
      {
        std::ofstream ofs(normalizingScaleFile.c_str());
        ofs << 1./stdev << std::endl;
      }
      
      if (normalizeScale)
      {        
        std::cout << "Normalizing scale: " << 1./stdev << std::endl;
        
        for (std::vector< boost::shared_ptr<Observation> >::iterator
             i = observations.begin();
             i != observations.end(); ++i)
        {
          kernel::base::ptr k = (*i)->getKernel();
          k->setLoc(k->getLoc()/stdev);
          (*i)->setKernel(*k);
        }
      }
      
      {
        KernelCollection kc1;

        for (std::vector< boost::shared_ptr<Observation> >::const_iterator
             i = observations.begin();
             i != observations.end(); ++i)
          kc1.add(*(*i)->getKernel());
        
        kc1.uniformizeWeights();
        coord_t stdev = kc1.moments()->getLocH();
        std::cout << stdev << std::endl;
      }
      
    }
    
    if (colorToLoc != Color::UNKNOWN)
    {
      std::vector< boost::shared_ptr<Observation> > tmp = observations;
      observations.clear();
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = tmp.begin();
           i != tmp.end(); ++i)
      {
        kernel::base::ptr k = (*i)->getKernel();
        NUKLEI_ASSERT(k->hasDescriptor());

        Color &icolor = dynamic_cast<ColorDescriptor&>(k->getDescriptor()).getColor();
        std::auto_ptr<Color> ocolor;
        switch (colorToLoc)
        {
          case Color::RGB:
            ocolor = std::auto_ptr<Color>(new RGBColor(icolor));
            break;
          case Color::HSV:
            ocolor = std::auto_ptr<Color>(new HSVColor(icolor));
            break;
          case Color::HSVCONE:
            ocolor = std::auto_ptr<Color>(new HSVConeColor(icolor));
            break;
          default:
            NUKLEI_ASSERT(false);
            break;
        }
        
        GVector v = ocolor->getVector();
        kernel::r3 colorKernel;
        for (int d = 0; d < 3; ++d)
        {
          colorKernel.loc_[d] = v[d];
        }
        colorKernel.setDescriptor(k->getDescriptor());
        observations.push_back(boost::shared_ptr<Observation>(new SerializedKernelObservation(colorKernel)));
      }
    }
    
    double minDist2 = minDist;
    
    if (minDist2 == -1)
    {
      std::vector<Vector3> posVec;
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
        posVec.push_back((*i)->getKernel()->getLoc());
      
      ProgressIndicator pi(observations.size(),
                           "Computing maximum distance between two nearest neighbors: ");
      for (std::vector<Vector3>::const_iterator
           i = posVec.begin();
           i != posVec.end(); ++i)
      {
        double nnDist = -1;
        for (std::vector<Vector3>::const_iterator
             j = posVec.begin();
             j != posVec.end(); ++j)
        {
          if (i == j) continue;
          double d = (*i-*j).SquaredLength();
          if ( nnDist == -1 || d < nnDist )
          {
            nnDist = d;
          }
        }
        nnDist = std::sqrt(nnDist);
        if (minDist2 == -1 || nnDist > minDist2)
          minDist2 = nnDist;
        pi.inc();
      }
      std::cout << "Max distance between two nearest neighbors: " << minDist2 << std::endl;
    }
    
    if (minDist2 > 0)
    {
      std::vector< boost::shared_ptr<Observation> > tmp = observations;
      observations.clear();
      std::vector<Vector3> posVec;
      ProgressIndicator pi(tmp.size(), "Filtering points: ");
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = tmp.begin();
           i != tmp.end(); ++i)
      {
        bool cnt = false;
        Vector3 iLoc = (*i)->getKernel()->getLoc();
        for (std::vector<Vector3>::const_iterator
             j = posVec.begin();
             j != posVec.end(); ++j)
        {
          if ( (iLoc - *j).SquaredLength() < minDist2*minDist2 )
          {
            cnt = true;
            break;
          }
        }
        if (!cnt)
        {
          observations.push_back(*i);
          posVec.push_back(iLoc);
        }
        pi.inc();
      }
    }
    
    if (nObs >= 0)
    {
#if NUKLEI_TRSL_VERSION_NR < 100030000
      typedef nuklei_trsl::is_picked_systematic<
        boost::shared_ptr<Observation>,
        weight_t,
        nuklei_trsl::weight_accessor<weight_t, boost::shared_ptr<Observation> >
      > is_picked;
      typedef nuklei_trsl::ppfilter_iterator<
        is_picked,
        std::vector< boost::shared_ptr<Observation> >::iterator> sample_iterator;
      sample_iterator i( is_picked(nObs, observations.size()),
                         observations.begin(),
                         observations.end());
#else
      typedef
      nuklei_trsl::stl_vector< boost::shared_ptr<Observation> >::systematic_sample_iterator
      sample_iterator;
      sample_iterator i(observations, nObs);
#endif
      for (; i != i.end(); ++i)
        writer->writeObservation(**i);
    }
    else
    {
      for (std::vector< boost::shared_ptr<Observation> >::const_iterator
           i = observations.begin();
           i != observations.end(); ++i)
      {
        writer->writeObservation(**i);
      }
    }
  }

  writer->writeBuffer();
}

int convert(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Convert App." );

  /* Standard arguments */

  TCLAP::ValueArg<int> niceArg
    ("", "nice",
     "Proccess priority.",
     false, NICEINC, "int", cmd);

  /* Custom arguments */

  TCLAP::UnlabeledMultiArg<std::string> fileListArg
    ("",
     "List of p files. The (p-1) first files are read as input, "
     "optionally transformed, "
     "concatenated, "
     "and written to the last file of the list.",
     true, "filename", cmd);
     
  TCLAP::ValueArg<std::string> transformationArg
    ("", "transformation",
     "File containing an nuklei::kernel::se3.",
     false, "", "string", cmd);

  TCLAP::ValueArg<std::string> invTransformationArg
  ("", "inv_transformation",
   "File containing an nuklei::kernel::se3.",
   false, "", "string", cmd);
  
  TCLAP::ValueArg<double> scaleArg
    ("", "scale",
     "Scale factor.",
     false, 0, "float", cmd);
     
  TCLAP::ValueArg<std::string> translationArg
    ("t", "translation",
     "Translation vector, as \"tx ty tz\".",
     false, "", "string", cmd);

  TCLAP::SwitchArg normalizePoseArg
    ("", "normalize_pose",
     "Make center of gravity 0, and normalize ori.", cmd);

  TCLAP::ValueArg<std::string> normalizingTransfoFileArg
    ("", "normalizing_transfo",
     "File to which the normalizing transformation will be written.",
     false, "", "string", cmd);

  TCLAP::SwitchArg normalizeScaleArg
  ("", "normalize_scale",
   "Make radius of bounding sphere .5.", cmd);
  
  TCLAP::ValueArg<std::string> normalizingScaleFileArg
  ("", "normalizing_scale",
   "File to which the normalizing scale will be written.",
   false, "", "string", cmd);
  
  TCLAP::SwitchArg uniformizeWeightsArg
    ("", "uniformize_weights",
     "Uniformize weights (1 or 1/size).", cmd);

  TCLAP::ValueArg<std::string> quaternionRotationArg
    ("q", "quaternion_rotation",
     "Rotation quaternion, as \"w x y z\".",
     false, "", "string", cmd);

  TCLAP::ConstrainedValueArg<std::string> inTypeArg
    ("r", "in_type",
     "Specifies the input file type.",
     false, nameFromType<Observation>(Observation::UNKNOWN),
     listTypeNames<Observation>(), cmd);

  TCLAP::ConstrainedValueArg<std::string> outTypeArg
    ("w", "out_type",
     "Specifies the output file type.",
     false, nameFromType<Observation>(Observation::UNKNOWN),
     listTypeNames<Observation>(), cmd);

  TCLAP::ValueArg<int> nObsArg
    ("n", "num_obs",
     "Number of output observations.",
     false, -1, "int", cmd);

  TCLAP::ValueArg<double> minDistArg
  ("", "min_dist",
   "In the output set, points are at least separated by the value of this "
   "argument.",
   false, 0, "float", cmd);

  TCLAP::SwitchArg removeDominantPlaneArg
  ("", "remove_dominant_plane",
   "Remove dominant plane.", cmd);

  TCLAP::ValueArg<int> ransacIterArg
  ("", "ransac_iter",
   "Number of RANSAC iterations in plane fitting. "
   "A value of 100 is a good start.",
   false, 100, "int", cmd);

  TCLAP::ValueArg<std::string> fittedPlaneArg
    ("", "fitted_plane",
     "Fit plane to data. Write the plane params to the provided file. "
     "The plane is written as an SE(3) point. The Z axis of the ori is normal "
     "to the plane. The other two axes indicate its principal components.",
     false, "", "string", cmd);

  TCLAP::ValueArg<double> removePlaneInlierTArg
  ("", "inlier_threshold",
   "Inlier threshold for removing plane with RANSAC. "
   "A value of 8mm is a good start",
   false, 8, "float", cmd);
  
  TCLAP::SwitchArg makeR3xs2pArg
    ("", "make_r3xs2p",
     "Compute a surface normal at each point, using local location differentials.", cmd);

  TCLAP::SwitchArg computeNormalsArg
    ("", "compute_normals",
     "Compute a surface normal at each point, using local location differentials.", cmd);

  TCLAP::SwitchArg removeNormalsArg
    ("", "remove_normals",
     "Remove surface normals.", cmd);

  TCLAP::ValueArg<std::string> filterRGBArg
    ("", "filter_rgb",
     "Keep only elements close to that color.",
     false, "", "RGB triplet", cmd);

  TCLAP::ValueArg<std::string> setRGBColorArg
    ("", "set_rgb",
     "Set all elements to that color.",
     false, "", "RGB triplet", cmd);

  TCLAP::MultiArg<std::string> sphereROIArg
    ("", "sphere_roi",
     "Specifies a sphere region of interest, through center-radius string.",
     false, "string", cmd);

  TCLAP::MultiArg<std::string> nsphereROIArg
    ("", "nsphere_roi",
     "Specifies a negated-sphere region of interest, through center-radius string.",
     false, "string", cmd);
  
  TCLAP::MultiArg<std::string> boxROIArg
    ("", "box_roi",
     "Specifies a box region of interest, in the form \"cx cy cz qw qx qy qz sx sy sz\" "
     "where c is the box center, q is the box quaternion orientation, and s is the box scale (edge sizes).",
     false, "string", cmd);

  TCLAP::ConstrainedValueArg<std::string> colorToLocArg
    ("", "color_to_loc",
     "Triggers replacement of all kernel location from their color, expressed in the supplied colorspace.",
     false, nameFromType<Color>(Color::UNKNOWN),
     listTypeNames<Color>(), cmd);

//  TCLAP::MultiArg<std::string> boxROIArg
//    ("", "box_roi",
//     "Specifies a box region of interest, in the form \"cx cy cz a1x a1y a1z a2x a2y a2z e3\" "
//     "where a1 is the axis going from the center of the box to the center of "
//     "gravity of one of its faces, a2 is the same for an adjacent face, and e3 "
//     "is the half of the box depth. ",
//     false, "string", cmd);
  
  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);

  NUKLEI_ASSERT(fileListArg.getValue().size() > 1);
  
  kernel::se3::ptr transfo;
  
  if (!transformationArg.getValue().empty())
  {
    if (!invTransformationArg.getValue().empty())
      NUKLEI_THROW("Please specify either " << transformationArg.getName() <<
                 " or " << invTransformationArg.getName() << ".");
    if (!translationArg.getValue().empty())
      NUKLEI_THROW("Please specify either " << transformationArg.getName() <<
                 " or " << translationArg.getName() << ".");
    if (!quaternionRotationArg.getValue().empty())
      NUKLEI_THROW("Please specify either " << transformationArg.getName() <<
                 " or " << quaternionRotationArg.getName() << ".");
    transfo.reset(new kernel::se3);
    
    try {
      *transfo = kernel::se3(*readSingleObservation(transformationArg.getValue()));
    } catch (std::exception& e)
    {
      Serial::readObject(*transfo, transformationArg.getValue());
    }
  }
  else if (!invTransformationArg.getValue().empty())
  {
    if (!translationArg.getValue().empty())
      NUKLEI_THROW("Please specify either " << invTransformationArg.getName() <<
                 " or " << translationArg.getName() << ".");
    if (!quaternionRotationArg.getValue().empty())
      NUKLEI_THROW("Please specify either " << invTransformationArg.getName() <<
                 " or " << quaternionRotationArg.getName() << ".");
    kernel::se3 rt;
    try {
      rt = kernel::se3(*readSingleObservation(invTransformationArg.getValue()));
    } catch (std::exception& e)
    {
      Serial::readObject(rt, invTransformationArg.getValue());
    }
    kernel::se3 it, origin;
    it = origin.transformationFrom(rt);
    transfo.reset(new kernel::se3(it));
  }
  else if (!translationArg.getValue().empty() || !quaternionRotationArg.getValue().empty())
  {
    transfo.reset(new kernel::se3);
    if (!translationArg.getValue().empty())
      transfo->loc_ = numify<Vector3>(translationArg.getValue());
    if (!quaternionRotationArg.getValue().empty())
      transfo->ori_ = la::normalized(numify<Quaternion>(quaternionRotationArg.getValue()));
  }

  boost::shared_ptr<RegionOfInterest> roi;

  for (std::vector<std::string>::const_iterator i = sphereROIArg.getValue().begin();
       i != sphereROIArg.getValue().end(); i++)
  {
    boost::shared_ptr<RegionOfInterest> sphereROI
      (new SphereROI(*i));
    if (roi)
      roi->enqueue(sphereROI);
    else
      roi = sphereROI;
  }
  for (std::vector<std::string>::const_iterator i = nsphereROIArg.getValue().begin();
       i != nsphereROIArg.getValue().end(); i++)
  {
    boost::shared_ptr<RegionOfInterest> sphereROI
    (new SphereROI(*i));
    sphereROI->setSign(false);
    if (roi)
      roi->enqueue(sphereROI);
    else
      roi = sphereROI;
  }
  for (std::vector<std::string>::const_iterator i = boxROIArg.getValue().begin();
       i != boxROIArg.getValue().end(); i++)
  {
    boost::shared_ptr<RegionOfInterest> boxROI
      (new BoxROI(*i));
    if (roi)
      roi->enqueue(boxROI);
    else
      roi = boxROI;
  }
  
  convert(fileListArg.getValue(), transfo.get(), scaleArg.getValue(),
          normalizePoseArg.getValue(), normalizingTransfoFileArg.getValue(),
          normalizeScaleArg.getValue(), normalizingScaleFileArg.getValue(),
          uniformizeWeightsArg.getValue(),
          typeFromName<Observation>(inTypeArg.getValue()),
          typeFromName<Observation>(outTypeArg.getValue()),
          roi,
          nObsArg.getValue(), minDistArg.getValue(),
          removeDominantPlaneArg.getValue(), ransacIterArg.getValue(),
          removePlaneInlierTArg.getValue(),
          fittedPlaneArg.getValue(),
          makeR3xs2pArg.getValue() || computeNormalsArg.getValue(),
          removeNormalsArg.getValue(),
          filterRGBArg.getValue(),
          setRGBColorArg.getValue(),
          typeFromName<Color>(colorToLocArg.getValue()));
  
  return 0;
  
  NUKLEI_TRACE_END();
}

int concatenate(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Concatenation App." );

  /* Standard arguments */

  TCLAP::ValueArg<int> niceArg
    ("", "nice",
     "Proccess priority.",
     false, NICEINC, "int", cmd);

  /* Custom arguments */

  TCLAP::UnlabeledMultiArg<std::string> fileListArg
    ("",
     "List of p files. The (p-1) first files are read as input, "
     "concatenated, "
     "and written to the last file of the list.",
     true, "filename", cmd);
  
  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);

  NUKLEI_ASSERT(fileListArg.getValue().size() > 1);

  convert(fileListArg.getValue());

  return 0;
  
  NUKLEI_TRACE_END();
}

int invert_transfo(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Transfo Invert App." );
  
  /* Standard arguments */
  
  TCLAP::ValueArg<int> niceArg
  ("", "nice",
   "Proccess priority.",
   false, NICEINC, "int", cmd);
  
  /* Custom arguments */
  
  TCLAP::UnlabeledValueArg<std::string> inFileArg
  ("input",
   "Input.",
   true, "", "filename", cmd);
  
  TCLAP::UnlabeledValueArg<std::string> outFileArg
  ("output",
   "Output.",
   true, "", "filename", cmd);
  
  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
  
  kernel::se3 t = kernel::se3(*readSingleObservation(inFileArg.getValue()));
  kernel::se3 o;
  kernel::se3 it = o.transformationFrom(t);
  writeSingleObservation(outFileArg.getValue(), it);
  
  return 0;
  
  NUKLEI_TRACE_END();
}


