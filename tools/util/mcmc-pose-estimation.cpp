// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/ProgressIndicator.h>
#include <nuklei/Stopwatch.h>
#include <tclap/CmdLine.h>

const bool WEIGHTED_SUM_EVIDENCE_EVAL = false;
const double WHITE_NOISE_POWER = 1e-4;

namespace nuklei
{
  
  // Temperature function (cooling factor)
  static inline coord_t Ti(const unsigned i, const unsigned F)
  {
    {
      const coord_t T0 = .5;
      const coord_t TF = .05;
      
      return std::max(T0 * std::pow(TF/T0, double(i)/F), TF);
    }
  }
  
  /**
   * This function implements the algorithm of Fig. 2: Simulated annealing
   * algorithm of the ACCV paper.
   * - T_j is given by @c temperature
   * - u is given by Random::uniform()
   * - w_j is @c current
   */
  static void
  metropolisHastings(const KernelCollection& objectEvidence,
                     const KernelCollection& sceneEvidence,
                     kernel::se3& current,
                     weight_t &currentWeight,
                     const weight_t temperature,
                     const bool firstRun,
                     const int n)
  
  {
    NUKLEI_TRACE_BEGIN();

    // Randomly select particles from the object model
    std::vector<int> indices;
    for (KernelCollection::const_sample_iterator
         i = objectEvidence.sampleBegin(n);
         i != i.end();
         i++)
    {
      indices.push_back(i.index());
    }
    std::random_shuffle(indices.begin(), indices.end(), Random::uniformInt);
    
    // Next chain state
    kernel::se3 next;
    // Whether we go for a local or independent proposal
    bool independentProposal = false;
    
    if (Random::uniform() < .75 || firstRun)
    {
      // Go for independent proposal
      
      independentProposal = true;
      
      kernel::se3::ptr k1 = objectEvidence.at(indices.front()).polySe3Proj();
      kernel::se3::ptr k2 =
      sceneEvidence.at(Random::uniformInt(sceneEvidence.size())).polySe3Proj();
      
      next = k2->transformationFrom(*k1);
    }
    else
    {
      // Go for local proposal
      
      independentProposal = false;
      NUKLEI_DEBUG_ASSERT(current.loc_h_ > 0 && current.ori_h_ > 0);
      next = current.sample();
    }
    
    weight_t weight = 0;
    
    double threshold = Random::uniform();

    // Go through the points of the model
    for (unsigned pi = 0; pi < indices.size(); ++pi)
    {
      const kernel::base& objectPoint = objectEvidence.at(indices.at(pi));

      kernel::base::ptr test = objectPoint.polyTransformedWith(next);
      
      weight_t w = 0;
      if (WEIGHTED_SUM_EVIDENCE_EVAL)
      {
        w = (sceneEvidence.evaluationAt(*test,
                                        KernelCollection::WEIGHTED_SUM_EVAL) +
             WHITE_NOISE_POWER/sceneEvidence.size() );
      }
      else
      {
        w = (sceneEvidence.evaluationAt(*test, KernelCollection::MAX_EVAL) +
             WHITE_NOISE_POWER );
      }
      weight += w;
      
      // At least consider sqrt(size(model)) points
      if (pi < std::sqrt(indices.size())) continue;
      
      
      weight_t nextWeight = weight/(pi+1);
      
      // For the first run, consider all the points of the model
      if (firstRun)
      {
        if (pi == indices.size()-1)
        {
          current = next;
          currentWeight = nextWeight;
          return;
        }
        else continue;
      }
      
      weight_t dec = std::pow(nextWeight/currentWeight, 1./temperature);
      if (independentProposal) dec *= currentWeight/nextWeight;
      
      // Early abort
      if (dec < .6*threshold)
      {
        return;
      }
      
      // MH decision
      if (pi == indices.size()-1)
      {
        if (dec > threshold)
        {
          current = next;
          currentWeight = nextWeight;
        }
        return;
      }
    }
    NUKLEI_THROW("Reached forbidden state.");
    NUKLEI_TRACE_END();
  }
  
  static kernel::se3
  mcmc(const KernelCollection& objectEvidence,
       const KernelCollection& sceneEvidence,
       const coord_t objectSize,
       const int n)
  
