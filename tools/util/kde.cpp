// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <string>
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/tuple/tuple.hpp>

#include <tclap/CmdLine.h>
#include <nuklei/KernelCollection.h>
#include <nuklei/SerializedKernelObservationIO.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/ProgressIndicator.h>
#include <nuklei/Stopwatch.h>

using namespace nuklei;

int kde(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Kernel Width Computation App." );

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
  
  TCLAP::ValueArg<coord_t> fixedWidthArg
    ("f", "fixed_width",
     "Fixed location width. Obsolete argument. Please use --fixed_loc_width "
     "instead.",
     false, -1, "float", cmd);

  TCLAP::ValueArg<coord_t> fixedLocWidthArg
    ("l", "fixed_loc_width",
     "Fixed location width.",
     false, -1, "float", cmd);

  TCLAP::ValueArg<coord_t> fixedOriWidthArg
    ("o", "fixed_ori_width",
     "Fixed orientation width.",
     false, -1, "float", cmd);

  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);

  KernelCollection kc;
  readObservations(inFileArg.getValue(), kc);
  
  coord_pair w(0,0);
  
  bool fwa = fixedWidthArg.getValue() >= 0,
    flwa = fixedLocWidthArg.getValue() >= 0,
    fowa = fixedOriWidthArg.getValue() >= 0;
  
  if (fwa || flwa || fowa)
  {
    if (fwa && flwa)
      NUKLEI_THROW("Please use either -l or -f.");
    if (! ((fwa || flwa) && fowa))
      NUKLEI_THROW("If fixed width is specified, it needs to be for both loc and ori.");
    
    if (fwa)
      w = coord_pair(fixedWidthArg.getValue(), fixedOriWidthArg.getValue());
    else
      w = coord_pair(fixedLocWidthArg.getValue(), fixedOriWidthArg.getValue());
    
    kc.setKernelLocH(w.first);
    kc.setKernelOriH(w.second);
  }
  else
  {
    NUKLEI_THROW("Not implemented.");
  }
    
  writeObservations(outFileArg.getValue(), kc);

  return 0;
  
  NUKLEI_TRACE_END();
}

int resample(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Resampling App." );

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
  
  TCLAP::ValueArg<int> nObsArg
    ("n", "num_obs",
     "Number of output observations.",
     true, -1, "int", cmd);

  TCLAP::SwitchArg uniformArg
    ("u", "uniform",
     "Use a box kernel.", cmd);

  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);

  // This was previously set through an env var.
  // Let's make sure that all scripts are updated to set this.
  //if (!exactArg.getValue())
  //  NUKLEI_WARN("Warning: using approximate densities.");

  NUKLEI_ASSERT(nObsArg.getValue() >= 0);

  std::auto_ptr<ObservationReader> reader =
    ObservationReader::createReader(inFileArg.getValue());

  KernelCollection kc, sample;
  readObservations(*reader, kc);
  kc.computeKernelStatistics();
  
  if (uniformArg.getValue())
  {
    NUKLEI_WARN("Warning: uniform sampling only works for SE(3)");
  }
  Stopwatch sw("");
  for (KernelCollection::const_sample_iterator i = as_const(kc).sampleBegin(nObsArg.getValue());
       i != i.end(); ++i)
  {
    if (uniformArg.getValue())
    {
      kernel::se3 c(*i);
      kernel::se3 s;
      {
        typedef nuklei::unnormalized_shape_dist_kernel
        <groupS::r3, shapeS::box> r3_kernel;
        
        typedef nuklei::sampler<r3_kernel> r3_sampler;
        s.loc_ = r3_sampler::s(c.loc_, c.loc_h_);
      }
      for (;;)
      {
        s.ori_ = Random::uniformQuaternion();
        if (dist<groupS::so3>::d(c.ori_, s.ori_) < c.ori_h_)
        {
          break;
        }
      }
      sample.add(s);
    }
    else
      sample.add(*i->polySample());
  }
  sw.lap("");
  
  KernelWriter writer(outFileArg.getValue());
  writer.init();
  writeObservations(writer, sample);
  writer.writeBuffer();

  return 0;
  
  NUKLEI_TRACE_END();
}

int homogeneous_subset(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  std::cout << "This app is obsolete. See \n"
  "  nuklei conv --min_dist DIST FILE1 FILE2\n"
  "instead." << std::endl;

  return 0;
  
  NUKLEI_TRACE_END();
}

#include <nuklei/Plotter.h>

