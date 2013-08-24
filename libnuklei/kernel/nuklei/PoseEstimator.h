// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_POSE_ESTIMATOR_H
#define NUKLEI_POSE_ESTIMATOR_H

#include <nuklei/parallelizer.h>
#include <nuklei/KernelCollection.h>
#include <nuklei/ObservationIO.h>
#include <boost/bind.hpp>

namespace nuklei {
  
  const bool WEIGHTED_SUM_EVIDENCE_EVAL = false;
  const double MESHTOL = 4;
  const double WHITE_NOISE_POWER = 1e-4;
  
  struct AlwaysReachable
  {
    bool operator() (const kernel::se3& k) const
    {
      return true;
    }
  };
  
  template<class IsReachable = AlwaysReachable>
  struct PoseEstimator
  {
    PoseEstimator(const double locH = 0,
                  const double oriH = .2,
                  const int nChains = -1,
                  const int n = -1,
                  const IsReachable& reachable = AlwaysReachable(),
                  const bool& partialview = false) :
    
    reachable_(reachable), partialview_(partialview),
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
    IsReachable reachable_;
    bool partialview_;
    KernelCollection::EvaluationStrategy evaluationStrategy_;
    double loc_h_;
    double ori_h_;
    int nChains_;
    int n_;
  };
  
  
  
  
  template<class IsReachable>
  kernel::se3 PoseEstimator<IsReachable>::modelToSceneTransformation() const
  {
    int n = -1;
    
    if (n_ <= 0)
    {
      n = objectModel_.size();
      if (n > 1000)
      {
        NUKLEI_WARN("Warning: Object model has more than 1000 points. "
                    "To keep computational cost low, only 1000 points will be "
                    "used at each inference loop. "
                    "Use -n to force a large number of model points.");
        n = 1000;
      }
    }
    else
      n = n_;
    
    KernelCollection poses;
    if (!hasOpenMP())
    {
      NUKLEI_WARN("Nuklei has not been compiled with OpenMP support. "
                  "Pose estimation will use a single core.");
    }

    parallelizer p(nChains_, parallelizer::FORK);
    std::vector<kernel::se3> retv =
    p.run<kernel::se3>(boost::bind(&PoseEstimator::mcmc, this, n));
    for (std::vector<kernel::se3>::const_iterator i = retv.begin();
         i != retv.end(); ++i)
      poses.add(*i);
    
    kernel::se3 pose(*poses.sortBegin(1));
    pose.setWeight(findMatchingScore(pose));
    
    return pose;
  }
  
  template<class IsReachable>
  double
  PoseEstimator<IsReachable>::findMatchingScore(const kernel::se3& pose) const
  {
    kernel::se3 t = pose;
    
    if (!partialview_)
    {
      double w1 = 0, w2 = 0;
      
      KernelCollection tmp = objectModel_;
      tmp.transformWith(t);
      tmp.computeKernelStatistics();
      tmp.buildKdTree();
      
      for (KernelCollection::const_iterator i = objectModel_.begin();
           i != objectModel_.end(); ++i)
      {
        w1 += sceneModel_.evaluationAt(*i->polyTransformedWith(t),
                                       evaluationStrategy_);
      }
      
      for (KernelCollection::const_iterator i = sceneModel_.begin();
           i != sceneModel_.end(); ++i)
      {
        w2 += tmp.evaluationAt(*i, evaluationStrategy_);
      }
      
      t.setWeight(w1/objectModel_.size());
      //t.setWeight(std::sqrt(w1/objectModel_.size()*w2/objectModel_.size()));
    }
    else
    {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
      // Use a mesh to compute the partial view of the model and compute the
      // matching score from that.
      
      KernelCollection::const_partialview_iterator viewIterator =
      objectModel_.partialViewBegin(viewpointInFrame(t), MESHTOL);
      for (KernelCollection::const_partialview_iterator i = viewIterator;
           i != i.end(); ++i)
      {
        weight_t w = sceneModel_.evaluationAt(*i->polyTransformedWith(t),
                                              evaluationStrategy_);
        t.setWeight(t.getWeight() + w);
      }
      t.setWeight(t.getWeight()/std::pow(std::distance(viewIterator, viewIterator.end()), 1.0));
      
      if (!reachable_(t))
        t.setWeight(0);
#else
      NUKLEI_THROW("Requires the partial view version of Nuklei.");
#endif
    }
    
    return t.getWeight();
  }
  
