// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/parallelizer.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace nuklei {
  
  const std::string parallelizer::TypeNames[] = { "openmp", "fork", "pthread", "single" };
  
  void parallelizer::reap()
  {
    // Reap completed child processes so that we don't end up with zombies.
    int status = 0;
    while (::waitpid(-1, &status, WNOHANG) > 0) {}
  }
  
}
