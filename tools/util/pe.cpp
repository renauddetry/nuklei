// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/PoseEstimator.h>
#include <nuklei/ProgressIndicator.h>
#include <nuklei/Stopwatch.h>
#include <tclap/CmdLine.h>

int pe(int argc, char ** argv)
{
  try {
    
    using namespace nuklei;
    using namespace TCLAP;
    
    CmdLine cmd("");
    
    UnlabeledValueArg<std::string> objectFileArg
    ("object_model",
     "Object file.",
     true, "", "filename", cmd);
    
    UnlabeledValueArg<std::string> sceneFileArg
    ("scene_model",
     "Scene file.",
     true, "", "filename", cmd);
    
    ValueArg<std::string> alignedObjectModelFileArg
    ("", "aligned",
     "Transformed object model, matching object pose.",
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
     "OBSOLETE ARGUMENT. NORMALS ARE ALWAYS COMPUTED. "
     "Compute a normal vector for all input points. "
     "Makes pose estimation more robust.", cmd);
    
    SwitchArg accurateScoreArg
    ("s", "accurate_score",
     "OBSOLETE ARGUMENT. ACCURATE SCORE IS ALWAYS COMPUTED. "
     "Recompute the matching score using all input points "
     "(instead of using N points as given by -n N).", cmd);
    
    SwitchArg useWholeSceneCloudArg
    ("", "slow",
     "By default, only 10000 points of the scene point cloud are used, "
     "for speed. If --slow is specified, all input points are used.", cmd);
    
    SwitchArg timeArg
    ("", "time",
     "Print computation time.", cmd);
    
    SwitchArg partialviewArg
    ("", "partial",
     "Match only the visible side of the model to the object.", cmd);
    
    cmd.parse( argc, argv );
    Stopwatch sw("");
    if (!timeArg.getValue())
      sw.setOutputType(Stopwatch::QUIET);
    
    // ------------- //
    // Read-in data: //
    // ------------- //
    
    PoseEstimator pe(locHArg.getValue(),
                     oriHArg.getValue(),
                     nChainsArg.getValue(),
                     nArg.getValue(),
                     boost::shared_ptr<CustomIntegrandFactor>(),
                     partialviewArg.getValue());
    
    pe.load(objectFileArg.getValue(),
            sceneFileArg.getValue(),
            "",
            "",
            !useWholeSceneCloudArg.getValue(),
            true);
    
    sw.lap("data read");
    
    // ------------------------------- //
    // Prepare density for evaluation: //
    // ------------------------------- //
    
    
    kernel::se3 t = pe.modelToSceneTransformation();
    
    sw.lap("alignment");
    
    std::cout << "Matching score: " << t.getWeight() << std::endl;
    
    if (!bestTransfoArg.getValue().empty())
    {
      writeSingleObservation(bestTransfoArg.getValue(), t);
    }
    
    if (!alignedObjectModelFileArg.getValue().empty())
    {
      pe.writeAlignedModel(alignedObjectModelFileArg.getValue(), t);
    }
    
    sw.lap("output");
    
    
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


