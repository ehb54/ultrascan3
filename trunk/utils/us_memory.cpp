//! \file us_memory.cpp
#include "us_memory.h"

#ifdef Q_WS_MAC                          // Mac includes
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif
#ifndef Q_WS_WIN                         // Unix and Mac includes
#include <sys/user.h>
#else                                    // Windows includes
#include <windows.h>
#include <psapi.h>
#include <process.h>
#endif

// Get currently used memory (RSS) in kilobytes
long int US_Memory::rss_now( void )
{
   long int rssnow = 0L;

#ifdef Q_WS_X11         // Unix: based on /proc/$PID/stat
   // Read /proc/$pid/stat
   QFile f( "/proc/" + QString::number( getpid() ) + "/stat" );
   f.open( QIODevice::ReadOnly );
   QByteArray ba = f.read( 512 );
   f.close();

   const static int kk = PAGE_SIZE / 1024;

   rssnow = QString( ba ).section( " ", 23, 23 ).toLong() * kk;
#endif

#ifdef Q_WS_MAC         // Mac : use task_info call
   struct task_basic_info task_stats;
   mach_msg_type_number_t inf_count = TASK_BASIC_INFO_COUNT;
   task_t   task    = current_task();

   int stat1 = task_info( task, TASK_BASIC_INFO, (task_info_t)&task_stats, &inf_count );
   if ( stat1 == KERN_SUCCESS )
   {
      rssnow  = ( (int64_t)task_stats.resident_size + 512 ) / 1024;
   }
#endif

#ifdef Q_WS_WIN         // Windows: direct use of GetProcessMemoryInfo
   HANDLE hProcess;
   DWORD processID;
   PROCESS_MEMORY_COUNTERS pmc;
   pmc.cb       = (DWORD)sizeof( pmc );
   processID    = _getpid();
   long int usedmem = 0;

   hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                           FALSE, processID );
   if ( hProcess == NULL )
      return maxrss;

   if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) )
   {
      rssnow  = ( (int64_t)pmc.PeakWorkingSetSize + 512 ) / 1024;
   }

   CloseHandle( hProcess );
#endif

   return rssnow;
}

// Update and return maximum used memory (RSS)
long int US_Memory::rss_max( long int& rssmax )
{
   rssmax = qMax( rssmax, rss_now() );

   return rssmax;
}

