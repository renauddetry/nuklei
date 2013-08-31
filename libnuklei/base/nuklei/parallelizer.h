// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_PARALLELIZER_H
#define NUKLEI_PARALLELIZER_H

#include <nuklei/Random.h>
#include <nuklei/Common.h>
#include <nuklei/BoostSerialization.h>

#include <cstdlib>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace nuklei {
  
  struct parallelizer
  {
    typedef enum { OPENMP = 0, FORK, PTHREAD, SINGLE, UNKNOWN } Type;
    static const Type defaultType = OPENMP;
    static const std::string TypeNames[];
    
    /**
     * If chosing the fork()-based implementation, make sure that your program
     * consists of a single thread at the time run() is called, or you will run
     * into problems (Google "forking a multithreaded program" to see why).
     */
    parallelizer(const int n,
                 const Type& type = OPENMP,
                 const unsigned long seed = 0) :
    n_(n), type_(type), seed_(seed) {}
    
    struct na_print_accessor
    {
      template<typename T>
      std::string operator()(const T& t)
      {
        return "n/a";
      }
    };

    struct print_accessor
    {
      template<typename T>
      const T& operator()(const T& t)
      {
        return t;
      }
    };
    
    template
    <typename R, typename Callable>
    std::vector<R> run(Callable callable) const
    {
      return run<R>(callable, na_print_accessor());
    }

    template
    <typename R, typename Callable, typename PrintAccessor>
    std::vector<R> run(Callable callable,
                       PrintAccessor pa) const
    {
      switch (type_)
      {
        case OPENMP:
          return run_openmp<R>(callable, pa);
          break;
        case FORK:
          return run_fork<R>(callable, pa);
          break;
        case PTHREAD:
          return run_pthread<R>(callable, pa);
          break;
        case SINGLE:
          return run_single<R>(callable, pa);
          break;
        default:
          NUKLEI_THROW("Unknown parallelization method.");
      }
      return std::vector<R>();
    }
    
  private:
    
    template<typename R, typename Callable, typename PrintAccessor>
    std::vector<R> run_openmp(Callable callable,
                              PrintAccessor pa) const
    {
      std::vector<R> retv;
#ifdef _OPENMP
#pragma omp parallel for
#endif
      for (int i = 0; i < n_; ++i)
      {
        R tmp = callable();
#ifdef _OPENMP
#pragma omp critical(nuklei_parallelizer_merge)
#endif
        {
          retv.push_back(tmp);
          NUKLEI_INFO("Finished OpenMP thread " << i << " with value "
                      << pa(tmp) << ".");
        }
      }
      return retv;
    }
    
    template<typename R, typename Callable, typename PrintAccessor>
    std::vector<R> run_fork(Callable callable,
                            PrintAccessor pa) const
    {
      boost::filesystem::path endpoint_name = boost::filesystem::unique_path("nuklei-%%%%-%%%%-%%%%-%%%%");
      //std::vector<pid_t> pids(n_, 0);
      std::vector<R> retv;
      for (int i = 0; i < n_; i++)
      {
        pid_t pid = fork();
        NUKLEI_ASSERT(pid >= 0);
        if (pid == 0)
        {
          Random::seed(seed_+i); // unsigned overflow wraps around.
          
          using boost::asio::local::stream_protocol;
          
          R tmp = callable();
          
          {
            stream_protocol::endpoint ep(endpoint_name.native());
            stream_protocol::iostream stream(ep);
            boost::archive::binary_oarchive oa(stream);
            oa & i & BOOST_SERIALIZATION_NVP(tmp);
          }
          
          _exit(0);
        }
        else
        {
          //pids.at(i) = pid;
        }
      }
      
      using boost::asio::local::stream_protocol;
      stream_protocol::endpoint ep(endpoint_name.native());
      boost::asio::io_service io_service;
      stream_protocol::acceptor acceptor(io_service, ep);
      
      for (int i = 0; i < n_; i++)
      {
        R tmp;
        int fork_i = 0;
        {
          stream_protocol::iostream stream;
          acceptor.accept(*stream.rdbuf());
          boost::archive::binary_iarchive ia(stream);
          ia & fork_i & BOOST_SERIALIZATION_NVP(tmp);
        }
        retv.push_back(tmp);
        
        NUKLEI_INFO("Finished fork " << fork_i << " with value "
                    << pa(tmp) << ".");
      }
      
      // clean up:
      boost::filesystem::remove(endpoint_name);
      reap();
      
      return retv;
    }
    
    template<typename R, typename Callable>
    struct pthread_wrapper
    {
      pthread_wrapper(Callable callable) : callable_(callable) {}
      void operator()(R& ret)
      {
        ret = callable_();
      }
    private:
      Callable callable_;
    };
    
    template<typename R, typename Callable, typename PrintAccessor>
    std::vector<R> run_pthread(Callable callable,
                               PrintAccessor pa) const
    {
      std::vector<R> retv(n_);
      std::vector< boost::shared_ptr<boost::thread> > threads;
      for (int i = 0; i < n_; ++i)
      {
        // This does not work without the first boost::ref (even if bind<void>
        // is used. The solution is to parametrize run_pthread_stub with
        // boost::_bi::protected_bind_t<C>, and boost::protect(callable),
        // but _bi is not public.
        //boost::shared_ptr<boost::thread> thrd
        //(new boost::thread(boost::bind(run_pthread_stub<R, Callable>,
        //                               boost::ref(callable),
        //                               boost::ref(retv.at(i)))));
        // For future ref, here's the helper function:
        //template<typename R, typename Callable>
        //void run_pthread_stub(Callable callable, R& ret)
        //{
        //  ret = callable();
        //}
        
        boost::shared_ptr<boost::thread> thread
        (new boost::thread
         (boost::bind<void>(pthread_wrapper<R, Callable>(callable),
                            boost::ref(retv.at(i)))));
        threads.push_back(thread);
      }
      for (int i = 0; i < n_; ++i)
      {
        threads.at(i)->join();
        NUKLEI_INFO("Finished thread " << i << " with value "
                    << pa(retv.at(i)) << ".");
      }
      return retv;
    }
    
    template<typename R, typename Callable, typename PrintAccessor>
    std::vector<R> run_single(Callable callable,
                              PrintAccessor pa) const
    {
      std::vector<R> retv;
      for (int i = 0; i < n_; ++i)
      {
        R tmp = callable();
        retv.push_back(tmp);
        NUKLEI_INFO("Finished slice " << i << " with value "
                    << pa(tmp) << ".");
      }
      return retv;
    }
    
    static void reap();
    
    int n_;
    Type type_;
    unsigned long seed_;
  };
  
}

#endif
