#include <cuda.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "us_cuda.h"

#if defined( USE_MPI )
#   include "us_semaphore.h"
    extern int myrank;;
    int env_mpi_node;
#else 
#   define us_semaphore_unlock(x) ()
#endif

static int semid;

#  define CUDA_SAFE_CALL_NO_SYNC( call) {                                    \
    cudaError err = call;                                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        us_semaphore_unlock( semid );                                        \
        return false;                                                        \
    } }

#  define CUDA_SAFE_CALL( call)     CUDA_SAFE_CALL_NO_SYNC(call);                                            \

#  define CUT_CHECK_ERROR(errorMessage) {                                    \
    cudaError_t err = cudaGetLastError();                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error: %s in file '%s' in line %i : %s.\n",    \
                errorMessage, __FILE__, __LINE__, cudaGetErrorString( err) );\
        us_semaphore_unlock( semid );                                        \
        return false;                                                        \
    }                                                                        \
    }

// Prototypes
__global__ void helloWorld( char * );
__global__ void cudaDebye( 
                          unsigned int  n,
                          unsigned int  q_points,
                          const float * q,
                          const float * pos,
                          const float * fp,
                          float *       I
                          );

#if defined( USE_MPI )
bool cuda_ipcrm()
{
  // printf( "%d:cuda_ipcrm called node %d\n", myrank, env_mpi_node ); 
   int deviceCount; 
   cudaGetDeviceCount(&deviceCount); 
   for ( int i = 0; i < deviceCount; i++ )
   {
     int okid = myrank % deviceCount;
     key_t key = (key_t) ( 0x54000001 + env_mpi_node * 0x00001000 + okid );
     // printf( "%d:cuda_ipcrm called node %d key 0x%x device %d\n", myrank, env_mpi_node, key, okid ); 
     if ( !us_semaphore_exists( key, 1, semid ) )
     {
       // perror( "us_semaphore_exists");
       // printf( "%d:cuda_ipcrm non existant semaphore\n", myrank ); 
       return true;
     }
     if ( !us_semaphore_free( semid ) ) 
     {
       // perror( "us_semaphore_free");
       // printf( "%d:cuda_ipcrm error freeing semaphore\n", myrank ); 
       return false;
     }
   }
   // printf( "%d:cuda_ipcrm successfully removed\n", myrank ); 
   return true;
}
#endif      

