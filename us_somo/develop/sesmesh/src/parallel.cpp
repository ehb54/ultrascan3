// sesmesh: the thread pool behind parallel.h.
#include "parallel.h"

#include <condition_variable>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace sesmesh {

namespace {

thread_local bool t_in_parallel = false;

int hardware_threads() {
   unsigned n = std::thread::hardware_concurrency();
   return n > 0 ? int( n ) : 1;
}

class Pool {
 public:
   Pool() { start( hardware_threads() ); }
   ~Pool() { stop(); }

   int size() const { return n_; }

   void resize( int n ) {
      std::lock_guard<std::mutex> run( run_m_ );
      stop();
      start( n );
   }

   void run( const std::function<void( int )>& job ) {
      if ( n_ <= 1 ) {
         call( job, 0 );
      } else {
         std::lock_guard<std::mutex> run( run_m_ );
         {
            std::lock_guard<std::mutex> lk( m_ );
            job_ = &job;
            pending_ = n_ - 1;
            error_ = nullptr;
            ++generation_;
         }
         cv_job_.notify_all();
         call( job, 0 );
         std::unique_lock<std::mutex> lk( m_ );
         cv_done_.wait( lk, [&] { return pending_ == 0; } );
         job_ = nullptr;
      }
      if ( error_ ) {
         std::exception_ptr e = error_;
         error_ = nullptr;
         std::rethrow_exception( e );
      }
   }

 private:
   void call( const std::function<void( int )>& job, int thread ) {
      t_in_parallel = true;
      try {
         job( thread );
      } catch ( ... ) {
         std::lock_guard<std::mutex> lk( m_ );
         if ( !error_ ) {
            error_ = std::current_exception();
         }
      }
      t_in_parallel = false;
   }

   void worker( int thread ) {
      long seen = 0;
      for ( ;; ) {
         const std::function<void( int )>* job;
         {
            std::unique_lock<std::mutex> lk( m_ );
            cv_job_.wait( lk, [&] { return quit_ || generation_ != seen; } );
            if ( quit_ ) {
               return;
            }
            seen = generation_;
            job = job_;
         }
         call( *job, thread );
         {
            std::lock_guard<std::mutex> lk( m_ );
            if ( --pending_ == 0 ) {
               cv_done_.notify_one();
            }
         }
      }
   }

   void start( int n ) {
      n_ = std::max( 1, n );
      quit_ = false;
      generation_ = 0;
      for ( int t = 1; t < n_; ++t ) {
         workers_.emplace_back( &Pool::worker, this, t );
      }
   }

   void stop() {
      {
         std::lock_guard<std::mutex> lk( m_ );
         quit_ = true;
      }
      cv_job_.notify_all();
      for ( std::thread& w : workers_ ) {
         w.join();
      }
      workers_.clear();
   }

   int n_ = 1;
   std::vector<std::thread> workers_;
   std::mutex m_, run_m_;
   std::condition_variable cv_job_, cv_done_;
   const std::function<void( int )>* job_ = nullptr;
   long generation_ = 0;
   int pending_ = 0;
   bool quit_ = false;
   std::exception_ptr error_;
};

Pool& pool() {
   static Pool p;
   return p;
}

}  // namespace

int threads() { return pool().size(); }

void set_threads( int n ) { pool().resize( n < 1 ? hardware_threads() : n ); }

bool in_parallel() { return t_in_parallel; }

void run_on_threads( const std::function<void( int )>& job ) { pool().run( job ); }

}  // namespace sesmesh
