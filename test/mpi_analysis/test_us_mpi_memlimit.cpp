//! \file test_us_mpi_memlimit.cpp
//! \brief Unit tests of US_MPI_MemLimit (no MPI):  the job's memory limit from
//!        the Slurm environment and from fake cgroup and proc files, the
//!        default simulation cache size taken from it, and the pressure rules.

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

TEST_F( MemLimit, CgroupV1PicksHighestLevelOfTheJob )
{
   // Slurm's task level has the same limit as the job:  only the job level
   //  counts all ranks of the job
   write( "proc/self/cgroup", "4:cpu:/x\n7:memory:/slurm/uid_1/job_2/step_0/task_3\n" );
   write( "cg/memory/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/uid_1/memory.limit_in_bytes", "9223372036854771712\n" );
   write( "cg/memory/slurm/uid_1/job_2/memory.limit_in_bytes", "6442450944\n" );
   write( "cg/memory/slurm/uid_1/job_2/step_0/memory.limit_in_bytes", "6442450944\n" );
   write( "cg/memory/slurm/uid_1/job_2/step_0/task_3/memory.limit_in_bytes", "6442450944\n" );
   write( "cg/memory/slurm/uid_1/job_2/memory.usage_in_bytes", "3000000000\n" );
   write( "cg/memory/slurm/uid_1/job_2/memory.stat", "cache 5\ninactive_file 7\ntotal_inactive_file 1000000000\n" );

   const auto cg = US_MPI_MemLimit::find_cgroup( paths );
   EXPECT_EQ( cg.limit, 6442450944LL );
   EXPECT_NE( cg.usage_file.find( "job_2/memory.usage" ), std::string::npos );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_usage( cg ), 2000000000LL );
}

TEST_F( MemLimit, CgroupV1InContainerSkipsHostPath )
{
   write( "proc/self/cgroup", "7:cpuset,memory:/docker/abcdef\n" );
   write( "cg/memory/memory.limit_in_bytes", "17179869184\n" );
   EXPECT_EQ( US_MPI_MemLimit::find_cgroup( paths ).limit, 17179869184LL );
}

TEST_F( MemLimit, CgroupV2TightestParentWins )
{
   write( "proc/self/cgroup", "0::/a/b/c\n" );
   write( "cg/a/memory.max", "max\n" );
   write( "cg/a/b/memory.max", "1000000\n" );
   write( "cg/a/b/c/memory.max", "max\n" );
   write( "cg/a/b/memory.current", "400000\n" );
   write( "cg/a/b/memory.stat", "anon 1\ninactive_file 100000\n" );

   const auto cg = US_MPI_MemLimit::find_cgroup( paths );
   EXPECT_EQ( cg.limit, 1000000 );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_usage( cg ), 300000 );
}

TEST_F( MemLimit, NoCgroupLimit )
{
   write( "proc/self/cgroup", "0::/a\n" );
   write( "cg/a/memory.max", "max\n" );
   const auto cg = US_MPI_MemLimit::find_cgroup( paths );
   EXPECT_EQ( cg.limit, 0 );
   EXPECT_EQ( US_MPI_MemLimit::cgroup_usage( cg ), -1 );
}

TEST_F( MemLimit, NoCgroupFile )
{
   EXPECT_EQ( US_MPI_MemLimit::find_cgroup( paths ).limit, 0 );
}

