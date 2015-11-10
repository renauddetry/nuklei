// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

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

namespace nuklei {

// Either use GSL random gen, or Boost random gen
//#define NUKLEI_USE_BOOST_RANDOM_GEN

// Sync before accessing random gen?
// -> NUKLEI_RANDOM_SYNC_OMP: use OMP exclusive access
//    This one is obsolete, because when OMP is enabled, omp_get_thread_num
//    allows Random methods to access thread-specific random gens.
// -> NUKLEI_RANDOM_SYNC_MUTEX: use posix mutex
//    This makes Nuklei much slower
// -> NUKLEI_RANDOM_SYNC_NONE: default, use this on single-thread, or with OMP.
  
#define NUKLEI_RANDOM_SYNC_NONE

#ifdef _OPENMP
#include <omp.h>
  static inline int nuklei_thread_num()
  {
    return omp_get_thread_num();
  }
  static inline int nuklei_max_threads()
  {
    // I had issues with omp_get_max_threads(), returning 1 while spawning
    // more than one thread.
    // I had this with static executables.
    // I guess it could be a race condition.
    // Let's just return a large number instead (who would have more than 1000
    // procs anyway?)
    //return omp_get_max_threads();
    return 1024;
  }
#else
  static inline int nuklei_thread_num()
  {
    return 0;
  }
  static inline int nuklei_max_threads()
  {
    return 1;
  }
#endif

  
  // bRandGens must be a pointer. If not, its construtor may be called after
  // init() is called, which will destroy the bRandGens setup in init().
  static std::vector<boost::mt19937>* bRandGens;
  // Same holds for these:
  static std::vector<gsl_rng*>* gRandGens;
  static std::vector<boost::shared_ptr<boost::mutex> >* mutexes;
  // Keep in mind that it's unsafe to store pointers in std::vectors because
  // they could be discarted without being deallocated. Here it's fine as we
  // won't be resizing or deleting the vector before the end if the program.
  
  bool Random::initialized_ = Random::init();
  
  bool Random::init()
  {
    unsigned seed = 0;

    const char * envValPara = getenv("NUKLEI_PARALLELIZATION");
    if (envValPara != NULL)
    {
      std::string para(envValPara);
      if (para == "single" || para == "openmp")
      {
        // all ok
      }
      else if (para == "pthread")
      {
#if defined(NUKLEI_RANDOM_SYNC_OMP) || defined(NUKLEI_RANDOM_SYNC_NONE) || !defined(NUKLEI_RANDOM_SYNC_MUTEX)
        std::cout << "NUKLEI_PARALLELIZATION is set to pthread. "
        "You should manually undefine all NUKLEI_RANDOM_SYNC_* in Random.cpp, "
        "and define NUKLEI_RANDOM_SYNC_MUTEX instead. Note that multithreading "
        "will be slower than with OpenMP. "
        "See http://nuklei.sourceforge.net/doxygen/group__faq.html" << std::endl;
        std::terminate();
#endif
      }
    }
    
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
        seed = time(NULL)*getpid(); // Unsigned don't overflow, they wrap around
    }
    
    bRandGens = new std::vector<boost::mt19937>(nuklei_max_threads());
    gRandGens = new std::vector<gsl_rng*>(nuklei_max_threads(), NULL);
    for (int i = 0; i < nuklei_max_threads(); i++)
      gRandGens->at(i) = gsl_rng_alloc(gsl_rng_mt19937);
    
    mutexes = new std::vector<boost::shared_ptr<boost::mutex> >();
    for (int i = 0; i < nuklei_max_threads(); i++)
      mutexes->push_back(boost::shared_ptr<boost::mutex>(new boost::mutex()));
    
