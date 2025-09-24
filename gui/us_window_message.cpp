#include "us_window_message.h"

// A class for sending IPC messages/commands to an external window.
// This is primarily meant for SOMO communications with RASMOL.


// Platform-specific C function definitions
#ifdef Q_OS_LINUX
extern "C" {
#include "us_x11_utils.h"
}
#endif
#ifdef Q_OS_MAC
extern "C" {
#include "us_mac_utils.h"
}
#endif
#ifdef Q_OS_WIN
extern "C" {
#include "us_win_utils.h"
}
#define GetLastError() LastErrorMsg()
#endif

// Constructor for subclass Interpreter
US_WindowMessage::Interpreter::Interpreter( ulong ntrpid, QString iname,
      QString fname )
{
   interp_id      = ntrpid;
   interp_name    = iname;
   file_name      = fname;
}

// Constructor that mainly just gets a list of present live interpreters
US_WindowMessage::US_WindowMessage( QObject* parent ) : QObject( parent )
{
   findLiveInterps();
}

// Function to issue a "winfo interps" command and populate an internal list
// of all interpreters.
int US_WindowMessage::findAllInterps ( void )
{
   char nameints[ 512 ];
   int nintrp    = 0;

   // Get the interpreters' list using platform-dependent methods
   int nchar     = ShowInterpNames( nameints, 511 );

   if ( nchar > 0 )
   {  // Parse the character array to build a list of interpreter objects
      nintrp        = fill_interps( nchar, nameints, all_interps );
   }

   return nintrp;
}

// Function to issue a "winfo interps" command, populate an internal list
// of all interpreters, and return a copy.
int US_WindowMessage::findAllInterps( QList< Interpreter >& interps )
{
   int nintrp = findAllInterps();   // Build the internal list of interps
   interps    = all_interps;        // Copy the list to the user's list
   return nintrp;
}

// Function to generate the internal list of active RasMol interpreters
int US_WindowMessage::findLiveInterps()
{
   live_interps.clear();
   zombie_list .clear();

   int nintrp = findAllInterps();   // Build the internal list of interps

   for ( int ii = 0; ii < nintrp; ii++ )
   {  // Loop to generate the "live" list from the "all"
      Interpreter intrp  = all_interps[ ii ];

      if ( isLive( intrp ) )
      {  // If live, add to the list
         live_interps << intrp;
      }

      else
      {  // Otherwise, add an identifying string to the zombie list
         zombie_list << QString::asprintf( "%lx : ", intrp.interp_id )
                        + intrp.interp_name;
      }
   }

   return live_interps.size();
}

// Function to regenerate the list of active RasMol interpreters and
//  to return a copy of the list
int US_WindowMessage::findLiveInterps( QList< Interpreter >& interps )
{
   findLiveInterps();           // Generate internal list of live interps

   interps    = live_interps;   // Return a copy

   return live_interps.size();  // Return with the count
}

// Function to return any zombie list
int US_WindowMessage::zombieList( QStringList& zlist )
{
   int nzomb  = zombie_list.size();

   if ( nzomb > 0 )
      zlist      = zombie_list;

   return nzomb;
}

// Function to add a new interpreter, identified by the name of the file
// displayed in its window, and return its ID.
ulong US_WindowMessage::addInterp( const QString filename )
{
   ulong id    = 0;

   // Save a list of IDs from last active list
   QList< ulong > ids;

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      ids    << live_interps[ ii ].interp_id;
   }

   // Update the live interpreters list
   int nintrp  = findLiveInterps( );
   int newx    = -1;

   // Interpreter in live list to update is the last new one
   for ( int ii = 0; ii < nintrp; ii++ )
   {
      ulong   lvid  = live_interps[ ii ].interp_id;

      if ( !ids.contains( lvid ) )
      {  // This is a newly added interpreter, so save its index
         newx          = ii;
         id            = lvid;
      }
   }

   if ( newx >= 0 )
   {  // Update the file name for the newly added interpreter
      live_interps[ newx ].file_name = filename;
   }

   return id;
}

// Function to return the list of active interpreter IDs.
int US_WindowMessage::interpIDs( QList< ulong >& ids )
{
   int nntrp    = live_interps.size();
   ids.clear();

   for ( int ii = 0; ii < nntrp; ii++ )
      ids << live_interps[ ii ].interp_id;

   return nntrp;
}

// Function to return the list of active interpreter names.
int US_WindowMessage::interpNames( QStringList& names )
{
   int nntrp    = live_interps.size();
   names.clear();

   for ( int ii = 0; ii < nntrp; ii++ )
      names << live_interps[ ii ].interp_name;

   return nntrp;
}

