//! \file us_manage_data.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_manage_data.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_editor.h"
#include "us_noise.h"
#include "us_util.h"

const QColor colorRed(   240,   0,   0 );
const QColor colorBlue(    0,   0, 255 );
const QColor colorBrown( 120,  60,   0 );
const QColor colorGreen(   0, 150,   0 );
const QColor colorGray(  110, 110, 110 );
const QColor colorWhite( 255, 255, 240 );

// format an item action text for a message box
QString US_ManageData::action_text( QString exstext, QString acttext )
{
   return tr( "This item exists on %1.<br>"
              "Are you sure you want to proceed with a %2?<ul>"
              "<li><b>No </b> to cancel the action;</li>"
              "<li><b>Yes</b> to proceed with the action.</li></ul>" )
          .arg( exstext ).arg( acttext );
}

// report the result of an item action
void US_ManageData::action_result( int stat, QString item_act )
{
   if ( stat != 999 )
   {  // proceed was selected:  test status of action

      if ( stat == 0 )
      {  // action was successful
         QMessageBox::information( this,
               item_act + tr( " Successful!" ),
               tr( "The \"%1\" action was successfully performed." )
               .arg( item_act ) );
      }

      else
      {  // action got an error
         QMessageBox::warning( this,
               item_act + tr( " *ERROR*!" ),
               tr( "The \"%1\" action had an error: %2" )
               .arg( item_act ).arg( stat ) );
      }
   }

   else
   {  // cancel was selected:  report it
      QMessageBox::information( this,
            item_act + tr( " Cancelled!" ),
            tr( "The \"%1\" action was cancelled." ).arg( item_act ) );
   }
}

// prepend a record type string to an item action
void US_ManageData::record_type( int recType, QString& item_act )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   int sizert = sizeof( rtyps ) / sizeof( rtyps[ 0 ] );

   if ( recType > 0  &&  recType <= sizert )
   {
      item_act = QString( rtyps[ recType - 1 ] ) + " " + item_act;
   }
}

// compose concatenation on which to sort (label:index:dataGUID:parentGUID)
QString US_ManageData::sort_string( DataDesc ddesc, int indx )
{  // create string for ascending sort on label
   QString ostr = ddesc.label                        // label to sort on
      + ":"     + QString().sprintf( "%4.4d", indx ) // index in desc. vector
      + ":"     + ddesc.dataGUID                     // data GUID
      + ":"     + ddesc.parentGUID;                  // parent GUID
   return ostr;
}

// compose string describing model type
QString US_ManageData::model_type( int imtype, int nassoc, int niters )
{
   QString mtype;

   // format the base model type string
   switch ( imtype )
   {
      default:
      case (int)US_Model::TWODSA:
      case (int)US_Model::MANUAL:
         mtype = "2DSA";
         break;
      case (int)US_Model::TWODSA_MW:
         mtype = "2DSA-MW";
         break;
      case (int)US_Model::GA:
      case (int)US_Model::GA_RA:
         mtype = "GA";
         break;
      case (int)US_Model::GA_MW:
         mtype = "GA-MW";
         break;
      case (int)US_Model::COFS:
         mtype = "COFS";
         break;
      case (int)US_Model::FE:
         mtype = "FE";
         break;
      case (int)US_Model::GLOBAL:
         mtype = "GLOBAL";
         break;
      case (int)US_Model::ONEDSA:
         mtype = "1DSA";
         break;
   }

   // add RA for Reversible Associations (if associations count > 1)
   if ( nassoc > 1 )
      mtype = mtype + "-RA";

   // add MC for Monte Carlo (if iterations count > 1)
   if ( niters > 1 )
      mtype = mtype + "-MC";

   return mtype;
}

// compose string describing model type
QString US_ManageData::model_type( US_Model model )
{
   // return model type string based on integer flags in the model object
   return model_type( (int)model.type,
                      model.associations.size(),
                      model.iterations );
}

// compose string describing model type
QString US_ManageData::model_type( QString modxml )
{
   QChar quo( '"' );
   int   jj;
   int   imtype;
   int   nassoc;
   int   niters;

   // model type number from type attribute
   jj       = modxml.indexOf( " type=" );
   imtype   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // count of associations is count of k_eq attributes present
   nassoc   = modxml.count( "k_eq=" );

   // number of iterations from iterations attribute value
   jj       = modxml.indexOf( " iterations=" );
   niters   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // return model type string based on integer flags
   return model_type( imtype, nassoc, niters );
}