// cuda debye, everything must be preallocated
bool cuda_debye( 
                bool         autocorrelate,
                unsigned int n,   // number of atoms
                float *      pos, // each atom will have 3 floats for coordinates, 
                // so this will have 3 * n entries

                unsigned int q_points,
                float *      q,   // this is the q grid

                float *      fp,  // this has the scattering factors for each atom
                // it is 2d and it will have n * q_points entries
                // currently it is in atom, q order
                // it would be good to test q, atom order since we are currently 
                // computing I(q) with q per thread
  
                // output:
                float *      I,    // the computed debye output, q_points
                unsigned int  threads_per_block
#if defined( USE_MPI )
		, int myrank
#endif
                )
{

#define MAXN 10000000

   if ( n > MAXN )
   {
      printf( "n was %u, now set to %u\n", n, MAXN );
      n = MAXN;
   }

   int deviceCount; 
   cudaGetDeviceCount(&deviceCount); 

   cudaDeviceProp props;
  
   int okid = -1;

   for ( int device = 0; device < deviceCount; ++device ) 
   { 
      CUDA_SAFE_CALL( cudaGetDeviceProperties( &props, device ) );

      printf("Device %d: \"%s\" with Compute %d.%d capability\n", device, props.name, props.major, props.minor);
      printf( 
             "totalGlobalMem               %lu\n"
             "sharedMemPerBlock            %d\n"
             "regsPerBlock                 %d\n"
             "warpSize                     %d\n"
             "memPitch                     %d\n"
             "maxThreadsPerBlock           %d\n"
             "maxThreadsDim                %d %d %d\n"
             "maxGridSize                  %d %d %d\n"
             "clockRate                    %d\n"
             "totalConstMem                %d\n"
             "multiProcessorCount          %d\n"
             "kernelExecTimeoutEnabled     %d\n"
             "canMapHostMemory             %d\n"
             "computeMode                  %d\n"
             "concurrentKernels            %d\n"
             "asyncEngineCount             %d\n"
             "unifiedAddressing            %d\n"
             "memoryClockRate              %d\n"
             "memoryBusWidth               %d\n"
             "l2CacheSize                  %d\n"
             "maxThreadsPerMultiProcessor  %d\n"
             
             , props.totalGlobalMem
             , props.sharedMemPerBlock
             , props.regsPerBlock
             , props.warpSize
             , props.memPitch
             , props.maxThreadsPerBlock
             , props.maxThreadsDim[0]
             , props.maxThreadsDim[1]
             , props.maxThreadsDim[2]
             , props.maxGridSize  [0]
             , props.maxGridSize  [1]
             , props.maxGridSize  [2]
             , props.clockRate
             , props.totalConstMem
             , props.multiProcessorCount
             , props.kernelExecTimeoutEnabled
             , props.canMapHostMemory
             , props.computeMode
             , props.concurrentKernels
             , props.asyncEngineCount
             , props.unifiedAddressing
             , props.memoryClockRate
             , props.memoryBusWidth
             , props.l2CacheSize
             , props.maxThreadsPerMultiProcessor
             );
      if ( props.kernelExecTimeoutEnabled )
      {
         printf( "Warning: cuda kernel timeout enabled on device %d.  Disconnect the display and try again!\n", device );
      } else {
         okid = device;
      }
   }

#if defined( USE_MPI )
   // swap per rank
   okid = myrank % deviceCount;
   key_t key = (key_t) ( 0x54000001 + env_mpi_node * 0x00001000 + okid );
   if ( !us_semaphore_init( key, 1, semid ) )
   {
      perror( "us_semaphore_init");
      return false;
   }
   printf( "%d:cuda_lock called node %d key 0x%x device %d\n", myrank, env_mpi_node, key, okid ); 
   if ( !us_semaphore_lock( semid ) ) 
   {
      perror( "us_semaphore_lock");
      return false;
   }
#endif

   if ( okid != -1 )
   {
      printf( "setting to device %d\n", okid );
      CUDA_SAFE_CALL( cudaSetDevice( okid ) );
   }

   // each thread will create its own I which we will have to sum at the end
   unsigned int threads         = q_points;
      
   unsigned int threadsPerBlock = threads_per_block;
   unsigned int blocksPerGrid   = (q_points + threadsPerBlock - 1) / threadsPerBlock;


   unsigned long memory_req     = 
      (unsigned long) ( q_points *     sizeof( float ) ) +
      (unsigned long) ( 3 * n *        sizeof( float ) ) +
      (unsigned long) ( n * q_points * sizeof( float ) ) +
      (unsigned long) ( q_points     * sizeof( float ) );;

   printf( "cuda_debye:\n"
           "memory required : %lu\n"
           "threads         : %u\n"
           "threadsperblock : %u\n"
           "blockspergrid   : %u\n"
           , memory_req
           , threads
           , threadsPerBlock
           , blocksPerGrid
           );

   if ( props.totalGlobalMem < memory_req )
   {
      printf( "insufficient memory to run cuda debye on this device" );
      return false;
   }

   printf( "starting cudaDebye\n" );

   // Allocate vectors in device memory
   float * d_pos;
   float * d_fp;
   float * d_q; 
   float * d_I; 

   CUDA_SAFE_CALL( cudaMalloc( (void**)&d_q  , q_points *     sizeof( float ) ) );
   CUDA_SAFE_CALL( cudaMalloc( (void**)&d_pos, 3 * n *        sizeof( float ) ) );
   CUDA_SAFE_CALL( cudaMalloc( (void**)&d_fp , n * q_points * sizeof( float ) ) );
   CUDA_SAFE_CALL( cudaMalloc( (void**)&d_I  , q_points     * sizeof( float ) ) );

   printf( "cudasync 0\n" );
   CUDA_SAFE_CALL( cudaDeviceSynchronize() );

   // Copy vectors from host memory to device memory
   CUDA_SAFE_CALL( cudaMemcpy( d_q  , q  , q_points *     sizeof( float ) , cudaMemcpyHostToDevice) );
   CUDA_SAFE_CALL( cudaMemcpy( d_pos, pos, 3 * n *        sizeof( float ) , cudaMemcpyHostToDevice) );
   CUDA_SAFE_CALL( cudaMemcpy( d_fp , fp , n * q_points * sizeof( float ) , cudaMemcpyHostToDevice) );

   printf( "cudasync 1\n" );
   CUDA_SAFE_CALL( cudaDeviceSynchronize() );

   // Invoke kernel
   cudaDebye<<<blocksPerGrid, threadsPerBlock>>>( n, q_points, d_q, d_pos, d_fp, d_I );

   CUT_CHECK_ERROR( "cudaDebye() execution failed\n" );

   // wait for kernel to finish
   printf( "cudasync 2\n" );
   cudaDeviceSynchronize();


   // Copy result from device memory to host memory
   // h_C contains the result in host memory
   CUDA_SAFE_CALL( cudaMemcpy( I, d_I, q_points * sizeof( float ), cudaMemcpyDeviceToHost) );

   bool all_zero = true;
   unsigned no_of_nonzeros = 0;
   for ( unsigned int i = 0; i < q_points; i++ )
   {
      if ( I[ i ] != 0e0 )
      {
         all_zero = false;
         no_of_nonzeros++;
      }
   }

   if ( all_zero )
   {
      printf( "hmm: cudaDebye returned I of all zeros!\n" );
   } else {
      printf( "hmm: cudaDebye had %u of %u nonzero\n", no_of_nonzeros, q_points );
   }      

   if ( d_q )
   {
      cudaFree( d_q );
   }
   if ( d_pos )
   {
      cudaFree( d_pos );
   }
   if ( d_fp )
   {
      cudaFree( d_fp );
   }
   if ( d_I )
   {
      cudaFree( d_I );
   }

   printf( "cudasync 3\n" );
   CUDA_SAFE_CALL( cudaDeviceSynchronize() );

#if defined( USE_MPI )
   if ( !us_semaphore_unlock( semid ) ) 
   {
      perror( "us_semaphore_lock");
      return false;
   }
#endif

   printf( "end cudaDebye\n" );
   return true;
}

