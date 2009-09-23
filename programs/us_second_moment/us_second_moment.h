#ifndef US_SECOND_MONENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base.h"

class US_SecondMoment : public US_AnalysisBase
{
  Q_OBJECT

  public:
  US_SecondMoment();

  private:

  private slots:
     void load( void );
     void help( void )
     { showHelp.show_help( "manual/us_second_moment.html" ); };
};
#endif
