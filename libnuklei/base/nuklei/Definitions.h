// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */


#ifndef NUKLEI_DEFINITIONS_H
#define NUKLEI_DEFINITIONS_H

#include <limits>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>

namespace nuklei {

/* --- Types --- */

  typedef double coord_t;
  typedef std::pair<coord_t, coord_t> coord_pair;
  
  // Used in Appearance.
  // KMeans takes vectors of doubles.
  // Mind it if changing the next line.
  typedef double appear_t;

  typedef double weight_t;
  
  typedef unsigned int bitfield_t;

  typedef unsigned int id_t;

  /* --- Constants & default values --- */

  // Constants are defined in a cpp file to prevent
  // re-compilation of the whole project at each modif.

  extern const coord_t FLOATTOL;

  extern const std::string INFOSTRING;

  // Variance for rotations until KDE is done.
  extern const coord_t ROTATION_STDEV;
  extern const coord_t OBSERVATION_ORIENTATION_STDEV;
  extern const coord_t OBSERVATION_LOCATION_STDEV;

  extern const bool OBSERVATION_COMPUTE_LOC_DIFF;

  // Default precision in text files.
  extern const int PRECISION;

  extern const unsigned N_PRIMITIVE_FEATURES;
  extern const unsigned SPATIAL_CLUSTERING;
  extern const double CLUSTER_SOFT_ASSIGN_THRESHOLD;
  extern const double CLUSTER_CLUSTER_CREATE_THRESHOLD;
  extern const bool LEARNING_ASSIGN_CLOSEST;
  
  extern const unsigned N_PASS;
  extern const unsigned N_PRIMARY_PASS;
  extern const unsigned REFINE_SIZE;
  extern const double PROPOSAL_SIZE_FACTOR;
  extern const bool MCMC_NBP;
  extern const unsigned MCMC_NBP_N_CHAINS;

  extern const bool KDTREE_DENSITY_EVAL;

  extern const coord_t LOCSTDEVMIN;
  extern const coord_t ORISTDEVMIN;

  extern const weight_t WHITE_NOISE_POWER;

  extern const unsigned int KDE_KTH_NEAREST_NEIGHBOR;

  extern const bool NORMALIZE_DENSITIES;

  extern const appear_t HSV_METRIC_VALUE_WEIGHT;

  // Be nice to other processes.
  extern const int NICEINC;

  // For object IO.
  extern const std::string SERIALIZATION_DEFAULT_BOOST_ARCHIVE;

  extern const unsigned IMAGE_PROJECTION_RADIUS;

  extern const bool ENABLE_CONSOLE_BACKSPACE;
  
  extern const unsigned LOG_LEVEL;

  extern bool LAST_OUTPUT_LINE_IS_PROGRESS;
  
  extern const bool INTERACTIVE_SHELL;
  
  extern const unsigned N_THREADS;
}

#endif


