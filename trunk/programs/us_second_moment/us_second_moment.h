#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base.h"
#include "us_editor.h"

class US_SecondMoment : public US_AnalysisBase
{
   Q_OBJECT

   public:
      US_SecondMoment();
      ~US_SecondMoment();

   private:
      double*    smPoints;
      double*    smSeconds;
      US_Editor* te_results;

      void    data_plot ( void );
      void    write_data( void );
      QString results   ( void );     

   private slots:
      void view     ( void );
      void save     ( void );
      void reset    ( void );
      
      void edit_done( QObject* )
      {  qDebug() << "edit_done";
         te_results = NULL; };
      
      void help     ( void )
      { showHelp.show_help( "manual/us_second_moment.html" ); };
};
#endif
