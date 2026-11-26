//! \file test_us_report_text.cpp
// US_ReportText against the report helpers it replaced in FEMatch and the GMP
// reporter, kept here word for word as they were at 94e74d4f3.
#include "qt_test_base.h"
#include "us_model.h"
#include "us_report_text.h"
#include <QObject>
#include <random>

namespace
{
// The old helpers were members; tr() was the class's.
struct FrozenFeMatchText
{
      static QString tr( const char* s )
      {
         return QObject::tr( s );
      }
      QString indent( const int spaces ) const;
      QString table_row( const QString& s1, const QString& s2 ) const;
      QString table_row( const QString& s1, const QString& s2, const QString& s3 ) const;
      QString table_row( const QString& s1, const QString& s2, const QString& s3, const QString& s4, const QString& s5 ) const;
      QString table_row( const QString& s1,
                         const QString& s2,
                         const QString& s3,
                         const QString& s4,
                         const QString& s5,
                         const QString& s6,
                         const QString& s7 ) const;
      QString text_model( US_Model model, int width );
};

struct FrozenReporterText
{
      static QString tr( const char* s )
      {
         return QObject::tr( s );
      }
      QString indent( const int spaces ) const;
      QString table_row( const QString& s1, const QString& s2 ) const;
      QString table_row( const QString& s1, const QString& s2, const QString& s3 ) const;
      QString table_row( const QString& s1, const QString& s2, const QString& s3, const QString& s4 ) const;
      QString table_row( const QString& s1, const QString& s2, const QString& s3, const QString& s4, const QString& s5 ) const;
      QString table_row( const QString& s1,
                         const QString& s2,
                         const QString& s3,
                         const QString& s4,
                         const QString& s5,
                         const QString& s6 ) const;
      QString table_row( const QString& s1,
                         const QString& s2,
                         const QString& s3,
                         const QString& s4,
                         const QString& s5,
                         const QString& s6,
                         const QString& s7 ) const;
      QString table_row( const QString& s1,
                         const QString& s2,
                         const QString& s3,
                         const QString& s4,
                         const QString& s5,
                         const QString& s6,
                         const QString& s7,
                         const QString& s8 ) const;
      QString table_row( const QString& s1,
                         const QString& s2,
                         const QString& s3,
                         const QString& s4,
                         const QString& s5,
                         const QString& s6,
                         const QString& s7,
                         const QString& s8,
                         const QString& s9 ) const;
      QString text_model( US_Model model, int width );
};

QString FrozenFeMatchText::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

QString FrozenFeMatchText::table_row( const QString& s1, const QString& s2 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

QString FrozenFeMatchText::table_row( const QString& s1, const QString& s2, const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td></tr>\n" );
}

QString FrozenFeMatchText::table_row( const QString& s1,
                                      const QString& s2,
                                      const QString& s3,
                                      const QString& s4,
                                      const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td></tr>\n" );
}

QString FrozenFeMatchText::table_row( const QString& s1,
                                      const QString& s2,
                                      const QString& s3,
                                      const QString& s4,
                                      const QString& s5,
                                      const QString& s6,
                                      const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td><td>" + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}

QString FrozenFeMatchText::text_model( US_Model model, int width )
{
   QString stitle = model.typeText();
   QString title  = stitle;

   if ( width != 0 )
   { // long title:  add any suffixes and check need to center
      switch ( (int) model.analysis )
      {
         case (int) US_Model::TWODSA:
         case (int) US_Model::TWODSA_MW:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;

         case (int) US_Model::GA:
         case (int) US_Model::GA_MW:
            title = tr( "Genetic Algorithm Analysis" );
            break;

         case (int) US_Model::COFS:
            title = tr( "C(s) Analysis" );
            break;

         case (int) US_Model::FE:
            title = tr( "Finite Element Analysis" );
            break;

         case (int) US_Model::PCSA:
            title = tr( "Parametrically Constrained Spectrum Analysis\n" );

            if ( stitle.contains( "-SL" ) )
            {
               title += tr( "(Straight Line)" );
            }

            else if ( stitle.contains( "-IS" ) )
            {
               title += tr( "(Increasing Sigmoid)" );
            }

            else if ( stitle.contains( "-DS" ) )
            {
               title += tr( "(Decreasing Sigmoid)" );
            }

            else if ( stitle.contains( "-HL" ) )
            {
               title += tr( "(Horizontal Line)" );
            }

            else if ( stitle.contains( "-2O" ) )
            {
               title += tr( "(2nd-Order Power Law)" );
            }

            break;

         case (int) US_Model::DMGA:
            title = tr( "Discrete Model Genetic Algorithm" );
            break;

         case (int) US_Model::MANUAL:
         default:
            title = tr( "2-Dimensional Spectrum Analysis" );
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

QString FrozenReporterText::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

QString FrozenReporterText::table_row( const QString& s1, const QString& s2 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1, const QString& s2, const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1, const QString& s2, const QString& s3, const QString& s4 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1,
                                       const QString& s2,
                                       const QString& s3,
                                       const QString& s4,
                                       const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1,
                                       const QString& s2,
                                       const QString& s3,
                                       const QString& s4,
                                       const QString& s5,
                                       const QString& s6 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td><td>" + s6 + "</td><td>\n" );
}

QString FrozenReporterText::table_row( const QString& s1,
                                       const QString& s2,
                                       const QString& s3,
                                       const QString& s4,
                                       const QString& s5,
                                       const QString& s6,
                                       const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td><td>" + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1,
                                       const QString& s2,
                                       const QString& s3,
                                       const QString& s4,
                                       const QString& s5,
                                       const QString& s6,
                                       const QString& s7,
                                       const QString& s8 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td><td>" + s6 + "</td><td>" + s7 + "</td><td>" + s8 + "</td></tr>\n" );
}

QString FrozenReporterText::table_row( const QString& s1,
                                       const QString& s2,
                                       const QString& s3,
                                       const QString& s4,
                                       const QString& s5,
                                       const QString& s6,
                                       const QString& s7,
                                       const QString& s8,
                                       const QString& s9 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 + "</td><td>" + s4 + "</td><td>" + s5 +
            "</td><td>" + s6 + "</td><td>" + s7 + "</td><td>" + s8 + "</td><td>" + s9 + "</td></tr>\n" );
}

QString FrozenReporterText::text_model( US_Model model, int width )
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
            title = tr( "2-Dimensional Spectrum Analysis" );

            if ( m_desc.contains( "-IT" ) )
            {
               title = title + " (IT)";
            }

            break;

         case (int) US_Model::GA:
         case (int) US_Model::GA_MW:
            title = tr( "Genetic Algorithm Analysis" );
            break;

         case (int) US_Model::COFS:
            title = tr( "C(s) Analysis" );
            break;

         case (int) US_Model::FE:
            title = tr( "Finite Element Analysis" );
            break;

         case (int) US_Model::PCSA:
            title = tr( "Parametrically Constrained Spectrum Analysis\n" );

            if ( stitle.contains( "-SL" ) )
            {
               title += tr( "(Straight Line)" );
            }

            else if ( stitle.contains( "-IS" ) )
            {
               title += tr( "(Increasing Sigmoid)" );
            }

            else if ( stitle.contains( "-DS" ) )
            {
               title += tr( "(Decreasing Sigmoid)" );
            }

            else if ( stitle.contains( "-HL" ) )
            {
               title += tr( "(Horizontal Line)" );
            }

            else if ( stitle.contains( "-2O" ) )
            {
               title += tr( "(2nd-Order Power Law)" );
            }

            break;

         case (int) US_Model::DMGA:
            title = tr( "Discrete Model Genetic Algorithm" );
            break;

         case (int) US_Model::MANUAL:
         default:
            title = tr( "2-Dimensional Spectrum Analysis" );
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

QString cell( std::mt19937& rng )
{
   static const QStringList pool = { "", "a", "1.23e-13", "<b>x</b>", "YES", "NO", "  ", "s20,w", "\u00b0C" };
   return pool[std::uniform_int_distribution<int>( 0, pool.size() - 1 )( rng )];
}
} // namespace

class TestUSReportText : public QtTestBase
{
};

TEST_F( TestUSReportText, IndentMatchesBothCopies )
{
   FrozenFeMatchText  fe;
   FrozenReporterText re;
   for ( int n = -1; n < 12; n++ )
   {
      EXPECT_EQ( US_ReportText::indent( n ), fe.indent( n ) ) << n;
      EXPECT_EQ( US_ReportText::indent( n ), re.indent( n ) ) << n;
   }
}

// Every table_row overload of both programs, with random cells
TEST_F( TestUSReportText, TableRowsMatchEveryOldOverload )
{
   FrozenFeMatchText  fe;
   FrozenReporterText re;
   std::mt19937       rng( 1243 );
   for ( int n = 0; n < 200; n++ )
   {
      const QString c[9] = { cell( rng ), cell( rng ), cell( rng ), cell( rng ), cell( rng ),
                             cell( rng ), cell( rng ), cell( rng ), cell( rng ) };
      using US_ReportText::table_row;
      EXPECT_EQ( table_row( c[0], c[1] ), fe.table_row( c[0], c[1] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2] ), fe.table_row( c[0], c[1], c[2] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4] ), fe.table_row( c[0], c[1], c[2], c[3], c[4] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6] ), fe.table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6] ) );
      EXPECT_EQ( table_row( c[0], c[1] ), re.table_row( c[0], c[1] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2] ), re.table_row( c[0], c[1], c[2] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3] ), re.table_row( c[0], c[1], c[2], c[3] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4] ), re.table_row( c[0], c[1], c[2], c[3], c[4] ) );
      // The reporter's six-cell row ended with an opened cell; it now closes like the others
      const QString six = table_row( c[0], c[1], c[2], c[3], c[4], c[5] );
      EXPECT_TRUE( six.endsWith( "</td></tr>\n" ) );
      EXPECT_EQ( six.chopped( 6 ) + "<td>\n", re.table_row( c[0], c[1], c[2], c[3], c[4], c[5] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6] ), re.table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7] ),
                 re.table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7] ) );
      EXPECT_EQ( table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8] ),
                 re.table_row( c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8] ) );
   }
}

