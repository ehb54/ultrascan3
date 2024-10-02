#include "us_fit_meniscus.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_FitMeniscus. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FitMeniscus w;

   // AutoFlow DEBUG

   /*
   QMap < QString, QString > triple_info_map;
   // triple_info_map[ "triple_name" ]     = QString("2 / A / 259");
   // triple_info_map[ "invID" ]           = QString("3");
   // triple_info_map[ "filename" ]        = QString("demo1_veloc");

   // triple_info_map[ "triple_name" ]     = QString("2 / A / 260");
   // triple_info_map[ "invID" ]           = QString("40");
   // triple_info_map[ "filename" ]        = QString("KulkarniJ_NP1-MWL-new-vs-old_120219-run622");

   // triple_info_map[ "triple_name" ]     = QString("2 / A / 280");
   // triple_info_map[ "invID" ]           = QString("2");
   // triple_info_map[ "filename" ]        = QString("test-022421-RIonly-FMB-A-run962");

   // triple_info_map[ "triple_name" ]     = QString("2 / A / 280");
   // triple_info_map[ "invID" ]           = QString("2");
   // triple_info_map[ "filename" ]        = QString("test-021821-RIonly-FM-run959");

   // triple_info_map[ "triple_name" ]     = QString("2 / A / 225");
   // triple_info_map[ "invID" ]           = QString("94");
   // triple_info_map[ "filename" ]        = QString("YeQ_Calpain3-lowTCEP_052721-run1464");


   triple_info_map[ "triple_name" ]     = QString("3 / B / 235");
   triple_info_map[ "invID" ]           = QString("143");
   triple_info_map[ "filename" ]        = QString("DicksionC_SDS-1_032122-run1281");
   
   US_FitMeniscus w( triple_info_map );
   */
   
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