  {
    kernel::se3 current, best;
    weight_t currentWeight = 0;
    best.setWeight(currentWeight);
    metropolisHastings(objectEvidence, sceneEvidence,
                       current, currentWeight, 1, true, n);
    
    const int nSteps = 10*n;
    for (int i = 0; i < nSteps; i++)
    {
      {
        // begin and end bandwidths for the local proposal
        coord_t bLocH = objectSize/10;
        coord_t eLocH = objectSize/40;
        coord_t bOriH = .1;
        coord_t eOriH = .02;
        
        unsigned e = nSteps-1;
        
        current.setLocH(double(e-i)/e * bLocH +
                        double(i)/e * eLocH);
        current.setOriH(double(e-i)/e * bOriH +
                        double(i)/e * eOriH);
        if (current.loc_h_ <= 0)
          NUKLEI_THROW("Unexpected value for curent.loc_h_");
      }
      
      metropolisHastings(objectEvidence, sceneEvidence,
                         current, currentWeight,
                         Ti(i, nSteps/5), false, n);
      
      if (currentWeight > best.getWeight())
      {
        best = current;
        best.setWeight(currentWeight);
      }
    }
    
    return best;
  }
  
  static kernel::se3 estimatePose(const KernelCollection &objectEvidence,
                                  const KernelCollection &sceneEvidence,
                                  const int nChains,
                                  const int n)
  {
    KernelCollection poses;
    coord_t objectSize = objectEvidence.moments()->getLocH();
    if (!hasOpenMP())
    {
      NUKLEI_WARN("Nuklei has not been compiled with OpenMP support. "
                  "Multithreaded execution of this program will not be optimal "
                  "(sync through POSIX threads, which seems slower than "
                  "OpenMP's critical.");
    }
#pragma omp parallel for
    for (int i = 0; i < nChains; ++i)
    {
      kernel::se3 tmp = mcmc(objectEvidence, sceneEvidence, objectSize, n);
#pragma omp critical(pe_merge_poses)
      {
        poses.add(tmp);
        std::cout << "Finished chain " << i << " with score " <<
        tmp.getWeight() << std::endl;
      }
    }
    Random::printRandomState();

    return kernel::se3(*poses.sortBegin(1));
  }
  
}


