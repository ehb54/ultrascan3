#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

class US_SecondMoment : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_SecondMoment();

   private:
      double               average_2nd;
      double*              smPoints;
      double*              smSeconds;
      US_Editor*           te_results;

      void data_plot   ( void );

   private slots:
      void write_report( QTextStream& );
      void view        ( void );
      void save        ( void );
      void help        ( void )
      { showHelp.show_help( "manual/us_second_moment.html" ); };
};
#endif