  template<class IsReachable>
  void PoseEstimator<IsReachable>::load(const std::string& objectFilename,
                                        const std::string& sceneFilename,
                                        const std::string& meshfile,
                                        const std::string& viewpointfile,
                                        const bool light,
                                        const bool computeNormals)
  {
    KernelCollection objectModel, sceneModel;
    readObservations(objectFilename, objectModel);
    readObservations(sceneFilename, sceneModel);
    Vector3 viewpoint(0, 0, 0);
    if (partialview_)
    {
      NUKLEI_ASSERT(!viewpointfile.empty());
      viewpoint = kernel::se3(*readSingleObservation(viewpointfile)).getLoc();
    }
    load(objectModel, sceneModel, meshfile, viewpoint,
         light, computeNormals);
  }
  
  
  template<class IsReachable>
  void PoseEstimator<IsReachable>::load(const KernelCollection& objectModel,
                                        const KernelCollection& sceneModel,
                                        const std::string& meshfile,
                                        const Vector3& viewpoint,
                                        const bool light,
                                        const bool computeNormals)
  {
    objectModel_ = objectModel;
    sceneModel_ = sceneModel;
    
    if (objectModel_.size() == 0 || sceneModel_.size() == 0)
      NUKLEI_THROW("Empty input cloud.");
    
    if (objectModel_.front().polyType() == kernel::base::R3)
    {
      if (computeNormals)
      {
        objectModel_.buildNeighborSearchTree();
        objectModel_.computeSurfaceNormals();
      }
    }
    
    if (sceneModel_.front().polyType() == kernel::base::R3)
    {
      if (computeNormals)
      {
        sceneModel_.buildNeighborSearchTree();
        sceneModel_.computeSurfaceNormals();
      }
    }
    
    if (objectModel_.front().polyType() != sceneModel_.front().polyType())
      NUKLEI_THROW("Input point clouds must be defined on the same domain.");
    
    if (light && sceneModel_.size() > 10000)
    {
      sceneModel_.computeKernelStatistics();
      KernelCollection tmp;
      KernelCollection::sample_iterator i =
      sceneModel_.sampleBegin(10000);
      for (; i != i.end(); i++)
      {
        tmp.add(*i);
      }
      sceneModel_ = tmp;
    }
    
    objectSize_ = objectModel_.moments()->getLocH();
    
    if (loc_h_ <= 0)
      loc_h_ = objectSize_ / 10;
    
    sceneModel_.setKernelLocH(loc_h_);
    sceneModel_.setKernelOriH(ori_h_);
    objectModel_.setKernelLocH(loc_h_);
    objectModel_.setKernelOriH(ori_h_);
    
    objectModel_.computeKernelStatistics();
    sceneModel_.computeKernelStatistics();
    sceneModel_.buildKdTree();
    
    if (partialview_)
    {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
      if (!meshfile.empty())
        objectModel_.readMeshFromOffFile(meshfile);
      else
        objectModel_.buildMesh();
#else
      NUKLEI_THROW("Requires the partial view version of Nuklei.");
#endif
    }
  }
  
  
  // Temperature function (cooling factor)
  template<class IsReachable>
  double PoseEstimator<IsReachable>::Ti(const unsigned i, const unsigned F)
  {
    {
      const double T0 = .5;
      const double TF = .05;
      
      return std::max(T0 * std::pow(TF/T0, double(i)/F), TF);
    }
  }
  