// Every analysis type with each suffix, description and width: FEMatch's
// title without the (IT) mark, the reporter's with it
TEST_F( TestUSReportText, ModelTitlesMatchBothCopies )
{
   FrozenFeMatchText                   fe;
   FrozenReporterText                  re;
   const QList<US_Model::AnalysisType> types    = { US_Model::MANUAL,     US_Model::TWODSA, US_Model::TWODSA_MW,  US_Model::GA,
                                                    US_Model::GA_MW,      US_Model::PCSA,   US_Model::COFS,       US_Model::FE,
                                                    US_Model::CUSTOMGRID, US_Model::DMGA,   US_Model::DMGA_CONSTR };
   const QList<US_Model::GlobalType>   globals  = { US_Model::NONE,        US_Model::MENISCUS, US_Model::GLOBAL,
                                                    US_Model::SUPERGLOBAL, US_Model::BOTTOM,   US_Model::MENIBOTT };
   int                                 compared = 0;
   for ( US_Model::AnalysisType type : types )
   {
      for ( US_Model::GlobalType global : globals )
      {
         for ( bool mc : { false, true } )
         {
            for ( int nassoc : { 0, 2 } )
            {
               for ( const char* desc : { "run.1A260.e1_a1_2DSA-IT_01.model", "run.1A260.e1_a1_PCSA-SL_01.model", "" } )
               {
                  for ( int width : { 0, 2, 40, 80 } )
                  {
                     US_Model m;
                     m.analysis    = type;
                     m.global      = global;
                     m.monteCarlo  = mc;
                     m.description = desc;
                     for ( int a = 0; a < nassoc; a++ )
                     {
                        m.associations << US_Model::Association();
                     }
                     EXPECT_EQ( US_ReportText::text_model( m, width, false ), fe.text_model( m, width ) );
                     EXPECT_EQ( US_ReportText::text_model( m, width, true ), re.text_model( m, width ) );
                     compared++;
                  }
               }
            }
         }
      }
   }
   EXPECT_GT( compared, 3000 );
}
