// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

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
