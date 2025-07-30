//! \file us_memory.cpp
#include <QRegularExpression>
#include <QProcess>
#include <QtMath>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "us_memory.h"

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

#ifdef Q_OS_MAC                          // Mac includes
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif
#ifndef Q_OS_WIN                         // Unix and Mac includes
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

#ifdef Q_OS_LINUX       // Unix: based on /proc/$PID/stat
   // Read /proc/$pid/stat
   QFile f( "/proc/" + QString::number( getpid() ) + "/stat" );
   f.open( QIODevice::ReadOnly );
   QByteArray ba = f.read( 512 );
   f.close();

   const static int kk = PAGE_SIZE / 1024;

   rssnow = QString( ba ).section( " ", 23, 23 ).toLong() * (long int)kk;
#endif

#ifdef Q_OS_MAC         // Mac : use task_info call
   struct task_basic_info task_stats;
   mach_msg_type_number_t inf_count = TASK_BASIC_INFO_COUNT;
   task_t   task    = current_task();

   int stat1 = task_info( task, TASK_BASIC_INFO, (task_info_t)&task_stats, &inf_count );
   if ( stat1 == KERN_SUCCESS )
   {
      rssnow  = ( (long)task_stats.resident_size + 512 ) / 1024;
   }
#endif

#ifdef Q_OS_WIN         // Windows: direct use of GetProcessMemoryInfo
   HANDLE hProcess;
   DWORD processID;
   PROCESS_MEMORY_COUNTERS pmc;
   pmc.cb       = (DWORD)sizeof( pmc );
   processID    = _getpid();

   hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                           FALSE, processID );
   if ( hProcess != NULL )
   {
      if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) )
      {
         rssnow  = ( (long)pmc.PeakWorkingSetSize + 512 ) / 1024;
      }

      CloseHandle( hProcess );
   }
#endif

   return rssnow;
}

// Update and return maximum used memory (RSS)
long int US_Memory::rss_max( long int& rssmax )
{
   rssmax = qMax( rssmax, rss_now() );

   return rssmax;
}

// Return the memory profile: used, available, total
int US_Memory::memory_profile( int* pMemA, int* pMemT, int* pMemU )
{
   int memavail;
   int memtotal;
   int memused;
   int memavpc;
#ifdef Q_OS_LINUX       // Unix: use free command
   QProcess qproc;
   qproc.start( "free", QStringList() << "-m" );
   qproc.waitForFinished( -1 );
   QString totmem   = QString( qproc.readAllStandardOutput() ).trimmed();
   totmem           = totmem.section( "\n", 1, 1 );
   totmem.replace( QRegularExpression( "\\s+" ), " " );
qDebug() << "  UsMEM:LINUX: totmem" << totmem;
   int fmtotal      = totmem.section( " ", 1, 1 ).toInt();
   int fmused       = totmem.section( " ", 2, 2 ).toInt();
   int fmfree       = totmem.section( " ", 3, 3 ).toInt();
   int fmbuffer     = totmem.section( " ", 5, 5 ).toInt();
   int fmcache      = totmem.section( " ", 6, 6 ).toInt();
qDebug() << "  UsMEM:LINUX: fmtotal,used,free,buffer,cache" << fmtotal << fmused
 << fmfree << fmbuffer << fmcache;
   memtotal         = fmtotal;
   memused          = qMax( fmused, ( fmtotal - fmfree - fmcache ) );
   memavail         = memtotal - memused;
#endif
#ifdef Q_OS_MAC         // Mac: use sysctl and rss_now()
   const double mb_bytes = ( 1024. * 1024. );
   const double kb_bytes = 1024.;
   QProcess qproc;
   qproc.start( "sysctl", QStringList() << "-n" << "hw.memsize" );
   qproc.waitForFinished( -1 );
   QString totmem   = QString( qproc.readAllStandardOutput() ).trimmed();
   memtotal         = qRound( totmem.toDouble() / mb_bytes );
   memused          = qRound( (double)rss_now() / kb_bytes );
   memavail         = memtotal - memused;
qDebug() << "  UsMEM:Mac:  totmem" << totmem << "memtotal" << memtotal;
#endif
#ifdef Q_OS_WIN         // Windows: direct use of GlobalMemoryStatusEx
   const double mb_bytes = ( 1024. * 1024. );
   MEMORYSTATUSEX mstatx;
   mstatx.dwLength  = sizeof( mstatx );
   GlobalMemoryStatusEx( &mstatx );
   long memload     = (long)mstatx.dwMemoryLoad;
   memtotal         = qRound( (double)mstatx.ullTotalPhys / mb_bytes );
   memavail         = qRound( (double)mstatx.ullAvailPhys / mb_bytes );
   memused          = memtotal - memavail;
   memavpc          = qRound( memavail * 100.0 / memtotal );
//qDebug() << "  UsMEM:Win:  mstatx.dwMemoryLoad" << mstatx.dwMemoryLoad;
//qDebug() << "  UsMEM:Win:  mstatx.ullTotalPhys" << mstatx.ullTotalPhys;
//qDebug() << "  UsMEM:Win:  mstatx.ullAvailPhys" << mstatx.ullAvailPhys;
//qDebug() << "  UsMEM:Win:  gb_bytes" << gb_bytes;
qDebug() << "  UsMEM:Win:  memload" << memload << "memavpc" << memavpc;
#endif
qDebug() << "  UsMEM: memtotal,avail,used" << memtotal << memavail << memused;

   memavpc          = qRound( memavail * 100.0 / memtotal );
   if ( pMemA != NULL )  *pMemA = memavail;
   if ( pMemT != NULL )  *pMemT = memtotal;
   if ( pMemU != NULL )  *pMemU = memused;

   return memavpc;
}

