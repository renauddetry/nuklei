// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_POSE_ESTIMATOR_H
#define NUKLEI_POSE_ESTIMATOR_H

#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>
#include <nuklei/Types.h>

#define NUKLEI_POSE_ESTIMATOR_POLYMORPHIC

namespace nuklei {
  
  const bool WEIGHTED_SUM_EVIDENCE_EVAL = false;
  const double MESHTOL = 4;
  const double WHITE_NOISE_POWER = 1e-4;
  
  struct Reachability
  {
    virtual ~Reachability() {}
    virtual bool test(const kernel::se3& k) const = 0;
  };
  
  struct PoseEstimator
  {
    PoseEstimator(const double locH = 0,
                  const double oriH = .2,
                  const int nChains = -1,
                  const int n = -1,
                  boost::shared_ptr<Reachability> reachability = boost::shared_ptr<Reachability>(),
                  const bool& partialview = false) :
    reachability_(reachability), partialview_(partialview),
    evaluationStrategy_(KernelCollection::MAX_EVAL),
    loc_h_(locH), ori_h_(oriH),
    nChains_(nChains), n_(n)
    {
      if (nChains_ <= 0) nChains_ = 8;
    }
    
    void load(const std::string& objectFilename,
              const std::string& sceneFilename,
              const std::string& meshfile = "",
              const std::string& viewpointfile = "",
              const bool light = true,
              const bool computeNormals = true);
    
    void load(const KernelCollection& objectModel,
              const KernelCollection& sceneModel,
              const std::string& meshfile = "",
              const Vector3& viewpoint = Vector3::ZERO,
              const bool light = true,
              const bool computeNormals = true);
    
    void usePartialViewEstimation(const Vector3& viewpoint)
    {
      viewpoint_ = viewpoint;
      partialview_ = true;
    }
    
    kernel::se3 modelToSceneTransformation() const;
    
    double findMatchingScore(const kernel::se3& pose) const;
    
    void writeAlignedModel(const std::string& filename,
                           const kernel::se3& t) const;
    
  private:
    
    Vector3 viewpointInFrame(kernel::se3& frame) const;
    
    // Temperature function (cooling factor)
    static double Ti(const unsigned i, const unsigned F);
    
    /**
     * This function implements the algorithm of Fig. 2: Simulated annealing
     * algorithm of the ACCV paper.
     * - T_j is given by @c temperature
     * - u is given by Random::uniform()
     * - w_j is @c currentPose
     */
    void
    metropolisHastings(kernel::se3& currentPose,
                       weight_t &currentWeight,
                       const weight_t temperature,
                       const bool firstRun,
                       const int n) const;
    
    kernel::se3
    mcmc(const int n) const;
    
    KernelCollection objectModel_;
    double objectSize_;
    KernelCollection sceneModel_;
    Vector3 viewpoint_;
    boost::shared_ptr<Reachability> reachability_;
    bool partialview_;
    KernelCollection::EvaluationStrategy evaluationStrategy_;
    double loc_h_;
    double ori_h_;
    int nChains_;
    int n_;
  };
  
}

#endif
