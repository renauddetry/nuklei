// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <iostream>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <nuklei/Random.h>
#include <nuklei/Common.h>
#include <nuklei/Log.h>

#include <boost/random.hpp>

//#define NUKLEI_USE_BOOST_RANDOM_GEN

#ifdef _OPENMP
#define NUKLEI_RANDOM_SYNC_OMP
#include <omp.h>
static inline int nuklei_thread_num()
{
  return omp_get_thread_num();
}
static inline int nuklei_max_threads()
{
  return omp_get_max_threads();
}
#else
#define NUKLEI_RANDOM_SYNC_MUTEX
static inline int nuklei_thread_num()
{
  return 0;
}
static inline int nuklei_max_threads()
{
  return 1;
}
#endif



namespace nuklei {
  
  /** @brief GSL random generator */
  static gsl_rng * randomRng;  
  
  static boost::mutex mutex;
  
  // generators must be a pointer. If not, its construtor may be called after
  // init() is called, which will destroy the generators setup in init().
  static std::vector<boost::mt19937>* generators;
  
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
    generators = new std::vector<boost::mt19937>();
    generators->resize(nuklei_max_threads());
    Random::seed(seed);
    return true;
  }
  
  void Random::seed(unsigned s)
  {
    if (getenv("NUKLEI_RANDOM_SEED") != NULL)
    {
      // Libraries Nuklei depends on may make use of random numbers.
      // Let's make sure we seed those randomly as well.
      srandom(s);
#ifdef __APPLE__
      //BSD implementation of rand differs from random.
      srand(s);
#endif
    }
    gsl_rng_set(randomRng, s);
    if (generators)
      for (int i = 0; i < generators->size(); ++i)
      {
        generators->at(i).seed(s+i);
      }
  }
  
  //This function returns a double precision floating point number
  //uniformly distributed in the range [0,1). The range includes 0.0 but
  //excludes 1.0.
  double Random::uniform()
  {
    double r;
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    boost::uniform_01<> dist;
    boost::variate_generator<boost::mt19937&, boost::uniform_01<> >
    die(generators->at(nuklei_thread_num()), dist);
    r = die();
#else
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    r = gsl_rng_uniform(randomRng);
#endif
    return r;
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
    unsigned long int r;
    // GSL has trouble with concurrent random number generation:
    //   - if a single generator is used, it must be mutexed.
    //   - using one random generator per thread is somehow very slow
    // Random::uniformInt is used *a lot*, everytime a KernelCollection is
    // iterated in random order. Using a mutex here entirely breaks
    // multithreading (n threads on n cpus takes as much time as the same work
    // on a single cpu).  Multithreading will only be fast if done through
    // OpenMP. This is because it is hard to map pthreads to a
    // number. boost::thread::id could be used to implement nuklei_thread_num()
    // (todo?), but random generators could not be cleaned when a thread exits.
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
#else
#if defined(NUKLEI_RANDOM_SYNC_OMP)
    // no need to mutex here, since we use one generator per thread.
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
//    r = gsl_rng_uniform_int(randomRng, n);
#endif
    boost::uniform_int<> dist(0, n-1);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
    die(generators->at(nuklei_thread_num()), dist);
    r = die();
    return r;
  }
  
  //This function returns a Gaussian random variate, with mean zero and
  //standard deviation sigma. Use the transformation z = \mu + x on the
  //numbers returned by gsl_ran_gaussian to obtain a Gaussian distribution
  //with mean \mu.
  double Random::gaussian(double sigma)
  {
    double r;
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    boost::normal_distribution<> dist(0, sigma);
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >
    die(generators->at(nuklei_thread_num()), dist);
    r = die();
#else
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    r = gsl_ran_gaussian(randomRng, sigma);
#endif
    return r;
  }
  
  double Random::beta(double a, double b)
  {
    double r;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    r = gsl_ran_beta(randomRng, a, b);
    return r;
  }
  
  Vector2 Random::uniformDirection2d()
  {
    Vector2 dir;
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    const int dim = 2;
    typedef boost::uniform_on_sphere<double, std::vector<double> > dist_t;
    dist_t dist(dim);
    boost::variate_generator<boost::mt19937&, dist_t >
    die(generators->at(nuklei_thread_num()), dist);
    std::vector<double> r = die();
    dir.X() = r.at(0);
    dir.Y() = r.at(1);
#else
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    gsl_ran_dir_2d(randomRng, &dir.X(), &dir.Y());
#endif
    return dir;
  }
  
  Vector3 Random::uniformDirection3d()
  {
    Vector3 dir;
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    const int dim = 3;
    typedef boost::uniform_on_sphere<double, std::vector<double> > dist_t;
    dist_t dist(dim);
    boost::variate_generator<boost::mt19937&, dist_t >
    die(generators->at(nuklei_thread_num()), dist);
    std::vector<double> r = die();
    dir.X() = r.at(0);
    dir.Y() = r.at(1);
    dir.Z() = r.at(2);
#else
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(mutex);
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    gsl_ran_dir_3d(randomRng, &dir.X(), &dir.Y(), &dir.Z());
#endif
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
              "\n              " << NUKLEI_NVP(Random::uniformInt(1000000)) <<
              "\n              " << NUKLEI_NVP(Random::uniform()));
  }
  
}