int mcmc_pose_estimation(int argc, char ** argv)
{
  try {
    
    using namespace nuklei;
    using namespace TCLAP;
    
    CmdLine cmd("");
    
    UnlabeledValueArg<std::string> objectFileArg
    ("object_evidence",
     "Object file.",
     true, "", "filename", cmd);
    
    UnlabeledValueArg<std::string> sceneFileArg
    ("scene_evidence",
     "Scene file.",
     true, "", "filename", cmd);
    
    ValueArg<std::string> alignedObjectEvidenceFileArg
    ("", "aligned",
     "Transformed object evidence, matching object pose.",
     false, "", "filename", cmd);
    
    ValueArg<int> nArg
    ("n", "n_model_points",
     "Number of particle supporting the object model.",
     false, 0, "int", cmd);
    
    ValueArg<double> locHArg
    ("l", "loc_h",
     "Location kernel width.",
     false, 0, "float", cmd);
    
    ValueArg<double> oriHArg
    ("o", "ori_h",
     "Orientation kernel width (in radians).",
     false, 0.2, "float", cmd);
    
    ValueArg<int> nChainsArg
    ("c", "n_chains",
     "Number of MCMC chains.",
     false, 0, "int", cmd);
    
    ValueArg<std::string> bestTransfoArg
    ("", "best_transfo",
     "File to write the most likely transformation to.",
     false, "", "filename", cmd);
    
    SwitchArg computeNormalsArg
    ("", "normals",
     "Compute a normal vector for all input points. Makes pose estimation more robust.", cmd);
    
    SwitchArg lightArg
    ("", "light",
     "Limit the scene model to 10000 points, for speed.", cmd);
    
    SwitchArg accurateScoreArg
    ("s", "accurate_score",
     "Recompute the matching score using all input points (instead of using N points as given by -n N).", cmd);
    
    SwitchArg timeArg
    ("", "time",
     "Print computation time.", cmd);
    
    cmd.parse( argc, argv );
    Stopwatch sw("");
    if (!timeArg.getValue())
      sw.setOutputType(Stopwatch::QUIET);

    // ------------- //
    // Read-in data: //
    // ------------- //
    
    KernelCollection objectEvidence, sceneEvidence;
    readObservations(objectFileArg.getValue(), objectEvidence);
    readObservations(sceneFileArg.getValue(), sceneEvidence);
    
    if (objectEvidence.size() == 0 || sceneEvidence.size() == 0)
      NUKLEI_THROW("Empty input cloud.");
    
    if (computeNormalsArg.getValue())
    {
      std::cout << "Computing normals for object model..." << std::endl;
      objectEvidence.buildNeighborSearchTree();
      objectEvidence.computeSurfaceNormals();
      std::cout << "Computing normals for object model... done." << std::endl;
    }
    else
    {
      if (objectEvidence.front().polyType() == kernel::base::R3)
      {
        std::cout << "Warning: object model is an R3 cloud. " <<
        "Pose estimation will be suboptimal. Use --normals to fix this." <<
        std::endl;
      }
    }
    
    if (computeNormalsArg.getValue())
    {
      std::cout << "Computing normals for scene model..." << std::endl;
      sceneEvidence.buildNeighborSearchTree();
      sceneEvidence.computeSurfaceNormals();
      std::cout << "Computing normals for scene model... done." << std::endl;
    }
    else
    {
      if (sceneEvidence.front().polyType() == kernel::base::R3)
      {
        std::cout << "Warning: scene model is an R3 cloud. " <<
        "Pose estimation will be suboptimal. Use --normals to fix this." <<
        std::endl;
      }
    }
    if (objectEvidence.front().polyType() != sceneEvidence.front().polyType())
      NUKLEI_THROW("Input point clouds must be defined on the same domain.");
    
    
    if (lightArg.getValue() && sceneEvidence.size() > 10000)
    {
      KernelCollection tmp;
      for (KernelCollection::sample_iterator i = sceneEvidence.sampleBegin(10000);
           i != i.end(); i++)
      {
        tmp.add(*i);
      }
      sceneEvidence = tmp;
    }
    
    if (sceneEvidence.size() > 10000)
      std::cout << "Warning: Scene model has more than 10000 points. "
      "To keep computation time low, keep the model under 10000 points. "
      "Use --light to fix this." << std::endl;
    
    
    // Kernel widths, for position and orientation:
    const double locH = (locHArg.getValue()<=0
                         ?
                         objectEvidence.moments()->getLocH()/10
                         :
                         locHArg.getValue()
                         );
    const double oriH = oriHArg.getValue(); // in radians
    
    // For best performances, choose a multiple of
    // the number of logical cores.
    const int nChains = (nChainsArg.getValue()<=0
                         ?
                         8
                         :
                         nChainsArg.getValue()
                         );
    
    int n = -1;
    
    if (nArg.getValue() <= 0)
    {
      n = objectEvidence.size();
      if (n > 1000)
      {
        std::cout << "Warning: Object model has more than 1000 points. "
        "To keep computational cost low, only 1000 points will be used at each "
        "inference loop. "
        "Use -n to force a large number of model points." << std::endl;
        n = 1000;
      }
    }
    else
      n = nArg.getValue();
    
    sw.lap("data read");
    
    // ------------------------------- //
    // Prepare density for evaluation: //
    // ------------------------------- //
    
    sceneEvidence.setKernelLocH(locH);
    sceneEvidence.setKernelOriH(oriH);
    objectEvidence.setKernelLocH(locH);
    objectEvidence.setKernelOriH(oriH);
    
    objectEvidence.computeKernelStatistics();
    sceneEvidence.computeKernelStatistics();
    sceneEvidence.buildKdTree();
    
    sw.lap("k-d tree");
    
    kernel::se3 t = estimatePose(objectEvidence, sceneEvidence, nChains, n);
    
    sw.lap("alignment");
    
    if (accurateScoreArg.getValue())
    {
      t.setWeight(0);
      for (KernelCollection::const_iterator i = objectEvidence.begin();
           i != objectEvidence.end(); ++i)
      {
        weight_t w = 0;
        if (WEIGHTED_SUM_EVIDENCE_EVAL)
        {
          w = sceneEvidence.evaluationAt(*i->polyTransformedWith(t),
                                         KernelCollection::WEIGHTED_SUM_EVAL);
        }
        else
        {
          w = sceneEvidence.evaluationAt(*i->polyTransformedWith(t), KernelCollection::MAX_EVAL);
        }
        t.setWeight(t.getWeight() + w);
      }
    }
    
    if (!bestTransfoArg.getValue().empty())
    {
      writeSingleObservation(bestTransfoArg.getValue(), t);
    }
    
    if (!alignedObjectEvidenceFileArg.getValue().empty())
    {
      objectEvidence.transformWith(t);
      writeObservations(alignedObjectEvidenceFileArg.getValue(), objectEvidence);
    }
    
    sw.lap("accurate score & output");
    
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