// compose string describing record state
QString US_ManageData::record_state( int istate )
{
   QString hexn    = QString().sprintf( "0x%3.3x", istate );

   QString flags   = "NOSTAT";         // by default, no state

   if ( istate & REC_DB )
      flags  = flags + "|REC_DB";      // record exists in db

   if ( istate & REC_LO )
      flags  = flags + "|REC_LO";      // record exists locally

   if ( istate & PAR_DB )
      flags  = flags + "|PAR_DB";      // parent exists in db

   if ( istate & PAR_LO )
      flags  = flags + "|PAR_LO";      // parent exists locally

   if ( istate & HV_DET )
      flags  = flags + "|DETAIL";      // content details are supplied

   if ( istate & IS_CON )
      flags  = flags + "|CONSIS";      // record is consistent in db+local

   if ( istate & ALL_OK )
      flags  = flags + "|ALL_OK";      // record is ok in all respects

   if ( flags != "NOSTAT" )
      flags  = flags.mid( 7, 999 );    // remove any "NOSTAT|"

    return "(" + hexn + ") " + flags;  // return hex flag and text version
}

// find index of substring at given position in strings of string list
int US_ManageData::index_substring( QString ss, int ixs, QStringList& sl )
{
   QString sexp;
   QRegExp rexp;
   int     kndx = -1;

   if ( ixs == 0 )
   {  // find label at beginning of strings in list
      sexp = ss + ":*";
   }

   else if ( ixs == 1  ||  ixs == 2 )
   {  // find RecIndex or recGUID in middle of strings in list
      sexp = "*:" + ss + ":*";
   }

   else if ( ixs == 3 )
   {  // find parentGUID at end of strings in list
      sexp = "*:" + ss;
   }

   rexp = QRegExp( sexp, Qt::CaseSensitive, QRegExp::Wildcard );
   kndx = sl.indexOf( rexp );
//qDebug() << "     index_sub kndx ixs search" << kndx << ixs << sexp;
   return kndx;
}

// get sublist from string list of substring matches at a given string position
QStringList US_ManageData::filter_substring( QString ss, int ixs,
   QStringList& sl )
{
   QStringList subl;

   if ( ixs == 0 )
   {  // match label at beginning of strings in list
      subl = sl.filter( QRegExp( "^" + ss + ":" ) );
   }

   else if ( ixs == 1  ||  ixs == 2 )
   {  // match RecIndex or recGUID in middle of strings in list
      subl = sl.filter( ":" + ss + ":" );
   }

   else if ( ixs == 3 )
   {  // match parentGUID at end of strings in list
      subl = sl.filter( QRegExp( ":" + ss + "$" ) );
   }

   return subl;
}

// list orphans of a record type (in rec list, no tie to parent list)
QStringList US_ManageData::list_orphans( QStringList& rlist,
   QStringList& plist )
{
   QStringList olist;

   for ( int ii = 0; ii < rlist.size(); ii++ )
   {  // examine parentGUID for each record in the list
      QString pGUID = rlist.at( ii ).section( ":", 3, 3 );

      // see if it is the recordGUID of any in the potential parent list
      int     pndx  = index_substring( pGUID, 2, plist );

      if ( pndx < 0 )
      {  // no parent found, so add to the orphan list
         olist << rlist.at( ii );
      }
   }

   return olist;
}

// return a record state flag with parent state ORed in
int US_ManageData::record_state_flag( DataDesc descr, int pstate )
{
   int state = descr.recState;

   if ( descr.recState == NOSTAT  ||
        descr.description.contains( "-ARTIFICIAL" ) )
      state = NOSTAT;                    // mark a dummy record

   else
   {  // detect and mark parentage of non-dummy
      if ( ( pstate & REC_DB ) != 0 )
         state = state | PAR_DB;         // mark a record with db parent

      if ( ( pstate & REC_LO ) != 0 )
         state = state | PAR_LO;         // mark a record with local parent
   }

   return state;
}

