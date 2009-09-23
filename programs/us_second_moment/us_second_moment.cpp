//! \file us_second_moment.cpp

#include <QApplication>

#include "us_second_moment.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for us_convert. Loads translators and starts
//         the class US_Convert.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_SecondMoment w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_SecondMoment::US_SecondMoment() : US_AnalysisBase()
{
   setWindowTitle( tr( "Second Moment Analysis" ) );

   connect( pb_load,  SIGNAL( clicked() ), SLOT( load() ) );
   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );

   connect( pb_reset,   SIGNAL( clicked() ), SLOT( tbd () ) );
   connect( pb_view,    SIGNAL( clicked() ), SLOT( tbd () ) );
   connect( pb_reset,   SIGNAL( clicked() ), SLOT( tbd () ) );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( tbd () ) );


   connect( ct_smoothing,       SIGNAL( valueChanged( double ) ), 
                                SLOT  ( tbd         ( double ) ) );
   
   connect( ct_boundaryPercent, SIGNAL( valueChanged( double ) ), 
                                SLOT  ( tbd         ( double ) ) );
   
   connect( ct_boundaryPos,     SIGNAL( valueChanged( double ) ), 
                                SLOT  ( tbd         ( double ) ) );
   
   connect( ct_from,            SIGNAL( valueChanged( double ) ), 
                                SLOT  ( tbd         ( double ) ) );
   
   connect( ct_to,              SIGNAL( valueChanged( double ) ), 
                                SLOT  ( tbd         ( double ) ) );
}

void US_SecondMoment::load( void )
{
   US_AnalysisBase::load();

   if ( load_status != 0 ) return;


   return; 
}
