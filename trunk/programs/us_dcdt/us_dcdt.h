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
      int               graphType;
      int               previousScanCount; // total # of scans before skipping and exclusion
      double            sMax;

      QVector< int >    arraySizes; 
      QVector< int >    arrayStart;
      QVector< double > avgDcdt;         // holds the average of all dcdt scans
      QVector< double > avgS;            // holds the transformation to s of avgDcdt

      static const int  arrayLength = 400;

      US_Editor*        te_results;

      QRadioButton*     rb_radius;
      QRadioButton*     rb_sed;
      QRadioButton*     rb_avg;

      QwtCounter*       ct_sValue;

      QList< QVector< double > > dcdt;    // holds all the dcdt scans
      QList< QVector< double > > sValues; // holds s-value transformations from the dcdt scans

      void    data_plot  ( void );
      QString results    ( void );     

   private slots:
      void    view          ( void );
      void    save          ( void );
      void    set_graph     ( int  );
      void    reset         ( void );
      void    sMaxChanged   ( double );
      void    exclude       ( void );
      void    reset_excludes( void );
      void    write_report  ( QTextStream& );

      void    help          ( void )
      { showHelp.show_help( "manual/us_dcdt.html" ); };
};
#endif