  /**
   * This function implements the algorithm of Fig. 2: Simulated annealing
   * algorithm of the ACCV paper.
   * - T_j is given by @c temperature
   * - u is given by Random::uniform()
   * - w_j is @c currentPose
   */
  template<class IsReachable>
  void
  PoseEstimator<IsReachable>::metropolisHastings(kernel::se3& currentPose,
                                                 weight_t &currentWeight,
                                                 const weight_t temperature,
                                                 const bool firstRun,
                                                 const int n) const
  {
    NUKLEI_TRACE_BEGIN();
    
    // Randomly select particles from the object model
    std::vector<int> indices;
    for (KernelCollection::const_sample_iterator
         i = objectModel_.sampleBegin(n);
         i != i.end();
         i++)
    {
      indices.push_back(i.index());
    }
    std::random_shuffle(indices.begin(), indices.end(), Random::uniformInt);
    
    // Next chain state
    kernel::se3 nextPose;
    // Whether we go for a local or independent proposal
    bool independentProposal = false;
    
    if (Random::uniform() < .75 || firstRun)
    {
      // Go for independent proposal
      
      independentProposal = true;
      
      for (int count = 0; ; ++count)
      {
        if (count == 100) return;
        kernel::se3::ptr k2 = objectModel_.at(indices.front()).polySe3Proj();
        kernel::se3::ptr k1 = sceneModel_.at(Random::uniformInt(sceneModel_.size())).polySe3Proj();
        
        nextPose = k1->transformationFrom(*k2);
        
        if (!reachable_(nextPose)) continue;
        
        if (partialview_)
        {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
          bool visible =
          objectModel_.isVisibleFrom(objectModel_.at(indices.front()).getLoc(),
                                     viewpointInFrame(nextPose),
                                     MESHTOL);
          if (!visible) continue;
#else
          NUKLEI_THROW("Requires the partial view version of Nuklei.");
#endif
        }
        
        break;
      }
    }
    else
    {
      // Go for local proposal
      
      independentProposal = false;
      NUKLEI_DEBUG_ASSERT(currentPose.loc_h_ > 0 && currentPose.ori_h_ > 0);
      for (int count = 0; ; ++count)
      {
        if (count == 100) return;
        nextPose = currentPose.sample();
        if (!reachable_(nextPose)) continue;
        break;
      }
    }
    
    weight_t weight = 0;
    
    double threshold = Random::uniform();
    
    if (partialview_)
    {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
      // Fixme: we should take at most n of these:
      indices = objectModel_.partialView(viewpointInFrame(nextPose),
                                         MESHTOL);
#else
      NUKLEI_THROW("Requires the partial view version of Nuklei.");
#endif
      
    }
    // Go through the points of the model
    for (unsigned pi = 0; pi < indices.size(); ++pi)
    {
      const kernel::base& objectPoint =
      objectModel_.at(indices.at(pi));
      
      kernel::base::ptr test = objectPoint.polyTransformedWith(nextPose);
      
      weight_t w = 0;
      if (WEIGHTED_SUM_EVIDENCE_EVAL)
      {
        w = (sceneModel_.evaluationAt(*test,
                                      KernelCollection::WEIGHTED_SUM_EVAL) +
             WHITE_NOISE_POWER/sceneModel_.size() );
      }
      else
      {
        w = (sceneModel_.evaluationAt(*test, KernelCollection::MAX_EVAL) +
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
          currentPose = nextPose;
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
          currentPose = nextPose;
          currentWeight = nextWeight;
        }
        return;
      }
    }
    NUKLEI_THROW("Reached forbidden state.");
    NUKLEI_TRACE_END();
  }
  
  template<class IsReachable>
  kernel::se3
  PoseEstimator<IsReachable>::mcmc(const int n) const
  
  {
    kernel::se3 currentPose, bestPose;
    weight_t currentWeight = 0;
    bestPose.setWeight(currentWeight);
    metropolisHastings(currentPose, currentWeight, 1, true, n);
    
    //fixme: See if nSteps should be computed as a function of n.
    int nSteps = 1000;
    if (partialview_) nSteps = 3000;
    
    //fixme:
    nSteps = 10*n;
    
    for (int i = 0; i < nSteps; i++)
    {
      {
        // begin and end bandwidths for the local proposal
        coord_t bLocH = objectSize_/10;
        coord_t eLocH = objectSize_/40;
        coord_t bOriH = .1;
        coord_t eOriH = .02;
        
        unsigned e = nSteps-1;
        
        currentPose.setLocH(double(e-i)/e * bLocH +
                            double(i)/e * eLocH);
        currentPose.setOriH(double(e-i)/e * bOriH +
                            double(i)/e * eOriH);
        if (currentPose.loc_h_ <= 0)
        {
          NUKLEI_THROW("Unexpected value for currentPose.loc_h_.");
        }
      }
      
      metropolisHastings(currentPose, currentWeight,
                         Ti(i, nSteps/5), false, n);
      
      if (currentWeight > bestPose.getWeight())
      {
        bestPose = currentPose;
        bestPose.setWeight(currentWeight);
      }
    }
    
    return bestPose;
  }
  
  template<class IsReachable>
  Vector3 PoseEstimator<IsReachable>::viewpointInFrame(kernel::se3& frame) const
  {
    kernel::se3 origin;
    kernel::se3 invt = origin.transformationFrom(frame);
    kernel::r3 v;
    v.loc_ = viewpoint_;
    return v.transformedWith(invt).getLoc();
  }
  
  template<class IsReachable>
  void
  PoseEstimator<IsReachable>::writeAlignedModel(const std::string& filename,
                                                const kernel::se3& t) const
  {
    KernelCollection objectModel = objectModel_;
    objectModel.transformWith(t);
    
    if (partialview_)
    {
#ifdef NUKLEI_HAS_PARTIAL_VIEW
      KernelCollection coloredOE;
      for (KernelCollection::const_iterator i = objectModel.begin();
           i != objectModel.end(); ++i)
      {
        coloredOE.add(*i);
        if (objectModel.isVisibleFrom(i->getLoc(),
                                      viewpoint_,
                                      MESHTOL))
        {
          RGBColor c(0, 0, 1);
          ColorDescriptor d;
          d.setColor(c);
          coloredOE.back().setDescriptor(d);
        }
      }
      objectModel = coloredOE;
#endif
    }
    writeObservations(filename,
                      objectModel,
                      Observation::SERIAL);
  }
  
}

#endif
