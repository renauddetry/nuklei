// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_UTIL_H
#define NUKLEI_UTIL_H

int convert(int argc, char ** argv);
int concatenate(int argc, char ** argv);
int info(int argc, char ** argv);
int kde(int argc, char ** argv);
int importance_sampling(int argc, char ** argv);
int resample(int argc, char ** argv);
int homogeneous_subset(int argc, char ** argv);
int evaluate(int argc, char ** argv);
int test(int argc, char ** argv);
int invert_transfo(int argc, char ** argv);
int mcmc_pose_estimation(int argc, char ** argv);
int partial_view(int argc, char ** argv);
int create_mesh(int argc, char ** argv);

#endif