int importance_sampling(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();

  /* Parse command line arguments */

  TCLAP::CmdLine cmd(INFOSTRING + "Importance Sampling App." );

  /* Standard arguments */

  TCLAP::ValueArg<int> niceArg
    ("", "nice",
     "Proccess priority.",
     false, NICEINC, "int", cmd);

  /* Custom arguments */

  TCLAP::ValueArg<std::string> importanceDistributionFileArg
    ("i", "importance_distribution",
     "Importance distribution.",
     true, "", "filename", cmd);

  TCLAP::ValueArg<std::string> weightedSampleFileArg
    ("s", "weighted_samples",
     "Weighted samples.",
     true, "", "filename", cmd);

  TCLAP::ValueArg<weight_t> uniformComponentPowerArg
    ("u", "uniform_component_scale",
     "Let c be the value of this argument. The weight of each sample is "
     "computed as 1/(importance(s)+c/size(s)).",
     false, 0, "float", cmd);
  
  TCLAP::UnlabeledValueArg<std::string> outFileArg
    ("output",
     "Output.",
     true, "", "filename", cmd);

  cmd.parse( argc, argv );

  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);

  KernelCollection importanceDistribution, weightedSamples, empiricalDensity;
  {
    std::auto_ptr<ObservationReader> reader =
      ObservationReader::createReader(importanceDistributionFileArg.getValue());
    readObservations(*reader, importanceDistribution);
  }
  {
    std::auto_ptr<ObservationReader> reader =
      ObservationReader::createReader(weightedSampleFileArg.getValue());
    readObservations(*reader, weightedSamples);
  }
  
  importanceDistribution.normalizeWeights();
  importanceDistribution.computeKernelStatistics();
  importanceDistribution.buildKdTree();

  Plotter p;
  for (KernelCollection::iterator i = weightedSamples.begin();
       i != weightedSamples.end(); ++i)
  {
    coord_t iv = 0;
    
    if (importanceDistribution.kernelType() == i->polyType())
      iv = as_const(importanceDistribution).evaluationAt(*i, KernelCollection::WEIGHTED_SUM_EVAL);
    else if (importanceDistribution.kernelType() == kernel::base::R3XS2P &&
             i->polyType() == kernel::base::SE3)
    {
      kernel::se3 se3k(*i);
      kernel::r3xs2p r3xs2pk;
      r3xs2pk.loc_ = se3k.getLoc();
      r3xs2pk.dir_ = la::normalized(la::matrixCopy(se3k.ori_).GetColumn(2));
      iv = as_const(importanceDistribution).evaluationAt(r3xs2pk, KernelCollection::WEIGHTED_SUM_EVAL);
    }
    else NUKLEI_THROW("Unsupported proposal type.");
    
    coord_t w = iv + uniformComponentPowerArg.getValue()/as_const(importanceDistribution).size();
    
    p.push("pure", iv);
    p.push("with_noise", w);
    
    empiricalDensity.add(*i);
    empiricalDensity.back().setWeight(empiricalDensity.back().getWeight() / w);
  }
  p.write_r("/tmp/r.r");
  
  KernelWriter writer(outFileArg.getValue());
  writer.init();
  writeObservations(writer, empiricalDensity);
  writer.writeBuffer();

  return 0;
  
  NUKLEI_TRACE_END();
}




int evaluate(int argc, char ** argv)
{
  NUKLEI_TRACE_BEGIN();
  
  /* Parse command line arguments */
  
  TCLAP::CmdLine cmd(INFOSTRING + "Density Evaluation App." );
  
  /* Standard arguments */
  
  TCLAP::ValueArg<int> niceArg
  ("", "nice",
   "Proccess priority.",
   false, NICEINC, "int", cmd);
  
  /* Custom arguments */
  
  TCLAP::ValueArg<std::string> densityFileArg
  ("d", "density",
   "Density to evaluate.",
   true, "", "filename", cmd);
  
  TCLAP::ValueArg<std::string> pointFileArg
  ("p", "points",
   "Points at which the density is to be evaluated.",
   true, "", "filename", cmd);
    
//  TCLAP::UnlabeledValueArg<std::string> outFileArg
//  ("output",
//   "Output.",
//   true, "", "filename", cmd);
  
  cmd.parse( argc, argv );
  
  NUKLEI_ASSERT(setpriority(PRIO_PROCESS, 0, niceArg.getValue()) == 0);
    
  KernelCollection density, points;
  {
    std::auto_ptr<ObservationReader> reader =
    ObservationReader::createReader(densityFileArg.getValue());
    readObservations(*reader, density);
  }
  {
    std::auto_ptr<ObservationReader> reader =
    ObservationReader::createReader(pointFileArg.getValue());
    readObservations(*reader, points);
  }
  
  density.normalizeWeights();
  density.computeKernelStatistics();
  density.buildKdTree();
  
  for (KernelCollection::iterator i = points.begin();
       i != points.end(); ++i)
  {
    coord_t iv = as_const(density).evaluationAt(*i, KernelCollection::WEIGHTED_SUM_EVAL);
    std::cout << iv << std::endl;
  }
  
//  KernelWriter writer(outFileArg.getValue());
//  writer.init();
//  writeObservations(writer, empiricalDensity);
//  writer.writeBuffer();
  
  return 0;
  
  NUKLEI_TRACE_END();
}


