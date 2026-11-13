// sesmesh: parallel loops on a small pool of std::threads (no OpenMP).
//
// The pool starts on first use with one thread per hardware thread (set_threads changes that).
// parallel_for hands out chunks of an index range to the pool's threads in turn; a call made from
// inside a parallel loop runs serially.  Each body also gets a thread number in [0, threads()), so it
// can use per-thread scratch space or partial results.
#pragma once

#include <algorithm>
#include <atomic>
#include <functional>

namespace sesmesh {

// The number of threads parallel loops use.
int threads();
// Sets it (n < 1: the number of hardware threads).
void set_threads( int n );
// True inside a parallel loop's body.
bool in_parallel();

// Runs job( thread ) once on each of the pool's threads (thread 0 is the caller) and waits for all
// of them.  An exception thrown by a job is passed on to the caller.
void run_on_threads( const std::function<void( int )>& job );

// Calls body( i, thread ) for every i in [0, n); the threads take chunks of `chunk` indices in turn.
template <class F>
void parallel_for( long n, F&& body, long chunk = 1 ) {
   if ( n <= 0 ) {
      return;
   }
   chunk = std::max( 1L, chunk );
   if ( threads() <= 1 || n <= chunk || in_parallel() ) {
      for ( long i = 0; i < n; ++i ) {
         body( i, 0 );
      }
      return;
   }
   std::atomic<long> next( 0 );
   run_on_threads( [&]( int thread ) {
      for ( ;; ) {
         const long b = next.fetch_add( chunk );
         if ( b >= n ) {
            return;
         }
         const long e = std::min( n, b + chunk );
         for ( long i = b; i < e; ++i ) {
            body( i, thread );
         }
      }
   } );
}

// The number of fixed blocks of `block` indices that cover [0, n).  Sums taken per block and added in
// block order do not depend on the number of threads.
inline long blocks( long n, long block ) { return n <= 0 ? 0 : ( n + block - 1 ) / block; }

}  // namespace sesmesh