    Random::seed(seed);
    return true;
  }
  
  void Random::seed(unsigned s)
  {
    if (getenv("NUKLEI_RANDOM_SEED") != NULL)
    {
      // Libraries Nuklei depends on may make use of random numbers.
      // Let's make sure we seed those randomly as well.
      ::srandom(s);
      //BSD implementation of rand differs from random.
      ::srand(s);
    }
    
    for (int i = 0; i < bRandGens->size(); ++i)
    {
      bRandGens->at(i).seed(s+i);
    }
    for (int i = 0; i < gRandGens->size(); ++i)
    {
      gsl_rng_set(gRandGens->at(i), s+i+1); // +1 because GSL complains when seed == 0
    }
  }
  
  //This function returns a double precision floating point number
  //uniformly distributed in the range [0,1). The range includes 0.0 but
  //excludes 1.0.
  double Random::uniform()
  {
    double r;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    {
      boost::uniform_01<> dist;
      boost::variate_generator<boost::mt19937&, boost::uniform_01<> >
      die(bRandGens->at(nuklei_thread_num()), dist);
      r = die();
    }
#else
    r = gsl_rng_uniform(gRandGens->at(nuklei_thread_num()));
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
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    {
      boost::uniform_int<unsigned long> dist(0, n-1);
      boost::variate_generator<boost::mt19937&, boost::uniform_int<unsigned long> >
      die(bRandGens->at(nuklei_thread_num()), dist);
      r = die();
    }
#else
    r = gsl_rng_uniform_int(gRandGens->at(nuklei_thread_num()), n);
#endif
    return r;
  }
  
  double Random::triangle(double b)
  {
    double r;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    {
      boost::triangle_distribution<> dist(-b/2, 0, b/2);
      boost::variate_generator<boost::mt19937&, boost::triangle_distribution<> >
      die(bRandGens->at(nuklei_thread_num()), dist);
      r = die();
    }
    return r;
  }
  
  //This function returns a Gaussian random variate, with mean zero and
  //standard deviation sigma. Use the transformation z = \mu + x on the
  //numbers returned by gsl_ran_gaussian to obtain a Gaussian distribution
  //with mean \mu.
  double Random::gaussian(double sigma)
  {
    double r;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    {
      boost::normal_distribution<> dist(0, sigma);
      boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >
      die(bRandGens->at(nuklei_thread_num()), dist);
      r = die();
    }
#else
    r = gsl_ran_gaussian(gRandGens->at(nuklei_thread_num()), sigma);
#endif
    return r;
  }
  
  double Random::beta(double a, double b)
  {
    double r;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
    r = gsl_ran_beta(gRandGens->at(nuklei_thread_num()), a, b);
    return r;
  }
  
  Vector2 Random::uniformDirection2d()
  {
    Vector2 dir;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    {
      const int dim = 2;
      typedef boost::uniform_on_sphere<double, std::vector<double> > dist_t;
      dist_t dist(dim);
      boost::variate_generator<boost::mt19937&, dist_t >
      die(bRandGens->at(nuklei_thread_num()), dist);
      std::vector<double> r = die();
      dir.X() = r.at(0);
      dir.Y() = r.at(1);
    }
#else
    gsl_ran_dir_2d(gRandGens->at(nuklei_thread_num()), &dir.X(), &dir.Y());
#endif
    return dir;
  }
  
  Vector3 Random::uniformDirection3d()
  {
    Vector3 dir;
#if defined(NUKLEI_RANDOM_SYNC_OMP)
#  pragma omp critical(nuklei_randomRng)
#elif defined(NUKLEI_RANDOM_SYNC_MUTEX)
    boost::unique_lock<boost::mutex> lock(*mutexes->at(nuklei_thread_num()));
#elif defined(NUKLEI_RANDOM_SYNC_NONE)
#else
#  error Undefined random sync method
#endif
#ifdef NUKLEI_USE_BOOST_RANDOM_GEN
    const int dim = 3;
    typedef boost::uniform_on_sphere<double, std::vector<double> > dist_t;
    dist_t dist(dim);
    boost::variate_generator<boost::mt19937&, dist_t >
    die(bRandGens->at(nuklei_thread_num()), dist);
    std::vector<double> r = die();
    dir.X() = r.at(0);
    dir.Y() = r.at(1);
    dir.Z() = r.at(2);
#else
    gsl_ran_dir_3d(gRandGens->at(nuklei_thread_num()), &dir.X(), &dir.Y(), &dir.Z());
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
