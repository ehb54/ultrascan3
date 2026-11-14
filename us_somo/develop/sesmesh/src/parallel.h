// sesmesh: parallel loops on a small pool of std::threads (no OpenMP).
//
// The pool starts on first use with one thread per hardware thread (set_threads changes that).
// parallel_for hands out chunks of an index range to the pool's threads in turn; a call made from
// inside a parallel loop runs serially.  Each body also gets a thread number in [0, threads()), so it
// can use per-thread scratch space or partial results.
//
// The long calls (build_mesh, clean_mesh, check_mesh) can also report their progress to a ProgressFn
// and stop when it asks them to.
#pragma once

#include <algorithm>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <thread>

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

// progress( fraction, stage ): the fraction of a long call done so far (0 to 1, an estimate) and the
// name of its current stage.  It is called only on the thread that made the call, about every 0.1 s
// while the call runs (set_progress_interval changes that).  Returning false stops the call: the
// loops end at their next chunk and the call throws Cancelled, leaving its output unusable.  The
// function must not call into sesmesh.
using ProgressFn = std::function<bool( double fraction, const char* stage )>;

// Thrown by a call whose ProgressFn returned false.
struct Cancelled : std::runtime_error {
   Cancelled() : std::runtime_error( "cancelled" ) {}
};

// The least time between two progress reports, in seconds (default 0.1; 0: after every chunk of a loop).
void set_progress_interval( double seconds );
double progress_interval();

// A long call's progress through its stages, passed on to a ProgressFn (none: it does nothing).
// Create it on the calling thread.
class Progress {
 public:
   explicit Progress( const ProgressFn& fn );
   bool active() const { return bool( fn_ ); }
   // The stage that starts now: it takes the call from fraction `from` to `to`.
   void stage( const char* name, double from, double to );
   // A fraction f of the current stage is done.  On the calling thread, this passes the progress on
   // (at most every progress_interval() seconds) and throws Cancelled once fn has asked to stop; on
   // other threads it does nothing.
   void at( double f );
   // True once fn has asked to stop.
   bool stopped() const { return stop_.load( std::memory_order_relaxed ); }

 private:
   ProgressFn fn_;
   std::thread::id owner_;
   double interval_, last_, from_ = 0, to_ = 0;
   const char* stage_ = "";
   std::atomic<bool> stop_;
};

// Calls body( i, thread ) for every i in [0, n); the threads take chunks of `chunk` indices in turn.
// With a progress, the loop reports the fraction of [0, n) done after each chunk, and ends early
// (throwing Cancelled) once the progress function asks to stop.
template <class F>
void parallel_for( long n, F&& body, long chunk = 1, Progress* progress = nullptr ) {
   if ( n <= 0 ) {
      return;
   }
   chunk = std::max( 1L, chunk );
   Progress* p = progress && progress->active() ? progress : nullptr;
   if ( threads() <= 1 || n <= chunk || in_parallel() ) {
      for ( long i = 0; i < n; ++i ) {
         body( i, 0 );
         if ( p && ( ( i + 1 ) % chunk == 0 || i + 1 == n ) ) {
            p->at( double( i + 1 ) / double( n ) );
         }
      }
      return;
   }
   std::atomic<long> next( 0 ), done( 0 );
   run_on_threads( [&]( int thread ) {
      for ( ;; ) {
         if ( p && p->stopped() ) {
            return;
         }
         const long b = next.fetch_add( chunk );
         if ( b >= n ) {
            return;
         }
         const long e = std::min( n, b + chunk );
         for ( long i = b; i < e; ++i ) {
            body( i, thread );
         }
         if ( p ) {
            const long d = done.fetch_add( e - b ) + ( e - b );
            if ( thread == 0 ) {
               p->at( double( d ) / double( n ) );
            }
         }
      }
   } );
   if ( p && p->stopped() ) {
      throw Cancelled();
   }
}

// The number of fixed blocks of `block` indices that cover [0, n).  Sums taken per block and added in
// block order do not depend on the number of threads.
inline long blocks( long n, long block ) { return n <= 0 ? 0 : ( n + block - 1 ) / block; }

}  // namespace sesmesh