// Host function
bool
cuda_hello_world()
{
   int deviceCount; 
   cudaGetDeviceCount(&deviceCount); 

   cudaDeviceProp props;

   int okid = -1;

   for ( int device = 0; device < deviceCount; ++device ) 
   { 
      CUDA_SAFE_CALL( cudaGetDeviceProperties( &props, device ) );

      printf("Device %d: \"%s\" with Compute %d.%d capability\n", device, props.name, props.major, props.minor);
      printf( 
             "totalGlobalMem               %lu\n"
             "sharedMemPerBlock            %d\n"
             "regsPerBlock                 %d\n"
             "warpSize                     %d\n"
             "memPitch                     %d\n"
             "maxThreadsPerBlock           %d\n"
             "maxThreadsDim                %d %d %d\n"
             "maxGridSize                  %d %d %d\n"
             "clockRate                    %d\n"
             "totalConstMem                %d\n"
             "multiProcessorCount          %d\n"
             "kernelExecTimeoutEnabled     %d\n"
             "canMapHostMemory             %d\n"
             "computeMode                  %d\n"
             "concurrentKernels            %d\n"
             "asyncEngineCount             %d\n"
             "unifiedAddressing            %d\n"
             "memoryClockRate              %d\n"
             "memoryBusWidth               %d\n"
             "l2CacheSize                  %d\n"
             "maxThreadsPerMultiProcessor  %d\n"
             
             , props.totalGlobalMem
             , props.sharedMemPerBlock
             , props.regsPerBlock
             , props.warpSize
             , props.memPitch
             , props.maxThreadsPerBlock
             , props.maxThreadsDim[0]
             , props.maxThreadsDim[1]
             , props.maxThreadsDim[2]
             , props.maxGridSize  [0]
             , props.maxGridSize  [1]
             , props.maxGridSize  [2]
             , props.clockRate
             , props.totalConstMem
             , props.multiProcessorCount
             , props.kernelExecTimeoutEnabled
             , props.canMapHostMemory
             , props.computeMode
             , props.concurrentKernels
             , props.asyncEngineCount
             , props.unifiedAddressing
             , props.memoryClockRate
             , props.memoryBusWidth
             , props.l2CacheSize
             , props.maxThreadsPerMultiProcessor
             );
      
      if ( props.kernelExecTimeoutEnabled )
      {
         printf( "Warning: cuda kernel timeout enabled on device %d.  Disconnect the display and try again!\n", device );
      } else {
         okid = device;
      }
   }

   if ( okid != -1 )
   {
      printf( "setting to device %d\n", okid );
      CUDA_SAFE_CALL( cudaSetDevice( okid ) );
   }

   int i;

   // desired output
   char str[] = "Hello World!";

   // mangle contents of output
   // the null character is left intact for simplicity
   for(i = 0; i < 12; i++)
      str[i] -= i;

   // allocate memory on the device 
   char *d_str;
   size_t size = sizeof(str);
   cudaMalloc((void**)&d_str, size);

   // copy the string to the device
   cudaMemcpy(d_str, str, size, cudaMemcpyHostToDevice);

   // set the grid and block sizes
   dim3 dimGrid(2);   // one block per word  
   dim3 dimBlock(6); // one thread per character
  
   // invoke the kernel
   helloWorld<<< dimGrid, dimBlock >>>(d_str);

   // retrieve the results from the device
   CUDA_SAFE_CALL( cudaMemcpy(str, d_str, size, cudaMemcpyDeviceToHost) );

   // free up the allocated memory on the device
   CUDA_SAFE_CALL ( cudaFree(d_str) );
  
   // everyone's favorite part
   printf("%s\n", str);

   return strncmp( str, "Hello World!" , 12 );
}

