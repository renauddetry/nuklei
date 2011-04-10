// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <iostream>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <nuklei/Random.h>
#include <nuklei/Common.h>
#include <nuklei/Log.h>

namespace nuklei {
  
  static gsl_rng * randomRng;  
  
  static boost::mutex mutex;
  
  bool Random::initialized_ = Random::init();
  
  bool Random::init()
  {
    const gsl_rng_type * T;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    randomRng = gsl_rng_alloc(T);
    
    
    unsigned seed = 0;
    const char * envVal = getenv("NUKLEI_RANDOM_SEED");
    if (envVal != NULL)
    {
      const char * log = getenv("NUKLEI_LOG_LEVEL");
      if (log != NULL && numify<unsigned>(log) >= Log::INFO)
        std::cout << "export " << "NUKLEI_RANDOM_SEED" << "="
        << numify<int>(envVal)
        << "\n";
      double seed_d = numify<double>(envVal);
      if (seed_d >= 0)
        seed = numify<unsigned>(envVal);
      else
        seed = time(NULL)*getpid();
    }
    Random::seed(seed);
    return true;
  }
  
  void Random::seed(unsigned s)
  {
    srandom(s);
#ifdef __APPLE__
    //BSD implementation of rand differs from random.
    srand(s);
#endif
    gsl_rng_set(randomRng, s);
  }
  
  long int Random::random()
  {
    //boost::unique_lock<boost::mutex> lock(mutex);
    return ::random();
  }
  
  //This function returns a double precision floating point number
  //uniformly distributed in the range [0,1). The range includes 0.0 but
  //excludes 1.0.
  double Random::uniform()
  {
    //boost::unique_lock<boost::mutex> lock(mutex);
    return gsl_rng_uniform(randomRng);
  }
  
  //This function returns a double precision floating point number
  //uniformly distributed in the range [a,b). The range includes a but
  //excludes b.
  double Random::uniform(double a, double b)
  {
    NUKLEI_FAST_ASSERT(a < b);
    return a + uniform()*(b-a);
  }
  
  //This function returns a random integer from 0 to n-1 inclusive by
  //scaling down and/or discarding samples from the generator r. All
  //integers in the range [0,n-1] are produced with equal probability.
  unsigned long int Random::uniformInt(unsigned long int n)
  {
    //boost::unique_lock<boost::mutex> lock(mutex);
    return gsl_rng_uniform_int(randomRng, n);
  }
  
  //This function returns a Gaussian random variate, with mean zero and
  //standard deviation sigma. Use the transformation z = \mu + x on the
  //numbers returned by gsl_ran_gaussian to obtain a Gaussian distribution
  //with mean \mu.
  double Random::gaussian(double sigma)
  {
    //boost::unique_lock<boost::mutex> lock(mutex);
    return gsl_ran_gaussian(randomRng, sigma);
  }
  
  double Random::beta(double a, double b)
  {
    //boost::unique_lock<boost::mutex> lock(mutex);
    return gsl_ran_beta(randomRng, a, b);
  }
  
  Vector2 Random::uniformDirection2d()
  {
    Vector2 dird;
    {
      //boost::unique_lock<boost::mutex> lock(mutex);
      gsl_ran_dir_2d(randomRng, &dird.X(), &dird.Y());
    }
    Vector2 dir(dird.X(), dird.Y());
    return dir;
  }
  
  Vector3 Random::uniformDirection3d()
  {
    Vector3 dird;
    {
      //boost::unique_lock<boost::mutex> lock(mutex);
      gsl_ran_dir_3d(randomRng, &dird.X(), &dird.Y(), &dird.Z());
    }
    Vector3 dir(dird.X(), dird.Y(), dird.Z());
    return dir;
  }
  
  Quaternion Random::uniformQuaternion()
  {
    // See Kuffner 2004 and Shoemake 1992.
    // A supposably "slightly faster" way could be read from
    // http://planning.cs.uiuc.edu/node198.html and Arvo 1992, but
    // would need to be tested.
    // Also, how would gsl_ran_dir_nd perform?
    
#if defined(NUKLEI_RANDOM_QUATERNION_MARSAGLIA_1972)
    coord_t x1, y1, s1, x2, y2, s2;
    for (;;)
    {
      x1 = uniform(-1, 1);
      y1 = uniform(-1, 1);
      s1 = x1*x1 + y1*y1;
      if (s1 < 1) break;
    }
    for (;;)
    {
      x2 = uniform(-1, 1);
      y2 = uniform(-1, 1);
      s2 = x2*x2 + y2*y2;
      if (s2 < 1) break;
    }
    coord_t root = std::sqrt( (1-s1)/s2 );
    Quaternion u(x1,
                 y1,
                 x2 * root,
                 y2 * root);
    return q;
#elif defined(NUKLEI_RANDOM_QUATERNION_GAUSSIAN_PROJ)
    // comparable to gsl_ran_dir_nd
    Quaternion u(gaussian(1),
                 gaussian(1),
                 gaussian(1),
                 gaussian(1));
    u.Normalize();
    return u;
#else // Fastest method (although MARSAGLIA is comparable)
    coord_t s = static_cast<coord_t>(uniform());
    //assert(s <= 1 && s >= 0);
    coord_t s1 = std::sqrt(1-s);
    coord_t s2 = std::sqrt(s);
    coord_t t1 = 2 * M_PI * static_cast<coord_t>(uniform());
    coord_t t2 = 2 * M_PI * static_cast<coord_t>(uniform());
    Quaternion u(std::cos(t2) * s2,
                 std::sin(t1) * s1,
                 std::cos(t1) * s1,
                 std::sin(t2) * s2);
    //assert(nuklei_wmf::Math<coord_t>::FAbs(u.Length()-1) < 1e-6);
    return u;
#endif
  }
  
  void Random::printRandomState()
  {
    NUKLEI_INFO("Random state: " <<
              NUKLEI_NVP(random()) << 
              "\n              " << NUKLEI_NVP(rand()) <<
              "\n              " << NUKLEI_NVP(Random::gaussian(1.0)));
  }
  
}
