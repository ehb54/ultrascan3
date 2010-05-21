#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

class US_Dcdt : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_Dcdt();

   private:
      int           graphType;
      int*          arraySizes;
      int*          arrayStart;
                    
      static const int arrayLength = 400;
                    
      US_Editor*    te_results;
                    
      double**      dcdt;
      double**      sValues;
      double*       avgDcdt;
      double*       avgS;
      double        sMax;

      QIcon         check;
      QPushButton*  pb_baseline;
      QRadioButton* rb_radius;
      
      QwtCounter*   ct_sValue;

      void    data_plot  ( void );
      QString results    ( void );     

   private slots:
      void    view       ( void );
      void    save       ( void );
      void    set_graph  ( int  );
      void    reset      ( void );
      void    subtract_bl( void );
      void    sMaxChanged( double );
      void    help       ( void )
      { showHelp.show_help( "manual/us_dcdt.html" ); };
};
#endif
