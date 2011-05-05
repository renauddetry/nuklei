// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_sf_bessel.h>

#include <nuklei/Math.h>

namespace nuklei
{
  double confluentHypergeometric1F1(const double a, const double b, const double x)
  {
    return gsl_sf_hyperg_1F1(a, b, x);
  }
  
  double besselI1(const double x)
  {
    return gsl_sf_bessel_I1(x);
  }

  bool rfe(const double a, const double b, double tol)
  {
    return gsl_fcmp(a, b, tol) == 0;
  }
}