TEST_F( MemLimit, NodeMemoryAndPrivateBytes )
{
   write( "proc/meminfo", "MemTotal:       1000 kB\nMemFree: 1 kB\nMemAvailable:    250 kB\n" );
   write( "proc/11/status", "Name: x\nRssAnon:\t   100 kB\nRssShmem: 7000 kB\n" );
   write( "proc/12/status", "RssAnon:\t   50 kB\n" );

   qint64 total = 0;
   qint64 avail = 0;
   EXPECT_TRUE( US_MPI_MemLimit::node_memory( total, avail, paths ) );
   EXPECT_EQ( total, 1024000 );
   EXPECT_EQ( avail, 256000 );
   // Shared pages do not count, and a gone process is no error
   EXPECT_EQ( US_MPI_MemLimit::private_bytes( { 11, 12, 13 }, paths ), 150 * 1024 );
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

// Where an MPI implementation keeps a shared window:  the directory of the
//  mapped file, none for System V or anonymous shared memory
TEST_F( MemLimit, MappedDirOfSharedWindows )
{
   write( "proc/self/maps",
          "00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/us_mpi_analysis\n"
          "7f0000000000-7f0000100000 rw-s 00000000 00:1a 4242        /dev/shm/mpich_shar_tmpAbC (deleted)\n"
          "7f1000000000-7f1000100000 rw-s 00000000 08:02 99          /tmp/ompi.host.1000/jf.0/1/osc_sm.seg\n"
          "7f2000000000-7f2000100000 rw-s 00000000 00:01 32768       /SYSV00000000 (deleted)\n"
          "7f3000000000-7f3000100000 rw-s 00000000 00:01 1           /dev/zero (deleted)\n"
          "7f4000000000-7f4000100000 rw-p 00000000 00:00 0 \n" );

   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f0000000010ULL, paths ), "/dev/shm" );
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f10000fffffULL, paths ), "/tmp/ompi.host.1000/jf.0/1" );
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f2000000000ULL, paths ), "" );
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f3000000000ULL, paths ), "" );
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f4000000000ULL, paths ), "" );
   // Past the end of a mapping, and not mapped at all
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x7f1000100000ULL, paths ), "" );
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( (const void*) 0x10ULL, paths ), "" );
}

TEST_F( MemLimit, NoMapsFile )
{
   int local = 0;
   EXPECT_EQ( US_MPI_MemLimit::mapped_dir( &local, paths ), "" );
   EXPECT_EQ( US_MPI_MemLimit::fs_free( "" ), -1 );
}

#ifdef Q_OS_UNIX
TEST_F( MemLimit, FreeBytesOfADirectory )
{
   EXPECT_GT( US_MPI_MemLimit::fs_free( tmp.path().toStdString() ), 0 );
   EXPECT_EQ( US_MPI_MemLimit::fs_free( ( tmp.path() + "/none" ).toStdString() ), -1 );
}
#endif

TEST( MemLimitRules, AllowedCapacity )
{
   EXPECT_EQ( US_MPI_MemLimit::allowed_capacity( 50000 * MB, 6000 * MB, -1 ), 3000 * MB );
   EXPECT_EQ( US_MPI_MemLimit::allowed_capacity( 1000 * MB, 6000 * MB, -1 ), 1000 * MB );
   EXPECT_EQ( US_MPI_MemLimit::allowed_capacity( 50000 * MB, 0, 1000 * MB ), 900 * MB );
   EXPECT_EQ( US_MPI_MemLimit::allowed_capacity( 50000 * MB, 0, -1 ), 50000 * MB );
   EXPECT_EQ( US_MPI_MemLimit::fit_shared_memory( 8000 * MB, 1000 * MB ), 900 * MB );
}

TEST( MemLimitRules, Pressure )
{
   using L = US_MPI_MemLimit;
   EXPECT_EQ( L::limit_pressure( 50, 100 ), L::NONE );
   EXPECT_EQ( L::limit_pressure( 80, 100 ), L::SOFT );
   EXPECT_EQ( L::limit_pressure( 90, 100 ), L::HARD );
   EXPECT_EQ( L::limit_pressure( 90, 0 ), L::NONE );
   EXPECT_EQ( L::limit_pressure( -1, 100 ), L::NONE );
   EXPECT_EQ( L::node_pressure( 1000, 500 ), L::NONE );
   EXPECT_EQ( L::node_pressure( 1000, 40 ), L::SOFT );
   EXPECT_EQ( L::node_pressure( 1000, 20 ), L::HARD );
   EXPECT_EQ( L::shm_pressure( 1000 * MB, 1000 * MB ), L::NONE );
   EXPECT_EQ( L::shm_pressure( 40 * MB, 1000 * MB ), L::SOFT );
   EXPECT_EQ( L::shm_pressure( 10 * MB, 1000 * MB ), L::HARD );
   EXPECT_EQ( L::shm_pressure( -1, 1000 * MB ), L::NONE );
}
} // namespace
