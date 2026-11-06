//! \file test_us_mpi_memlimit.cpp
//! \brief Unit tests of US_MPI_MemLimit (no MPI):  the job's memory limit from
//!        the Slurm environment and from fake cgroup and proc files, and the
//!        default simulation cache size taken from it.

#include <gtest/gtest.h>

#include "us_mpi_memlimit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

namespace
{
const qint64 MB = 1024 * 1024;

class MemLimit : public ::testing::Test
{
   protected:
      QTemporaryDir          tmp;
      US_MPI_MemLimit::Paths paths;

      void                   SetUp() override
      {
         ASSERT_TRUE( tmp.isValid() );
         paths.proc   = ( tmp.path() + "/proc" ).toStdString();
         paths.cgroup = ( tmp.path() + "/cg" ).toStdString();
         for ( const char* name : { "SLURM_MEM_PER_NODE", "SLURM_MEM_PER_CPU", "SLURM_CPUS_ON_NODE" } )
         {
            qunsetenv( name );
         }
      }

      void TearDown() override
      {
         for ( const char* name : { "SLURM_MEM_PER_NODE", "SLURM_MEM_PER_CPU", "SLURM_CPUS_ON_NODE" } )
         {
            qunsetenv( name );
         }
      }

      void write( const QString& name, const QString& text )
      {
         const QString path = tmp.path() + "/" + name;
         QDir().mkpath( QFileInfo( path ).path() );
         QFile file( path );
         ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
         file.write( text.toUtf8() );
      }
};

TEST_F( MemLimit, NoSlurmNoLimit )
{
   EXPECT_EQ( US_MPI_MemLimit::slurm_limit( 16 ), 0 );
}

TEST_F( MemLimit, SlurmPerNode )
{
   qputenv( "SLURM_MEM_PER_NODE", "6000" );
   EXPECT_EQ( US_MPI_MemLimit::slurm_limit( 16 ), 6000 * MB );
}

TEST_F( MemLimit, SlurmAllMemoryIsNoLimit )
{
   qputenv( "SLURM_MEM_PER_NODE", "0" );
   EXPECT_EQ( US_MPI_MemLimit::slurm_limit( 16 ), 0 );
}

TEST_F( MemLimit, SlurmPerCpuUsesCpusOnNode )
{
   qputenv( "SLURM_MEM_PER_CPU", "500" );
   qputenv( "SLURM_CPUS_ON_NODE", "8" );
   EXPECT_EQ( US_MPI_MemLimit::slurm_limit( 16 ), 4000 * MB );
}

TEST_F( MemLimit, SlurmPerCpuFallsBackToRanks )
{
   qputenv( "SLURM_MEM_PER_CPU", "500" );
   EXPECT_EQ( US_MPI_MemLimit::slurm_limit( 16 ), 8000 * MB );
}

TEST_F( MemLimit, CgroupV1TightestLevel )
{
   write( "proc/self/cgroup", "4:cpu:/x\n7:memory:/slurm/uid_1/job_2/step_0/task_3\n" );
   write( "cg/memory/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/uid_1/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/uid_1/job_2/memory.limit_in_bytes", "6442450944\n" );
   write( "cg/memory/slurm/uid_1/job_2/step_0/memory.limit_in_bytes", "6442450944\n" );
   write( "cg/memory/slurm/uid_1/job_2/step_0/task_3/memory.limit_in_bytes", "6442450944\n" );

   EXPECT_EQ( US_MPI_MemLimit::cgroup_limit( paths ), 6442450944LL );
}

TEST_F( MemLimit, CgroupV1InContainerSkipsHostPath )
{
   write( "proc/self/cgroup", "7:cpuset,memory:/docker/abcdef\n" );
   write( "cg/memory/memory.limit_in_bytes", "17179869184\n" );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_limit( paths ), 17179869184LL );
}

TEST_F( MemLimit, CgroupV2TightestParentWins )
{
   write( "proc/self/cgroup", "0::/a/b/c\n" );
   write( "cg/a/memory.max", "max\n" );
   write( "cg/a/b/memory.max", "1000000\n" );
   write( "cg/a/b/c/memory.max", "max\n" );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_limit( paths ), 1000000 );
}

TEST_F( MemLimit, NoCgroupLimit )
{
   write( "proc/self/cgroup", "0::/a\n" );
   write( "cg/a/memory.max", "max\n" );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_limit( paths ), 0 );
}

TEST_F( MemLimit, NoCgroupFile )
{
   EXPECT_EQ( US_MPI_MemLimit::cgroup_limit( paths ), 0 );
}

TEST_F( MemLimit, JobLimitIsTheTighterOfSlurmAndCgroup )
{
   EXPECT_EQ( US_MPI_MemLimit::job_limit( 4, paths ), 0 );

   qputenv( "SLURM_MEM_PER_NODE", "2000" );
   EXPECT_EQ( US_MPI_MemLimit::job_limit( 4, paths ), 2000 * MB );

   write( "proc/self/cgroup", "0::/job\n" );
   write( "cg/job/memory.max", QString::number( 1000 * MB ) + "\n" );
   EXPECT_EQ( US_MPI_MemLimit::job_limit( 4, paths ), 1000 * MB );

   qputenv( "SLURM_MEM_PER_NODE", "500" );
   EXPECT_EQ( US_MPI_MemLimit::job_limit( 4, paths ), 500 * MB );
}

TEST_F( MemLimit, NodeMemory )
{
   write( "proc/meminfo", "MemTotal:       1000 kB\nMemFree: 1 kB\nMemAvailable:    250 kB\n" );

   qint64 total = 0;
   qint64 avail = 0;
   EXPECT_TRUE( US_MPI_MemLimit::node_memory( total, avail, paths ) );
   EXPECT_EQ( total, 1024000 );
   EXPECT_EQ( avail, 256000 );
}

TEST_F( MemLimit, NoNodeMemory )
{
   qint64 total = 0;
   qint64 avail = 0;
   EXPECT_FALSE( US_MPI_MemLimit::node_memory( total, avail, paths ) );
}

// The reviewer's case:  16 ranks given 2 GB each on a 256 GB node must not
//  size the cache from the node's memory
TEST( MemLimitRules, DefaultCacheStaysInTheJobLimit )
{
   const qint64 max_bytes = 4096 * MB;
   const qint64 fallback  = 512 * MB;
   const qint64 job       = 16 * 2048 * MB;
   const qint64 node      = 256 * 1024 * MB;

   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( job, node, 16, 0.25, max_bytes, fallback ), 512 * MB );
   // Without a job limit the node's memory is shared by the ranks
   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( 0, node, 16, 0.25, max_bytes, fallback ), max_bytes );
   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( 0, 32 * 1024 * MB, 16, 0.25, max_bytes, fallback ), 512 * MB );
   // The node's available memory, when less than the job's limit
   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( job, 8 * 1024 * MB, 16, 0.25, max_bytes, fallback ), 128 * MB );
   // Nothing known:  the fixed fallback
   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( 0, -1, 16, 0.25, max_bytes, fallback ), fallback );
   EXPECT_EQ( US_MPI_MemLimit::default_cache_bytes( job, -1, 16, 0.25, max_bytes, fallback ), 512 * MB );
}
} // namespace
