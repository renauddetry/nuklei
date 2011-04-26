// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */


#ifndef NUKLEI_RANDOM_H
#define NUKLEI_RANDOM_H

#include <nuklei/Definitions.h>
#include <nuklei/LinearAlgebraTypes.h>

namespace nuklei {
  
  /**
   * @author Renaud Detry <detryr@montefiore.ulg.ac.be>
   */
  class Random
  {
  public:
    Random() {};
    ~Random() {};
    
    static void seed(unsigned s);
    
    //Traditionnal random number.
    static long int random();
    
    //This function returns a double precision floating point number
    //uniformly distributed in the range [0,1). The range includes 0.0 but
    //excludes 1.0.
    static double uniform();
    
    //This function returns a double precision floating point number
    //uniformly distributed in the range [a,b). The range includes a but
    //excludes b.
    static double uniform(double a, double b);
    
    //This function returns a random integer from 0 to n-1 inclusive by
    //scaling down and/or discarding samples from the generator r. All
    //integers in the range [0,n-1] are produced with equal probability.
    static unsigned long int uniformInt(unsigned long int n);
    
    //This function returns a Gaussian random variate, with mean zero and
    //standard deviation sigma. Use the transformation z = \mu + x on the
    //numbers returned by gsl_ran_gaussian to obtain a Gaussian distribution
    //with mean \mu.
    static double gaussian(double sigma);
    
    // This function returns a random variate from the beta distribution.
    static double beta(double a, double b);
    
    //This function returns a random direction vector v = (x,y) in two
    //dimensions. The vector is normalized such that |v|^2 = x^2 + y^2 = 1.
    static Vector2 uniformDirection2d();
    
    //This function returns a random direction vector v = (x,y,z) in three
    //dimensions. The vector is normalized such that |v|^2 = x^2 + y^2 + z^2
    //= 1.
    static Vector3 uniformDirection3d();
    
    static Quaternion uniformQuaternion();
    
    static void printRandomState();
    
    static bool init();
  private:
    static bool initialized_;
  };
  
}

#endif
