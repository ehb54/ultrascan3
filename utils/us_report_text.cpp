//! \file us_report_text.cpp
#include "us_report_text.h"

#include <QObject>

QString US_ReportText::indent( const int spaces )
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

QString US_ReportText::table_row( const QStringList& cells )
{
   return ( indent( 6 ) + "<tr><td>" + cells.join( "</td><td>" ) + "</td></tr>\n" );
}

QString US_ReportText::text_model( US_Model model, int width, bool mark_iterative )
{
   QString stitle = model.typeText();
   QString title  = stitle;
   QString m_desc = model.description;

   if ( width != 0 )
   { // long title:  add any suffixes and check need to center
      switch ( (int) model.analysis )
      {
         case (int) US_Model::TWODSA:
         case (int) US_Model::TWODSA_MW:
            title = QObject::tr( "2-Dimensional Spectrum Analysis" );

            if ( mark_iterative && m_desc.contains( "-IT" ) )
            {
               title = title + " (IT)";
            }

            break;

         case (int) US_Model::GA:
         case (int) US_Model::GA_MW:
            title = QObject::tr( "Genetic Algorithm Analysis" );
            break;

         case (int) US_Model::COFS:
            title = QObject::tr( "C(s) Analysis" );
            break;

         case (int) US_Model::FE:
            title = QObject::tr( "Finite Element Analysis" );
            break;

         case (int) US_Model::PCSA:
            title = QObject::tr( "Parametrically Constrained Spectrum Analysis\n" );

            if ( stitle.contains( "-SL" ) )
            {
               title += QObject::tr( "(Straight Line)" );
            }

            else if ( stitle.contains( "-IS" ) )
            {
               title += QObject::tr( "(Increasing Sigmoid)" );
            }

            else if ( stitle.contains( "-DS" ) )
            {
               title += QObject::tr( "(Decreasing Sigmoid)" );
            }

            else if ( stitle.contains( "-HL" ) )
            {
               title += QObject::tr( "(Horizontal Line)" );
            }

            else if ( stitle.contains( "-2O" ) )
            {
               title += QObject::tr( "(2nd-Order Power Law)" );
            }

            break;

         case (int) US_Model::DMGA:
            title = QObject::tr( "Discrete Model Genetic Algorithm" );
            break;

         case (int) US_Model::MANUAL:
         default:
            title = QObject::tr( "2-Dimensional Spectrum Analysis" );
            break;
      }


      if ( stitle.contains( "-SL" ) )
      {
         if ( model.associations.size() > 1 )
         {
            title = title + " (RA)";
         }
      }

      if ( model.global == US_Model::MENISCUS )
      {
         title = title + " (Menisc.)";
      }

      else if ( model.global == US_Model::GLOBAL )
      {
         title = title + " (Global)";
      }

      else if ( model.global == US_Model::SUPERGLOBAL )
      {
         title = title + " (S.Glob.)";
      }

      if ( model.monteCarlo )
      {
         title = title + " (MC)";
      }

      if ( width > title.length() )
      { // long title centered:  center it in fixed-length string
         int lent = title.length();
         int lenl = ( width - lent ) / 2;
         int lenr = width - lent - lenl;
         title    = QString( " " ).repeated( lenl ) + title + QString( " " ).repeated( lenr );
      }
   }

   return title;
}
