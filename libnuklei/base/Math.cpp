// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <gsl/gsl_math.h>

#include <nuklei/Math.h>

namespace nuklei
{
    bool rfe(const double a, const double b, double tol)
    {
      return gsl_fcmp(a, b, tol) == 0;
    }
}
