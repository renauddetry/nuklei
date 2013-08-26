// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

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
