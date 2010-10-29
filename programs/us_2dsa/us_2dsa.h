#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

class US_2dsa : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_2dsa();

   private:
      QGridLayout*         progressLayout;

      US_Editor*           te_results;

      QTextEdit*           te_status;
      QProgressBar*        b_progress;

   private slots:
      void data_plot( void );
      void view     ( void );
      void save     ( void );
      void help     ( void )
      { showHelp.show_help( "manual/us_2dsa.html" ); };
};
#endif