// Function to return a list of active interpreter file names.
int US_WindowMessage::interpFiles( QStringList& fnames )
{
   int nntrp    = live_interps.size();
   fnames.clear();

   for ( int ii = 0; ii < nntrp; ii++ )
      fnames << live_interps[ ii ].file_name;

   return nntrp;
}

// Function to return the interp ID for a given file name.
ulong US_WindowMessage::interpId( const QString filename )
{
   ulong id      = 0;

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].file_name == filename )
      {
         id         = live_interps[ ii ].interp_id;
         break;
      }
   }

   return id;
}

// Function to return the interp ID for a given interp name.
ulong US_WindowMessage::interpIdByName( const QString iname )
{
   ulong id      = 0;

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].interp_name == iname )
      {
         id         = live_interps[ ii ].interp_id;
         break;
      }
   }

   return id;
}

// Function to return the interp name for a given ID.
QString US_WindowMessage::interpName( const ulong id )
{
   QString iname = "";

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].interp_id == id )
      {
         iname      = live_interps[ ii ].interp_name;
         break;
      }
   }

   return iname;
}

// Function to return the interp name for a given file name.
QString US_WindowMessage::interpName( const QString filename )
{
   QString iname = "";

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].file_name == filename )
      {
         iname      = live_interps[ ii ].interp_name;
         break;
      }
   }

   return iname;
}

// Function to return the file name for a given ID.
QString US_WindowMessage::interpFileName( const ulong id )
{
   QString fname = "";

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].interp_id == id )
      {
         fname      = live_interps[ ii ].file_name;
         break;
      }
   }

   return fname;
}

// Function to return the file name for a given interp name.
QString US_WindowMessage::interpFileName( const QString iname )
{
   QString fname = "";

   for ( int ii = 0; ii < live_interps.size(); ii++ )
   {
      if ( live_interps[ ii ].interp_name == iname )
      {
         fname      = live_interps[ ii ].file_name;
         break;
      }
   }

   return fname;
}

// Function to send a message to an interpreter as identified by an ID number.
int US_WindowMessage::sendMessage( const ulong id, const QString wmsg )
{
   QString iname   = interpName( id );
   return SendCommand( id, iname.toLatin1().data(), wmsg.toLatin1().data() );
}

// Function to send a message to an interpreter as identified by interp name.
int US_WindowMessage::sendMessage( const QString iname, const QString wmsg )
{
   ulong id        = interpIdByName( iname );
   return SendCommand( id, iname.toLatin1().data(), wmsg.toLatin1().data() );
}

// Function to query an interpreter by interp ID and return the response.
QString US_WindowMessage::sendQuery( const ulong id, const QString wmsg )
{
   char resp[ 1024 ];
   QString iname   = interpName( id );
   SendQuery( id, iname.toLatin1().data(), wmsg.toLatin1().data(), resp, 1024 );
   return QString( resp );
}

// Function to query an interpreter by interp name and return the response.
QString US_WindowMessage::sendQuery( const QString iname, const QString wmsg )
{
   char resp[ 1024 ];
   ulong id        = interpIdByName( iname );
   SendQuery( id, iname.toLatin1().data(), wmsg.toLatin1().data(), resp, 1024 );
   return QString( resp );
}

// Function to determine if an interpreter is live
bool US_WindowMessage::isLive( Interpreter& interp )
{
   char  ctest[ 64 ] = "echo hello ";
   char* iname   = interp.interp_name.toLatin1().data();
   strcat( ctest, iname );

   return ( SendCommand( interp.interp_id, iname, ctest ) == 0 );
}

// Function to return a status/error message from the last interpreter
//  send action.
QString US_WindowMessage::lastSendResult( void )
{
   return QString( GetLastError() );
}

// Internal function to parse "winfo interps" results to build a list
//  of interpreters.
int US_WindowMessage::fill_interps( int nchar, char* wiresult,
      QList< Interpreter >& intrps )
{
   int niknt     = 0;
   intrps.clear();

   bool ok;
   int kchar     = nchar;
   char* ptr1    = wiresult;

   while ( kchar > 0 )
   {
      int jchar     = (int)strlen( ptr1 );

      if ( jchar == 0 )  break;

      QString ires  = QString( ptr1 );
      ulong iid     = ires.section( " ", 0,  0 ).toULong( &ok, 16 );
      QString iname = ires.section( " ", 1, -1 ).simplified();
      ptr1         += ( ++jchar );
      kchar        -= jchar;

      intrps << Interpreter( iid, iname, "" );
      niknt++;
   }

   return niknt;
}

