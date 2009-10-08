#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base.h"
#include "us_editor.h"
#include "us_math.h"

class US_SecondMoment : public US_AnalysisBase
{
   Q_OBJECT

   public:
      US_SecondMoment();

   private:
      double               average_2nd;
      double               time_correction;
      double*              smPoints;
      double*              smSeconds;
      US_Editor*           te_results;
      struct solution_data solution;

      void    data_plot ( void );
      QString results   ( void );     

   private slots:
      void view     ( void );
      void save     ( void );
      void help     ( void )
      { showHelp.show_help( "manual/us_second_moment.html" ); };
};
#endif