bool
cuda_reset()
{
   CUDA_SAFE_CALL( cudaDeviceReset        (               ) );
   return true;
}
   

// Device kernels
__global__ void

/*
** Hello World using CUDA
** 
** The string "Hello World!" is mangled then restored using a common CUDA idiom
**
** Byron Galbraith
** 2009-02-18
*/

helloWorld( char* str )
{
   // determine where in the thread grid we are
   int idx = blockIdx.x * blockDim.x + threadIdx.x;

   // unmangle output
   str[idx] += idx;
}

// currently autocorrolate always on

__global__ void
cudaDebye( 
          unsigned int  n,
          unsigned int  q_points,
          const float * q,
          const float * pos,
          const float * fp,
          float *       I
          )
{
   int idx = blockDim.x * blockIdx.x + threadIdx.x;
   // all blocks are full of threads so we may have more than needed
   if ( idx < q_points )
   {
      I[ idx ] = 0.0;

      const unsigned int nm1 = n - 1;
      unsigned int       iofs;
      unsigned int       jofs;
      float              sqrikd;
      float              qrik;
      const unsigned int fpbase = idx * n;

      float              fpi;

      unsigned int       i;
      unsigned int       j;

      for ( i = 0; i < nm1; ++i )
      {
         iofs = i * 3;
         fpi  = fp[ fpbase + i ];

         for ( j = i + 1; j < n; ++j )
         {
            jofs = j * 3;
            qrik = 
               q[ idx ] *
               sqrt(
                    ( pos[ iofs ]     - pos[ jofs ]     ) * ( pos[ iofs ]     - pos[ jofs ]     ) +
                    ( pos[ iofs + 1 ] - pos[ jofs + 1 ] ) * ( pos[ iofs + 1 ] - pos[ jofs + 1 ] ) +
                    ( pos[ iofs + 2 ] - pos[ jofs + 2 ] ) * ( pos[ iofs + 2 ] - pos[ jofs + 2 ] ) 
                    );

            sqrikd = ( fabs( qrik ) < 1e-20 ) ? 1.0 : sin( qrik ) / qrik;

            I[ idx ] += 2.0 * sqrikd * fpi * fp[ fpbase + j ];
         }
         I[ idx ] += fpi * fpi;
      }
      I[ idx ] += fp[ fpbase + nm1 ] * fp[ fpbase + nm1 ];
   }
}
