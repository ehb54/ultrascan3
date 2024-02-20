#include "us_csv_loader.h"
#include "us_gui_settings.h"

US_CSV_Loader::US_CSV_Loader(QWidget* parent) : US_WidgetsDialog(parent, 0)
{
   setWindowTitle( tr( "Loas CSV File" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize(QSize(600,600));


}
