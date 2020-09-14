// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */


#include <string>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unistd.h>
#include <nuklei/Common.h>
#include <nuklei/Math.h>

namespace nuklei {
  
  const std::string INFOSTRING = 
  std::string("Built on " __DATE__ " at " __TIME__) +
  " with `" + NUKLEI_BUILDTYPE + "' flags.\n"
  "This application is part of Nuklei http://renaud-detry.net/nuklei\n"
  "To contact the author, see email at http://renaud-detry.net\n";
    
  template<typename T>
  static T initConst(const char *envVar, T def)
  {
    const char * envVal = getenv(envVar);
    if (envVal != NULL)
    {
      T val = numify<T>(envVal);
      
      const char * log = getenv("NUKLEI_LOG_LEVEL");
      if (log != NULL && numify<unsigned>(log) >= Log::INFO)
        std::cout << "export " << envVar << "=" << val
                  << " #(def: " << def << ").\n";
      return val;
    }
    else
      return def;
  }
  
#define defConst(TYPE, VAR, VAL) \
const TYPE VAR = initConst<TYPE>("NUKLEI_" #VAR, VAL)
  
  const coord_t FLOATTOL = 1e-12;
  
  // Default precision in text files.
  const int PRECISION = std::numeric_limits<double>::digits10 + 2;
    
  defConst(bool, KDTREE_DENSITY_EVAL, true);
  defConst(bool, KDTREE_NANOFLANN, true);
  
  defConst(unsigned int, KDE_KTH_NEAREST_NEIGHBOR, 8);
  
  // Remember that hue/saturation are projected on a disk of diameter 2.
  // A value of 1.0 for the following constant already quite lowers its
  // importance.
  // Rangoe is [0,1].
  defConst(appear_t, HSV_METRIC_VALUE_WEIGHT, .6);

  const int NICEINC = std::numeric_limits<int>::max();

  defConst(std::string, SERIALIZATION_DEFAULT_OUTPUT_TYPE, "serial");
  defConst(std::string, SERIALIZATION_DEFAULT_BOOST_ARCHIVE, "bbinc");

  defConst(unsigned, IMAGE_PROJECTION_RADIUS, 3);

#ifdef __APPLE__
  defConst(std::string, PARALLELIZATION, "single");
#else
  defConst(std::string, PARALLELIZATION, "openmp");
#endif

  defConst(bool, ENABLE_CONSOLE_BACKSPACE, true);
  
  defConst(unsigned, LOG_LEVEL, 0);
  
  // Should be moved to Log and protected by a mutex, when
  // ProgressIndicator is used in mt contexts.
  bool LAST_OUTPUT_LINE_IS_PROGRESS = false;
  
  const bool INTERACTIVE_SHELL =
    isatty(STDIN_FILENO) && isatty(STDOUT_FILENO) && isatty(STDERR_FILENO) &&
    (getenv("TERM") != NULL) &&
    (std::string(getenv("TERM")) != "dumb");

  bool hasOpenMP()
  {
#ifdef _OPENMP
    return true;
#else
    return false;
#endif
  }
  
#ifdef NDEBUG
#warning Compiling with NDEBUG set.
#endif
}
